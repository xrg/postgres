/*-------------------------------------------------------------------------
 *
 * pg_operator.c
 *	  routines to support manipulation of the pg_operator relation
 *
 * Portions Copyright (c) 1996-2001, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header$
 *
 * NOTES
 *	  these routines moved here from commands/define.c and somewhat cleaned up.
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/heapam.h"
#include "catalog/catname.h"
#include "catalog/indexing.h"
#include "catalog/namespace.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_type.h"
#include "miscadmin.h"
#include "parser/parse_func.h"
#include "parser/parse_oper.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/syscache.h"


static Oid OperatorGet(const char *operatorName,
					   Oid operatorNamespace,
					   Oid leftObjectId,
					   Oid rightObjectId,
					   bool *defined);

static Oid OperatorLookup(List *operatorName,
						  Oid leftObjectId,
						  Oid rightObjectId,
						  bool *defined);

static Oid OperatorShellMake(const char *operatorName,
							 Oid operatorNamespace,
							 Oid leftTypeId,
							 Oid rightTypeId);

static void OperatorUpd(Oid baseId, Oid commId, Oid negId);

static Oid get_other_operator(List *otherOp,
							  Oid otherLeftTypeId, Oid otherRightTypeId,
							  const char *operatorName, Oid operatorNamespace,
							  Oid leftTypeId, Oid rightTypeId,
							  bool isCommutator);


/*
 * Check whether a proposed operator name is legal
 *
 * This had better match the behavior of parser/scan.l!
 *
 * We need this because the parser is not smart enough to check that
 * the arguments of CREATE OPERATOR's COMMUTATOR, NEGATOR, etc clauses
 * are operator names rather than some other lexical entity.
 */
static bool
validOperatorName(const char *name)
{
	size_t		len = strlen(name);

	/* Can't be empty or too long */
	if (len == 0 || len >= NAMEDATALEN)
		return false;

	/* Can't contain any invalid characters */
	/* Test string here should match op_chars in scan.l */
	if (strspn(name, "~!@#^&|`?$+-*/%<>=") != len)
		return false;

	/* Can't contain slash-star or dash-dash (comment starts) */
	if (strstr(name, "/*") || strstr(name, "--"))
		return false;

	/*
	 * For SQL92 compatibility, '+' and '-' cannot be the last char of a
	 * multi-char operator unless the operator contains chars that are not
	 * in SQL92 operators. The idea is to lex '=-' as two operators, but
	 * not to forbid operator names like '?-' that could not be sequences
	 * of SQL92 operators.
	 */
	if (len > 1 &&
		(name[len - 1] == '+' ||
		 name[len - 1] == '-'))
	{
		int			ic;

		for (ic = len - 2; ic >= 0; ic--)
		{
			if (strchr("~!@#^&|`?$%", name[ic]))
				break;
		}
		if (ic < 0)
			return false;		/* nope, not valid */
	}

	/* != isn't valid either, because parser will convert it to <> */
	if (strcmp(name, "!=") == 0)
		return false;

	return true;
}


/*
 * OperatorGet
 *
 *		finds an operator given an exact specification (name, namespace,
 *		left and right type IDs).
 *
 *		*defined is set TRUE if defined (not a shell)
 */
static Oid
OperatorGet(const char *operatorName,
			Oid operatorNamespace,
			Oid leftObjectId,
			Oid rightObjectId,
			bool *defined)
{
	HeapTuple	tup;
	Oid			operatorObjectId;

	tup = SearchSysCache(OPERNAMENSP,
						 PointerGetDatum(operatorName),
						 ObjectIdGetDatum(leftObjectId),
						 ObjectIdGetDatum(rightObjectId),
						 ObjectIdGetDatum(operatorNamespace));
	if (HeapTupleIsValid(tup))
	{
		RegProcedure oprcode = ((Form_pg_operator) GETSTRUCT(tup))->oprcode;

		operatorObjectId = tup->t_data->t_oid;
		*defined = RegProcedureIsValid(oprcode);
		ReleaseSysCache(tup);
	}
	else
	{
		operatorObjectId = InvalidOid;
		*defined = false;
	}

	return operatorObjectId;
}

/*
 * OperatorLookup
 *
 *		looks up an operator given a possibly-qualified name and
 *		left and right type IDs.
 *
 *		*defined is set TRUE if defined (not a shell)
 */
static Oid
OperatorLookup(List *operatorName,
			   Oid leftObjectId,
			   Oid rightObjectId,
			   bool *defined)
{
	Oid			operatorObjectId;
	RegProcedure oprcode;

	operatorObjectId = LookupOperName(operatorName, leftObjectId,
									  rightObjectId);
	if (!OidIsValid(operatorObjectId))
	{
		*defined = false;
		return InvalidOid;
	}

	oprcode = get_opcode(operatorObjectId);
	*defined = RegProcedureIsValid(oprcode);

	return operatorObjectId;
}


/*
 * OperatorShellMake
 *		Make a "shell" entry for a not-yet-existing operator.
 */
static Oid
OperatorShellMake(const char *operatorName,
				  Oid operatorNamespace,
				  Oid leftTypeId,
				  Oid rightTypeId)
{
	Relation	pg_operator_desc;
	Oid			operatorObjectId;
	int			i;
	HeapTuple	tup;
	Datum		values[Natts_pg_operator];
	char		nulls[Natts_pg_operator];
	NameData	oname;
	TupleDesc	tupDesc;

	/*
	 * validate operator name
	 */
	if (!validOperatorName(operatorName))
		elog(ERROR, "\"%s\" is not a valid operator name", operatorName);

	/*
	 * initialize our *nulls and *values arrays
	 */
	for (i = 0; i < Natts_pg_operator; ++i)
	{
		nulls[i] = ' ';
		values[i] = (Datum) NULL;		/* redundant, but safe */
	}

	/*
	 * initialize values[] with the operator name and input data types.
	 * Note that oprcode is set to InvalidOid, indicating it's a shell.
	 */
	i = 0;
	namestrcpy(&oname, operatorName);
	values[i++] = NameGetDatum(&oname);				/* oprname */
	values[i++] = ObjectIdGetDatum(operatorNamespace);	/* oprnamespace */
	values[i++] = Int32GetDatum(GetUserId());		/* oprowner */
	values[i++] = UInt16GetDatum(0);				/* oprprec */
	values[i++] = CharGetDatum(leftTypeId ? (rightTypeId ? 'b' : 'r') : 'l');	/* oprkind */
	values[i++] = BoolGetDatum(false);				/* oprisleft */
	values[i++] = BoolGetDatum(false);				/* oprcanhash */
	values[i++] = ObjectIdGetDatum(leftTypeId);		/* oprleft */
	values[i++] = ObjectIdGetDatum(rightTypeId);	/* oprright */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprresult */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprcom */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprnegate */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprlsortop */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprrsortop */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprltcmpop */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprgtcmpop */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprcode */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprrest */
	values[i++] = ObjectIdGetDatum(InvalidOid);		/* oprjoin */

	/*
	 * open pg_operator
	 */
	pg_operator_desc = heap_openr(OperatorRelationName, RowExclusiveLock);
	tupDesc = pg_operator_desc->rd_att;

	/*
	 * create a new operator tuple
	 */
	tup = heap_formtuple(tupDesc, values, nulls);

	/*
	 * insert our "shell" operator tuple
	 */
	heap_insert(pg_operator_desc, tup);
	operatorObjectId = tup->t_data->t_oid;

	if (RelationGetForm(pg_operator_desc)->relhasindex)
	{
		Relation	idescs[Num_pg_operator_indices];

		CatalogOpenIndices(Num_pg_operator_indices, Name_pg_operator_indices, idescs);
		CatalogIndexInsert(idescs, Num_pg_operator_indices, pg_operator_desc, tup);
		CatalogCloseIndices(Num_pg_operator_indices, idescs);
	}

	heap_freetuple(tup);

	/*
	 * close the operator relation and return the oid.
	 */
	heap_close(pg_operator_desc, RowExclusiveLock);

	return operatorObjectId;
}

/*
 * OperatorCreate
 *
 * "X" indicates an optional argument (i.e. one that can be NULL or 0)
 *		operatorName			name for new operator
 *		operatorNamespace		namespace for new operator
 *		leftTypeId				X left type ID
 *		rightTypeId				X right type ID
 *		procedureName			procedure for operator
 *		precedence				operator precedence
 *		isLeftAssociative		operator is left associative
 *		commutatorName			X commutator operator
 *		negatorName				X negator operator
 *		restrictionName			X restriction sel. procedure
 *		joinName				X join sel. procedure
 *		canHash					hash join can be used with this operator
 *		leftSortName			X left sort operator (for merge join)
 *		rightSortName			X right sort operator (for merge join)
 *		ltCompareName			X L<R compare operator (for merge join)
 *		gtCompareName			X L>R compare operator (for merge join)
 *
 * This routine gets complicated because it allows the user to
 * specify operators that do not exist.  For example, if operator
 * "op" is being defined, the negator operator "negop" and the
 * commutator "commop" can also be defined without specifying
 * any information other than their names.	Since in order to
 * add "op" to the PG_OPERATOR catalog, all the Oid's for these
 * operators must be placed in the fields of "op", a forward
 * declaration is done on the commutator and negator operators.
 * This is called creating a shell, and its main effect is to
 * create a tuple in the PG_OPERATOR catalog with minimal
 * information about the operator (just its name and types).
 * Forward declaration is used only for this purpose, it is
 * not available to the user as it is for type definition.
 *
 * Algorithm:
 *
 * check if operator already defined
 *	  if so, but oprcode is null, save the Oid -- we are filling in a shell
 *	  otherwise error
 * get the attribute types from relation descriptor for pg_operator
 * assign values to the fields of the operator:
 *	 operatorName
 *	 owner id (simply the user id of the caller)
 *	 precedence
 *	 operator "kind" either "b" for binary or "l" for left unary
 *	 isLeftAssociative boolean
 *	 canHash boolean
 *	 leftTypeObjectId -- type must already be defined
 *	 rightTypeObjectId -- this is optional, enter ObjectId=0 if none specified
 *	 resultType -- defer this, since it must be determined from
 *				   the pg_procedure catalog
 *	 commutatorObjectId -- if this is NULL, enter ObjectId=0
 *					  else if this already exists, enter its ObjectId
 *					  else if this does not yet exist, and is not
 *						the same as the main operatorName, then create
 *						a shell and enter the new ObjectId
 *					  else if this does not exist but IS the same
 *						name & types as the main operator, set the ObjectId=0.
 *						(We are creating a self-commutating operator.)
 *						The link will be fixed later by OperatorUpd.
 *	 negatorObjectId   -- same as for commutatorObjectId
 *	 leftSortObjectId  -- same as for commutatorObjectId
 *	 rightSortObjectId -- same as for commutatorObjectId
 *	 operatorProcedure -- must access the pg_procedure catalog to get the
 *				   ObjectId of the procedure that actually does the operator
 *				   actions this is required.  Do a lookup to find out the
 *				   return type of the procedure
 *	 restrictionProcedure -- must access the pg_procedure catalog to get
 *				   the ObjectId but this is optional
 *	 joinProcedure -- same as restrictionProcedure
 * now either insert or replace the operator into the pg_operator catalog
 * if the operator shell is being filled in
 *	 access the catalog in order to get a valid buffer
 *	 create a tuple using ModifyHeapTuple
 *	 get the t_self from the modified tuple and call RelationReplaceHeapTuple
 * else if a new operator is being created
 *	 create a tuple using heap_formtuple
 *	 call heap_insert
 */
void
OperatorCreate(const char *operatorName,
			   Oid operatorNamespace,
			   Oid leftTypeId,
			   Oid rightTypeId,
			   List *procedureName,
			   uint16 precedence,
			   bool isLeftAssociative,
			   List *commutatorName,
			   List *negatorName,
			   List *restrictionName,
			   List *joinName,
			   bool canHash,
			   List *leftSortName,
			   List *rightSortName,
			   List *ltCompareName,
			   List *gtCompareName)
{
	Relation	pg_operator_desc;
	HeapTuple	tup;
	char		nulls[Natts_pg_operator];
	char		replaces[Natts_pg_operator];
	Datum		values[Natts_pg_operator];
	Oid			operatorObjectId;
	bool		operatorAlreadyDefined;
	Oid			procOid;
	Oid			operResultType;
	Oid			commutatorId,
				negatorId,
				leftSortId,
				rightSortId,
				ltCompareId,
				gtCompareId,
				restOid,
				joinOid;
	bool		selfCommutator = false;
	Oid			typeId[FUNC_MAX_ARGS];
	int			nargs;
	NameData	oname;
	TupleDesc	tupDesc;
	int			i;

	/*
	 * Sanity checks
	 */
	if (!validOperatorName(operatorName))
		elog(ERROR, "\"%s\" is not a valid operator name", operatorName);

	if (!OidIsValid(leftTypeId) && !OidIsValid(rightTypeId))
		elog(ERROR, "at least one of leftarg or rightarg must be specified");

	if (!(OidIsValid(leftTypeId) && OidIsValid(rightTypeId)))
	{
		/* If it's not a binary op, these things mustn't be set: */
		if (commutatorName)
			elog(ERROR, "only binary operators can have commutators");
		if (joinName)
			elog(ERROR, "only binary operators can have join selectivity");
		if (canHash)
			elog(ERROR, "only binary operators can hash");
		if (leftSortName || rightSortName || ltCompareName || gtCompareName)
			elog(ERROR, "only binary operators can mergejoin");
	}

	operatorObjectId = OperatorGet(operatorName,
								   operatorNamespace,
								   leftTypeId,
								   rightTypeId,
								   &operatorAlreadyDefined);

	if (operatorAlreadyDefined)
		elog(ERROR, "OperatorDef: operator \"%s\" already defined",
			 operatorName);

	/*
	 * At this point, if operatorObjectId is not InvalidOid then we are
	 * filling in a previously-created shell.
	 */

	/*
	 * Look up registered procedures -- find the return type of
	 * procedureName to place in "result" field. Do this before shells are
	 * created so we don't have to worry about deleting them later.
	 */
	MemSet(typeId, 0, FUNC_MAX_ARGS * sizeof(Oid));
	if (!OidIsValid(leftTypeId))
	{
		typeId[0] = rightTypeId;
		nargs = 1;
	}
	else if (!OidIsValid(rightTypeId))
	{
		typeId[0] = leftTypeId;
		nargs = 1;
	}
	else
	{
		typeId[0] = leftTypeId;
		typeId[1] = rightTypeId;
		nargs = 2;
	}
	procOid = LookupFuncName(procedureName, nargs, typeId);
	if (!OidIsValid(procOid))
		func_error("OperatorDef", procedureName, nargs, typeId, NULL);
	operResultType = get_func_rettype(procOid);

	/*
	 * find restriction estimator
	 */
	if (restrictionName)
	{
		MemSet(typeId, 0, FUNC_MAX_ARGS * sizeof(Oid));
		typeId[0] = 0;			/* Query (opaque type) */
		typeId[1] = OIDOID;		/* operator OID */
		typeId[2] = 0;			/* args list (opaque type) */
		typeId[3] = INT4OID;	/* varRelid */

		restOid = LookupFuncName(restrictionName, 4, typeId);
		if (!OidIsValid(restOid))
			func_error("OperatorDef", restrictionName, 4, typeId, NULL);
	}
	else
		restOid = InvalidOid;

	/*
	 * find join estimator
	 */
	if (joinName)
	{
		MemSet(typeId, 0, FUNC_MAX_ARGS * sizeof(Oid));
		typeId[0] = 0;			/* Query (opaque type) */
		typeId[1] = OIDOID;		/* operator OID */
		typeId[2] = 0;			/* args list (opaque type) */

		joinOid = LookupFuncName(joinName, 3, typeId);
		if (!OidIsValid(joinOid))
			func_error("OperatorDef", joinName, 3, typeId, NULL);
	}
	else
		joinOid = InvalidOid;

	/*
	 * set up values in the operator tuple
	 */

	for (i = 0; i < Natts_pg_operator; ++i)
	{
		values[i] = (Datum) NULL;
		replaces[i] = 'r';
		nulls[i] = ' ';
	}

	i = 0;
	namestrcpy(&oname, operatorName);
	values[i++] = NameGetDatum(&oname);			/* oprname */
	values[i++] = ObjectIdGetDatum(operatorNamespace);	/* oprnamespace */
	values[i++] = Int32GetDatum(GetUserId());		/* oprowner */
	values[i++] = UInt16GetDatum(precedence);		/* oprprec */
	values[i++] = CharGetDatum(leftTypeId ? (rightTypeId ? 'b' : 'r') : 'l');	/* oprkind */
	values[i++] = BoolGetDatum(isLeftAssociative);	/* oprisleft */
	values[i++] = BoolGetDatum(canHash);			/* oprcanhash */
	values[i++] = ObjectIdGetDatum(leftTypeId);		/* oprleft */
	values[i++] = ObjectIdGetDatum(rightTypeId);	/* oprright */
	values[i++] = ObjectIdGetDatum(operResultType);	/* oprresult */

	/*
	 * Set up the other operators.	If they do not currently exist, create
	 * shells in order to get ObjectId's.
	 */

	if (commutatorName)
	{
		/* commutator has reversed arg types */
		commutatorId = get_other_operator(commutatorName,
										  rightTypeId, leftTypeId,
										  operatorName, operatorNamespace,
										  leftTypeId, rightTypeId,
										  true);
		/*
		 * self-linkage to this operator; will fix below. Note
		 * that only self-linkage for commutation makes sense.
		 */
		if (!OidIsValid(commutatorId))
			selfCommutator = true;
	}
	else
		commutatorId = InvalidOid;
	values[i++] = ObjectIdGetDatum(commutatorId);	/* oprcom */

	if (negatorName)
	{
		/* negator has same arg types */
		negatorId = get_other_operator(negatorName,
									   leftTypeId, rightTypeId,
									   operatorName, operatorNamespace,
									   leftTypeId, rightTypeId,
									   false);
	}
	else
		negatorId = InvalidOid;
	values[i++] = ObjectIdGetDatum(negatorId);		/* oprnegate */

	if (leftSortName)
	{
		/* left sort op takes left-side data type */
		leftSortId = get_other_operator(leftSortName,
									   leftTypeId, leftTypeId,
									   operatorName, operatorNamespace,
									   leftTypeId, rightTypeId,
									   false);
	}
	else
		leftSortId = InvalidOid;
	values[i++] = ObjectIdGetDatum(leftSortId);		/* oprlsortop */

	if (rightSortName)
	{
		/* right sort op takes right-side data type */
		rightSortId = get_other_operator(rightSortName,
										 rightTypeId, rightTypeId,
										 operatorName, operatorNamespace,
										 leftTypeId, rightTypeId,
										 false);
	}
	else
		rightSortId = InvalidOid;
	values[i++] = ObjectIdGetDatum(rightSortId);	/* oprrsortop */

	if (ltCompareName)
	{
		/* comparator has same arg types */
		ltCompareId = get_other_operator(ltCompareName,
										 leftTypeId, rightTypeId,
										 operatorName, operatorNamespace,
										 leftTypeId, rightTypeId,
										 false);
	}
	else
		ltCompareId = InvalidOid;
	values[i++] = ObjectIdGetDatum(ltCompareId);	/* oprltcmpop */

	if (gtCompareName)
	{
		/* comparator has same arg types */
		gtCompareId = get_other_operator(gtCompareName,
										 leftTypeId, rightTypeId,
										 operatorName, operatorNamespace,
										 leftTypeId, rightTypeId,
										 false);
	}
	else
		gtCompareId = InvalidOid;
	values[i++] = ObjectIdGetDatum(gtCompareId);	/* oprgtcmpop */

	values[i++] = ObjectIdGetDatum(procOid);		/* oprcode */
	values[i++] = ObjectIdGetDatum(restOid);		/* oprrest */
	values[i++] = ObjectIdGetDatum(joinOid);		/* oprjoin */

	pg_operator_desc = heap_openr(OperatorRelationName, RowExclusiveLock);

	/*
	 * If we are adding to an operator shell, update; else insert
	 */
	if (operatorObjectId)
	{
		tup = SearchSysCacheCopy(OPEROID,
								 ObjectIdGetDatum(operatorObjectId),
								 0, 0, 0);
		if (!HeapTupleIsValid(tup))
			elog(ERROR, "OperatorDef: operator %u not found",
				 operatorObjectId);

		tup = heap_modifytuple(tup,
							   pg_operator_desc,
							   values,
							   nulls,
							   replaces);

		simple_heap_update(pg_operator_desc, &tup->t_self, tup);
	}
	else
	{
		tupDesc = pg_operator_desc->rd_att;
		tup = heap_formtuple(tupDesc, values, nulls);

		heap_insert(pg_operator_desc, tup);
		operatorObjectId = tup->t_data->t_oid;
	}

	/* Must update the indexes in either case */
	if (RelationGetForm(pg_operator_desc)->relhasindex)
	{
		Relation	idescs[Num_pg_operator_indices];

		CatalogOpenIndices(Num_pg_operator_indices, Name_pg_operator_indices, idescs);
		CatalogIndexInsert(idescs, Num_pg_operator_indices, pg_operator_desc, tup);
		CatalogCloseIndices(Num_pg_operator_indices, idescs);
	}

	heap_close(pg_operator_desc, RowExclusiveLock);

	/*
	 * If a commutator and/or negator link is provided, update the other
	 * operator(s) to point at this one, if they don't already have a
	 * link. This supports an alternate style of operator definition
	 * wherein the user first defines one operator without giving negator
	 * or commutator, then defines the other operator of the pair with the
	 * proper commutator or negator attribute.	That style doesn't require
	 * creation of a shell, and it's the only style that worked right
	 * before Postgres version 6.5. This code also takes care of the
	 * situation where the new operator is its own commutator.
	 */
	if (selfCommutator)
		commutatorId = operatorObjectId;

	if (OidIsValid(commutatorId) || OidIsValid(negatorId))
		OperatorUpd(operatorObjectId, commutatorId, negatorId);
}

/*
 * Try to lookup another operator (commutator, etc)
 *
 * If not found, check to see if it is exactly the operator we are trying
 * to define; if so, return InvalidOid.  (Note that this case is only
 * sensible for a commutator, so we error out otherwise.)  If it is not
 * the same operator, create a shell operator.
 */
static Oid
get_other_operator(List *otherOp, Oid otherLeftTypeId, Oid otherRightTypeId,
				   const char *operatorName, Oid operatorNamespace,
				   Oid leftTypeId, Oid rightTypeId, bool isCommutator)
{
	Oid			other_oid;
	bool		otherDefined;
	char	   *otherName;
	Oid			otherNamespace;

	other_oid = OperatorLookup(otherOp,
							   otherLeftTypeId,
							   otherRightTypeId,
							   &otherDefined);

	if (OidIsValid(other_oid))
	{
		/* other op already in catalogs */
		return other_oid;
	}

	otherNamespace = QualifiedNameGetCreationNamespace(otherOp,
													   &otherName);

	if (strcmp(otherName, operatorName) == 0 &&
		otherNamespace == operatorNamespace &&
		otherLeftTypeId == leftTypeId &&
		otherRightTypeId == rightTypeId)
	{
		/*
		 * self-linkage to this operator; caller will fix later. Note
		 * that only self-linkage for commutation makes sense.
		 */
		if (!isCommutator)
			elog(ERROR, "operator cannot be its own negator or sort operator");
		return InvalidOid;
	}

	/* not in catalogs, different from operator, so make shell */
	other_oid = OperatorShellMake(otherName,
								  otherNamespace,
								  otherLeftTypeId,
								  otherRightTypeId);
	if (!OidIsValid(other_oid))
		elog(ERROR,
			 "OperatorDef: can't create operator shell \"%s\"",
			 NameListToString(otherOp));
	return other_oid;
}

/*
 * OperatorUpd
 *
 *	For a given operator, look up its negator and commutator operators.
 *	If they are defined, but their negator and commutator fields
 *	(respectively) are empty, then use the new operator for neg or comm.
 *	This solves a problem for users who need to insert two new operators
 *	which are the negator or commutator of each other.
 */
static void
OperatorUpd(Oid baseId, Oid commId, Oid negId)
{
	int			i;
	Relation	pg_operator_desc;
	HeapTuple	tup;
	char		nulls[Natts_pg_operator];
	char		replaces[Natts_pg_operator];
	Datum		values[Natts_pg_operator];

	for (i = 0; i < Natts_pg_operator; ++i)
	{
		values[i] = (Datum) 0;
		replaces[i] = ' ';
		nulls[i] = ' ';
	}

	/*
	 * check and update the commutator & negator, if necessary
	 *
	 * First make sure we can see them...
	 */
	CommandCounterIncrement();

	pg_operator_desc = heap_openr(OperatorRelationName, RowExclusiveLock);

	tup = SearchSysCacheCopy(OPEROID,
							 ObjectIdGetDatum(commId),
							 0, 0, 0);

	/*
	 * if the commutator and negator are the same operator, do one update.
	 * XXX this is probably useless code --- I doubt it ever makes sense
	 * for commutator and negator to be the same thing...
	 */
	if (commId == negId)
	{
		if (HeapTupleIsValid(tup))
		{
			Form_pg_operator t = (Form_pg_operator) GETSTRUCT(tup);

			if (!OidIsValid(t->oprcom) || !OidIsValid(t->oprnegate))
			{
				if (!OidIsValid(t->oprnegate))
				{
					values[Anum_pg_operator_oprnegate - 1] = ObjectIdGetDatum(baseId);
					replaces[Anum_pg_operator_oprnegate - 1] = 'r';
				}

				if (!OidIsValid(t->oprcom))
				{
					values[Anum_pg_operator_oprcom - 1] = ObjectIdGetDatum(baseId);
					replaces[Anum_pg_operator_oprcom - 1] = 'r';
				}

				tup = heap_modifytuple(tup,
									   pg_operator_desc,
									   values,
									   nulls,
									   replaces);

				simple_heap_update(pg_operator_desc, &tup->t_self, tup);

				if (RelationGetForm(pg_operator_desc)->relhasindex)
				{
					Relation	idescs[Num_pg_operator_indices];

					CatalogOpenIndices(Num_pg_operator_indices, Name_pg_operator_indices, idescs);
					CatalogIndexInsert(idescs, Num_pg_operator_indices, pg_operator_desc, tup);
					CatalogCloseIndices(Num_pg_operator_indices, idescs);
				}
			}
		}

		heap_close(pg_operator_desc, RowExclusiveLock);

		return;
	}

	/* if commutator and negator are different, do two updates */

	if (HeapTupleIsValid(tup) &&
		!(OidIsValid(((Form_pg_operator) GETSTRUCT(tup))->oprcom)))
	{
		values[Anum_pg_operator_oprcom - 1] = ObjectIdGetDatum(baseId);
		replaces[Anum_pg_operator_oprcom - 1] = 'r';

		tup = heap_modifytuple(tup,
							   pg_operator_desc,
							   values,
							   nulls,
							   replaces);

		simple_heap_update(pg_operator_desc, &tup->t_self, tup);

		if (RelationGetForm(pg_operator_desc)->relhasindex)
		{
			Relation	idescs[Num_pg_operator_indices];

			CatalogOpenIndices(Num_pg_operator_indices, Name_pg_operator_indices, idescs);
			CatalogIndexInsert(idescs, Num_pg_operator_indices, pg_operator_desc, tup);
			CatalogCloseIndices(Num_pg_operator_indices, idescs);
		}

		values[Anum_pg_operator_oprcom - 1] = (Datum) NULL;
		replaces[Anum_pg_operator_oprcom - 1] = ' ';
	}

	/* check and update the negator, if necessary */

	tup = SearchSysCacheCopy(OPEROID,
							 ObjectIdGetDatum(negId),
							 0, 0, 0);

	if (HeapTupleIsValid(tup) &&
		!(OidIsValid(((Form_pg_operator) GETSTRUCT(tup))->oprnegate)))
	{
		values[Anum_pg_operator_oprnegate - 1] = ObjectIdGetDatum(baseId);
		replaces[Anum_pg_operator_oprnegate - 1] = 'r';

		tup = heap_modifytuple(tup,
							   pg_operator_desc,
							   values,
							   nulls,
							   replaces);

		simple_heap_update(pg_operator_desc, &tup->t_self, tup);

		if (RelationGetForm(pg_operator_desc)->relhasindex)
		{
			Relation	idescs[Num_pg_operator_indices];

			CatalogOpenIndices(Num_pg_operator_indices, Name_pg_operator_indices, idescs);
			CatalogIndexInsert(idescs, Num_pg_operator_indices, pg_operator_desc, tup);
			CatalogCloseIndices(Num_pg_operator_indices, idescs);
		}
	}

	heap_close(pg_operator_desc, RowExclusiveLock);
}
