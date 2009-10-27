/* 
 * Interface to the FastSMS message service.
 * Copyright (c) 2009, Jeremy Tregunna, All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 */
#include <switch.h>
#include <curl/curl.h>

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_fastsms_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_fastsms_runtime);
SWITCH_MODULE_LOAD_FUNCTION(mod_fastsms_load);
SWITCH_MODULE_DEFINITION(mod_fastsms, mod_fastsms_load, mod_fastsms_shutdown, NULL);

static char* companyid;
static char* userid;
static char* password;
static CURL* curl;

static switch_status_t do_config(switch_bool_t reload)
{
	switch_xml_t cfg, xml, settings, auth;

	if (!(xml = switch_xml_open_cfg("fastsms.conf", &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not open fastsms.conf\n");
		return SWITCH_STATUS_FALSE;
	}

	if ((settings = switch_xml_child(cfg, "settings"))) {
		auth = switch_xml_child(settings, "auth");
		companyid = strdup(switch_xml_attr_soft(auth, "companyid"));
		userid = strdup(switch_xml_attr_soft(auth, "userid"));
		password = strdup(switch_xml_attr_soft(auth, "password"));
	}

	if (xml)
		switch_xml_free(xml);

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t fastsms_send_msg(switch_core_session_t* session, const char* msisdn, const char* text, const char* originator)
{
	char* params;
	int error;
	long response = 0;

	if (originator) {
		asprintf(&params, "CompanyId=%s&UserId=%s&Password=%s&Msisdn=%s&MessageText=%s&OasText=%s", companyid,
			userid, password, msisdn, text, originator);
	} else {
		asprintf(&params, "CompanyId=%s&UserId=%s&Password=%s&Msisdn=%s&MessageText=%s", companyid, userid,
			password, msisdn, text);
	}

	curl_easy_setopt(curl, CURLOPT_URL, "https://secure.bayhamsystems.com/mfi/sendMessage");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	if ((error = curl_easy_perform(curl)) != CURLE_OK)
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "FastSMS Error: %s\n", curl_easy_strerror(error));
	curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &response);
	free(params);

	switch (response) {
		case 200:
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "SMS message sent.\n");
			return SWITCH_STATUS_SUCCESS;
			break;
		case 403:
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error: Forbidden.\n");
			break;
		default:
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FastSMS Error: Response code %ld\n", response);
			break;
	}

	return SWITCH_STATUS_FALSE;
}

#define FASTSMS_DESC "FastSMS SMS Message Service"
#define FASTSMS_USAGE "<number> <text message>"

SWITCH_STANDARD_APP(fastsms_function)
{
	switch_channel_t* channel = switch_core_session_get_channel(session);
	char* msisdn = NULL;
	char* text = NULL;
	const char* originator = switch_channel_get_variable(channel, "fastsms_originator");

	if (!zstr(data)) {
		msisdn = switch_core_session_strdup(session, data);
		text = strchr(msisdn, ' ');
		if (text)
			*text++ = '\0';
		fastsms_send_msg(session, msisdn, text, originator);
	}
}

SWITCH_STANDARD_API(fastsms_api_function)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Reloading FastSMS configuration file.\n");
	do_config(SWITCH_TRUE);
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_fastsms_load)
{
	switch_status_t status;
	switch_application_interface_t *app_interface;
	switch_api_interface_t *api_interface;

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "FastSMS module loaded.\n");
	
	if ((status = do_config(SWITCH_FALSE)) != SWITCH_STATUS_SUCCESS) {
		return status;
	}

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);
	curl = curl_easy_init();
	
	SWITCH_ADD_API(api_interface, "fastsms_api", "fastsms_api", fastsms_api_function, "syntax");
	SWITCH_ADD_APP(app_interface, "fastsms", "fastsms", FASTSMS_DESC, fastsms_function, FASTSMS_USAGE, SAF_NONE);

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_fastsms_shutdown)
{
	curl_easy_cleanup(curl);
	free(companyid);
	free(userid);
	free(password);
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
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4
 */
