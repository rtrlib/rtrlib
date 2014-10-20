/*
 * This file is part of RTRlib.
 *
 * RTRlib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * RTRlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RTRlib; see the file COPYING.LESSER.
 *
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
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
