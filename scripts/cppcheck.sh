#!/bin/bash

cppcheck --error-exitcode=1 --enable=all --inline-suppr \
	--template='{id}:{file}({line}):({severity}) {message}' \
	-i rtrlib/spki/hashtable/tommyds-2.2/ tools/ rtrlib/
