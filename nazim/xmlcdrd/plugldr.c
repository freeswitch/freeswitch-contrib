#include <stdio.h>
#include <syslog.h>
#include <pcre.h>
#include <string.h>
#include <libconfig.h>
#include "plugcommon.h"
#include "plugldr.h"
#include "log.h"


#define XCDR_FILE_PATH_MAX 1024
#define XCDR_STR_LEN_MAX 1024



int xcdr_plug_ldr_sort_plugins(XCDR_PLUGTAB *plugtab)
{
    unsigned int a, b;
    XCDR_PLUGTAG x;


    for (a = 1; a < plugtab->tag_count; ++a)
    {
        for (b = plugtab->tag_count - 1; b >= a; --b)
        {
            if (plugtab->tags[b - 1].priority > plugtab->tags[b].priority)
            {
                x = plugtab->tags[b - 1];
                plugtab->tags[b - 1] = plugtab->tags[b];
                plugtab->tags[b] = x;
            }
        }
    }

    return 0;
}


int
xcdr_plugldr_load_plugins(XCDR_PLUGTAB *plugtab)
{
    unsigned int i;

    if (NULL == plugtab->tags || 0 >= plugtab->tag_count)
    {
        syslog_msg(LOG_ALERT, 1,
                   "xcdr_plugldr_load_plugins %s:%d No plugins configured",
                   __FILE__, __LINE__);
        return -1;
    }

    for (i = 0; i < plugtab->tag_count; i++)
    {

        if (PLUG_STATUS_ERROR == plug_init(plugtab->tags[i].fname,
                            plugtab->tags[i].options,
                            &plugtab->tags[i].plug))
        {
            syslog_msg(LOG_ALERT, 1,
                       "plugldr_load_plugins %s:%d can't load plugin '%s'",
                       __FILE__, __LINE__, plugtab->tags[i].fname);
            return -1;
        }

        fprintf(stderr, "plugin: '%s' loaded priority:'%d' options:'%s'\n",
                plugtab->tags[i].fname, plugtab->tags[i].priority,
                plugtab->tags[i].options);

    }// ~for (i

    if (1 < plugtab->tag_count)
        xcdr_plug_ldr_sort_plugins(plugtab);


    return 0;
}





int xcdr_plugldr_load_config(XCDR_PLUGTAB *plugtab, const char* config_file)
{

    config_t cfg;
    config_setting_t *setting;

    config_init(&cfg);

    fprintf (stderr, "xcdr_plugldr_load_config. Reading config: '%s'\n", config_file);

	if (!config_read_file(&cfg, config_file)) 
    {
		fprintf(stderr, "%s:%d - %s\n",
                config_file,
                config_error_line(&cfg),
                config_error_text(&cfg));
		config_destroy(&cfg);

		return -1;
	}


    const char *plugin_dir;
    config_lookup_string (&cfg, "plugin_dir", &plugin_dir);

    setting = config_lookup(&cfg, "Plugins");
    if(setting != NULL)
    {
        int count = config_setting_length(setting);
        int i;

        plugtab->tags = NULL;
        plugtab->tag_count = count;

        plugtab->tags = (XCDR_PLUGTAG*) 
            calloc(plugtab->tag_count, sizeof (XCDR_PLUGTAG));
        if (NULL == plugtab->tags)
        {
            config_destroy(&cfg);
            return -1;
        }

        for(i = 0; i < count; ++i)
        {
            config_setting_t *plugin = config_setting_get_elem(setting, i);

            const char *name, *options;
            int priority;

            if(!(config_setting_lookup_string(plugin, "name", &name)
                 && config_setting_lookup_string(plugin, "options", &options)
                 && config_setting_lookup_int(plugin, "priority", &priority)))
                continue;

            fprintf(stderr, "  Plugin [%d] %s:\n", i, name);
            snprintf(plugtab->tags[i].fname, XCDR_PLUGIN_STRMAXLEN - 1,
                     "%s/%s.so", plugin_dir, name);

            snprintf(plugtab->tags[i].options, XCDR_PLUGIN_STRMAXLEN - 1,
                     "%s", options);

            plugtab->tags[i].priority = priority;

            fprintf(stderr, "  %u  plugin = '%s' options = '%s'\n",
                    plugtab->tags[i].priority,
                    plugtab->tags[i].fname,
                    plugtab->tags[i].options);
        }

 
        for(i = 0; i < count; ++i)
        {
            char metric_name [XCDR_STR_LEN_MAX];
            snprintf (metric_name, XCDR_STR_LEN_MAX - 1, "Plugins.[%d].Metrics", i);
            config_setting_t *setting_m = config_lookup(&cfg, metric_name);
            if (NULL == setting_m) 
                continue;


            int metric_count = config_setting_length(setting_m);
                 
            plugtab->tags[i].metrics.tag_count = metric_count;
             
            plugtab->tags[i].metrics.tags = 
                (XCDR_METRTAG*) calloc(metric_count,
                                       sizeof (XCDR_METRTAG));
            
            int j;
            const char *name, *regexp;
            for (j = 0; j < metric_count; j++)
            {
                config_setting_t *metric = config_setting_get_elem(setting_m, j);

                if(!(config_setting_lookup_string(metric, "metric", &name)
                     && config_setting_lookup_string(metric, "regexp", &regexp)))
                    continue;

                snprintf (plugtab->tags[i].metrics.tags[j].metric,
                          XCDR_METRIC_ID_MAX,
                          "%s",
                          name);

                snprintf (plugtab->tags[i].metrics.tags[j].regexp,
                          XCDR_METRIC_REGEXP_MAX,
                          "%s",
                          regexp);

                fprintf(stderr, "  Loaded metric %s:%s:%d [%s] '%s'\n", 
                        plugtab->tags[i].fname,
                        plugtab->tags[i].options,
                        plugtab->tags[i].priority,
                        name, regexp);
            }
        }
    }

    config_destroy(&cfg);

    return 0;
}



void
xcdr_plugldr_free_plugins(XCDR_PLUGTAB *plugtab)
{
    unsigned int i;

    if (NULL == plugtab) return;

    for (i = 0; i < plugtab->tag_count; i++)
        if (NULL != plugtab->tags[i].fname)
        {
            plug_free(&plugtab->tags[i].plug);
            free(plugtab->tags[i].metrics.tags);

            fprintf(stderr, "plugldr_free_plugins %s\n", plugtab->tags[i].fname);
        }

    if (NULL != plugtab->tags)
    {
        free(plugtab->tags);
        plugtab->tag_count = 0;
    }

    plugtab->tags = NULL;

    fprintf(stderr, "plugldr_free_plugins done\r\n");

    return;
}

/*
        Returns non -negative value on success
    Negative value on failure
 */
int xcdr_plugldr_match_regexp(const char *regexp,
                              const char *text, int text_len)
{
    pcre *re;
    const char *error;
    int ovector[30];
    int erroffset;
    int res;

    re = pcre_compile(regexp, 0, &error, &erroffset, NULL);

    res = pcre_exec(re, NULL, text, text_len, 0, 0, ovector, 30);

    pcre_free (re);

    return res;
}
