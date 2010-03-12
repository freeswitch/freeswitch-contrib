#ifndef STMTEXPTAB_H
#define STMTEXPTAB_H

#include <glib.h>

typedef struct
{
    GHashTable *ght;
    GString *expanded;
} stmtexp_tab_t, *pstmtexp_tab_t, STMTEXP_TAB, *PSTMTEXP_TAB;



#endif
