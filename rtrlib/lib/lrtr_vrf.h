/*
 * This file is part of RTRlib.
 *
 * Copyright (c) 2019, 6WIND
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */
#ifndef LRTR_UTILS_VRF_H
#define LRTR_UTILS_VRF_H

#include <stdbool.h>

/**
 * @brief This API sets up the VRF mode to be used, as well as permission rights
 * if not called, vrf-lite mechanism is chosen
 * for permission rights, user rights and caps rights are passed as param
 * if none of the permission rights are given, or if not called, no permission right is chose
 * @param[cap_admin] caps rights used to raise privileges
 * @param[cap_user] initial caps rights of user
 * @param[num] number of caps rights pointed by cap_admin
 * @param[uid_admin] admin user rights, when caps are not used
 * @param[uid_user] original user rights, when caps are not used
 * @param[netns_mode] boolean set to true if vrf netns backend is used
 * @return none
 */
void lrtr_vrf_init_capabilities(void **cap_admin, void *cap_user, int num,
				void  *uid_admin, void *uid_user,
				bool netns_mode);

#endif
