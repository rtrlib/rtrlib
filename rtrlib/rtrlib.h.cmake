/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTRLIB_H
#define RTRLIB_H

#cmakedefine RTRLIB_HAVE_LIBSSH
#define RTRLIB_VERSION_MAJOR @RTRLIB_VERSION_MAJOR@
#define RTRLIB_VERSION_MINOR @RTRLIB_VERSION_MINOR@
#define RTRLIB_VERSION_PATCH @RTRLIB_VERSION_PATCH@

#include "rtrlib/transport/transport.h"
#include "rtrlib/transport/tcp/tcp_transport.h"
#include "rtrlib/rtr_mgr.h"
#include "rtrlib/pfx/lpfst/lpfst-pfx.h"
#include "rtrlib/rtr/rtr.h"
#include "rtrlib/lib/utils.h"
#ifdef RTRLIB_HAVE_LIBSSH
#include "rtrlib/transport/ssh/ssh_transport.h"
#endif

#endif
