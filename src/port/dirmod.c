/*-------------------------------------------------------------------------
 *
 * dirmod.c
 *	  rename/unlink()
 *
 * Portions Copyright (c) 1996-2003, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *	These are replacement versions of unlink and rename that work on
 *	Win32 (NT, Win2k, XP).	replace() doesn't work on Win95/98/Me.
 *
 * IDENTIFICATION
 *	  $PostgreSQL$
 *
 *-------------------------------------------------------------------------
 */

#ifndef TEST_VERSION

#if defined(WIN32) || defined(__CYGWIN__)


#ifndef FRONTEND
#include "postgres.h"
#else
#include "postgres_fe.h"
#endif
#include "miscadmin.h"

#undef rename
#undef unlink

int
pgrename(const char *from, const char *to)
{
	int			loops = 0;

#ifdef WIN32
	while (!MoveFileEx(from, to, MOVEFILE_REPLACE_EXISTING))
#endif
#ifdef __CYGWIN__
	while (rename(from, to) < 0)
#endif
	{
#ifdef WIN32
		if (GetLastError() != ERROR_ACCESS_DENIED)
#endif
#ifdef __CYGWIN__
		if (errno != EACCES)
#endif
			/* set errno? */
			return -1;
		pg_usleep(100000);				/* us */
		if (loops == 30)
#ifndef FRONTEND
			elog(LOG, "could not rename \"%s\" to \"%s\", continuing to try",
				 from, to);
#else
			fprintf(stderr, "could not rename \"%s\" to \"%s\", continuing to try\n",
					from, to);
#endif
		loops++;
	}

	if (loops > 30)
#ifndef FRONTEND
		elog(LOG, "completed rename of \"%s\" to \"%s\"", from, to);
#else
		fprintf(stderr, "completed rename of \"%s\" to \"%s\"\n", from, to);
#endif
	return 0;
}


int
pgunlink(const char *path)
{
	int			loops = 0;

	while (unlink(path))
	{
		if (errno != EACCES)
			/* set errno? */
			return -1;
		pg_usleep(100000);				/* us */
		if (loops == 30)
#ifndef FRONTEND
			elog(LOG, "could not unlink \"%s\", continuing to try",
				 path);
#else
			fprintf(stderr, "could not unlink \"%s\", continuing to try\n",
					path);
#endif
		loops++;
	}

	if (loops > 30)
#ifndef FRONTEND
		elog(LOG, "completed unlink of \"%s\"", path);
#else
		fprintf(stderr, "completed unlink of \"%s\"\n", path);
#endif
	return 0;
}

#endif

#else


/*
 *	Illustrates problem with Win32 rename() and unlink()
 *	under concurrent access.
 *
 *	Run with arg '1', then less than 5 seconds later, run with
 *	 arg '2' (rename) or '3'(unlink) to see the problem.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <windows.h>

#define halt(str) \
do { \
	fputs(str, stderr); \
	exit(1); \
} while (0)

int
main(int argc, char *argv[])
{
	FILE	   *fd;

	if (argc != 2)
		halt("Arg must be '1' (test), '2' (rename), or '3' (unlink)\n"
			 "Run '1' first, then less than 5 seconds later, run\n"
			 "'2' to test rename, or '3' to test unlink.\n");

	if (atoi(argv[1]) == 1)
	{
		if ((fd = fopen("/rtest.txt", "w")) == NULL)
			halt("Can not create file\n");
		fclose(fd);
		if ((fd = fopen("/rtest.txt", "r")) == NULL)
			halt("Can not open file\n");
		Sleep(5000);
	}
	else if (atoi(argv[1]) == 2)
	{
		unlink("/rtest.new");
		if ((fd = fopen("/rtest.new", "w")) == NULL)
			halt("Can not create file\n");
		fclose(fd);
		while (!MoveFileEx("/rtest.new", "/rtest.txt", MOVEFILE_REPLACE_EXISTING))
		{
			if (GetLastError() != ERROR_ACCESS_DENIED)
				halt("Unknown failure\n");
			else
				fprintf(stderr, "move failed\n");
			Sleep(500);
		}
		halt("move successful\n");
	}
	else if (atoi(argv[1]) == 3)
	{
		while (unlink("/rtest.txt"))
		{
			if (errno != EACCES)
				halt("Unknown failure\n");
			else
				fprintf(stderr, "unlink failed\n");
			Sleep(500);
		}
		halt("unlink successful\n");
	}
	else
		halt("invalid arg\n");

	return 0;
}

#endif
