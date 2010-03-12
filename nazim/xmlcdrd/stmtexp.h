#ifndef STMTEXP_H
#define STMTEXP_H

#include "stmtexptab.h"
#include "stmtexpldr.h"
#include <glib.h>

/*
  Very simple and slow variable expansion. No text escaping is provided.
  No recursion checks, so if your expanded var contains variable definition
  you're in trouble.
*/


/* Callback to modify each key and value */
typedef void  (* stmtexp_foreach_callback_t)  (stmtexp_key_t *key, 
                                               stmtexp_val_t *val,
                                               void *userdata);


/*
  Returns:
     -1 if error occured, any other value on success
*/
int stmtexp_load_config (stmtexp_tab_t *vex, const char* config_file);

void stmtexp_free (stmtexp_tab_t *vex);

const char* stmtexp_lookup_var (stmtexp_tab_t *vex, const char *varid);

void stmtexp_foreach_callback (stmtexp_tab_t *vex,
                               stmtexp_foreach_callback_t c,
                               void *data);

void stmtexp_assign_var (stmtexp_tab_t *vex, 
                            const char *varid, const char *vartext);

/*
  @vex - variables to expand
  Returns:
    pointer to a static buffer which gets overwritten on next call to the func.
    NULL on errror
*/
const char *stmtexp_expand_str (stmtexp_tab_t * vex, 
                         const char *src, int srclen);




#endif


