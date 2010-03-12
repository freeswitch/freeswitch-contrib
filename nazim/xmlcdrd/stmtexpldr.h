#ifndef STMTEXPLDR_H
#define STMTEXPLDR_H

#include "stmtexptab.h"

#define STMTEXPAND_STRLEN_MAX  512

typedef enum
{
    STMTEXP_DB_DT_VARCHAR = 1,
    STMTEXP_DB_DT_INT
} stmtexp_db_field_dt;

typedef struct
{
    /*stmt_var - variable id inside a statement: $<SOMEVAR> */
    char stmt_var [STMTEXPAND_STRLEN_MAX];

    /*XPATH of value inside the XMLCDR hierarchy*/
    char xpath [STMTEXPAND_STRLEN_MAX];
   
} stmtexp_key_t, *pstmtexp_key_t;

typedef struct
{
    char *val;
} stmtexp_val_t, *pstmtexp_val_t;



void stmtexpldr_init (stmtexp_tab_t *vex);

/* Returns:

    -1 if error occured*/
int stmtexpldr_load_config (stmtexp_tab_t *vex, const char* config_file);

void stmtexpldr_clear (stmtexp_tab_t *vex);

void stmtexpldr_free (stmtexp_tab_t *vex);




#endif
