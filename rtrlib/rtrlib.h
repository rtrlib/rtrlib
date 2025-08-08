/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RTRLIB_H
#define RTRLIB_H

#define RTRLIB_HAVE_LIBSSH
#define RTRLIB_VERSION_MAJOR 0
#define RTRLIB_VERSION_MINOR 8
#define RTRLIB_VERSION_PATCH 0

#include "config.h"
#include "lib/alloc_utils.h"
#include "lib/ip.h"
#include "lib/ipv4.h"
#include "lib/ipv6.h"
#include "pfx/pfx.h"
#include "rtr/rtr.h"
#include "rtr_mgr.h"
#include "spki/spkitable.h"
#include "transport/tcp/tcp_transport.h"
#include "transport/transport.h"
#ifdef RTRLIB_HAVE_LIBSSH
#include "rtrlib/transport/ssh/ssh_transport.h"
#endif
#ifdef RTRLIB_BGPSEC_ENABLED
#include "rtrlib/bgpsec/bgpsec.h"
#endif

#endif

#ifdef __cplusplus
}
#endif
