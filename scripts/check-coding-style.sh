#!/bin/bash

script_dir=$(dirname "$(readlink -f "$0")")

cd $script_dir/..
$script_dir/checkpatch.pl -f --strict --no-tree --terse --show-types \
	--ignore PREFER_KERNEL_TYPES \
	rtrlib/pfx/lpfst/lpfst.c \
	rtrlib/pfx/lpfst/lpfst.h
