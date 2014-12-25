%define git_repo postgresql
%define git_head HEAD
%global release_class pub

%if %_lib == lib64
%define _requires_exceptions devel(libtcl8.4(64bit))
%else
%define _requires_exceptions devel(libtcl8.4)
%endif

%define Werror_cflags %{nil}
%define _disable_ld_no_undefined 1

%define perl_version %(rpm -q --qf "%{VERSION}" perl)
%define perl_epoch %(rpm -q --qf "%{EPOCH}" perl)

%define pgdata /var/lib/pgsql
%define logrotatedir %{_sysconfdir}/logrotate.d

%define major 5
%define major_ecpg 6

%define bname postgresql
%define current_major_version %(echo %{git_get_ver} | sed 's/^\\([0-9]\\+\\.[0-9]\\+\\)\\..*$/\\1/' )

%define libname %mklibname pq%{current_major_version} _%{major}
%define libecpg %mklibname ecpg%{current_major_version} _%{major_ecpg}

%define withuuid 0

Summary: 	PostgreSQL client programs and libraries
Name:		%{bname}%{current_major_version}
Version: 	%git_get_ver
Release: 	%mkrel %git_get_rel2
License:	BSD
Group:		Databases
URL:		http://www.postgresql.org/ 
Source0:	%git_bs_source %{name}-%{version}.tar.gz
Requires:	perl
Provides:	postgresql-clients = %{version}-%{release}
Conflicts:	postgresql-clients < %{version}-%{release}
# BuildRequires:	X11-devel
BuildRequires: bison flex
BuildRequires: gettext
BuildRequires: termcap-devel
BuildRequires: ncurses-devel
BuildRequires: openssl-devel
BuildRequires:	pam-devel
BuildRequires: perl-devel
BuildRequires: python-devel
BuildRequires: readline-devel >= 4.3
BuildRequires: tk
BuildRequires:  tcl tcl-devel
BuildRequires:  libxml2-devel
BuildRequires:  libxslt-devel
BuildRequires:	edit-devel
BuildRequires:  zlib-devel
%if %withuuid
BuildRequires:  ossp_uuid-devel
%endif

# Need to build doc
BuildRequires:  docbook-dtd42-sgml openjade docbook-utils xsltproc
# Buildroot:	%{_tmppath}/%{name}-%{version}-%{release}-buildroot
Provides: %{bname}-virtual = %{current_major_version}
Conflicts: %{bname}-virtual < %{current_major_version}
Conflicts: %{bname}-virtual > %{current_major_version}
Provides: %{bname} = %{version}-%{release}
Conflicts: %{bname} < %{version}-%{release}

%description
PostgreSQL is an advanced Object-Relational database management system (DBMS)
that supports almost all SQL constructs (including transactions, subselects and
user-defined types and functions). The postgresql package includes the client
programs and libraries that you'll need to access a PostgreSQL DBMS server.
These PostgreSQL client programs are programs that directly manipulate the
internal structure of PostgreSQL databases on a PostgreSQL server. These client
programs can be located on the same machine with the PostgreSQL server, or may
be on a remote machine which accesses a PostgreSQL server over a network
connection. This package contains the client libraries for C and C++, as well
as command-line utilities for managing PostgreSQL databases on a PostgreSQL
server.

If you want to manipulate a PostgreSQL database on a remote PostgreSQL server,
you need this package. You also need to install this package if you're
installing the postgresql-server package.

%package -n	%{libname}
Summary:	The shared libraries required for any PostgreSQL clients
Group:		System/Libraries
Conflicts:	postgresql-libs < %{version}-%{release}
Provides:	postgresql-libs = %{version}-%{release}
Provides:   libpq = %{version}-%{release}
Provides:   %{libname}-virtual = %{current_major_version}
Conflicts:  %{libname}-virtual > %{current_major_version}
Conflicts:  %{libname}-virtual < %{current_major_version}
# Avoid conflicts with lib having bad major
Conflicts:  libpq3 = 8.0.2

%description -n	%{libname}
C and C++ libraries to enable user programs to communicate with the PostgreSQL
database backend. The backend can be on another machine and accessed through
TCP/IP.


%package -n	%{libecpg}
Summary:	Shared library libecpg for PostgreSQL
Group:		System/Libraries
Requires:	postgresql%{current_major_version} = %{version}-%{release}
Provides:	libecpg = %{version}-%{release}
Provides:   %{libecpg}-virtual = %{current_major_version}
Conflicts:  %{libecpg}-virtual < %{current_major_version}
Conflicts:  %{libecpg}-virtual > %{current_major_version}

%description -n	%{libecpg}
Libecpg is used by programs built with ecpg (Embedded PostgreSQL for C)
Use postgresql-dev to develop such programs.

%package	server
Summary:	The programs needed to create and run a PostgreSQL server
Group:		Databases
Provides:	sqlserver
Requires(post):   %{libname} >= %{version}-%{release}
Requires(preun):   %{libname} >= %{version}-%{release}
# add/remove services
Requires(post): rpm-helper
Requires(preun): rpm-helper
# add/del user
Requires(pre): rpm-helper
Requires(postun): rpm-helper
Requires(pre):	postgresql%{current_major_version} >= %{version}-%{release}
Requires(post):	postgresql%{current_major_version} >= %{version}-%{release}
Conflicts:	postgresql < 7.3
Provides: %{?arch_tagged:%arch_tagged %{bname}-server-ABI}%{?!arch_tagged:%{bname}-server-ABI} = %{current_major_version}
Provides: %{bname}-server-virtual = %{current_major_version}
Conflicts: %{bname}-server-virtual < %{current_major_version}
Conflicts: %{bname}-server-virtual > %{current_major_version}
Provides: %{bname}-server = %{version}-%{release}
Conflicts: %{bname}8.3-test < %{version}-%{release}

%description	server
The postgresql-server package includes the programs needed to create and run a
PostgreSQL server, which will in turn allow you to create and maintain
PostgreSQL databases.  PostgreSQL is an advanced Object-Relational database
management system (DBMS) that supports almost all SQL constructs (including
transactions, subselects and user-defined types and functions). You should
install postgresql-server if you want to create and maintain your own PostgreSQL
databases and/or your own PostgreSQL server. You also need to install the
postgresql and postgresql-devel packages.

After installing this package, please read postgresql.%{_vendor}.releasenote.

%package	docs
Summary:	Extra documentation for PostgreSQL
Group:		Databases
BuildArch:     noarch
Provides: %{bname}-docs-virtual = %{current_major_version}
Conflicts: %{bname}-docs-virtual < %{current_major_version}
Conflicts: %{bname}-docs-virtual > %{current_major_version}

%description	docs
The postgresql-docs package includes the SGML source for the documentation
as well as the documentation in other formats, and some extra documentation.
Install this package if you want to help with the PostgreSQL documentation
project, or if you want to generate printed documentation.

%package       contrib
Summary:       Contributed binaries distributed with PostgreSQL
Group:         Databases
Requires:      postgresql%{current_major_version}-server = %{version}-%{release}
Provides:      %{bname}-contrib-virtual = %{current_major_version}
Conflicts:     %{bname}-contrib-virtual < %{current_major_version}

%description   contrib
The postgresql-contrib package includes the contrib tree distributed with the
PostgreSQL tarball.  Selected contrib modules are prebuilt.


%package	devel
Summary:	PostgreSQL development header files and libraries
Group:		Development/Databases
Requires:	postgresql%{current_major_version} = %{version}-%{release}
Provides: %{bname}-devel-virtual = %{current_major_version}
Conflicts: %{bname}-devel-virtual < %{current_major_version}
Conflicts: %{bname}-devel-virtual > %{current_major_version}
Requires:	%{libname} = %{version}-%{release}
Provides:	postgresql-libs-devel = %{version}-%{release}
Provides:       pq-devel = %{version}-%{release}
%if %_lib != lib
Provides:   libpq-devel = %{version}-%{release}
%endif
Provides:   %{_lib}pq-devel = %{version}-%{release}
Conflicts:  %{_lib}pq-devel < %{version}
Conflicts:  %{_lib}pq-devel > %{version}
Requires:   %{libecpg} = %{version}-%{release}
Provides:   libecpg-devel = %{version}-%{release}
Provides:   %{_lib}ecpg-devel = %{version}-%{release}
Conflicts:  %{_lib}ecpg-devel < %{version}-%{release}
Conflicts:  %{_lib}ecpg-devel > %{version}-%{release}
Conflicts:  %mklibname -d ecpg 5
Conflicts:  %mklibname -d pq 5
Conflicts:  %mklibname -d pq8.3
Conflicts:  %mklibname -d ecpg8.3
Provides:   %{bname}-devel = %{version}-%{release}
Conflicts:  %{bname}-devel < %{version}-%{release}

%description	devel
The postgresql-devel package contains the header files and libraries needed to
compile C or C++ applications which will directly interact with a PostgreSQL
database management server and the ecpg Embedded C Postgres preprocessor. You
need to install this package if you want to develop applications which will
interact with a PostgreSQL server. If you're installing postgresql-server, you
need to install this package.


%package	pl
Summary:	Procedurals languages for PostgreSQL
Group:		Databases
Conflicts:	libpgsql2
Requires:	%{name}-plpython = %{version}-%{release} 
Requires:	%{name}-plperl = %{version}-%{release} 
Requires:	%{name}-pltcl = %{version}-%{release} 
Requires:	%{name}-plpgsql = %{version}-%{release} 
Provides: %{bname}-pl-virtual = %{current_major_version}
Conflicts: %{bname}-pl-virtual < %{current_major_version}
Conflicts: %{bname}-pl-virtual > %{current_major_version}
Conflicts: %{bname}-pl < %{version}-%{release}
Provides:  %{bname}-pl = %{version}-%{release}

%description	pl
PostgreSQL is an advanced Object-Relational database management system.  The
postgresql-pl will install the the PL/Perl, PL/Tcl, and PL/Python procedural
languages for the backend. PL/Pgsql is part of the core server package.

%package    plpython
Summary:    The PL/Python procedural language for PostgreSQL
Group:      Databases
Requires:   postgresql%{current_major_version}-server = %{version}
Conflicts:  postgresql-pl < %version-%release
Requires: %{?arch_tagged:%arch_tagged %{bname}-server-ABI}%{?!arch_tagged:%{bname}-server-ABI} = %{current_major_version}
Provides: %{bname}-plpython-virtual = %{current_major_version}
Conflicts: %{bname}-plpython-virtual < %{current_major_version}
Conflicts: %{bname}-plpython-virtual > %{current_major_version}
Conflicts: %{bname}-plpython < %{version}-%{release}
Provides:  %{bname}-plpython = %{version}-%{release}

%description	plpython
PostgreSQL is an advanced Object-Relational database management system.  The
postgresql-plpython package contains the the PL/Python procedural languages for
the backend. PL/Python is part of the core server package.


%package    plperl
Summary:    The PL/Perl procedural language for PostgreSQL
Group:      Databases
Requires:   postgresql%{current_major_version}-server = %{version}
Requires:   perl-base = %{perl_epoch}:%{perl_version}
Conflicts:  postgresql-pl < %version-%release
Requires: %{?arch_tagged:%arch_tagged %{bname}-server-ABI}%{?!arch_tagged:%{bname}-server-ABI} = %{current_major_version}
Provides: %{bname}-plperl-virtual = %{current_major_version}
Conflicts: %{bname}-plperl-virtual < %{current_major_version}
Conflicts: %{bname}-plperl-virtual > %{current_major_version}
Conflicts: %{bname}-plperl < %{version}-%{release}
Provides:  %{bname}-plperl = %{version}-%{release}

%description	plperl
PostgreSQL is an advanced Object-Relational database management system.  The
postgresql-plperl package contains the the PL/Perl procedural languages for the
backend. PL/Perl is part of the core server package.

%package    pltcl
Summary:    The PL/Tcl procedural language for PostgreSQL
Group:      Databases
Requires:   postgresql%{current_major_version}-server = %{version}
Conflicts:  postgresql-pl < %version-%release
Requires: %{?arch_tagged:%arch_tagged %{bname}-server-ABI}%{?!arch_tagged:%{bname}-server-ABI} = %{current_major_version}
Provides: %{bname}-pltcl-virtual = %{current_major_version}
Conflicts: %{bname}-pltcl-virtual < %{current_major_version}
Conflicts: %{bname}-pltcl-virtual > %{current_major_version}
Conflicts: %{bname}-pltcl < %{version}-%{release}
Provides:  %{bname}-pltcl = %{version}-%{release}

%description	pltcl
PostgreSQL is an advanced Object-Relational database management system.  The
postgresql-pltcl package contains the the PL/Tcl procedural languages for the
backend. PL/Tcl is part of the core server package.

%package    plpgsql
Summary:    The PL/PgSQL procedural language for PostgreSQL
Group:      Databases
Requires:   postgresql%{current_major_version}-server = %{version}
Conflicts:  postgresql-pl < %version-%release
Requires: %{?arch_tagged:%arch_tagged %{bname}-server-ABI}%{?!arch_tagged:%{bname}-server-ABI} = %{current_major_version}
Provides: %{bname}-plpgsql-virtual = %{current_major_version}
Conflicts: %{bname}-plpgsql-virtual < %{current_major_version}
Conflicts: %{bname}-plpgsql-virtual > %{current_major_version}
Conflicts: %{bname}-plpgsql < %{version}-%{release}
Provides:  %{bname}-plpgsql = %{version}-%{release}

%description	plpgsql
PostgreSQL is an advanced Object-Relational database management system.  The
postgresql-plpgsql package contains the the PL/PgSQL procedural languages for
the backend. PL/PgSQL is part of the core server package.

%prep
%git_get_source
%setup -q

%build

%serverbuild

%configure2_5x \
        --disable-rpath \
        --enable-hba \
        --enable-locale \
        --enable-multibyte \
        --enable-syslog\
        --with-CXX \
        --enable-odbc \
        --with-perl \
        --with-python \
        --with-tcl --with-tclconfig=%{_libdir} \
        --with-openssl \
        --with-pam \
        --with-libxml \
        --with-libxslt \
        --libdir=%{_libdir} \
        --mandir=%{_mandir} \
        --prefix=%_prefix \
        --sysconfdir=%{_sysconfdir}/pgsql \
        --enable-nls \
%if %{withuuid}
        --with-ossp-uuid
%endif

# $(rpathdir) come from Makefile
perl -pi -e 's|^all:|LINK.shared=\$(COMPILER) -shared -Wl,-rpath,\$(rpathdir),-soname,\$(soname)\nall:|' src/pl/plperl/GNUmakefile


%make world

pushd src/test
make all
popd

%check
#remove test suite from src.rpm for now
rm -rf src/test

#disabled for now, per CVE-2014-0067 - test suite allows local users to gain privileges
#make check



%install
make DESTDIR=%{buildroot} install-world install-docs

# install odbcinst.ini
mkdir -p %{buildroot}%{_sysconfdir}/pgsql

# copy over Makefile.global to the include dir....
#install -m755 src/Makefile.global %{buildroot}%{_includedir}/pgsql/

# PGDATA needs removal of group and world permissions due to pg_pwd hole.
install -d -m 700 %{buildroot}/var/lib/pgsql/data

# backups of data go here...
install -d -m 700 %{buildroot}/var/lib/pgsql/backups

# Create the multiple postmaster startup directory
install -d -m 700 %{buildroot}/etc/sysconfig/pgsql

%if %{mgaver} >= 3
# install systemd units
mkdir -p %{buildroot}%{_unitdir}
install -m 644 contrib/mandriva/postgresql.service %{buildroot}%{_unitdir}/%{bname}.service

# Create the directory for sockets.
install -d -m 755 %{buildroot}/var/run/postgresql

# ... and make a tmpfiles script to recreate it at reboot.
mkdir -p %{buildroot}%{_tmpfilesdir}
install -m 0644 contrib/mandriva/postgresql.tmpfiles.d %{buildroot}%{_tmpfilesdir}/postgresql.conf

# install helper script for env initialisation
mkdir -p %{buildroot}%{_libexecdir}
install -m 755 contrib/mandriva/postgresql_initdb.sh %{buildroot}%{_libexecdir}/
%else
# old, SysV init script
install -D -m755 contrib/mandriva/postgresql.init %{buildroot}%{_initrddir}/postgresql

%endif

mkdir -p %{buildroot}/var/log/postgres

mkdir -p %{buildroot}%{logrotatedir}
cat > %{buildroot}%{logrotatedir}/%{bname} <<EOF
/var/log/postgres/postgresql {
    notifempty
    missingok
    copytruncate
}
EOF

mv %{buildroot}%{_docdir}/%{bname}/html %{buildroot}%{_docdir}/%{name}-docs-%{version}

echo -n '' > main.lst
for i in \
    pg_ctl initdb pg_config psql pg_dump pgscripts libpq libecpg \
    ecpg libpq%{major} ecpglib%{major_ecpg}; do
    %find_lang $i-%{current_major_version}
    cat $i-%{current_major_version}.lang >> main.lst
    %find_lang $i
    cat $i.lang >> main.lst
done
echo -n '' > server.lst
for i in postgres pg_resetxlog pg_controldata plpgsql plpython plperl pltcl pg_basebackup; do
    %find_lang $i
    cat $i.lang >> server.lst
    %find_lang $i-%{current_major_version}
    cat $i-%{current_major_version}.lang >> server.lst
done

# pg_ctl.lang initdb.lang pg_config.lang psql.lang pg_dump.lang pgscripts.lang \
# postgres.lang pg_resetxlog.lang pg_controldata.lang \
# libpq.lang libecpg.lang \

# taken directly in build dir.
rm -fr %{buildroot}%{_datadir}/doc/postgresql/contrib/
rm -fr %{buildroot}%{_datadir}/doc/postgresql/extension/

mkdir -p %{buildroot}/%{_sys_macros_dir}
cat > %{buildroot}/%{_sys_macros_dir}/%{name}.macros <<EOF
%%postgresql_version %{version}
%%postgresql_major   %{current_major_version}
%%postgresql_minor   %{current_minor_version}
%%pgmodules_req Requires: %{?arch_tagged:%arch_tagged %{bname}-server-ABI}%{?!arch_tagged:%{bname}-server-ABI} = %{current_major_version}
EOF

cat contrib/mandriva/postgresql.mdv.releasenote > postgresql.%{_vendor}.releasenote
cat > README.urpmi <<EOF
You just installed or updated %{bname} server.
You can found important informations about %{distribution} %{bname} rpms and database
management in:

%{_defaultdocdir}/%{name}-server/postgresql.%{_vendor}.releasenote

Please read it.
EOF

# postgres' .profile and .bashrc
install -D -m 700 contrib/mandriva/postgres.profile %{buildroot}/var/lib/pgsql/.profile
(
cd %{buildroot}/var/lib/pgsql/
ln -s .profile .bashrc
)

cat > %{buildroot}%{_sysconfdir}/sysconfig/postgresql <<EOF
# Olivier Thauvin <nanardon@mandriva.org>

# The database location:
# You probably won't change this
# PGDATA=/var/lib/pgsql/data

# What is the based locales for postgresql
# Setting locales to C allow to use any encoding
# ISO or UTF, any other choice will restrict you
# either ISO or UTF.
LC_ALL=C

# These are additional to pass to pg_ctl when starting/restarting postgresql.
# PGOPTIONS=
EOF

%pre server
%_pre_useradd postgres /var/lib/pgsql /bin/bash

[ ! -f %{pgdata}/data/PG_VERSION ] && exit 0
mypgversion=`cat %{pgdata}/data/PG_VERSION`
[ $mypgversion = %{current_major_version} ] && exit 0

echo ""
echo "You currently have database tree for Postgresql $mypgversion"
echo "You must use postgresql${mypgversion}-server"
echo "To update you Postgresql server, dump your databases"
echo "delete /var/lib/pgsql/data/ content, upgrade the server, then"
echo "restore your databases from your backup"
echo ""

exit 1

%post server
%_post_service %{bname}
%if %{mgaver} >= 3
%_tmpfilescreate %{_tmpfilesdir}/%{bname}
%endif

%preun server
%_preun_service %{bname}

%postun server
%_postun_userdel postgres

%files -f main.lst 
%defattr(-,root,root)
%doc doc/KNOWN_BUGS doc/MISSING_FEATURES 
%doc COPYRIGHT README HISTORY doc/bug.template
%{_bindir}/clusterdb
%{_bindir}/createdb
%{_bindir}/createlang
%{_bindir}/createuser
%{_bindir}/dropdb
%{_bindir}/droplang
%{_bindir}/dropuser
%{_bindir}/pg_dump
%{_bindir}/pg_dumpall
%{_bindir}/pg_isready
%{_bindir}/pg_restore
%{_bindir}/psql
%{_bindir}/reindexdb
%{_bindir}/vacuumdb
%{_mandir}/man1/clusterdb.*
%{_mandir}/man1/createdb.*
%{_mandir}/man1/createlang.*
%{_mandir}/man1/createuser.*
%{_mandir}/man1/dropdb.*
%{_mandir}/man1/droplang.*
%{_mandir}/man1/dropuser.*
%{_mandir}/man1/pg_dump.*
%{_mandir}/man1/pg_dumpall.*
%{_mandir}/man1/pg_isready.*
%{_mandir}/man1/pg_restore.*
%{_mandir}/man1/psql.*
%{_mandir}/man1/vacuumdb.*
%{_mandir}/man1/reindexdb.*
%{_mandir}/man7/*
%{_sys_macros_dir}/%{name}.macros

%files -n %{libname} 
%defattr(-,root,root)
%{_libdir}/libpq.so.%{major}*

%files -n %{libecpg}
%defattr(-,root,root)
%{_libdir}/libecpg.so.%{major_ecpg}*
%{_libdir}/libecpg_compat.so.*
%{_libdir}/libpgtypes.so.*

%files docs
%defattr(-,root,root)
%doc %{_docdir}/%{name}-docs-%{version}

%files contrib
# %doc contrib/*/README.* contrib/spi/*.example
%{_libdir}/postgresql/_int.so
%{_libdir}/postgresql/btree_gist.so
%{_libdir}/postgresql/chkpass.so
%{_libdir}/postgresql/cube.so
%{_libdir}/postgresql/dblink.so
%{_libdir}/postgresql/earthdistance.so
%{_libdir}/postgresql/fuzzystrmatch.so
%{_libdir}/postgresql/insert_username.so
%{_libdir}/postgresql/lo.so
%{_libdir}/postgresql/ltree.so
%{_libdir}/postgresql/moddatetime.so
%{_libdir}/postgresql/pgcrypto.so
%{_libdir}/postgresql/pgstattuple.so
%{_libdir}/postgresql/refint.so
%{_libdir}/postgresql/seg.so
%{_libdir}/postgresql/tablefunc.so
%{_libdir}/postgresql/timetravel.so
%{_libdir}/postgresql/pg_trgm.so
%{_libdir}/postgresql/autoinc.so
%{_libdir}/postgresql/pg_buffercache.so
%{_libdir}/postgresql/adminpack.so
%{_libdir}/postgresql/hstore.so
%{_libdir}/postgresql/isn.so
%{_libdir}/postgresql/pg_freespacemap.so
%{_libdir}/postgresql/pgrowlocks.so
%{_libdir}/postgresql/sslinfo.so
%{_libdir}/postgresql/pageinspect.so
%{_libdir}/postgresql/tcn.so
%{_libdir}/postgresql/postgres_fdw.so
%{_libdir}/postgresql/worker_spi.so
%{_libdir}/postgresql/pg_prewarm.so
%{_libdir}/postgresql/test_decoding.so
%{_libdir}/postgresql/test_shm_mq.so

%{_bindir}/oid2name
%{_bindir}/pgbench
%{_bindir}/vacuumlo
%{_bindir}/pg_test_timing
%{_bindir}/pg_xlogdump
%{_mandir}/man1/oid2name.1*
%{_mandir}/man1/pgbench.1*
%{_mandir}/man1/pg_test_timing.1*
%{_mandir}/man1/pg_xlogdump.1*
%{_mandir}/man1/vacuumlo.1*

%files server -f server.lst
%defattr(-,root,root)

%config(noreplace) %{_sysconfdir}/sysconfig/postgresql
%doc README.urpmi postgresql.%{_vendor}.releasenote
%if %{mgaver} >= 3
%{_unitdir}/%{bname}.service
%{_tmpfilesdir}/postgresql.conf
%{_libexecdir}/postgresql_initdb.sh
%else
%config(noreplace) %{_initrddir}/postgresql
%endif
%{_bindir}/initdb
%{_bindir}/pg_controldata 
%{_bindir}/pg_ctl
%{_bindir}/pg_resetxlog
%{_bindir}/postgres
%{_bindir}/postmaster
%{_bindir}/pg_standby
%{_bindir}/pg_archivecleanup
%{_bindir}/pg_upgrade
%{_bindir}/pg_basebackup
%{_bindir}/pg_recvlogical
%{_bindir}/pg_receivexlog
%{_bindir}/pg_test_fsync
%{_mandir}/man1/initdb.1*
%{_mandir}/man1/pg_archivecleanup.1*
%{_mandir}/man1/pg_controldata.*
%{_mandir}/man1/pg_ctl.1*
%{_mandir}/man1/pg_recvlogical.1*
%{_mandir}/man1/pg_receivexlog.1*
%{_mandir}/man1/pg_standby.1*
%{_mandir}/man1/pg_resetxlog.*
%{_mandir}/man1/postgres.1*
%{_mandir}/man1/postmaster.1*
%{_mandir}/man1/pg_basebackup.1*
%{_mandir}/man1/pg_test_fsync.1*
%{_mandir}/man1/pg_upgrade.1*
%dir %{_libdir}/postgresql
%dir %{_datadir}/postgresql
%attr(755,postgres,postgres) %ghost %dir /var/run/postgresql
%config(noreplace) /var/lib/pgsql/.bashrc
%attr(-,postgres,postgres) /var/lib/pgsql/.profile
%attr(700,postgres,postgres) %dir %{pgdata}
%attr(-,postgres,postgres) %{pgdata}/data
%attr(700,postgres,postgres) %dir %{pgdata}/backups
%{_libdir}/postgresql/*_and_*.so
%{_libdir}/postgresql/pgxml.so
%{_libdir}/postgresql/dict_int.so
%{_libdir}/postgresql/dict_xsyn.so
%{_libdir}/postgresql/test_parser.so
%{_libdir}/postgresql/tsearch2.so
%{_libdir}/postgresql/dict_snowball.so
%{_libdir}/postgresql/auto_explain.so
%{_libdir}/postgresql/btree_gin.so
%{_libdir}/postgresql/citext.so
%{_libdir}/postgresql/pg_stat_statements.so
%{_libdir}/postgresql/auth_delay.so
%{_libdir}/postgresql/dummy_seclabel.so
%{_libdir}/postgresql/file_fdw.so
%if %{withuuid}
%{_libdir}/postgresql/uuid-ossp.so
%endif

%{_libdir}/postgresql/euc2004_sjis2004.so
%{_libdir}/postgresql/libpqwalreceiver.so
%{_libdir}/postgresql/passwordcheck.so
%{_libdir}/postgresql/pg_upgrade_support.so
%{_libdir}/postgresql/unaccent.so
%{_datadir}/postgresql/postgres.bki
%{_datadir}/postgresql/postgres.description
%{_datadir}/postgresql/*.sample
%{_datadir}/postgresql/timezone
%{_datadir}/postgresql/system_views.sql
%{_datadir}/postgresql/conversion_create.sql
%{_datadir}/postgresql/information_schema.sql
%{_datadir}/postgresql/snowball_create.sql
%{_datadir}/postgresql/sql_features.txt

%{_datadir}/postgresql/postgres.shdescription
%{_datadir}/postgresql/timezonesets
%{_datadir}/postgresql/tsearch_data
%{_datadir}/postgresql/extension

%attr(700,postgres,postgres) %dir /var/log/postgres
%{logrotatedir}/%{bname}

%files devel
%defattr(-,root,root)
# %doc doc/TODO doc/TODO.detail
%{_includedir}/*
%{_bindir}/ecpg
%{_libdir}/lib*.so
%{_libdir}/lib*.a
%{_libdir}/pkgconfig/*.pc
%{_libdir}/postgresql/pgxs/
%{_mandir}/man1/ecpg.1*
%{_bindir}/pg_config
%{_mandir}/man1/pg_config.1*
%{_mandir}/man3/SPI_*.3*
%{_mandir}/man3/dblink*.3*

%files pl 
%defattr(-,root,root) 

%files plpython
%defattr(-,root,root) 
%{_libdir}/postgresql/plpython2.so

%files plperl
%defattr(-,root,root) 
%{_libdir}/postgresql/plperl.so 

%files pltcl
%defattr(-,root,root) 
%{_libdir}/postgresql/pltcl.so 
%{_bindir}/pltcl_delmod 
%{_bindir}/pltcl_listmod 
%{_bindir}/pltcl_loadmod 
%{_datadir}/postgresql/unknown.pltcl 

%files plpgsql
%defattr(-,root,root) 
%{_libdir}/postgresql/plpgsql.so

%changelog -f %{_sourcedir}/%{name}-changelog.gitrpm.txt
