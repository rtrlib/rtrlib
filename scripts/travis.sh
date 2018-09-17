#!/usr/bin/env bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

function run_command {
	eval $@

	ret=$?
	if [ $ret != 0 ]; then
		colour=$RED
	else
		colour=$GREEN
	fi
	echo -e "\n${colour}The command \"$@\" exited with $ret.$NC\n\n"

	if [ $ret != 0 ]; then
		exit $ret
	fi
}

run_command scripts/cppcheck.sh
run_command scripts/check-coding-style.sh
run_command cmake -D CMAKE_BUILD_TYPE=NoSSH .
run_command make
run_command make test
run_command make clean
run_command cmake -D CMAKE_BUILD_TYPE=Release -DENABLE_COVERAGE=On -DUNIT_TESTING=On .
run_command make
run_command make test
run_command make gcov
run_command scripts/check-exports.sh
