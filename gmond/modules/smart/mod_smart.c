/*******************************************************************************
* Copyright (C) 2007 Novell, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* - Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* - Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* - Neither the name of Novell, Inc. nor the names of its
* contributors may be used to endorse or promote products derived from this
* software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* Author: Brad Nicholes (bnicholes novell.com)
******************************************************************************/

/*
* The ganglia metric "C" interface, required for building DSO modules.
*/
#include <gm_metric.h>

#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>

/*
* Declare ourselves so the configuration routines can find and know us.
* We'll fill it in at the end of the module.
*/
extern mmodule smart_module;

#define MAX_CHECK_DEVS 256

static char *smartctl_path = "/usr/sbin/smartctl";
static char *smartctl_prefix = "sg";


int list_devices(char **devices, char *prefix)
{
    DIR *d;
    struct dirent *dir;
    unsigned int i = 0;
    char * tmp;
    size_t len, pref_len = strlen(prefix);

    d = opendir("/dev");
    if (d) {
        while ((dir = readdir(d)) != NULL) {X_CHECK_DEVS 256
            if ((dir->d_type == DT_CHR) && (!strncmp(dir->d_name, prefix, sizeof(char)*pref_len))) {
                len = strlen(dir->d_name);
                printf("%s %d %d\n", dir->d_name, len, pref_len);
                if ((len > pref_len) && isdigit(dir->d_name[pref_len])) {
                    tmp = malloc(sizeof(char)*(len + 1));
                    if (tmp) {
                        strncpy(tmp, dir->d_name, len + 1);
                        devices[i++] = tmp;
                        if (i > MAX_CHECK_DEVS)
                            return -1;
                    }
                    else {
                        return -2;
                    }
                }
            }
        }
    }
    return i;
}

static int ex_metric_init ( apr_pool_t *p )
{
    const char* str_params = smart_module.module_params;
    apr_array_header_t *list_params = smart_module.module_params_list;
    mmparam *params;
    int i;

    /* Read the parameters from the gmond.conf file. */
    /* Single raw string parameter */
    if (str_params) {
        debug_msg("[mod_smart]Received string params: %s", str_params);
    }
    /* Multiple name/value pair parameters. */
    if (list_params) {
        debug_msg("[mod_smart]Received following params list: ");
        params = (mmparam*) list_params->elts;
        for(i=0; i< list_params->nelts; i++) {
            debug_msg("\tParam: %s = %s", params[i].name, params[i].value);
            if (!strcasecmp(params[i].name, "SmartctlPath")) {
                smartctl_path = params[i].value;
            }
            if (!strcasecmp(params[i].name, "SmartCtlPrefix")) {
                smartctl_prefix = params[i].value;
            }
        }
    }

    /* Initialize the metadata storage for each of the metrics and then
* store one or more key/value pairs. The define MGROUPS defines
* the key for the grouping attribute. */
    MMETRIC_INIT_METADATA(&(smart_module.metrics_info[0]),p);
    MMETRIC_ADD_METADATA(&(smart_module.metrics_info[0]),MGROUP,"smart");
    
    return 0;
}

static void ex_metric_cleanup ( void )
{
}

static g_val_t ex_metric_handler ( int metric_index )
{
    g_val_t val;
    int i, k, total_len, ret, rret = 0;
    char * devs[MAX_CHECK_DEVS];
    char *cmd;
    const char *template = "sudo -n %s -H /dev/%s >/dev/null 2>&1\n";

    i = list_devices(devs, smartctl_prefix);
    total_len = strlen(smartctl_path) + (strlen(smartctl_prefix + 1)) + strlen(template);
    cmd = malloc(sizeof(char)*total_len);
    if (cmd == NULL)
        return -1;
    if (i > 0) {
        for (k = 0; k < i; k++) {
            sprintf(cmd, template, smartctl_path, devs[k]);
            free(devs[k]);
            printf("%s", cmd);
            ret = system(cmd);
            rret =  (ret > rret) ? ret : rret;
        }
    }
    free(cmd);
    val.uint32 = rret;
    return val;
}

static Ganglia_25metric ex_metric_info[] =
{
    {0, "SMART_status", 90, GANGLIA_VALUE_UNSIGNED_INT, "Num", "zero", "%u", UDP_HEADER_SIZE+8, "SMART module metric (highest smart status)"},
    {0, NULL}
};

mmodule smart_module =
{
    STD_MMODULE_STUFF,
    ex_metric_init,
    ex_metric_cleanup,
    ex_metric_info,
    ex_metric_handler,
};
