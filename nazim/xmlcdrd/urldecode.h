#ifndef URLDECODE_H
#define URLDECODE_H


#include <apreq2/apreq_util.h>
#include <glib.h>


/*
Status codes
*/

#define URLUTIL_STATUS_OK                   0
#define URLUTIL_STATUS_MEM_ALLOC_ERR       -1
#define URLUTIL_STATUS_INVALID_PARAMS      -2
#define URLUTIL_STATUS_EMPTY_SRCH_KEY      -3
#define URLUTIL_STATUS_SRCH_KEY_NOT_FOUND  -4
#define URLUTIL_STATUS_URL_STR_TOO_SHORT   -5
#define URLUTIL_STATUS_URL_KEY_VAL_BROKEN  -6


const char * urldecode_expand_value (GString *url,
                                     const char *key, int key_len,
                                     int case_sensative,
                                     GString *temp, int *status, GString *val);


#endif
