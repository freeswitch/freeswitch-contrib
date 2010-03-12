#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "plug.h"

/*returns 0 on success, -1 on error*/
int plug_init(const char *filename, const char *options,
              XCDR_PLUGINFO *plug)
{

    char *error;

    if (NULL == (plug->handle = dlopen(filename, RTLD_LAZY | RTLD_GLOBAL)))
    {
        fprintf(stderr, "%s\r\n", dlerror());
        return -1;
    }

    //clear any existing dl error.
    dlerror();
    *(void **) (&plug->plugin_get_name) = dlsym(plug->handle, "plugin_get_name");
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }

    dlerror();
    *(void **) (&plug->plugin_get_description) = dlsym(plug->handle, "plugin_get_description");
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }

    dlerror();
    *(void **) (&plug->plugin_init) = dlsym(plug->handle, "plugin_init");
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }

    dlerror();
    *(void **) (&plug->plugin_free) = dlsym(plug->handle, "plugin_free");
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }

    dlerror();
    *(void **) (&plug->plugin_main) = dlsym(plug->handle, "plugin_main");
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }



    strncpy(plug->name, plug->plugin_get_name(), XCDR_PLUGIN_NAME_LEN_MAX);

    if (-1 == plug->plugin_init(filename, options, &plug->state))
    {
        fprintf(stderr, "Failed to load plugin:'%s' InternalName:'%s' Options:'%s'\n", plug->name, filename, options);
        return -1;
    }


    return 0;
}

int plug_free(XCDR_PLUGINFO *plug)
{

    if (NULL != plug->handle)
    {
        plug->plugin_free(&plug->state);
    }

    return 0;
}


int plug_destroy(XCDR_PLUGINFO *plug)
{

    if (NULL != plug->handle)
    {
        return dlclose(plug->handle);
    }

    return 0;
}

