[![Build Status][travis-badge]][travis-link]
[![GitHub release][release-badge]][release-link]
[![License][license-badge]][license-link]
[![API docs][api-badge]][api-link]
[![Wiki][wiki-badge]][wiki-link]
[![Read the Docs][rtd-badge]][rtd-link]
[![Help wanted][helpwanted-badge]][helpwanted-link]

Introduction
------------
The RTRlib implements the client-side of the RPKI-RTR protocol 
([RFC 6810](https://tools.ietf.org/html/rfc6810)), 
([RFC 8210](https://tools.ietf.org/html/rfc8210)) and BGP Prefix Origin
Validation ([RFC 6811](https://tools.ietf.org/html/rfc6811)). This also enables
the maintenance of router keys. Router keys are required to deploy BGPSEC.

The software was successfully tested on Linux and FreeBSD.

This package contains also the `rtrclient` program. It connects to an
RTR-Server over TCP or SSH and shows on STDOUT prefix origin data and
router keys that have been received from the RTR server. The program can
be found in the bin/ directory.

Requirements
------------
To build the RTRlib, the CMake build system must be installed.

To establish an SSH connection between RTR-Client and RTR-Server, the
libssh 0.6.x or higher library must also be installed.

To enable BGPsec support for validating and signing AS paths, libssl
1.0 or higher needs to be installed.

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

      -D LIBSSH_LIBRARIES=<path-to-libssh.so>
      -D LIBSSH_INCLUDE_DIRS=<include-directory>

  If libssh is installed but you do not want to build rtrlib with ssh
  support, you can disable it with the following parameter:

      -D RTRLIB_TRANSPORT_SSH=No

  Or to enforce ssh support:

      -D RTRLIB_TRANSPORT_SSH=Yes

  To specify another directory where the RTRlib will be installed, you
  can pass the following argument to cmake:

      -D CMAKE_INSTALL_PREFIX=<path>

  BGPsec support is enabled by default. If dependencies cannot be
  resolved, rtrlib builds without BGPsec.
  
  To explicitly disable BGPsec:

      -D WITH_BGPSEC=No

  To explicitly enable BGPsec and fail the build if dependencies
  cannot be resolved:

      -D WITH_BGPSEC=Yes

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

`rpki-cache.netd.cs.tu-dresden.de` is an open RTR-Server instance for testing
purposes, which runs the RIPE Validator. It listens on port 3323 and
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


CONTRIBUTE
----------
To contribute something to RTRlib, please refer to our [contributing document](CONTRIBUTING).


Contact
-------
Website: [http://rtrlib.realmv6.org/](http://rtrlib.realmv6.org/)

Mailing List: [rtrlib@googlegroups.com](mailto:rtrlib@googlegroups.com)


[travis-badge]:https://travis-ci.com/rtrlib/rtrlib.svg?branch=master
[travis-link]:https://travis-ci.com/rtrlib/rtrlib
[release-badge]: https://img.shields.io/github/release/rtrlib/rtrlib.svg
[release-link]: https://github.com/rtrlib/rtrlib/releases/latest
[license-badge]: https://img.shields.io/github/license/rtrlib/rtrlib
[license-link]: https://github.com/rtrlib/rtrlib/blob/master/LICENSE
[api-badge]: https://img.shields.io/badge/docs-API-informational.svg
[api-link]: http://rtrlib.realmv6.org/doxygen/latest/
[wiki-badge]: https://img.shields.io/badge/docs-Wiki-informational.svg
[wiki-link]: https://github.com/rtrlib/rtrlib/wiki
[rtd-badge]: https://readthedocs.org/projects/rtrlib/badge/?version=latest 
[rtd-link]: http://rtrlib.readthedocs.io/en/latest/?badge=latest
[helpwanted-badge]: https://img.shields.io/badge/help-wanted-orange.svg
[helpwanted-link]: https://github.com/rtrlib/rtrlib/issues
