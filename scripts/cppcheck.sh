#!/bin/bash

cppcheck --error-exitcode=1 --enable=all \
	--suppress="duplicateExpression:rtrlib/rtr/packets.c:974" \
	--suppress="duplicateExpression:rtrlib/rtr/packets.c:976" \
	--suppress="unusedFunction:rtrlib/lib/ip.c:66" \
	-i rtrlib/spki/hashtable/tommyds-1.8/ tools/ rtrlib/
