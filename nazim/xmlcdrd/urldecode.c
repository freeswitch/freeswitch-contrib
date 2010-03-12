#include <glib.h>
#include "urldecode.h"



const char * urldecode_expand_value (GString *url,
                             const char *key, int key_len,
                             int case_sensative,
                             GString *temp, int *status, GString *val)
{
    const char *p;
    char *end;
    apr_size_t len;

    if (  NULL == key )
    {
        *status = URLUTIL_STATUS_INVALID_PARAMS;
        return NULL;
    }

    if (0 == strlen(key) || 0 == key_len)
    {
        *status = URLUTIL_STATUS_EMPTY_SRCH_KEY;
        return NULL;
    }
	
    if (strlen((char*)url->str) < key_len )
    {
        *status =  URLUTIL_STATUS_URL_STR_TOO_SHORT;
        return NULL;
    }


    //convert key to urlencoded string,
    //since we are searching for urlencoded keys

    //max length of urlencoded key is 3 * key length
    len = 3 * key_len + 1;
    g_string_set_size(temp, len);
    memset (temp->str, 0x0, len);

    apreq_encode(temp->str, key, key_len );
    //append '=' to the key_name    
    temp->str [strlen(temp->str)] = '=';


    if (0 == case_sensative)
    {
        if (NULL == (p = strcasestr ((const char*) url->str, temp->str)) )
        {
            *status = URLUTIL_STATUS_SRCH_KEY_NOT_FOUND;
            return NULL;
        }
    } else
    {
        if (NULL == (p = strstr ((const char*) url->str, temp->str)) )
        {
            *status = URLUTIL_STATUS_SRCH_KEY_NOT_FOUND;
            return NULL;
        }
    } //~else case_sensative

    
    //urlencoded key is found in cgi request string
    if (NULL == (end = strchr (p, '=')) )
    {
        *status = URLUTIL_STATUS_URL_KEY_VAL_BROKEN;
        return NULL;
    } 
    
    //get value
    //if terminating '&' is not found,
    //then our value ends at the end of the @url string
    p = ++end;
    if (NULL == (end = strchr (p, '&')) )
        end = url->str + strlen(url->str);

    len = 3 * (end - p) + 1;
    g_string_set_size(val, len);
    memset (val->str, 0x0, len);

    apreq_decode (val->str, &len, p, end - p);    

    return val->str;
}

