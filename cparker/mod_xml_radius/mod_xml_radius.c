/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2009, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Anthony Minessale II <anthm@freeswitch.org>
 * Chris Parker <cparker@segv.org>
 *
 * mod_xml_radius.c -- use radius to realtime create xml directory, dialplan bindings 
 *
 */
#include <switch.h>
#include <sys/stat.h>
#include <freeradius-client.h>
#include "mod_xml_radius.h"

typedef enum {
        XML_RADIUS_DIRECTORY = 0,
        XML_RADIUS_DIALPLAN = 0,
} xml_radius_query_type_t;

#define XML_RADIUS_SYNTAX "[debug_on|debug_off]"

static char cf[] = "mod_xml_radius.conf";
static char my_dictionary[PATH_MAX];
static char my_seqfile[PATH_MAX];
static char *my_deadtime;       /* 0 */
static char *my_timeout;        /* 5 */
static char *my_retries;        /* 3 */
static char my_servers[SERVER_MAX][255];

static struct {
    int shutdown;
	switch_memory_pool_t *pool;
	switch_mutex_t *mutex;
    switch_thread_rwlock_t *rwlock;
	switch_xml_t templates_tag;
} globals;

SWITCH_MODULE_LOAD_FUNCTION(mod_xml_radius_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_xml_radius_shutdown);
SWITCH_MODULE_DEFINITION(mod_xml_radius, mod_xml_radius_load, mod_xml_radius_shutdown, NULL);

static switch_bool_t debug = SWITCH_FALSE;

#if 0
static int logi = 0;
static void logger(char *str)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG[%i] [%s]\n", ++logi, str);
}
#endif


static rc_handle *my_radius_init(void)
{
    int i = 0;
    rc_handle *rad_config;

    rad_config = rc_new();

    if (rad_config == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Error initializing rc_handle!\n");
        return NULL;
    }

    rad_config = rc_config_init(rad_config);

    if (rad_config == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "error initializing radius config!\n");
        rc_destroy(rad_config);
        return NULL;
    }

    /* Some hardcoded ( for now ) defaults needed to initialize radius */
    if (rc_add_config(rad_config, "auth_order", "radius", "mod_xml_radius.c", 0) != 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "setting auth_order = radius failed\n");
        rc_destroy(rad_config);
        return NULL;
    }

    if (rc_add_config(rad_config, "seqfile", my_seqfile, "mod_xml_radius.c", 0) != 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "setting seqfile = %s failed\n", my_seqfile);
        rc_destroy(rad_config);
        return NULL;
    }


    /* Add the module configs to initialize rad_config */

    for (i = 0; i < SERVER_MAX && my_servers[i][0] != '\0'; i++) {
        if (rc_add_config(rad_config, "authserver", my_servers[i], cf, 0) != 0) {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "setting authserver = %s failed\n", my_servers[i]);
            rc_destroy(rad_config);
            return NULL;
        }
    }

    if (rc_add_config(rad_config, "dictionary", my_dictionary, cf, 0) != 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "failed setting dictionary = %s failed\n", my_dictionary);
        rc_destroy(rad_config);
        return NULL;
    }

    if (rc_add_config(rad_config, "radius_deadtime", my_deadtime, cf, 0) != 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "setting radius_deadtime = %s failed\n", my_deadtime);
        rc_destroy(rad_config);
        return NULL;
    }

    if (rc_add_config(rad_config, "radius_timeout", my_timeout, cf, 0) != 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "setting radius_timeout = %s failed\n", my_timeout);
        rc_destroy(rad_config);
        return NULL;
    }

    if (rc_add_config(rad_config, "radius_retries", my_retries, cf, 0) != 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "setting radius_retries = %s failed\n", my_retries);
        rc_destroy(rad_config);
        return NULL;
    }

    /* Read the dictionary file(s) */
    if (rc_read_dictionary(rad_config, rc_conf_str(rad_config, "dictionary")) != 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "reading dictionary file(s): %s\n", my_dictionary);
        rc_destroy(rad_config);
        return NULL;
    }

    return rad_config;
}


SWITCH_STANDARD_API(xml_radius_function)
{
	if (session) {
		return SWITCH_STATUS_FALSE;
	}

	if (switch_strlen_zero(cmd)) {
		goto usage;
	}

	if (!strcasecmp(cmd, "debug_on")) {
		debug = SWITCH_TRUE;
	} else if (!strcasecmp(cmd, "debug_off")) {
		debug = SWITCH_FALSE;
	} else {
		goto usage;
	}

	stream->write_function(stream, "OK\n");
	return SWITCH_STATUS_SUCCESS;

  usage:
	stream->write_function(stream, "USAGE: %s\n", XML_RADIUS_SYNTAX);
	return SWITCH_STATUS_SUCCESS;
}


typedef struct xml_binding {
	char *bindings;
} xml_binding_t;


static switch_status_t xml_radius_expand_tag(switch_xml_t xml_in, switch_event_t *params, switch_xml_t xml_out, int *off)
{
    switch_xml_t xml_in_tmp = NULL;
    int i;

	if (switch_strlen_zero(xml_out->name)) {
		xml_out->name = strdup(xml_in->name);
	} else if (!(xml_out = switch_xml_add_child_d(xml_out, xml_in->name, *off++))) {
        return SWITCH_STATUS_FALSE;
    }
    for (i = 0; xml_in->attr[i]; i+=2) {
        char *tmp_attr;
        tmp_attr = switch_event_expand_headers(params, xml_in->attr[i+1]);
            switch_xml_set_attr(xml_out, xml_in->attr[i], tmp_attr);
    }
	for (xml_in_tmp = xml_in->child; xml_in_tmp; xml_in_tmp = xml_in_tmp->ordered) {
        xml_radius_expand_tag(xml_in_tmp, params, xml_out, off);
    }
	return SWITCH_STATUS_SUCCESS;
}
	

static switch_status_t xml_radius_expand_template(char *template_name, switch_event_t *params, switch_xml_t xml_out, int *off)
{
    switch_xml_t template_tag = NULL, sub_tag = NULL;

    for (template_tag = switch_xml_child(globals.templates_tag, "template"); template_tag; template_tag = template_tag->next) {
        char *template_tag_name = (char*) switch_xml_attr_soft(template_tag, "name");
        if (!strcmp(template_tag_name, template_name)) {
			for (sub_tag = template_tag->child; sub_tag; sub_tag = sub_tag->ordered) {
		        xml_radius_expand_tag(sub_tag, params, xml_out, off);
	        }
            return SWITCH_STATUS_SUCCESS;
        }
    }
    return SWITCH_STATUS_FALSE;
}



static switch_xml_t xml_radius_search(const char *section, const char *tag_name, const char *key_name, const char *key_value, switch_event_t *params, void *user_data)
{
	xml_binding_t *binding = (xml_binding_t *) user_data;
	switch_event_header_t *hi;
	xml_radius_query_type_t query_type;
	switch_event_t *rad_event;
	rc_handle *rad_config;
    VALUE_PAIR *send = NULL;
	VALUE_PAIR *received = NULL;
	VALUE_PAIR *vp = NULL;
	char msg[4096];
    uint32_t client_port = 0;
	char *uuid_str, *user_name, *digest_response, *digest_realm, *digest_nonce, *digest_cnonce, 
         *digest_method, *digest_uri, *digest_algorithm, *digest_username, *digest_qop, *digest_nc ;

	switch_xml_t xml_out = NULL;
	if ((xml_out = switch_xml_new(""))) {
		//switch_xml_set_attr_d(xml_out, "document type", "freeswitch/xml");
	}

	/*int off = 0; */
	int off = 0, ret = 1;

	if (debug) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "DEBUG in xml_radius_search Section: %s, Tag: %s, Key: %s, Value: %s\n", section, tag_name, key_name, key_value);
	}

	if (!binding) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No bindings... sorry bud returning now\n");
		return NULL;
	}
	if (!strcmp(section, "directory")) {
		query_type = XML_RADIUS_DIRECTORY;
	} else if (!strcmp(section, "dialplan")) {
		query_type = XML_RADIUS_DIALPLAN;
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid section: %s\n", section);
		return NULL;
	}

	if (params) {
		if ((hi = params->headers)) {
			for (; hi; hi = hi->next) {
				if (debug == SWITCH_TRUE) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG in xml_radius_search, header [%s]=[%s]\n", hi->name, hi->value);
				}
			}
			//xml_odbc_render_template(globals.template, params, xml_out, &off); // TODO globals.template should be replace with something specific for this section
		}
	} else {
		goto cleanup;
	}

	uuid_str = switch_event_get_header(params, "Core-UUID");
	digest_response = switch_event_get_header(params, "sip_auth_response");
	digest_realm = switch_event_get_header(params, "sip_auth_realm");
	digest_nonce = switch_event_get_header(params, "sip_auth_nonce");
	digest_cnonce = switch_event_get_header(params, "sip_auth_cnonce");
	digest_method = switch_event_get_header(params, "sip_auth_method");
    digest_uri = switch_event_get_header(params, "sip_auth_uri");
    digest_algorithm = switch_event_get_header(params, "sip_auth_algorithm");
    digest_username = switch_event_get_header(params, "sip_auth_username");
	digest_qop = switch_event_get_header(params, "sip_auth_qop");
	digest_nc = switch_event_get_header(params, "sip_auth_nc");

	if(digest_algorithm == NULL) {
		digest_algorithm = switch_mprintf("MD5");
	} 

	if( (uuid_str == NULL) || 
        (digest_response == NULL) ||
		(digest_realm == NULL) ||
		(digest_nonce == NULL) ||
		(digest_method == NULL) ||
		(digest_uri == NULL) ||
		(digest_algorithm == NULL) ||
		(digest_username == NULL)
	  ) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "[mod_xml_radius] Insufficient headers to perform digest auth\n");
		/* switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "%s:%s:%s:%s:%s:%s:%s:%s", uuid_str, digest_response, digest_realm, digest_nonce, digest_method, digest_uri, digest_algorithm, digest_username); */
		return NULL;
	}


    user_name = switch_mprintf("%s@%s", digest_username, digest_realm);

	/* Initialize RADIUS and add attributes to the AUTH REQUEST packet */

	if(switch_event_dup(&rad_event, params) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed creating radius event.\n");
	    return NULL;
	}
	
    switch_thread_rwlock_rdlock(globals.rwlock);

    rad_config = my_radius_init();

	if (rad_config == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Error initializing radius\n");
        goto cleanup;
    }

	if (rc_avpair_add(rad_config, &send, PW_ACCT_SESSION_ID, uuid_str, -1, 0) == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Acct-Session-ID: %s\n", uuid_str);
        rc_destroy(rad_config);
        goto cleanup;
    }

	if (rc_avpair_add(rad_config, &send, PW_USER_NAME, user_name, -1, 0) == NULL) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding User-Name: %s\n", user_name);
		rc_destroy(rad_config);
		goto cleanup;
	}

    if (rc_avpair_add(rad_config, &send, PW_DIGEST_RESPONSE, digest_response, -1, 0) == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest Response: %s\n", digest_response);
        rc_destroy(rad_config);
        goto cleanup;
    }

    if (rc_avpair_add(rad_config, &send, PW_DIGEST_REALM, digest_realm, -1, 0) == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest Realm: %s\n", digest_realm);
        rc_destroy(rad_config);
        goto cleanup;
    }

    if (rc_avpair_add(rad_config, &send, PW_DIGEST_NONCE, digest_nonce, -1, 0) == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest Nonce: %s\n", digest_nonce);
        rc_destroy(rad_config);
        goto cleanup;
    }

	if(digest_cnonce != NULL) {
		if (rc_avpair_add(rad_config, &send, PW_DIGEST_CNONCE, digest_cnonce, -1, 0) == NULL) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest CNonce: %s\n", digest_cnonce);
			rc_destroy(rad_config);
			goto cleanup;
		}
	} else {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Missing Digest CNonce\n");
    }

    if (rc_avpair_add(rad_config, &send, PW_DIGEST_METHOD, digest_method, -1, 0) == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest Method: %s\n", digest_method);
        rc_destroy(rad_config);
        goto cleanup;
    }

    if (rc_avpair_add(rad_config, &send, PW_DIGEST_URI, digest_uri, -1, 0) == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest URI: %s\n", digest_uri);
        rc_destroy(rad_config);
        goto cleanup;
    }

    if (rc_avpair_add(rad_config, &send, PW_DIGEST_ALGORITHM, digest_algorithm, -1, 0) == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest Algorithm: %s\n", digest_algorithm);
        rc_destroy(rad_config);
        goto cleanup;
    }

    if (rc_avpair_add(rad_config, &send, PW_DIGEST_USER_NAME, digest_username, -1, 0) == NULL) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest Username: %s\n", digest_username);
        rc_destroy(rad_config);
        goto cleanup;
    }

	if(digest_qop != NULL) {
		if (rc_avpair_add(rad_config, &send, PW_DIGEST_QOP, digest_qop, -1, 0) == NULL) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest QOP: %s\n", digest_qop);
			rc_destroy(rad_config);
			goto cleanup;
		}
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Missing Digest QOP.\n");
	}

	
    if(digest_nc != NULL) {
        if (rc_avpair_add(rad_config, &send, PW_DIGEST_NONCE_COUNT, digest_nc, -1, 0) == NULL) {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed adding Digest Nonce Counce: %s\n", digest_nc);
            rc_destroy(rad_config);
            goto cleanup;
        }
    } else {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Missing Nonce Count.\n");
    }

	/* SEND IT! */

    if(rc_auth(rad_config, client_port, send, &received, msg) == OK_RC) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Auth Success.\n");

		char name[2048], value[2048]; /* more than enough for radius attribute */
		char *split[2];	
		/* walk the received radius value-pairs and print them out */
		vp = received;
		while(vp) {
			strncpy(name, "radius_", 7);
			rc_avpair_tostr(rad_config, vp, name+7, sizeof(name)-7, value, sizeof(value));	
			if(debug == SWITCH_TRUE) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "[mod_xml_radius] Received AVP: %s = %s\n", name, value);
			}
			/* if name = Freeradius-AVPair then value contains a 'name="value"' string we need to parse out */
			if(strcmp(name,"Freeradius-AVPair")) {
				if(switch_separate_string(value,'=',split, (sizeof(split) / sizeof(split[0]))) == 2) {
					strncpy(name+7, split[0], sizeof(name)-7);
					strncpy(value, split[1], sizeof(value));
				}
			}
			if(switch_event_add_header_string(rad_event, SWITCH_STACK_BOTTOM, name, value)	!= SWITCH_STATUS_SUCCESS) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Failed to add header: %s = %s\n", name, value);
			}
			vp = vp->next;

		}

		if(query_type == XML_RADIUS_DIRECTORY) {
			xml_radius_expand_template("directory", rad_event, xml_out, &off);
		} else if(query_type == XML_RADIUS_DIALPLAN) {
			xml_radius_expand_template("dialplan", rad_event, xml_out, &off);
		}

	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[mod_xml_radius] Auth Rejected!\n");

		/* TO DO: Generate the Failure XML Here fromt the Reply Attributes */

	    /* Return "NOT FOUND" template */

		xml_radius_expand_template("not_found", rad_event, xml_out, &off);	

    }

	if ((hi = rad_event->headers)) {
        for (; hi; hi = hi->next) {
            if (debug == SWITCH_TRUE) {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG in xml_radius_search, new haeader [%s]=[%s]\n", hi->name, hi->value);
            }
        }
    }

    rc_destroy(rad_config);
    rc_avpair_free(send);
	rc_avpair_free(received);

	/* End of RADIUS bits */

	if (debug == SWITCH_TRUE) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Debug dump of XML generated:\n%s", switch_xml_toxml(xml_out, SWITCH_FALSE));
	}

	ret = 0;

  cleanup:

	switch_event_destroy(&rad_event);
	switch_thread_rwlock_unlock(globals.rwlock);

	if (ret) {
		switch_xml_free(xml_out);
		return NULL;
	}
	
	return xml_out;
}


static switch_status_t load_config(void)
{
    switch_xml_t cfg, xml, bindings_tag, binding_tag, templates_tag, settings, param;
    xml_binding_t *binding = NULL;

	switch_status_t status = SWITCH_STATUS_SUCCESS;

    int num_servers = 0;
    int i = 0;

    my_timeout = "5";
    my_retries = "3";
    my_deadtime = "0";
    strncpy(my_seqfile, "/var/run/radius_auth.seq", PATH_MAX - 1);
    strncpy(my_dictionary, "/usr/local/freeswitch/conf/radius/dictionary", PATH_MAX - 1);

    for (i = 0; i < SERVER_MAX; i++) {
        my_servers[i][0] = '\0';
    }

    if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", cf);
        return SWITCH_STATUS_TERM;
    }


    if (!(bindings_tag = switch_xml_child(cfg, "bindings"))) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <bindings> tag!\n");
		status = SWITCH_STATUS_TERM;
        goto done;
    } 

    if (!(templates_tag = switch_xml_child(cfg, "templates"))) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <templates> tag!\n");
        status = SWITCH_STATUS_TERM;
        goto done;
    }

	globals.templates_tag = templates_tag;

	for (binding_tag = switch_xml_child(bindings_tag, "binding"); binding_tag; binding_tag = binding_tag->next) {
        char *bname = (char*) switch_xml_attr_soft(binding_tag, "name");
		if (!(binding = malloc(sizeof(*binding)))) {
			goto done;
		}
		memset(binding, 0, sizeof(*binding));

		binding->bindings = strdup(bname);

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Binding [%s] XML RADIUS Fetch Function [%s]\n",
			switch_strlen_zero(bname) ? "N/A" : bname, binding->bindings ? binding->bindings : "all");
		switch_xml_bind_search_function(xml_radius_search, switch_xml_parse_section_string(bname), binding);
		binding = NULL;
	}

    if ((settings = switch_xml_child(cfg, "settings"))) {
        for (param = switch_xml_child(settings, "param"); param; param = param->next) {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            if (!strcmp(var, "authserver")) {
                if (num_servers < SERVER_MAX) {
                    strncpy(my_servers[num_servers], val, 255 - 1);
                    num_servers++;
                } else {
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
                                      "you can only specify %d radius servers, ignoring excess server entry\n", SERVER_MAX);
                }
            } else if (!strcmp(var, "dictionary")) {
                strncpy(my_dictionary, val, PATH_MAX - 1);
            } else if (!strcmp(var, "seqfile")) {
                strncpy(my_seqfile, val, PATH_MAX - 1);
            } else if (!strcmp(var, "radius_timeout")) {
                my_timeout = strdup(val);
            } else if (!strcmp(var, "radius_retries")) {
                my_retries = strdup(val);
            } else if (!strcmp(var, "radius_deadtime")) {
                my_deadtime = strdup(val);
            }
        }
    }
    switch_xml_free(xml);

    if (num_servers < 1) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "you must specify at least 1 radius server\n");
        return SWITCH_STATUS_TERM;
    }

  done:

    return status;
}


SWITCH_MODULE_LOAD_FUNCTION(mod_xml_radius_load)
{

    switch_core_new_memory_pool(&globals.pool);
    switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, globals.pool);

    switch_api_interface_t *xml_radius_api_interface;

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "XML RADIUS module loading...\n");

    *module_interface = switch_loadable_module_create_module_interface(pool, modname);

    SWITCH_ADD_API(xml_radius_api_interface, "xml_radius", "XML RADIUS", xml_radius_function, XML_RADIUS_SYNTAX);
    switch_console_set_complete("add xml_radius debug_on");
    switch_console_set_complete("add xml_radius debug_off");
    //switch_console_set_complete("add xml_odbc render_template");

    switch_thread_rwlock_create(&globals.rwlock, pool);

    if (load_config() != SWITCH_STATUS_SUCCESS) {
        return SWITCH_STATUS_TERM;
    }

    /* indicate that the module should continue to be loaded */
    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_xml_radius_shutdown)
{

    globals.shutdown = 1;
    switch_thread_rwlock_wrlock(globals.rwlock);
    switch_thread_rwlock_unlock(globals.rwlock);

	return SWITCH_STATUS_SUCCESS;
}


/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
* vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */
