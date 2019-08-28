#!/bin/bash

# ---HELP---
# to check kernel coding style of file(s):
# a) either pass a filename as cmdline parameter
# b) run without cmdline parameters and check all non third-party code
# ---HELP---

READLINK=$(which greadlink)
[ -z "$READLINK" ] && {
	READLINK=$(which readlink)
}
SCRIPT_DIR=$(dirname "$($READLINK -f "$0")")
SCRIPT_FILE="$SCRIPT_DIR/check-coding-files.txt"
SOURCE_DIR_NAMES="rtrlib tools tests"
EXIT_CODE=0
if [ -z "$1" ] ; then
    for dir in ${SOURCE_DIR_NAMES}; do
        normalized_dir=$($READLINK -f "${SCRIPT_DIR}/../${dir}")
        CHECKSOURCE+=" $(find ${normalized_dir} -name '*.c' -or -name '*.h')"
    done
else
	CHECKSOURCE=$($READLINK -f "$1")
fi
cd $SCRIPT_DIR/..
for i in $CHECKSOURCE; do
	echo "> check coding style of $i ..."
    IGNORE="PREFER_KERNEL_TYPES,CONST_STRUCT,OPEN_BRACE,SPDX_LICENSE_TAG,OPEN_ENDED_LINE,UNNECESSARY_PARENTHESES,PREFER_PRINTF,GLOBAL_INITIALISERS,PREFER_PACKED,BOOL_MEMBER,STATIC_CONST_CHAR_ARRAY,LONG_LINE_STRING"
    if [[ $i == *"unittest"* ]]; then
        IGNORE="${IGNORE},CAMELCASE"
    fi
        $SCRIPT_DIR/checkpatch.pl -f --strict --no-tree --terse --show-types \
               --max-line-length 120 --ignore ${IGNORE} $i

	if [ $? -ne "0" ]; then
		EXIT_CODE=1
	fi
done

exit $EXIT_CODE
