#!/bin/sh
# $PostgreSQL: pgsql-server/src/test/bench/runwisc.sh,v 1.6 2004/09/01 17:25:40 tgl Exp $
# 
# Note that in our published benchmark numbers, we executed the command in the
# following fashion:
#
# time $POSTGRES -texecutor -tplanner -f hashjoin bench
#
if [ ! -d $1 ]; then
        echo " you must specify a valid data directory " >&2
        exit
fi

if [ -d ./obj ]; then
	cd ./obj
fi

echo =============== vacuuming benchmark database... ================= >&2
echo "vacuum" | postgres -D${1} bench > /dev/null

echo =============== running benchmark... ================= >&2
time postgres -D${1} -texecutor -tplanner -c log_min_messages=log -c log_destination=stderr -c redirect_stderr=off bench < bench.sql 2>&1
