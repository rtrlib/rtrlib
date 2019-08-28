#!/usr/bin/env bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

BUILDSTEP_FAILED=0

function run_command {
	eval $@

	ret=$?
	if [ $ret != 0 ]; then
		colour=$RED
		BUILDSTEP_FAILED=1
	else
		colour=$GREEN
	fi
	echo -e "\n${colour}The command \"$@\" exited with $ret.$NC\n\n"

	return $ret
}

function checkpatch {
	git diff $TRAVIS_BRANCH {rtrlib,tools,tests}/**/*.[ch] > /tmp/patch
	run_command scripts/checkpatch.pl --ignore FILE_PATH_CHANGES,PREFER_KERNEL_TYPES,CONST_STRUCT,OPEN_BRACE,SPDX_LICENSE_TAG,OPEN_ENDED_LINE,UNNECESSARY_PARENTHESES,PREFER_PRINTF,GLOBAL_INITIALISERS,PREFER_PACKED,BOOL_MEMBER,STATIC_CONST_CHAR_ARRAY,LONG_LINE_STRING --terse --no-tree --strict --show-types --max-line-length 120 /tmp/patch
	ret=$?
	if [ $ret != 0 ]; then
		cat -n /tmp/patch
	fi
}

[[ $TRAVIS = "true" ]] && run_command git fetch --unshallow
run_command scripts/cppcheck.sh
run_command scripts/check-coding-style.sh
[[ $TRAVIS_EVENT_TYPE = "pull_request" ]] && run_command checkpatch
run_command cmake -D RTRLIB_TRANSPORT_SSH=Off .
run_command make
run_command make test
run_command make clean
run_command cmake -D CMAKE_BUILD_TYPE=Release -DENABLE_COVERAGE=On -DUNIT_TESTING=On -DRTRLIB_TRANSPORT_SSH=On .
run_command make
run_command make test
run_command make gcov
run_command scripts/check-exports.sh

exit $BUILDSTEP_FAILED
