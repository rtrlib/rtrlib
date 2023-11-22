#!/bin/bash

cppcheck  --error-exitcode=1 --enable=all --inline-suppr \
	--template='{id}:{file}({line}):({severity}) {message}' \
	--suppress=missingInclude --suppress=constParameter --suppress=unmatchedSuppression \
	-i third-party/ tools/ rtrlib/
