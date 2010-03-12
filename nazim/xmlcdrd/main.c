#include <fcgi_stdio.h> /* fcgi library; put it first*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "log.h"
#include "urldecode.h"
#include "stmtexp.h"
#include "stmtexpldr.h"
#include "plugldr.h"
#include "plug.h"

#define STR_MAX 512

#define CONFIG "/etc/xmlcdrd/xmlcdr.conf"
#define XURL_CDR "cdr"


typedef struct
{
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtx; 
} stmtexp_cb_data_t;



#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)


stmtexp_tab_t vex;



void stmtexp_callback (stmtexp_key_t *key, stmtexp_val_t *val, void *userdata);
void stmtexp_callback_clean (stmtexp_key_t *key, stmtexp_val_t *val, 
                             void *userdata);

int execute_xpath_expression(const char* buf, int buflen);


int execute_xpath_expression(const char* buf, int buflen)
{
    xmlDocPtr doc;
    stmtexp_cb_data_t cbd;
    memset (&cbd, 0x0, sizeof (stmtexp_cb_data_t));


    /* Load XML document */
    doc = xmlParseMemory (buf, buflen);

    if (doc == NULL)
    {
        fprintf(stderr, "Error: unable to parse memory\n");
        return(-1);
    }

    /* Create xpath evaluation context */
    cbd.xpathCtx = xmlXPathNewContext(doc);
    if(cbd.xpathCtx == NULL)
    {
        fprintf(stderr,"Error: unable to create new XPath context\n");
        xmlFreeDoc(doc); 
        return(-1);
    }

    /*run callback*/
    stmtexp_foreach_callback (&vex, stmtexp_callback, &cbd);

    
    xmlXPathFreeContext(cbd.xpathCtx); 
    xmlFreeDoc(doc); 
    
    return(0);
}


void stmtexp_callback_clean (stmtexp_key_t *key, stmtexp_val_t *val, void *userdata)
{
    if (NULL != val)
    {
        if (NULL != val->val)
        {
            free (val->val);
            val->val = NULL;
        }
    }
}


void stmtexp_callback (stmtexp_key_t *key, stmtexp_val_t *val, void *userdata)
{
    xmlXPathObjectPtr xpathObj;
    xmlNodePtr cur;

    stmtexp_cb_data_t * cbd = (stmtexp_cb_data_t *) userdata;
    


    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression((xmlChar*)key->xpath, cbd->xpathCtx);
    if(xpathObj == NULL)
    {
        fprintf(stderr,
                "Error: unable to evaluate xpath expression \"%s\"\n",
                key->xpath);
        return;
    }

    /* Print results */
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    if (NULL != nodes)
    {
        if (0 < nodes->nodeNr)
        {
            if (nodes->nodeTab[0])
            {
                if(nodes->nodeTab[0]->type == XML_ELEMENT_NODE) 
                {
                    cur = nodes->nodeTab[0];   	    
                    if(cur->ns)
                    { 
                        fprintf(stderr, "= element node \"%s:%s\"\n", 
                                cur->ns->href, cur->name);
                    } else
                    {
                        fprintf(stderr, "= element node \"%s\"\n", 
                                cur->name);
                        xmlChar *content = xmlNodeGetContent(cur);

                        if (NULL != val)
                        {
                            if (NULL != val->val)
                            {
                                free (val->val);
                                val->val = NULL;
                            }
                            val->val = strdup((const char*)content);
                
                            fprintf(stderr, "%s\n", val->val);
                            xmlFree (content);
                        }
                    }        
                }
            }
        } else  /* if (0 < nodes->nodeNr) */
        {
            if (NULL != val)
            {
                if (NULL != val->val)
                {
                    free (val->val);
                    val->val = NULL;
                }
            }
        
        }
    }
    
    /* Cleanup */
    xmlXPathFreeObject(xpathObj);

}




/*
 * 
 */
int main(int argc, char** argv)
{



    XCDR_PLUGTAB plugtab;

    if (-1 == xcdr_plugldr_load_config(&plugtab, CONFIG))
    {
        fprintf (stderr, "Failed to configure plugins from config: '%s'", CONFIG);
        return -1;
    }


    if (-1 == xcdr_plugldr_load_plugins(&plugtab) )
    {
        fprintf (stderr, "Failed to load plugins from config: '%s'", CONFIG);
        xcdr_plugldr_free_plugins (&plugtab);
        return -1;
    }

    if (-1 == stmtexp_load_config (&vex, CONFIG))
    {
        fprintf (stderr, "Failed to load vars from config: '%s'", CONFIG);
        xcdr_plugldr_free_plugins (&plugtab);
        stmtexp_free (&vex);
        return -1;
    }


    /* Init libxml */     
    xmlInitParser();
    LIBXML_TEST_VERSION




    GString * cgi_req_str = g_string_sized_new (102400);
    GString * cgi_decoded_str = g_string_sized_new (102400);
    GString * temp = g_string_sized_new (102400);
        

    while (FCGI_Accept() >= 0)
    {
        //Set length of received request to 0
        int cgi_req_len = 0;
        g_string_set_size (cgi_req_str, 0);


        char c;
        while (EOF != (c = fgetc (stdin)) )
        {
            g_string_append_c (cgi_req_str, c);
            cgi_req_len++;
        } //~while We have received the full request string

            
        //fprintf (stderr, "%s\n", cgi_req_str->str);
        stmtexp_foreach_callback (&vex, stmtexp_callback_clean, NULL);
            
        int decoded_len = cgi_req_len * 3 + 1;
        g_string_set_size (cgi_decoded_str, decoded_len);
        memset (cgi_decoded_str->str, 0x0, decoded_len);


        int status;
        if (NULL !=  urldecode_expand_value (cgi_req_str,
                                             XURL_CDR, strlen(XURL_CDR),
                                             1,
                                             temp, &status,
                                             cgi_decoded_str))
        {
            execute_xpath_expression(cgi_decoded_str->str,
                                     strlen(cgi_decoded_str->str) );

            /*Dump entire dom tree on stdout*/
            //fprintf (stderr, "%s\n", cgi_decoded_str->str);


            int i;
            XCDR_PLUGINFO *plug;
            
            for (i = 0; i < plugtab.tag_count; i ++)
            {
                plug = &plugtab.tags[i].plug;
                
                //If no metrics defined, run anyway
                if (0 == plugtab.tags[i].metrics.tag_count)
                {
                    plug->plugin_main(&vex, &plug->state);
                    continue;
                }

                //match metric regexps
                int j;
                int run = 0;
                for (j = 0; j < plugtab.tags[i].metrics.tag_count; j++)
                {
                    const char *metric_val = 
                        stmtexp_lookup_var (&vex,
                                            plugtab.tags[i].metrics.tags[j].metric);



                    //if metric is not found, replace with an empty string.
                    if (NULL == metric_val)
                    {
                        metric_val = "";
                    }

                    fprintf (stderr, "metric [%s] '%s':'%s'\n", 
                             plugtab.tags[i].metrics.tags[j].metric,
                             plugtab.tags[i].metrics.tags[j].regexp,
                             metric_val);
                             
                             

                    if (-1 < 
                        xcdr_plugldr_match_regexp (
                            plugtab.tags[i].metrics.tags[j].regexp,
                            metric_val, strlen(metric_val)) )
                        run = 1;
                    else run = 0;
                }

                if (0 < run)
                {
                    fprintf (stderr, "metrics matched\n");
                    plug->plugin_main(&vex, &plug->state);
                }
            }
           
        } else fprintf (stderr, "'%s' not found in '%s'\n",
                        XURL_CDR, cgi_req_str->str);
            

    }
        
    g_string_free (cgi_req_str, TRUE);
    g_string_free (cgi_decoded_str, TRUE);
    g_string_free (temp, TRUE);
        
    stmtexp_free (&vex);
    xcdr_plugldr_free_plugins(&plugtab);

    xmlCleanupParser();


    return 0;
}



#else
int main(void) {
    fprintf(stderr, "XPath support not compiled in\n");
    exit(1);
}
#endif
