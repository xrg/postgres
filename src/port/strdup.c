/*-------------------------------------------------------------------------
 *
 * strdup.c
 *	  copies a null-terminated string.
 *
 * Portions Copyright (c) 1996-2006, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL$
 *
 *-------------------------------------------------------------------------
 */

#include "c.h"


char *
strdup(const char *string)
{
	char	   *nstr;

	nstr = (char *) malloc(strlen(string) + 1);
	if (nstr)
		strcpy(nstr, string);
	return nstr;
}
