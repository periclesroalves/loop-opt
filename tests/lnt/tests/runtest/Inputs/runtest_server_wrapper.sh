#!/bin/bash
# This script wraps a call to lnt runtest with a local server
# instance.  It is intended for testing full runtest invocations
# that need a real server instnace to work.
# ./runtest_server_wrapper <location of server files> <runtest type> [arguments for lnt runtest]
# ./runtest_server_wrapper /tmp/ nt --cc /bin/clang --sandbox /tmp/sandbox

# First launch the server.
# TODO: we can't run the tests in parrelle unless we do something smart
# with this port.

PROGRAM="$(basename $0)"

usage() {
	echo "usage: $PROGRAM <location of server files> <runtest type> [arguments for lnt runtest]"
	echo "e.g:   $PROGRAM /tmp/ nt --cc /bin/clang --sandbox /tmp/sandbox"
}

error() {
	echo "error: $PROGRAM: $*" >&2
	usage >&2
	exit 1
}

main() {
	[ $# -lt 2 ] &&
		error "not enough arguments"

	lnt runserver $1 --hostname localhost --port 9090 &
	local pid=$!
	local type=$2
	shift 2
	lnt runtest $type --submit http://localhost:9090/db_default/submitRun $@
	local rc=$?

	kill -15 $pid
	local kill_rc=$?
	[ $kill_rc -ne 0 ] &&
	    error "wha happen??  $kill_rc"
	
	wait $pid
	exit $rc
}

main "$@"