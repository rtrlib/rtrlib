#!/bin/bash

# ---HELP---
# check if exported symbols match up with header content and the shared object symbol table
# Must be run from rtrlibs root dir.
# ---HELP---

SO_SYMBOL_WHITELIST=(__bss_start _edata _end _fini _init)

ERROR=0

# Functions annoted with RTRLIB_EXPORT
EXPORTS=$(
    grep -r '^RTRLIB_EXPORT' rtrlib |
    grep -Po '(?<= ).*?(?=\(.*?)' | grep -o  '\w*$' | # extract function name
    sort)

# Functions found in public headers
HEADER_SYMBOLS=$(
    cat $(find rtrlib -iname '*.h' -type f ! -name '*_private.h' ! -name '*tommy*') | # cat all public headers
    gcc -o - -xc -fpreprocessed  -dD -E -P  - | # strip comments via gcc
    grep -Po '(?<= ).*?(?=\(.*?)' | grep -o '\w*$' | # extract function name
    sort)

# Symbols found in librtrs dynamic export table
SO_SYMBOLS=$(nm -g --defined-only librtr.so | awk '{ print $3 }' | sort)

# Filter known false positives in dynamic symbol table
for symbol in "${SO_SYMBOL_WHITELIST[@]}"; do
    SO_SYMBOLS=$(echo "${SO_SYMBOLS}" | grep -v "^${symbol}\$")
done

# check for equality of $EXPORTS AND $HEADER_SYMBOLS
diff -q <(echo "$EXPORTS") <(echo "$HEADER_SYMBOLS") > /dev/null
if [[ $? -ne 0 ]]; then
    echo "Functions annotated for export are not equal to functions found in public header!"
    comm <(echo "$EXPORTS") <(echo "$HEADER_SYMBOLS")
    ERROR=1
fi

echo

# check for equality of $HEADER_SYMBOLS and $SO_SYMBOLS
diff -q <(echo "$HEADER_SYMBOLS") <(echo "$SO_SYMBOLS") > /dev/null
if [[ $? -ne 0 ]]; then
    echo "Functions found in public header are not equal to exported functions!"
    comm <(echo "$HEADER_SYMBOLS") <(echo "$SO_SYMBOLS")
    ERROR=1
fi


exit $ERROR
