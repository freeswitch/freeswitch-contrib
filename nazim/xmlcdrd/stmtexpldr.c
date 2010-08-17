#include "stmtexptab.h"
#include "stmtexpldr.h"
#include <libconfig.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>


#define EXPANDED_STR_DEF_LEN  10240



gboolean stmtexpldr_equal (gconstpointer a, gconstpointer b);
guint stmtexpldr_str_hash (gconstpointer v);
void stmtexpldr_destroy_val (gpointer data);
void stmtexpldr_destroy_key (gpointer data);


gboolean stmtexpldr_equal (gconstpointer a, gconstpointer b)
{
    const stmtexp_key_t* ta = (stmtexp_key_t *) a;
    const stmtexp_key_t* tb = (stmtexp_key_t *) b;

    return g_str_equal (ta->stmt_var, tb->stmt_var);
}


guint stmtexpldr_str_hash (gconstpointer v)
{
    const stmtexp_key_t* tv = (stmtexp_key_t *) v;

    return (g_str_hash(tv->stmt_var) );
}


void stmtexpldr_destroy_val (gpointer data)
{
    if (NULL == data)
        return;

    stmtexp_val_t * val = (stmtexp_val_t*) data;
    
    if (NULL != val->val)
        free (val->val);

    free (val);
}

void stmtexpldr_destroy_key (gpointer data)
{
    if (NULL == data)
        return;

    stmtexp_key_t * key = (stmtexp_key_t*) data;
    
    if (NULL != key)
        free (key);
}



void stmtexpldr_init (stmtexp_tab_t *vex)
{
    vex->ght = g_hash_table_new_full (g_str_hash, stmtexpldr_equal,
                                      stmtexpldr_destroy_key, stmtexpldr_destroy_val);

    vex->expanded = g_string_sized_new (EXPANDED_STR_DEF_LEN);
}


int stmtexpldr_load_config (stmtexp_tab_t *vex, const char* config_file)
{

    int fret;
    config_t cfg;
    config_setting_t *setting;

	config_init(&cfg);

	if (!config_read_file(&cfg, config_file)) 
    {
		fprintf(stderr, "%s:%d - %s\n",
                config_file,
                config_error_line(&cfg),
                config_error_text(&cfg));
		config_destroy(&cfg);

		return -1;
	}


    setting = config_lookup(&cfg, "Variables");
    if(setting != NULL)
    {
        int count = config_setting_length(setting);
        printf("%d configured variables:\n", count);

        int i;
        for(i = 0; i < count; ++i)
        {
            config_setting_t *var = config_setting_get_elem(setting, i);

            const char *var_id, *xpath; 

            if(!(config_setting_lookup_string(var, "var_id",
                                              &var_id)
                 && config_setting_lookup_string(var, "xpath", &xpath))
                )
                continue;

            stmtexp_key_t* sk = calloc (1, sizeof(stmtexp_key_t) );
            if (NULL != sk)
            {
                fprintf(stderr, "  Variable [%d] '%s':'%s'\n", i, 
                        var_id, xpath );

                snprintf (sk->stmt_var, STMTEXPAND_STRLEN_MAX - 1, "%s",
                          var_id);
                
                snprintf (sk->xpath, STMTEXPAND_STRLEN_MAX - 1, "%s", xpath );

                //add index and value
                stmtexp_val_t *sv = calloc (1, sizeof(stmtexp_val_t) );
                g_hash_table_replace (vex->ght, sk, sv);
            } else
            {
                perror ("stmtexpldr_load_config() malloc error\n");
                fret = -1;
                break;
            }


        }
        fret = 0;

    }
    

    config_destroy(&cfg);

    return fret;

}


void stmtexpldr_clear (stmtexp_tab_t *vex)
{
    g_hash_table_remove_all (vex->ght);
}


void stmtexpldr_free (stmtexp_tab_t *vex)
{
    g_hash_table_destroy (vex->ght);
    g_string_free (vex->expanded, TRUE);
}

