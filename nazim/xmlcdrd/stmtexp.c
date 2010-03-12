#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "stmtexp.h"
#include "stmtexpldr.h"


#define VARSTART "$<"
#define VAREND ">"

#define VARSTART_LEN strlen (VARSTART)
#define VAREND_LEN strlen (VAREND)

#define VARID_LEN  512

int stmtexp_load_config (stmtexp_tab_t *vex, const char* config_file)
{
    stmtexpldr_init (vex);
    
    return stmtexpldr_load_config (vex, config_file);
}

void stmtexp_free (stmtexp_tab_t *vex)
{
    stmtexpldr_clear (vex);
}

void stmtexp_assign_var (stmtexp_tab_t *vex, 
                            const char *varid, const char *vartext)
{
    stmtexp_key_t t;
    memset (&t, 0x0, sizeof(stmtexp_key_t) );
    
    snprintf (t.stmt_var, STMTEXPAND_STRLEN_MAX - 1, "%s", varid);

    stmtexp_val_t *tv = g_hash_table_lookup (vex->ght, &t);

    if (NULL == tv)
    {
        /*
        fprintf (stderr, 
                 "Index '%s' is not found. Can't assign var.\n",
                 varid);
        */
        return;
    }

    if (NULL != tv->val)
    {
        /*
          fprintf (stderr, "free old val %s\n", tv->val);
        */
        free (tv->val);
        tv->val = NULL;
    }

    tv->val = strdup (vartext);

}

const char* stmtexp_lookup_var (stmtexp_tab_t *vex, const char *varid)
{
    stmtexp_key_t t;
    memset (&t, 0x0, sizeof(stmtexp_key_t) );
    
    snprintf (t.stmt_var, STMTEXPAND_STRLEN_MAX - 1, "%s", varid);

    
    stmtexp_val_t *tv = g_hash_table_lookup (vex->ght, &t);

    //fprintf (stderr, "stmtexp_lookup_var('%s')", varid);

    if (NULL != tv)
        return tv->val;
    else return NULL;
}


void stmtexp_foreach_callback (stmtexp_tab_t *vex,
                               stmtexp_foreach_callback_t c,
                               void *data)
{
    g_hash_table_foreach (vex->ght, (GHFunc)c, data);
}


const char *stmtexp_expand_str (stmtexp_tab_t *vex, 
                         const char *src, int srclen)
{
    const char *s; //src
    char *e; //end
    char varid [VARID_LEN];
    
    s = src;
    g_string_set_size (vex->expanded, 0);

    while (s < src + srclen)
    {
        if (NULL != (e = strstr (s, VARSTART)) )
        {
            g_string_append_len (vex->expanded, s, e - s);
            s = e++;

            if (NULL != (e = strstr (s, VAREND)) )
            {
                
                /* Remove placeholders VARSTART and VAREND, then lookup */
                memset (varid, 0x0, VARID_LEN);
                strncpy (varid, s + VARSTART_LEN, (e - s) - VARSTART_LEN );
                
                const char* ev = stmtexp_lookup_var (vex, varid);
                if (NULL != ev)
                {
                    /*append expanded var to destination*/
                    g_string_append (vex->expanded, ev);
                }
                else
                {
                    /*variable was not found, skip it*/
                }
                
                s = e + 1;
                //fprintf (stderr, "s: '%s'\n", s);
         
            }
            else   /*no more signatures found, append the rest of the str*/
            {
                if (NULL != s)
                    g_string_append (vex->expanded, s);
                break;
            }
        }
       else /*no more signatures found, append the rest of the str*/
        {
            if (NULL != s)
                g_string_append (vex->expanded, s);
            break;
        }
    }//while (s < src + src_len)

    return vex->expanded->str;
    
}
