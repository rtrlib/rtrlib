/*
 * This file is part of RTRlib.
 *
 * Copyright (c) 2019, 6WIND
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/lib/lrtr_vrf.h"

#include "rtrlib/lib/lrtr_vrf_private.h"
#include "rtrlib/rtrlib_export_private.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef RTRLIB_HAVE_CAPABILITIES
#include <sys/capability.h>
#endif

#define _GNU_SOURCE
#include <sched.h>

#define LRTR_VRF_NETNS_DEFAULT_NAME    "/proc/self/ns/net"
#define LRTR_VRF_NETNS_RUNDIR_NAME    "/var/run/netns/"

#define LRTR_VRF_MODE_NONE 0
#define LRTR_VRF_MODE_CAPS 1
#define LRTR_VRF_MODE_UID 2

static int lrtr_vrf_netns_default_fd;
static int lrtr_vrf_netns_current_fd;
static int lrtr_vrf_netns_activate;
static int lrtr_vrf_rights_mode;
static uid_t lrtr_admin_uid;
static uid_t lrtr_user_uid;
#ifdef RTRLIB_HAVE_CAPABILITIES
static cap_value_t *lrtr_admin_cap;
static cap_t lrtr_user_cap;
static int lrtr_admin_num;
#endif

static void lrtr_vrf_raise_rights(void)
{
	if (lrtr_vrf_rights_mode == LRTR_VRF_MODE_NONE)
		return;
	if (lrtr_vrf_rights_mode == LRTR_VRF_MODE_UID)
		seteuid(lrtr_admin_uid);
#ifdef RTRLIB_HAVE_CAPABILITIES
	else
		cap_set_flag(&lrtr_user_cap, CAP_EFFECTIVE,
			     lrtr_admin_num, lrtr_admin_cap,
			     CAP_SET);
#endif
}

static void lrtr_vrf_lower_rights(void)
{
	if (lrtr_vrf_rights_mode == LRTR_VRF_MODE_NONE)
		return;
	if (lrtr_vrf_rights_mode == LRTR_VRF_MODE_UID)
		seteuid(lrtr_user_uid);
#ifdef RTRLIB_HAVE_CAPABILITIES
	else
		cap_set_flag(&lrtr_user_cap, CAP_EFFECTIVE,
			     lrtr_admin_num, lrtr_admin_cap,
			     CAP_CLEAR);
#endif
}

static char *lrtr_vrf_netns_pathname(const char *netns_name)
{
	static char pathname[PATH_MAX];
	char *result;

	if (netns_name[0] == '/') /* absolute pathname */
		result = realpath(netns_name, pathname);
	else {
		/* relevant pathname */
		char tmp_name[PATH_MAX];

		snprintf(tmp_name, PATH_MAX, "%s%s",
			 LRTR_VRF_NETNS_RUNDIR_NAME, netns_name);
		result = realpath(tmp_name, pathname);
	}

	if (!result)
		return NULL;
	return pathname;
}

RTRLIB_EXPORT void lrtr_vrf_init_capabilities(void **_cap_admin __attribute__((unused)),
					      void *_cap_user __attribute__((unused)),
					      int num __attribute__((unused)),
					      void *_uid_admin, void *_uid_user,
					      bool netns_mode)
{
#ifdef RTRLIB_HAVE_CAPABILITIES
	cap_value_t **cap_admin = (cap_value_t **)_cap_admin;
	cap_t *cap_user = (cap_t *)_cap_user;
#endif
	uid_t *uid_admin = (uid_t *)_uid_admin;
	uid_t *uid_user = (uid_t *)_uid_user;

	if (netns_mode) {
		lrtr_vrf_netns_default_fd = open(LRTR_VRF_NETNS_DEFAULT_NAME, O_RDONLY);
		if (lrtr_vrf_netns_default_fd < 0)
			lrtr_vrf_netns_activate = 0;
		else
			lrtr_vrf_netns_activate = 1;
	}

	if (uid_admin && uid_user) {
		lrtr_admin_uid = *uid_admin;
		lrtr_user_uid = *uid_user;
		lrtr_vrf_rights_mode = LRTR_VRF_MODE_UID;
#ifdef RTRLIB_HAVE_CAPABILITIES
	} else if (cap_admin && cap_user) {
		lrtr_admin_cap = *cap_admin;
		lrtr_user_cap = *cap_user;
		lrtr_vrf_rights_mode = LRTR_VRF_MODE_CAPS;
		lrtr_admin_num = num;
#endif
	} else
		lrtr_vrf_rights_mode = LRTR_VRF_MODE_NONE;
}

int lrtr_vrf_switch_to(const char *name)
{
	int ret, fd;
	char *path;

	/* ignore errors */
	if (!lrtr_vrf_netns_activate || !name ||
	   lrtr_vrf_netns_default_fd == -1)
		return 0;
	path = lrtr_vrf_netns_pathname(name);
	if (!path)
		return -1;
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		errno = EINVAL;
		return -1;
	}
	lrtr_vrf_raise_rights();
#ifndef RTRLIB_HAVE_NETNS
	ret = -1;
#else
	ret = setns(fd, CLONE_NEWNET);
#endif
	if (ret < 0)
		return ret;
	lrtr_vrf_netns_current_fd = fd;
	close(fd);
	return ret;
}

int lrtr_vrf_api_usable(const char *name)
{
	char path[255];
	int fd;

	if (!name)
		return 1;
	/* if netns is not enabled, return true */
	if (!lrtr_vrf_netns_activate)
		return 1;
	snprintf(path, sizeof(path), "%s%s",
		 LRTR_VRF_NETNS_RUNDIR_NAME, name);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	close(fd);
	return 1;
}

int lrtr_vrf_switchback(void)
{
	int ret = 0;

	if (!lrtr_vrf_netns_activate)
		return ret;
	if (lrtr_vrf_netns_current_fd != -1 && lrtr_vrf_netns_default_fd != -1) {
#ifndef RTRLIB_HAVE_NETNS
		ret = -1;
#else
		ret = setns(lrtr_vrf_netns_default_fd, CLONE_NEWNET);
#endif
		lrtr_vrf_netns_current_fd = -1;
		lrtr_vrf_lower_rights();
	}
	return ret;
}

int lrtr_vrf_bind(int socket, const char *vrfname)
{
	int ret = 0;

	if (lrtr_vrf_netns_activate || !vrfname)
		return ret;
#ifdef SO_BINDTODEVICE
	ret = setsockopt(socket, SOL_SOCKET, SO_BINDTODEVICE, vrfname, strlen(vrfname) + 1);
#endif /* SO_BINDTODEVICE */
	return ret;
}
