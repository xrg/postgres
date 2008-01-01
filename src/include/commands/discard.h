/*-------------------------------------------------------------------------
 *
 * discard.h
 *	  prototypes for discard.c.
 *
 *
 * Copyright (c) 1996-2008, PostgreSQL Global Development Group
 *
 * $PostgreSQL$
 *
 *-------------------------------------------------------------------------
 */
#ifndef DISCARD_H
#define DISCARD_H

#include "nodes/parsenodes.h"

extern void DiscardCommand(DiscardStmt *stmt, bool isTopLevel);

#endif   /* DISCARD_H */
