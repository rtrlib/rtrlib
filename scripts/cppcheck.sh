#!/bin/bash

cppcheck --error-exitcode=1 --enable=all \
	--suppress="duplicateExpression:rtrlib/rtr/packets.c:974" \
	--suppress="duplicateExpression:rtrlib/rtr/packets.c:976" \
	-i rtrlib/spki/hashtable/tommyds-1.8/ tools/ rtrlib/
