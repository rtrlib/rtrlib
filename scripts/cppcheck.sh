#!/bin/bash

cppcheck --error-exitcode=1 --enable=all \
	--template='{id}:{file}({line}):({severity}) {message}' \
	--suppress="duplicateExpression:rtrlib/rtr/packets.c:979" \
	--suppress="duplicateExpression:rtrlib/rtr/packets.c:981" \
	--suppress="unusedFunction:rtrlib/lib/ip.c:66" \
	--suppress="unusedFunction:rtrlib/transport/transport.c:23" \
	--suppress="unusedFunction:rtrlib/rtr_mgr.c:327" \
	--suppress="unusedFunction:rtrlib/rtr_mgr.c:332" \
	--suppress="unusedFunction:rtrlib/rtr_mgr.c:352" \
	--suppress="unusedFunction:rtrlib/rtr_mgr.c:357" \
	--suppress="unusedFunction:rtrlib/spki/hashtable/ht-spkitable.c:188" \
	--suppress="CastIntegerToAddressAtReturn:rtrlib/spki/hashtable/tommyds-1.8/tommyarrayof.h:114" \
	--suppress="CastIntegerToAddressAtReturn:rtrlib/spki/hashtable/tommyds-1.8/tommyarrayblkof.h:95" \
	-i rtrlib/spki/hashtable/tommyds-1.8/ tools/ rtrlib/
