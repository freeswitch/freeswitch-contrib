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
 * Joao Mesquita <mesquita@khomp.com.br>
 *
 *
 * mod_khomp.c -- Khomp board Endpoint Module
 *
 */

#define KHOMP_SYNTAX "khomp show [info|links]"

/* Our includes */
#include "k3lapi.hpp"

extern "C"
{
    #include <switch.h>
    #include "k3l.h"
}

SWITCH_MODULE_LOAD_FUNCTION(mod_khomp_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_khomp_shutdown);
//SWITCH_MODULE_RUNTIME_FUNCTION(mod_khomp_runtime);
SWITCH_MODULE_DEFINITION(mod_khomp, mod_khomp_load, mod_khomp_shutdown, NULL);	//mod_khomp_runtime);


switch_endpoint_interface_t *khomp_endpoint_interface;
static switch_memory_pool_t *module_pool = NULL;
static int running = 1;

static K3LAPI * k3l;


typedef enum {
	TFLAG_IO = (1 << 0),
	TFLAG_INBOUND = (1 << 1),
	TFLAG_OUTBOUND = (1 << 2),
	TFLAG_DTMF = (1 << 3),
	TFLAG_VOICE = (1 << 4),
	TFLAG_HANGUP = (1 << 5),
	TFLAG_LINEAR = (1 << 6),
	TFLAG_CODEC = (1 << 7),
	TFLAG_BREAK = (1 << 8)
} TFLAGS;

typedef enum {
	GFLAG_MY_CODEC_PREFS = (1 << 0)
} GFLAGS;


static struct {
	int debug;
	char *ip;
	int port;
	char *dialplan;
	char *codec_string;
	char *codec_order[SWITCH_MAX_CODECS];
	int codec_order_last;
	char *codec_rates_string;
	char *codec_rates[SWITCH_MAX_CODECS];
	int codec_rates_last;
	unsigned int flags;
	int calls;
	switch_mutex_t *mutex;
} globals;

struct private_object {
	unsigned int flags;
	switch_codec_t read_codec;
	switch_codec_t write_codec;
	switch_frame_t read_frame;
	unsigned char databuf[SWITCH_RECOMMENDED_BUFFER_SIZE];
	switch_core_session_t *session;
	switch_caller_profile_t *caller_profile;
	switch_mutex_t *mutex;
	switch_mutex_t *flag_mutex;
	//switch_thread_cond_t *cond;
};

typedef struct private_object private_t;


SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_dialplan, globals.dialplan);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_codec_string, globals.codec_string);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_codec_rates_string, globals.codec_rates_string);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_ip, globals.ip);

/* Macros to define specific API functions */
SWITCH_STANDARD_API(khomp);


static switch_status_t channel_on_init(switch_core_session_t *session);
static switch_status_t channel_on_hangup(switch_core_session_t *session);
static switch_status_t channel_on_routing(switch_core_session_t *session);
static switch_status_t channel_on_exchange_media(switch_core_session_t *session);
static switch_status_t channel_on_soft_execute(switch_core_session_t *session);
static switch_call_cause_t channel_outgoing_channel(switch_core_session_t *session, switch_event_t *var_event,
													switch_caller_profile_t *outbound_profile,
													switch_core_session_t **new_session, switch_memory_pool_t **pool, switch_originate_flag_t flags);
static switch_status_t channel_read_frame(switch_core_session_t *session, switch_frame_t **frame, switch_io_flag_t flags, int stream_id);
static switch_status_t channel_write_frame(switch_core_session_t *session, switch_frame_t *frame, switch_io_flag_t flags, int stream_id);
static switch_status_t channel_kill_channel(switch_core_session_t *session, int sig);

/* My function prototypes */
static void printBoardsInfo(switch_stream_handle_t*);
static const char* linkStatus(unsigned int device, unsigned int link);



static void tech_init(private_t *tech_pvt, switch_core_session_t *session)
{
	tech_pvt->read_frame.data = tech_pvt->databuf;
	tech_pvt->read_frame.buflen = sizeof(tech_pvt->databuf);
	switch_mutex_init(&tech_pvt->mutex, SWITCH_MUTEX_NESTED, switch_core_session_get_pool(session));
	switch_mutex_init(&tech_pvt->flag_mutex, SWITCH_MUTEX_NESTED, switch_core_session_get_pool(session));
	switch_core_session_set_private(session, tech_pvt);
	tech_pvt->session = session;
}

/* 
   State methods they get called when the state changes to the specific state 
   returning SWITCH_STATUS_SUCCESS tells the core to execute the standard state method next
   so if you fully implement the state you can return SWITCH_STATUS_FALSE to skip it.
*/
static switch_status_t channel_on_init(switch_core_session_t *session)
{
	switch_channel_t *channel;
	private_t *tech_pvt = NULL;

	tech_pvt = static_cast< private_t* >(switch_core_session_get_private(session));
	assert(tech_pvt != NULL);

	channel = switch_core_session_get_channel(session);
	assert(channel != NULL);
	switch_set_flag_locked(tech_pvt, TFLAG_IO);

	/* Move channel's state machine to ROUTING. This means the call is trying
	   to get from the initial start where the call because, to the point
	   where a destination has been identified. If the channel is simply
	   left in the initial state, nothing will happen. */
	switch_channel_set_state(channel, CS_ROUTING);
	switch_mutex_lock(globals.mutex);
	globals.calls++;
	switch_mutex_unlock(globals.mutex);

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t channel_on_routing(switch_core_session_t *session)
{
	switch_channel_t *channel = NULL;
	private_t *tech_pvt = NULL;

	channel = switch_core_session_get_channel(session);
	assert(channel != NULL);

	tech_pvt = static_cast<private_t *>(switch_core_session_get_private(session));
	assert(tech_pvt != NULL);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%s CHANNEL ROUTING\n", switch_channel_get_name(channel));

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t channel_on_execute(switch_core_session_t *session)
{

	switch_channel_t *channel = NULL;
	private_t *tech_pvt = NULL;

	channel = switch_core_session_get_channel(session);
	assert(channel != NULL);

	tech_pvt = static_cast<private_t*>(switch_core_session_get_private(session));
	assert(tech_pvt != NULL);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%s CHANNEL EXECUTE\n", switch_channel_get_name(channel));


	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t channel_on_hangup(switch_core_session_t *session)
{
	switch_channel_t *channel = NULL;
	private_t *tech_pvt = NULL;

	channel = switch_core_session_get_channel(session);
	assert(channel != NULL);

	tech_pvt = static_cast<private_t*>(switch_core_session_get_private(session));
	assert(tech_pvt != NULL);

	switch_clear_flag_locked(tech_pvt, TFLAG_IO);
	switch_clear_flag_locked(tech_pvt, TFLAG_VOICE);
	//switch_thread_cond_signal(tech_pvt->cond);

	if (tech_pvt->read_codec.implementation) {
		switch_core_codec_destroy(&tech_pvt->read_codec);
	}

	if (tech_pvt->write_codec.implementation) {
		switch_core_codec_destroy(&tech_pvt->write_codec);
	}


	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%s CHANNEL HANGUP\n", switch_channel_get_name(channel));
	switch_mutex_lock(globals.mutex);
	globals.calls--;
	if (globals.calls < 0) {
		globals.calls = 0;
	}
	switch_mutex_unlock(globals.mutex);

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t channel_kill_channel(switch_core_session_t *session, int sig)
{
	switch_channel_t *channel = NULL;
	private_t *tech_pvt = NULL;

	channel = switch_core_session_get_channel(session);
	assert(channel != NULL);

	tech_pvt = static_cast<private_t*>(switch_core_session_get_private(session));
	assert(tech_pvt != NULL);

	switch (sig) {
	case SWITCH_SIG_KILL:
		switch_clear_flag_locked(tech_pvt, TFLAG_IO);
		switch_clear_flag_locked(tech_pvt, TFLAG_VOICE);
		switch_channel_hangup(channel, SWITCH_CAUSE_NORMAL_CLEARING);
		//switch_thread_cond_signal(tech_pvt->cond);
		break;
	case SWITCH_SIG_BREAK:
		switch_set_flag_locked(tech_pvt, TFLAG_BREAK);
		break;
	default:
		break;
	}

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t channel_on_exchange_media(switch_core_session_t *session)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "CHANNEL LOOPBACK\n");
	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t channel_on_soft_execute(switch_core_session_t *session)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "CHANNEL TRANSMIT\n");
	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t channel_send_dtmf(switch_core_session_t *session, const switch_dtmf_t *dtmf)
{
	private_t *tech_pvt = static_cast<private_t*>(switch_core_session_get_private(session));
	switch_assert(tech_pvt != NULL);

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t channel_read_frame(switch_core_session_t *session, switch_frame_t **frame, switch_io_flag_t flags, int stream_id)
{
	switch_channel_t *channel = NULL;
	private_t *tech_pvt = NULL;
	//switch_time_t started = switch_time_now();
	//unsigned int elapsed;
	switch_byte_t *data;

	channel = switch_core_session_get_channel(session);
	assert(channel != NULL);

	tech_pvt = static_cast<private_t*>(switch_core_session_get_private(session));
	assert(tech_pvt != NULL);
	tech_pvt->read_frame.flags = SFF_NONE;
	*frame = NULL;

	while (switch_test_flag(tech_pvt, TFLAG_IO)) {

		if (switch_test_flag(tech_pvt, TFLAG_BREAK)) {
			switch_clear_flag(tech_pvt, TFLAG_BREAK);
			goto cng;
		}

		if (!switch_test_flag(tech_pvt, TFLAG_IO)) {
			return SWITCH_STATUS_FALSE;
		}

		if (switch_test_flag(tech_pvt, TFLAG_IO) && switch_test_flag(tech_pvt, TFLAG_VOICE)) {
			switch_clear_flag_locked(tech_pvt, TFLAG_VOICE);
			if (!tech_pvt->read_frame.datalen) {
				continue;
			}
			*frame = &tech_pvt->read_frame;
#ifdef BIGENDIAN
			if (switch_test_flag(tech_pvt, TFLAG_LINEAR)) {
				switch_swap_linear((*frame)->data, (int) (*frame)->datalen / 2);
			}
#endif
			return SWITCH_STATUS_SUCCESS;
		}

		switch_cond_next();
	}


	return SWITCH_STATUS_FALSE;

  cng:
	data = (switch_byte_t *) tech_pvt->read_frame.data;
	data[0] = 65;
	data[1] = 0;
	tech_pvt->read_frame.datalen = 2;
	tech_pvt->read_frame.flags = SFF_CNG;
	*frame = &tech_pvt->read_frame;
	return SWITCH_STATUS_SUCCESS;

}

static switch_status_t channel_write_frame(switch_core_session_t *session, switch_frame_t *frame, switch_io_flag_t flags, int stream_id)
{
	switch_channel_t *channel = NULL;
	private_t *tech_pvt = NULL;
	//switch_frame_t *pframe;

	channel = switch_core_session_get_channel(session);
	assert(channel != NULL);

	tech_pvt = static_cast<private_t*>(switch_core_session_get_private(session));
	assert(tech_pvt != NULL);

	if (!switch_test_flag(tech_pvt, TFLAG_IO)) {
		return SWITCH_STATUS_FALSE;
	}
#ifdef BIGENDIAN
	if (switch_test_flag(tech_pvt, TFLAG_LINEAR)) {
		switch_swap_linear(frame->data, (int) frame->datalen / 2);
	}
#endif


	return SWITCH_STATUS_SUCCESS;

}

static switch_status_t channel_answer_channel(switch_core_session_t *session)
{
	private_t *tech_pvt;
	switch_channel_t *channel = NULL;

	channel = switch_core_session_get_channel(session);
	assert(channel != NULL);

	tech_pvt = static_cast<private_t*>(switch_core_session_get_private(session));
	assert(tech_pvt != NULL);


	return SWITCH_STATUS_SUCCESS;
}


static switch_status_t channel_receive_message(switch_core_session_t *session, switch_core_session_message_t *msg)
{
	switch_channel_t *channel;
	private_t *tech_pvt;

	channel = switch_core_session_get_channel(session);
	assert(channel != NULL);

	tech_pvt = (private_t *) switch_core_session_get_private(session);
	assert(tech_pvt != NULL);

	switch (msg->message_id) {
	case SWITCH_MESSAGE_INDICATE_ANSWER:
		{
			channel_answer_channel(session);
		}
		break;
	default:
		break;
	}

	return SWITCH_STATUS_SUCCESS;
}

/* Make sure when you have 2 sessions in the same scope that you pass the appropriate one to the routines
   that allocate memory or you will have 1 channel with memory allocated from another channel's pool!
*/
static switch_call_cause_t channel_outgoing_channel(switch_core_session_t *session, switch_event_t *var_event,
													switch_caller_profile_t *outbound_profile,
													switch_core_session_t **new_session, switch_memory_pool_t **pool, switch_originate_flag_t flags)
{
	if ((*new_session = switch_core_session_request(khomp_endpoint_interface, SWITCH_CALL_DIRECTION_OUTBOUND, pool)) != 0) {
		private_t *tech_pvt;
		switch_channel_t *channel;
		switch_caller_profile_t *caller_profile;

		switch_core_session_add_stream(*new_session, NULL);
		if ((tech_pvt = (private_t *) switch_core_session_alloc(*new_session, sizeof(private_t))) != 0) {
			channel = switch_core_session_get_channel(*new_session);
			tech_init(tech_pvt, *new_session);
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Hey where is my memory pool?\n");
			switch_core_session_destroy(new_session);
			return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
		}

		if (outbound_profile) {
			char name[128];

			snprintf(name, sizeof(name), "Khomp/%s", outbound_profile->destination_number);
			switch_channel_set_name(channel, name);

			caller_profile = switch_caller_profile_clone(*new_session, outbound_profile);
			switch_channel_set_caller_profile(channel, caller_profile);
			tech_pvt->caller_profile = caller_profile;
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Doh! no caller profile\n");
			switch_core_session_destroy(new_session);
			return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
		}



		switch_channel_set_flag(channel, CF_OUTBOUND);
		switch_set_flag_locked(tech_pvt, TFLAG_OUTBOUND);
		switch_channel_set_state(channel, CS_INIT);
		return SWITCH_CAUSE_SUCCESS;
	}

	return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;

}

static switch_status_t channel_receive_event(switch_core_session_t *session, switch_event_t *event)
{
	struct private_object *tech_pvt = static_cast<private_t*>(switch_core_session_get_private(session));
	char *body = switch_event_get_body(event);
	switch_assert(tech_pvt != NULL);

	if (!body) {
		body = "";
	}

	return SWITCH_STATUS_SUCCESS;
}



switch_state_handler_table_t khomp_state_handlers = {
	/*.on_init */ channel_on_init,
	/*.on_routing */ channel_on_routing,
	/*.on_execute */ channel_on_execute,
	/*.on_hangup */ channel_on_hangup,
	/*.on_exchange_media */ channel_on_exchange_media,
	/*.on_soft_execute */ channel_on_soft_execute
};

switch_io_routines_t khomp_io_routines = {
	/*.outgoing_channel */ channel_outgoing_channel,
	/*.read_frame */ channel_read_frame,
	/*.write_frame */ channel_write_frame,
	/*.kill_channel */ channel_kill_channel,
	/*.send_dtmf */ channel_send_dtmf,
	/*.receive_message */ channel_receive_message,
	/*.receive_event */ channel_receive_event
};

static switch_status_t load_config(void)
{
	const char *cf = "khomp.conf";
	switch_xml_t cfg, xml, settings, param;

	memset(&globals, 0, sizeof(globals));
	switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, module_pool);
	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", cf);
		return SWITCH_STATUS_TERM;
	}

	if ((settings = switch_xml_child(cfg, "settings"))) {
		for (param = switch_xml_child(settings, "param"); param; param = param->next) {
			char *var = (char *) switch_xml_attr_soft(param, "name");
			char *val = (char *) switch_xml_attr_soft(param, "value");

			if (!strcmp(var, "debug")) {
				globals.debug = atoi(val);
			} else if (!strcmp(var, "port")) {
				globals.port = atoi(val);
			} else if (!strcmp(var, "ip")) {
				set_global_ip(val);
			} else if (!strcmp(var, "codec-master")) {
				if (!strcasecmp(val, "us")) {
					switch_set_flag(&globals, GFLAG_MY_CODEC_PREFS);
				}
			} else if (!strcmp(var, "dialplan")) {
				set_global_dialplan(val);
			} else if (!strcmp(var, "codec-prefs")) {
				set_global_codec_string(val);
				globals.codec_order_last = switch_separate_string(globals.codec_string, ',', globals.codec_order, SWITCH_MAX_CODECS);
			} else if (!strcmp(var, "codec-rates")) {
				set_global_codec_rates_string(val);
				globals.codec_rates_last = switch_separate_string(globals.codec_rates_string, ',', globals.codec_rates, SWITCH_MAX_CODECS);
			}
		}
	}

	if (!globals.dialplan) {
		set_global_dialplan("default");
	}

	if (!globals.port) {
		globals.port = 4569;
	}

	switch_xml_free(xml);

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_khomp_load)
{

	module_pool = pool;

	memset(&globals, 0, sizeof(globals));

	load_config();

	switch_api_interface_t *api_interface;

	*module_interface = switch_loadable_module_create_module_interface(pool, "mod_khomp");
	khomp_endpoint_interface = static_cast<switch_endpoint_interface_t*>(switch_loadable_module_create_interface(*module_interface, SWITCH_ENDPOINT_INTERFACE));
	khomp_endpoint_interface->interface_name = "khomp";
	khomp_endpoint_interface->io_routines = &khomp_io_routines;
	khomp_endpoint_interface->state_handler = &khomp_state_handlers;

    /* 
       Spawn our k3l global var that will be used along the module
       for sending info to the boards
    */
    k3l = new K3LAPI();

    /* Start the API and connect to KServer */
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Starting K3L...\n");
    try {
        k3l->start();
    } catch (...) {
        /* TODO: Catch the proper error and display it. */
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "K3L not started.");
        return SWITCH_STATUS_TERM;
    }
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "K3L started.\n");

    /* Add all the specific API functions */
    SWITCH_ADD_API(api_interface, "khomp", "Khomp Menu", khomp, KHOMP_SYNTAX);

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/*
SWITCH_MODULE_RUNTIME_FUNCTION(mod_khomp_runtime)
{
	return SWITCH_STATUS_TERM;
}
*/

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_khomp_shutdown)
{
	int x = 0;

	running = -1;

	while (running) {
		if (x++ > 100) {
			break;
		}
		switch_yield(20000);
	}
	
	/* Free dynamically allocated strings */
	switch_safe_free(globals.dialplan);
	switch_safe_free(globals.codec_string);
	switch_safe_free(globals.codec_rates_string);
	switch_safe_free(globals.ip);

	/* Finnish him! */
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Stopping K3L...\n");
    k3l->stop();
    delete k3l;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "the K3L API has been stopped!\n");
	
	return SWITCH_STATUS_SUCCESS;
}

/* 
   khomp API definition
*/
SWITCH_STANDARD_API(khomp)
{
   	char *argv[10] = { 0 };
	int argc = 0;
	void *val;
	char *myarg = NULL;
	switch_status_t status = SWITCH_STATUS_SUCCESS;

    /* We should not ever get a session here */
	if (session) return status;

	if (switch_strlen_zero(cmd) || !(myarg = strdup(cmd))) {
		stream->write_function(stream, "USAGE: %s\n", KHOMP_SYNTAX);
		return SWITCH_STATUS_FALSE;
	}

	if ((argc = switch_separate_string(myarg, ' ', argv, (sizeof(argv) / sizeof(argv[0])))) < 1) {
		stream->write_function(stream, "USAGE: %s\n", KHOMP_SYNTAX);
		goto done;
	}

    /* Below show ... */
	if (argv[0] && !strncasecmp(argv[0], "show", 4)) {
        /* Show the API summary and information */
        if (argv[1] && !strncasecmp(argv[1], "info", 4)) {
            printBoardsInfo(stream);
        }
        /* Show all the links and their status */
        if (argv[1] && !strncasecmp(argv[1], "links", 5)) {
            for(int device=0 ; device < k3l->device_count() ; device++)
            {
                stream->write_function(stream, "___________________________________________\n");
                stream->write_function(stream, "|--------------Khomp Links----------------|\n");
                stream->write_function(stream, "|----Board----|----Link----|----Status----|\n");
                for(int link=0 ; link < k3l->link_count(device) ; link++)
                {
                    stream->write_function(stream, "|%13d|%12d|%s|\n"
                                            , device
                                            , link
                                            , linkStatus(device, link));
                }
            }
            stream->write_function(stream, "-------------------------------------------\n");
        }

	} else {
		stream->write_function(stream, "USAGE: %s\n", KHOMP_SYNTAX);
	}

done:
	switch_safe_free(myarg);
	return status;

}

/* Helper functions */
static const char * linkStatus(unsigned int device, unsigned int link)
{

    K3L_LINK_CONFIG & config = k3l->link_config(device, link);
    K3L_LINK_STATUS   status;
    
    KLibraryStatus ret = (KLibraryStatus) k3lGetDeviceStatus (device, link + ksoLink, &status, sizeof(status));

    if (ret == ksSuccess)
    {
        switch (config.Signaling)
        {
            case ksigInactive:
                return "[ksigInactive]";

            case ksigAnalog:
                return "[ksigAnalog]";

            case ksigSIP:
                return "[ksigSIP]";

            case ksigOpenCAS:
            case ksigOpenR2:
            case ksigR2Digital:
            case ksigUserR2Digital:
            case ksigOpenCCS:
            case ksigPRI_EndPoint:
            case ksigPRI_Network:
            case ksigPRI_Passive:
            case ksigLineSide:
            case ksigCAS_EL7:
            case ksigAnalogTerminal:
                if (kesOk == status.E1)
                {
                    return "kesOk";
                }
                else
                {
                    if (kesSignalLost & status.E1)         return "SignalLost";
                    if (kesNetworkAlarm & status.E1)       return "NetworkAlarm";
                    if (kesFrameSyncLost & status.E1)      return "FrameSyncLost";
                    if (kesMultiframeSyncLost & status.E1) return "MultiframeSyncLost";
                    if (kesRemoteAlarm & status.E1)        return "RemoteAlarm";
                    if (kesHighErrorRate & status.E1)      return "HighErrorRate";
                    if (kesUnknownAlarm & status.E1)       return "UnknownAlarm";
                    if (kesE1Error & status.E1)            return "E1Error";

                }
            default:
                return "NotImplementedBoard";
        }
    }

    return "<unknown>";
}


static void printBoardsInfo(switch_stream_handle_t* stream) {

    K3L_API_CONFIG apiCfg;

    stream->write_function(stream," ------------------------------------------------------------------\n");
    stream->write_function(stream, "|---------------------- Khomp System Summary ----------------------|\n");
    stream->write_function(stream, "|------------------------------------------------------------------|\n");

    if (k3lGetDeviceConfig(-1, ksoAPI, &apiCfg, sizeof(apiCfg)) == ksSuccess)
    {
        stream->write_function(stream, "| K3L API %d.%d.%d [m.VPD %d] - %-38s |\n"
                     , apiCfg.MajorVersion , apiCfg.MinorVersion , apiCfg.BuildVersion
                     , apiCfg.VpdVersionNeeded , apiCfg.StrVersion);
    }

    for (unsigned int i = 0; i < k3l->device_count(); i++)
    {
        K3L_DEVICE_CONFIG & devCfg = k3l->device_config(i);

        stream->write_function(stream, " ------------------------------------------------------------------\n");

        switch (k3l->device_type(i))
        {
            /* E1 boards */
            case kdtE1:
            case kdtConf:
            case kdtPR:
            case kdtE1GW:
            case kdtE1IP:
            case kdtE1Spx:
            case kdtGWIP:
            case kdtFXS:
            case kdtFXSSpx:
            {
                K3L_E1600A_FW_CONFIG dspAcfg;
                K3L_E1600B_FW_CONFIG dspBcfg;

                if ((k3lGetDeviceConfig(i, ksoFirmware + kfiE1600A, &dspAcfg, sizeof(dspAcfg)) == ksSuccess) &&
                    (k3lGetDeviceConfig(i, ksoFirmware + kfiE1600B, &dspBcfg, sizeof(dspBcfg)) == ksSuccess))
                {
                    stream->write_function(stream, "| [[ %02d ]] %s, serial '%04d', %02d channels, %d links.|\n"
                                , i , "E1" , atoi(devCfg.SerialNumber) , devCfg.ChannelCount , devCfg.LinkCount);
                    stream->write_function(stream, "| * DSP A: %s, DSP B: %s - PCI bus: %02d, PCI slot: %02d %s|\n"
                                , dspAcfg.DspVersion , dspBcfg.DspVersion , devCfg.PciBus , devCfg.PciSlot
                                , std::string(18 - strlen(dspAcfg.DspVersion) - strlen(dspBcfg.DspVersion), ' ').c_str());
                    stream->write_function(stream, "| * %-62s |\n" , dspAcfg.FwVersion);
                    stream->write_function(stream, "| * %-62s |\n" , dspBcfg.FwVersion);

                }

                break;
            }

            /* analog boards */
            case kdtFXO:
            case kdtFXOVoIP:
            {
                K3L_FXO80_FW_CONFIG dspCfg;

                if (k3lGetDeviceConfig(i, ksoFirmware + kfiFXO80, &dspCfg, sizeof(dspCfg)) == ksSuccess)
                {
                    stream->write_function(stream, "| [[ %02d ]] %s, serial '%04d', %02d channels. %s|\n"
                                , i , "KFXO80" , atoi(devCfg.SerialNumber) , devCfg.ChannelCount
                                , std::string(26 - 6, ' ').c_str());
                    stream->write_function(stream, "| * DSP: %s - PCI bus: %02d, PCI slot: %02d%s|\n"
                                , dspCfg.DspVersion , devCfg.PciBus , devCfg.PciSlot
                                , std::string(30 - strlen(dspCfg.DspVersion), ' ').c_str());
                    stream->write_function(stream, "| * %-62s |\n" , dspCfg.FwVersion);
                }

            }

            case kdtGSM:
            case kdtGSMSpx:
            {
                K3L_GSM40_FW_CONFIG dspCfg;

                if (k3lGetDeviceConfig(i, ksoFirmware + kfiGSM40, &dspCfg, sizeof(dspCfg)) == ksSuccess)
                {
                    stream->write_function(stream, "| [[ %02d ]] %s, serial '%04d', %02d channels. %s|\n"
                        , i , "KGSM" , atoi(devCfg.SerialNumber) , devCfg.ChannelCount
                        , std::string(26 - 4, ' ').c_str());

                    stream->write_function(stream, "| * DSP: %s - PCI bus: %02d, PCI slot: %02d%s|\n"
                                , dspCfg.DspVersion , devCfg.PciBus , devCfg.PciSlot
                                , std::string(30 - strlen(dspCfg.DspVersion), ' ').c_str());

                    stream->write_function(stream, "| * %-62s |\n" , dspCfg.FwVersion);
                }

                break;
            }
            default:
                stream->write_function(stream, "| [[ %02d ]] Unknown type '%02d'! Please contact Khomp support for help! |\n"
                    , i , k3l->device_type(i));
                break;
        }
    }

    stream->write_function(stream, " ------------------------------------------------------------------\n");
}
/* End of helper functions */


/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 expandtab:
 */
