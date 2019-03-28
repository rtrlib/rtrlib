/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */


struct pfx_output_template {
	const char *name;
	const char *template;
};


const struct pfx_output_template templates[] = {
${TEMPLATES}
{NULL, NULL}
};
