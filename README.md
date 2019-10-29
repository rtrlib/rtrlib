Introduction
------------
The RTRlib implements the client-side of the RPKI-RTR protocol (RFC
6810, RFC 8210) and BGP Prefix Origin Validation (RFC 6811). This also
enables the maintenance of router keys. Router keys are required to
deploy BGPSEC.

The software was successfully tested on Linux and FreeBSD.

This package contains also the rtrclient program. It connects to an
RTR-Server over TCP or SSH and shows on STDOUT prefix origin data and
router keys that have been received from the RTR server. The program can
be found in the bin/ directory.

Requirements
------------
To build the RTRlib, the CMake build system must be installed.

To establish an SSH connection between RTR-Client and RTR-Server, the
libssh 0.6.x or higher library must also be installed.

cmocka (optional) is required for unit tests
Doxygen (optional) is required to create the HTML documentation.


Compilation
-----------

* Generate Makefile:

  Without debugging options

    cmake -D CMAKE_BUILD_TYPE=Release .

  With debug symbols and debug messages:

    cmake -D CMAKE_BUILD_TYPE=Debug .

  If the libssh isn't installed within the systems include and library
  directories you can run cmake with the following parameters:

      -D LIBSSH_LIBRARY=<path-to-libssh.so>
      -D LIBSSH_INCLUDE=<include-directory>

  If libssh is installed but you do not want to build rtrlib with ssh
  support, you can disable it with the following parameter:

      -D RTRLIB_TRANSPORT_SSH=No

  Or to enforce ssh support:

      -D RTRLIB_TRANSPORT_SSH=Yes

  To specify another directory where the RTRlib will be installed, you
  can pass the following argument to cmake:

      -D CMAKE_INSTALL_PREFIX=<path>

* Build library, tests, and tools

    make


Installation
------------
To copy libraries and headers to system directories, run (optional):

    make install


Linking to RTRlib
----------------------
The name of the shared library is rtr. To link programs to the RTRlib,
pass the following parameter to gcc:

    -lrtr

In case an error such as

    -/usr/bin/ld: cannot find -lrtr
    -collect2: error: ld returned 1 exit status

occurs, the location of the library can be passed explicitly as a parameter

    -L<path_to_librtr.so>

e.g.,
    -L/usr/local/lib64/


API Documentation
-----------------
The RTRlib includes a HTML documentation of the API. To build them,
doxygen must be installed. The documentation will be located in the
docs/ directory after the execution of:

    make doc


Test RTR-Server Connection
--------------------------
The following command establishes a plain TCP connection to an
RTR-Server using the rtrclient command line tool:

    bin/rtrclient tcp rpki-validator.realmv6.org 8282

rpki-validator.realmv6.org is an open RTR-Server instance for testing
purposes, which runs the RIPE Validator. It listens on port 8282 and
validates ROAs from the following trust anchors: AfriNIC, APNIC, ARIN,
LACNIC, RIPE.


Directories
-----------
* cmake/      - CMake modules
* doxygen/    - Example code and graphics used in the Doxygen
                documentation
* rtrlib/     - Header and source code files of the RTRlib
* tests/      - Unit tests
* tools/      - Contains the rtrclient


Contact
-------
Website: http://rtrlib.realmv6.org/

Mailing List: rtrlib@googlegroups.com





/* vim: set tw=72 sts=2 sw=2 ts=2 expandtab: */
