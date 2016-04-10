#!/bin/bash

cppcheck --error-exitcode=1--enable=all -i rtrlib/spki/hashtable/tommyds-1.8/ tools/ rtrlib/
