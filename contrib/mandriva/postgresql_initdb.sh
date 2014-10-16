#! /bin/sh
# based on the init script for starting up the PostgreSQL server

PGDATA=$1

RETVAL=0

# Check for the PGDATA structure
if [ ! -f ${PGDATA}/PG_VERSION ]; then
	if [ ! -d ${PGDATA} ]; then
		mkdir -p ${PGDATA}
		chown postgres.postgres ${PGDATA}
		chmod go-rwx ${PGDATA}
	fi
	# Initialize the database
	/usr/bin/initdb --pgdata=${PGDATA} &>> /var/log/postgres/postgresql && test -f ${PGDATA}/PG_VERSION
	RETVAL=$?
	echo
fi
exit $RETVAL
