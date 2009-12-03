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
 * Joao Mesquita <mesquita (at) khomp.com.br>
 * Raul Fragoso <raulfragoso (at) gmail.com>
 *
 *
 * mod_khomp.c -- Khomp board Endpoint Module
 *
 */

/**
 * @file mod_khomp.cpp
 * @brief Khomp Endpoint Module
 * @see mod_khomp
 */

#include "mod_khomp.h"

/*!
 \brief Callback generated from K3L API for every new event on the board.
 \param[in] obj Object ID (could be a channel or a board, depends on device type) which generated the event.
 \param[in] e The event itself.
 \return ksSuccess if the event was treated
 \see K3L_EVENT Event specification
 */
int32 Kstdcall khomp_event_callback(int32 obj, K3L_EVENT * e);

/*!
 \brief Callback generated from K3L API everytime audio is available on the board.
 @param[in] deviceid Board on which we get the event
 @param[in] objectid The channel we are getting the audio from
 @param[out] read_buffer The audio buffer itself (RAW)
 @param[in] read_size The buffer size, meaning the amount of data to be read
 \return ksSuccess if the event was treated
 */
void Kstdcall khomp_audio_listener(int32 deviceid, int32 objectid,
                                          byte * read_buffer, int32 read_size);

/*!
  \brief Load the module. Expadend by a FreeSWITCH macro.
  Things we do here:
  \li Initialize a static structure on KhompPvt
  \li Load the configuration
  \li Start the K3L API, responsible for connecting to KServer
  \li Register mod APIs and APPs
  \li Register audio callback for KServer
  \li Register event callback for KServer
  \see Opt Where all the configs are handled
  \see khomp_event_callback To where we bind the event handler
  \see khomp_audio_listener To where we bind the audio handlers
  */
SWITCH_MODULE_LOAD_FUNCTION(mod_khomp_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_khomp_shutdown);
SWITCH_MODULE_DEFINITION(mod_khomp, mod_khomp_load, mod_khomp_shutdown, NULL);

/* State handlers for FreeSWITCH */
switch_status_t channel_on_init(switch_core_session_t *session);
switch_status_t channel_on_routing(switch_core_session_t *session);
switch_status_t channel_on_execute(switch_core_session_t *session);
switch_status_t channel_on_hangup(switch_core_session_t *session);
switch_status_t channel_on_exchange_media(
        switch_core_session_t *session);
switch_status_t channel_on_soft_execute(switch_core_session_t *session);
switch_status_t channel_on_destroy(switch_core_session_t *session);

switch_state_handler_table_t khomp_state_handlers = {
    /*.on_init */ channel_on_init,
    /*.on_routing */ channel_on_routing,
    /*.on_execute */ channel_on_execute,
    /*.on_hangup */ channel_on_hangup,
    /*.on_exchange_media */ channel_on_exchange_media,
    /*.on_soft_execute */ channel_on_soft_execute,
	/*.on_consume_media */ NULL,
    /*.on_hibernate */ NULL,
    /*.on_reset */ NULL,
    /*.on_park*/ NULL,
    /*.on_reporting*/ NULL,
    /*.on_destroy*/ channel_on_destroy    
};

/* Callbacks for FreeSWITCH */
switch_call_cause_t channel_outgoing_channel(
        switch_core_session_t *session, 
        switch_event_t *var_event,
        switch_caller_profile_t *outbound_profile,
        switch_core_session_t **new_session, 
        switch_memory_pool_t **pool, 
        switch_originate_flag_t flags,
		switch_call_cause_t * cancel_cause);
switch_status_t channel_read_frame(switch_core_session_t *session, 
        switch_frame_t **frame, 
        switch_io_flag_t flags, 
        int stream_id);
switch_status_t channel_write_frame(switch_core_session_t *session, 
        switch_frame_t *frame, 
        switch_io_flag_t flags, 
        int stream_id);
switch_status_t channel_kill_channel(switch_core_session_t *session, 
        int sig);
switch_status_t channel_send_dtmf(switch_core_session_t *session, 
        const switch_dtmf_t *dtmf);
switch_status_t channel_receive_message(switch_core_session_t *session, 
        switch_core_session_message_t *msg);
switch_status_t channel_receive_event(switch_core_session_t *session, 
        switch_event_t *event);


switch_io_routines_t khomp_io_routines = {
    /*.outgoing_channel */ channel_outgoing_channel,
    /*.read_frame */ channel_read_frame,
    /*.write_frame */ channel_write_frame,
    /*.kill_channel */ channel_kill_channel,
    /*.send_dtmf */ channel_send_dtmf,
    /*.receive_message */ channel_receive_message,
    /*.receive_event */ channel_receive_event
};

/* Macros to define specific API functions */
SWITCH_STANDARD_API(khomp);

/*!
 \brief Print a system summary for all the boards. [khomp show info]
 */
void printSystemSummary(switch_stream_handle_t* stream);
/*!
 \brief Print link status. [khomp show links]
 */
void printLinks(switch_stream_handle_t* stream, unsigned int device, 
        unsigned int link);
/*!
 \brief Print board channel status. [khomp show channels]
 */
void printChannels(switch_stream_handle_t* stream, unsigned int device, 
        unsigned int link);



/*!
 \brief Will init part of our private structure and setup all the read/write
 buffers along with the proper codecs. Right now, only PCMA.
 */
switch_status_t tech_init(KhompPvt *tech_pvt, switch_core_session_t *session)
{
    tech_pvt->flags = 0;
    
    tech_pvt->_read_frame.data = tech_pvt->_databuf;
    tech_pvt->_read_frame.buflen = sizeof(tech_pvt->_databuf);

    switch_mutex_init(&tech_pvt->_mutex, SWITCH_MUTEX_NESTED, switch_core_session_get_pool(session));
    switch_mutex_init(&tech_pvt->flag_mutex, SWITCH_MUTEX_NESTED, switch_core_session_get_pool(session));
    
    switch_core_session_set_private(session, tech_pvt);

    tech_pvt->_session = session;

    if (switch_core_codec_init(&tech_pvt->_read_codec,
                               "PCMA",
                               NULL,
                               8000,
                               20,
                               1,
                               SWITCH_CODEC_FLAG_ENCODE | SWITCH_CODEC_FLAG_DECODE,
                               NULL, switch_core_session_get_pool(tech_pvt->_session)) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Can't load codec?\n");
        return SWITCH_STATUS_GENERR;
    } else {
        if (switch_core_codec_init(&tech_pvt->_write_codec,
                                   "PCMA",
                                   NULL,
                                   8000,
                                   20,
                                   1,
                                   SWITCH_CODEC_FLAG_ENCODE | SWITCH_CODEC_FLAG_DECODE,
                                   NULL, switch_core_session_get_pool(tech_pvt->_session)) != SWITCH_STATUS_SUCCESS) {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Can't load codec?\n");
            switch_core_codec_destroy(&tech_pvt->_read_codec);
            return SWITCH_STATUS_GENERR;
        }
    }

    switch_core_session_set_read_codec(tech_pvt->_session, &tech_pvt->_read_codec);
    switch_core_session_set_write_codec(tech_pvt->_session, &tech_pvt->_write_codec);

    switch_set_flag_locked(tech_pvt, TFLAG_CODEC);

    tech_pvt->_read_frame.codec = &tech_pvt->_read_codec;

    switch_set_flag_locked(tech_pvt, TFLAG_IO);

    return SWITCH_STATUS_SUCCESS;

}

/*!
   \brief State methods they get called when the state changes to the specific state 
   returning SWITCH_STATUS_SUCCESS tells the core to execute the standard state method next
   so if you fully implement the state you can return SWITCH_STATUS_FALSE to skip it.
*/
switch_status_t channel_on_init(switch_core_session_t *session)
{
    KhompPvt * tech_pvt = static_cast< KhompPvt* >(switch_core_session_get_private(session));
    assert(tech_pvt != NULL);

    switch_channel_t *channel = switch_core_session_get_channel(session);
    assert(channel != NULL);

    switch_set_flag_locked(tech_pvt, TFLAG_IO);

    /* Move channel's state machine to ROUTING. This means the call is trying
       to get from the initial start where the call because, to the point
       where a destination has been identified. If the channel is simply
       left in the initial state, nothing will happen. */
    switch_channel_set_state(channel, CS_ROUTING);

    switch_mutex_lock(Globals::_mutex);
    Globals::_calls++;
    switch_mutex_unlock(Globals::_mutex);

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_routing(switch_core_session_t *session)
{
    switch_channel_t *channel = NULL;
    KhompPvt *tech_pvt = NULL;

    channel = switch_core_session_get_channel(session);
    assert(channel != NULL);

    tech_pvt = static_cast<KhompPvt *>(switch_core_session_get_private(session));
    assert(tech_pvt != NULL);

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%s CHANNEL ROUTING\n", switch_channel_get_name(channel));

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_execute(switch_core_session_t *session)
{

    switch_channel_t *channel = NULL;
    KhompPvt *tech_pvt = NULL;

    channel = switch_core_session_get_channel(session);
    assert(channel != NULL);

    tech_pvt = static_cast<KhompPvt*>(switch_core_session_get_private(session));
    assert(tech_pvt != NULL);

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%s CHANNEL EXECUTE\n", switch_channel_get_name(channel));


    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_hangup(switch_core_session_t *session)
{
    switch_channel_t *channel = NULL;
    KhompPvt *tech_pvt = NULL;

    channel = switch_core_session_get_channel(session);
    assert(channel != NULL);

    tech_pvt = static_cast<KhompPvt*>(switch_core_session_get_private(session));
    assert(tech_pvt != NULL);

    switch_clear_flag_locked(tech_pvt, TFLAG_IO);
    switch_clear_flag_locked(tech_pvt, TFLAG_VOICE);
    //switch_thread_cond_signal(tech_pvt->_cond);

    if (tech_pvt->_read_codec.implementation) {
        switch_core_codec_destroy(&tech_pvt->_read_codec);
    }

    if (tech_pvt->_write_codec.implementation) {
        switch_core_codec_destroy(&tech_pvt->_write_codec);
    }

    /* Make the channel available again */
    tech_pvt->session(NULL);
    
    try 
    {
        Globals::_k3lapi.command(tech_pvt->_KDeviceId, tech_pvt->_KChannelId, CM_DISCONNECT, NULL);
    }
    catch(K3LAPI::failed_command & e)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "WE COULD NOT HANGUP THE CHANNEL! rc:%d\n", e.rc);
        return SWITCH_STATUS_TERM;
    }
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "%s Originator Hangup.\n", switch_channel_get_name(channel));
    switch_mutex_lock(Globals::_mutex);
    Globals::_calls--;
    if (Globals::_calls < 0) {
        Globals::_calls = 0;
    }
    switch_mutex_unlock(Globals::_mutex);

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_destroy(switch_core_session_t *session)
{
    /* Doesn't do anything for now */
    
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_kill_channel(switch_core_session_t *session, int sig)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "CHANNEL KILL, kill = %d\n", sig);
    switch_channel_t *channel = NULL;
    KhompPvt *tech_pvt = NULL;

    channel = switch_core_session_get_channel(session);
    assert(channel != NULL);

    tech_pvt = static_cast<KhompPvt*>(switch_core_session_get_private(session));
    assert(tech_pvt != NULL);

    switch (sig) {
    case SWITCH_SIG_KILL:
        switch_clear_flag_locked(tech_pvt, TFLAG_IO);
        switch_clear_flag_locked(tech_pvt, TFLAG_VOICE);
        switch_channel_hangup(channel, SWITCH_CAUSE_NORMAL_CLEARING);
        //switch_thread_cond_signal(tech_pvt->_cond);
        break;
    case SWITCH_SIG_BREAK:
        switch_set_flag_locked(tech_pvt, TFLAG_BREAK);
        break;
    default:
        break;
    }

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_exchange_media(switch_core_session_t *session)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "CHANNEL LOOPBACK\n");
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_soft_execute(switch_core_session_t *session)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "CHANNEL TRANSMIT\n");
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_send_dtmf(switch_core_session_t *session, const switch_dtmf_t *dtmf)
{
    KhompPvt *tech_pvt = static_cast<KhompPvt*>(switch_core_session_get_private(session));
    switch_assert(tech_pvt != NULL);

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_read_frame(switch_core_session_t *session, switch_frame_t **frame, switch_io_flag_t flags, int stream_id)
{
    switch_channel_t *channel = NULL;
    KhompPvt *tech_pvt = NULL;
    //switch_time_t started = switch_time_now();
    //unsigned int elapsed;
    switch_byte_t *data;

    channel = switch_core_session_get_channel(session);
    assert(channel != NULL);

    tech_pvt = static_cast<KhompPvt*>(switch_core_session_get_private(session));
    assert(tech_pvt != NULL);
    tech_pvt->_read_frame.flags = SFF_NONE;
    *frame = NULL;

    while (switch_test_flag(tech_pvt, TFLAG_IO)) {

        if (switch_test_flag(tech_pvt, TFLAG_BREAK)) {
            switch_clear_flag(tech_pvt, TFLAG_BREAK);
            goto cng;
        }

        if (!switch_test_flag(tech_pvt, TFLAG_IO)) {
            return SWITCH_STATUS_FALSE;
        }

        if (!tech_pvt->_read_frame.datalen) {
            continue;
        }
        *frame = &tech_pvt->_read_frame;
#ifdef BIGENDIAN
        if (switch_test_flag(tech_pvt, TFLAG_LINEAR)) {
            switch_swap_linear((*frame)->data, (int) (*frame)->datalen / 2);
        }
#endif
        return SWITCH_STATUS_SUCCESS;

        switch_cond_next();
    }


    return SWITCH_STATUS_FALSE;

  cng:
    data = (switch_byte_t *) tech_pvt->_read_frame.data;
    data[0] = 65;
    data[1] = 0;
    tech_pvt->_read_frame.datalen = 2;
    tech_pvt->_read_frame.flags = SFF_CNG;
    *frame = &tech_pvt->_read_frame;
    return SWITCH_STATUS_SUCCESS;

}

switch_status_t channel_write_frame(switch_core_session_t *session, switch_frame_t *frame, switch_io_flag_t flags, int stream_id)
{
    switch_channel_t *channel = NULL;
    KhompPvt *tech_pvt = NULL;
    //switch_frame_t *pframe;

    channel = switch_core_session_get_channel(session);
    assert(channel != NULL);

    tech_pvt = static_cast<KhompPvt*>(switch_core_session_get_private(session));
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

switch_status_t channel_answer_channel(switch_core_session_t *session)
{
    KhompPvt *tech_pvt;
    switch_channel_t *channel = NULL;

    channel = switch_core_session_get_channel(session);
    assert(channel != NULL);

    tech_pvt = static_cast<KhompPvt*>(switch_core_session_get_private(session));
    assert(tech_pvt != NULL);

    return SWITCH_STATUS_SUCCESS;
}


switch_status_t channel_receive_message(switch_core_session_t *session, switch_core_session_message_t *msg)
{
    switch_channel_t *channel;
    KhompPvt *tech_pvt;

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "When the fuck is this called?.\n");

    channel = switch_core_session_get_channel(session);
    assert(channel != NULL);

    tech_pvt = (KhompPvt *) switch_core_session_get_private(session);
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

/*!
  \brief Make sure when you have 2 sessions in the same scope that you pass the appropriate one to the routines
  that allocate memory or you will have 1 channel with memory allocated from another channel's pool!
*/
switch_call_cause_t channel_outgoing_channel(switch_core_session_t *session, switch_event_t *var_event,
                                                    switch_caller_profile_t *outbound_profile,
                                                    switch_core_session_t **new_session, switch_memory_pool_t **pool, switch_originate_flag_t flags, switch_call_cause_t * cancel_cause)
{

    char *argv[3] = { 0 };
    int argc = 0;
    KhompPvt *tech_pvt;
    switch_call_cause_t cause = SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;

    /* We first need to find an available KhompPvt object to serve the session */
    
    if ((*new_session = switch_core_session_request(Globals::_khomp_endpoint_interface, SWITCH_CALL_DIRECTION_OUTBOUND, pool)) != 0) {
        switch_channel_t *channel;
        switch_caller_profile_t *caller_profile;
        char name[128];

        if (outbound_profile) 
        {
            snprintf(name, sizeof(name), "%s", outbound_profile->destination_number);
            
            if ((argc = switch_separate_string(outbound_profile->destination_number, '/', argv, (sizeof(argv) / sizeof(argv[0])))) < 3)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid dial string. Should be on the format:[Khomp/BoardID (or A for first free board)/CHANNEL (or A for first free channel)]\n");
                switch_core_session_destroy(new_session);
                return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
            }

            tech_pvt = KhompPvt::find_channel(name, *new_session, &cause);

            if(tech_pvt == NULL || cause != SWITCH_CAUSE_SUCCESS)
            {
                switch_core_session_destroy(new_session);
                return cause;
            }
                    
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Dialing to %s out from Board:%u, Channel:%u.\n",
                                                                argv[2],
                                                                tech_pvt->_KDeviceId,
                                                                tech_pvt->_KChannelId);
        }
        else
        {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Doh! no caller profile\n");
            switch_core_session_destroy(new_session);
            return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
        }
        
        switch_core_session_add_stream(*new_session, NULL);
        channel = switch_core_session_get_channel(*new_session);
        tech_init(tech_pvt, *new_session);

        snprintf(name, sizeof(name), "Khomp/%d/%d/%s", tech_pvt->_KDeviceId, tech_pvt->_KChannelId, argv[2]);
        switch_channel_set_name(channel, name);
        
        caller_profile = switch_caller_profile_clone(*new_session, outbound_profile);
        switch_channel_set_caller_profile(channel, caller_profile);
        tech_pvt->_caller_profile = caller_profile;

        switch_channel_set_flag(channel, CF_OUTBOUND);
        switch_set_flag_locked(tech_pvt, TFLAG_OUTBOUND);
        switch_channel_set_state(channel, CS_INIT);

        try 
        {
            /* Lets make the call! */
            char params[ 255 ];
            snprintf(params, sizeof(params), "dest_addr=\"%s\" orig_addr=\"%s\"", argv[2], outbound_profile->caller_id_number);
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "We are calling with params: %s.\n", params);
            Globals::_k3lapi.command(tech_pvt->_KDeviceId,tech_pvt->_KChannelId, CM_MAKE_CALL, params); 
        }
        catch(K3LAPI::failed_command & e)
        {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not place call! Cause: code%x and rc%d.\n", e.code, e.rc);
            switch_core_session_destroy(new_session);
            return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
        }

        return SWITCH_CAUSE_SUCCESS;
    }

    return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;

}

switch_status_t channel_receive_event(switch_core_session_t *session, switch_event_t *event)
{
    struct KhompPvt *tech_pvt = static_cast<KhompPvt*>(switch_core_session_get_private(session));
    char *body = switch_event_get_body(event);
    switch_assert(tech_pvt != NULL);

    if (!body) {
        body = (char *)"";
    }

    return SWITCH_STATUS_SUCCESS;
}




SWITCH_MODULE_LOAD_FUNCTION(mod_khomp_load)
{

    Globals::_module_pool = pool;

    /* start config system! */
    Opt::initialize();
    
    Opt::obtain();

    //load_config();


    /* 
       Spawn our k3l global var that will be used along the module
       for sending info to the boards
    */

    /* Start the API and connect to KServer */
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Starting K3L...\n");
    try {
        Globals::_k3lapi.start();
        KhompPvt::initialize();
    } catch (K3LAPI::start_failed & e) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "K3L not started. Reason:%s.\n", e.msg.c_str());
        return SWITCH_STATUS_TERM;
    }
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "K3L started.\n");
    
    k3lRegisterEventHandler( khomp_event_callback );
    k3lRegisterAudioListener (NULL, khomp_audio_listener);

    *module_interface = switch_loadable_module_create_module_interface(pool, "mod_khomp");
    Globals::_khomp_endpoint_interface = static_cast<switch_endpoint_interface_t*>(switch_loadable_module_create_interface(*module_interface, SWITCH_ENDPOINT_INTERFACE));
    Globals::_khomp_endpoint_interface->interface_name = "khomp";
    Globals::_khomp_endpoint_interface->io_routines = &khomp_io_routines;
    Globals::_khomp_endpoint_interface->state_handler = &khomp_state_handlers;

    /* Add all the specific API functions */
    SWITCH_ADD_API(Globals::_api_interface, "khomp", "Khomp Menu", khomp, KHOMP_SYNTAX);

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

    Globals::_running = -1;

    while (Globals::_running) {
        if (x++ > 100) {
            break;
        }
        switch_yield(20000);
    }
    
    /* Free dynamically allocated strings */
    switch_safe_free(Opt::_dialplan);
    switch_safe_free(Opt::_codec_string);
    switch_safe_free(Opt::_codec_rates_string);
    switch_safe_free(Opt::_ip);

    /* Finnish him! */
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Stopping K3L...\n");
    KhompPvt::terminate();
    Globals::_k3lapi.stop();
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "the K3L API has been stopped!\n");
    
    return SWITCH_STATUS_SUCCESS;
}

/*!
   \brief khomp API definition
   TODO: Add as xml modifier
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
            printSystemSummary(stream);
        }
        /* Show all the links and their status */
        if (argv[1] && !strncasecmp(argv[1], "links", 5)) {
            /* TODO: Let show specific boards/links */
            printLinks(stream, NULL, NULL);
        }
        // Show all channels from all boards and all links
        if (argv[1] && !strncasecmp(argv[1], "channels", 8)) {
            /* TODO: Let show specific channels */
            printChannels(stream, NULL, NULL);
        }

    } else {
        stream->write_function(stream, "USAGE: %s\n", KHOMP_SYNTAX);
    }

done:
    switch_safe_free(myarg);
    return status;

}

void printChannels(switch_stream_handle_t* stream, unsigned int device, unsigned int link) {
    if (!device) {
        // Print all channels from all boards and links
        stream->write_function(stream, "|--------- Khomp ----------|\n");
        stream->write_function(stream, "| Board | Channel | Status |\n");
        for (int board=0 ; board < Globals::_k3lapi.device_count() ; board++) {
            for (int channel=0 ; channel < Globals::_k3lapi.channel_count(board) ; channel++) {
                try {
                    K3L_CHANNEL_CONFIG channelConfig;
                    channelConfig = Globals::_k3lapi.channel_config( board, channel );
                }
                catch (...){
                    stream->write_function(stream, "OOOPSS. Something went wrong, cleanup this mess!\n");
                    return;
                }
                K3L_CHANNEL_STATUS status;
                if (k3lGetDeviceStatus( board, channel + ksoChannel, &status, sizeof(status) ) != ksSuccess) {
                    stream->write_function(stream, "Damn, again something bad happened.\n");
                    return;
                }
                switch(status.CallStatus) {
                    case kcsFree: 
                        stream->write_function(stream, "| %6u| %8u| Free |\n", board, channel);
                        break;
                    case kcsOutgoing: 
                        stream->write_function(stream, "| %6u| %8u| Outgoing |\n", board, channel);
                        break;
                    case kcsIncoming: 
                        stream->write_function(stream, "| %6u| %8u| Incoming |\n", board, channel);
                        break;
                    default:
                        stream->write_function(stream, "| %6u| %8u| UNKNOWN : %x |\n", board, channel, status.AddInfo);
                }
                        
            }
        }
        stream->write_function(stream, "----------------------------\n");
    }
}

void printLinks(switch_stream_handle_t* stream, unsigned int device, unsigned int link)
{

    stream->write_function(stream, "___________________________________________\n");
    stream->write_function(stream, "|--------------Khomp Links----------------|\n");
    stream->write_function(stream, "|----Board----|----Link----|----Status----|\n");

    // We want to see all links from all devices
    if (!device) {
        for(int device=0 ; device < Globals::_k3lapi.device_count() ; device++)
        {
            K3L_LINK_CONFIG & config = Globals::_k3lapi.link_config(device, link);
            K3L_LINK_STATUS   status;

            for(int link=0 ; link < Globals::_k3lapi.link_count(device) ; link++)
            {
                const char * E1Status = "";
                if (k3lGetDeviceStatus (device, link + ksoLink, &status, sizeof(status)) == ksSuccess)
                {
                    switch (config.Signaling)
                    {
                        case ksigInactive:
                            E1Status = "[ksigInactive]";
                            break;

                        case ksigAnalog:
                            E1Status = "[ksigAnalog]";
                            break;

                        case ksigSIP:
                            E1Status = "[ksigSIP]";
                            break;

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
                            switch (status.E1) {

                                case (kesOk):
                                    E1Status = "kesOk";
                                    break;
                                case (kesSignalLost):
                                    E1Status = "kesSignalLost";
                                    break;
                                case (kesNetworkAlarm):
                                    E1Status = "kesNetworkAlarm";
                                    break;
                                case (kesFrameSyncLost):
                                    E1Status = "kesFrameSyncLost";
                                    break;
                                case (kesMultiframeSyncLost):
                                    E1Status = "kesMultiframeSyncLost";
                                    break;
                                case (kesRemoteAlarm):
                                    E1Status = "kesRemoteAlarm";
                                    break;
                                case (kesHighErrorRate):
                                    E1Status = "kesHighErrorRate";
                                    break;
                                case (kesUnknownAlarm):
                                    E1Status = "kesUnknownAlarm";
                                    break;
                                case (kesE1Error):
                                    E1Status = "kesE1Error";
                                    break;
                                case (kesNotInitialized):
                                    E1Status = "kesNotInitialized";
                                    break;
                                default:
                                    E1Status = "UnknownE1Status";
                            }
                            break;
                        default:
                            E1Status = "NotImplementedBoard";
                    }
                }
                stream->write_function(stream, "|%13d|%12d|%s|\n"
                                        , device
                                        , link
                                        , E1Status);
            }
        }
    }
    stream->write_function(stream, "-------------------------------------------\n");
}


void printSystemSummary(switch_stream_handle_t* stream) {

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

    for (unsigned int i = 0; i < Globals::_k3lapi.device_count(); i++)
    {
        K3L_DEVICE_CONFIG & devCfg = Globals::_k3lapi.device_config(i);

        stream->write_function(stream, " ------------------------------------------------------------------\n");

        switch (Globals::_k3lapi.device_type(i))
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
                    , i , Globals::_k3lapi.device_type(i));
                break;
        }
    }

    stream->write_function(stream, " ------------------------------------------------------------------\n");
}
/* End of helper functions */


/* Create a new channel on incoming call */
KLibraryStatus khomp_channel_from_event(unsigned int KDeviceId, unsigned int KChannel, K3L_EVENT * event)
{
	switch_core_session_t *session = NULL;
	KhompPvt *tech_pvt = NULL;
	switch_channel_t *channel = NULL;
	char name[128];
	
	if (!(session = switch_core_session_request(Globals::_khomp_endpoint_interface, SWITCH_CALL_DIRECTION_INBOUND, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Initilization Error!\n");
		return ksFail;
	}
	
	switch_core_session_add_stream(session, NULL);
	
	tech_pvt = KhompPvt::khompPvt(KDeviceId, KChannel);
	assert(tech_pvt != NULL);
	channel = switch_core_session_get_channel(session);
	if (tech_init(tech_pvt, session) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Initilization Error!\n");
		switch_core_session_destroy(&session);
		return ksFail;
	}
	

    /* Get all data from event */
    std::string cidNum; 
    std::string destination_number; 
    try
    {
        cidNum = Globals::_k3lapi.get_param(event, "orig_addr");
        destination_number = Globals::_k3lapi.get_param(event, "dest_addr");
    }
    catch ( K3LAPI::get_param_failed & err )
    {
        // TODO: Can we set NULL variables? What should we do if this fails?
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Could not get param %s on channel %u, board %u.\n", err.name.c_str(), KChannel, KDeviceId);
    }

	/*if (switch_strlen_zero(sigmsg->channel->caller_data.cid_num.digits)) {
		if (!switch_strlen_zero(sigmsg->channel->caller_data.ani.digits)) {
			switch_set_string(sigmsg->channel->caller_data.cid_num.digits, sigmsg->channel->caller_data.ani.digits);
		} else {
			switch_set_string(sigmsg->channel->caller_data.cid_num.digits, sigmsg->channel->chan_number);
		}
	}
    */

    /* Set the caller profile - Look at documentation */
	tech_pvt->_caller_profile = switch_caller_profile_new(switch_core_session_get_pool(session),
														 "Khomp",
														 "XML", // TODO: Dialplan module to use?
                                                         NULL,
                                                         NULL,
														 NULL,
                                                         cidNum.c_str(),
                                                         NULL,
                                                         NULL,
														 (char *) modname,
														 "default", // TODO: Context to look for on the dialplan?
														 destination_number.c_str());

	assert(tech_pvt->_caller_profile != NULL);
    /* END */

    /* WHAT??? - Look at documentation */
    //switch_set_flag(tech_pvt->caller_profile, SWITCH_CPF_NONE);
    /* END */
	
    /* */
	snprintf(name, sizeof(name), "Khomp/%u/%u/%s", KDeviceId, KChannel, tech_pvt->_caller_profile->destination_number);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Connect inbound channel %s\n", name);
	switch_channel_set_name(channel, name);
	switch_channel_set_caller_profile(channel, tech_pvt->_caller_profile);
    /* END */

		
	switch_channel_set_state(channel, CS_INIT);
	if (switch_core_session_thread_launch(session) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Error spawning thread\n");
		switch_core_session_destroy(&session);
		return ksFail;
	}

    /* WHAT?
	if (zap_channel_add_token(sigmsg->channel, switch_core_session_get_uuid(session), 0) != ZAP_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Error adding token\n");
		switch_core_session_destroy(&session);
		return ZAP_FAIL;
	}
    */

    /* Set the session to the channel */
    KhompPvt::khompPvt(KDeviceId, KChannel)->session(session);

    return ksSuccess;
}

int32 Kstdcall khomp_event_callback(int32 obj, K3L_EVENT * e)
{                
    /* TODO: How do we make sure channels inside FreeSWITCH only change to valid states on K3L? */

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "New Khomp Event: on %u/%u: %x\n", e->DeviceId, obj, e->Code);
    
    switch(e->Code)
    {
        case EV_NEW_CALL:   
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "New call on %u to %s. [EV_NEW_CALL]\n", obj, Globals::_k3lapi.get_param(e, "dest_addr").c_str());
            if (khomp_channel_from_event(e->DeviceId, obj, e) != ksSuccess )
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Something bad happened while getting channel session. Device:%u/Channel:%u. [EV_CONNECT]\n", e->DeviceId, obj);
                return ksFail;
            }
            try 
            {
                Globals::_k3lapi.command(e->DeviceId, obj, CM_RINGBACK, NULL);
                Globals::_k3lapi.command(e->DeviceId, obj, CM_CONNECT, NULL); 
            }
            catch (K3LAPI::failed_command & err)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not set board channel status! [EV_NEW_CALL]\n");
            }
            break;
        case EV_DISCONNECT:
            {
                switch_core_session_t * session = KhompPvt::khompPvt(e->DeviceId, obj)->session();
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Called party dropped the call on: %u. Releasing channel. [EV_DISCONNECT]\n", obj);
                if(session == NULL)
                    break;

                if (channel_on_hangup(session) != SWITCH_STATUS_SUCCESS)
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not hangup channel: %u on board %u. Releasing board channel anyway. [EV_DISCONNECT]\n", obj, e->DeviceId);
                try
                {
                    /* Stop the audio callbacks */
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Stopping audio callbacks ...\n");
                    KhompPvt::khompPvt(e->DeviceId, obj)->stop_listen();
                    KhompPvt::khompPvt(e->DeviceId, obj)->stop_stream();
                    KhompPvt::khompPvt(e->DeviceId, obj)->session(NULL);
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Audio callbacks stopped successfully\n");
                }
                catch(K3LAPI::invalid_channel & err)
                {
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not send CM_DISCONNECT!\n");
                }
            }
            break;
        case EV_CONNECT:
           switch_core_session_t* session;
            try
            {
                session = KhompPvt::khompPvt(e->DeviceId, obj)->session();
                switch_channel_t *channel;
                channel = switch_core_session_get_channel(session);
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Call will be answered on board %u, channel %u. [EV_CONNECT]\n", e->DeviceId, obj);
                switch_channel_mark_answered(channel);
                /* Start listening for audio */
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Starting audio callbacks ...\n");
                KhompPvt::khompPvt(e->DeviceId, obj)->start_stream();
                KhompPvt::khompPvt(e->DeviceId, obj)->start_listen();
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Audio callbacks initialized successfully\n");
            }
            catch (K3LAPI::invalid_session & err)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Something bad happened while getting channel session. Device:%u/Channel:%u. [EV_CONNECT]\n", e->DeviceId, obj);
            }
            break;
        case EV_CALL_SUCCESS:
            /* TODO: Should we bridge here? Maybe check a certain variable if we should generate ringback? */
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Call on board %u, channel %u is ringing. [EV_CALL_SUCESS]\n", e->DeviceId, obj);
            break;
        case EV_CHANNEL_FREE:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Channel %u on board %u is now free. [EV_CHANNEL_FREE]\n", obj, e->DeviceId);
            break;
        case EV_NO_ANSWER:
            /* TODO: Destroy sessions and channels */
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "No one answered the call on board %u channel %u. [EV_NO_ANSWER]\n", e->DeviceId, obj);
            break;
        case EV_CALL_ANSWER_INFO:
            {
                /* TODO: Set channel variable if we get this event */
                /* TODO: Fire an event so ESL can get it? */
                /* Call Analyser has to be enabled on k3lconfig */
                const char * startInfo = "";
                switch (e->AddInfo)
                {
                    case (kcsiHumanAnswer):
                        startInfo = "kcsiHumanAnswer";
                        break;
                    case (kcsiAnsweringMachine):
                        startInfo = "kcsiAnsweringMachine";
                        break;
                    case (kcsiCellPhoneMessageBox):
                        startInfo = "kcsiCellPhoneMessageBox";
                        break;
                    case (kcsiUnknown):
                        startInfo = "kcsiUnknown";
                        break;
                    case (kcsiCarrierMessage):
                        startInfo = "kcsiCarrierMessage";
                        break;
                    default:
                        startInfo = "Error or unknown code!";

                }
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Channel %u, board %u detected: \"%s\". [EV_CALL_ANSWER_INFO]\n", e->DeviceId, obj, startInfo);
                    break;
            }
        case EV_DTMF_DETECTED:
            /* TODO: What to do with DTMF? */
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Board %u detected DTMF (%d) on channel %u. [EV_DTMF_DETECTED]\n", e->DeviceId, e->AddInfo, obj);
            break;
        case EV_DTMF_SEND_FINISH:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Channel %u on board %u has sucessfully generated DTMF. [EV_DTMF_SEND_FINNISH]\n", obj, e->DeviceId);
            break;
        case EV_CALL_FAIL:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Channel %u on board %u reported call fail. [EV_CALL_FAIL]\n", obj, e->DeviceId);
            break;
        case EV_CHANNEL_FAIL:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Channel %u on board %u reported failure. [EV_CHANNEL_FAIL]\n", obj, e->DeviceId);
            break;
        case EV_LINK_STATUS:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Link %u on board %u changed. [EV_LINK_STATUS]\n", e->DeviceId, obj);
            break;
        case EV_PHYSICAL_LINK_DOWN:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Link %u on board %u is DOWN. [EV_PHYSICAL_LINK_DOWN]\n", e->DeviceId, obj);
            break;
        case EV_PHYSICAL_LINK_UP:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Link %u on board %u is UP. [EV_PHYSICAL_LINK_UP]\n", e->DeviceId, obj);
            break;
        case EV_INTERNAL_FAIL:
            {
                const char * msg = "";
                switch(e->AddInfo)
                {
                    case kifInterruptCtrl:
                        msg = "kifInterruptCtrl";
                        break;
                    case kifCommunicationFail:
                        msg = "kifCommunicationFail";
                        break;
                    case kifProtocolFail:
                        msg = "kifProtocolFail";
                        break;
                    case kifInternalBuffer:
                        msg = "kifInternalBuffer";
                        break;
                    case kifMonitorBuffer:
                        msg = "kifMonitorBuffer";
                        break;
                    case kifInitialization:
                        msg = "kifInitialization";
                        break;
                    case kifInterfaceFail:
                        msg = "kifInterfaceFail";
                        break;
                    case kifClientCommFail:
                        msg = "kifClientCommFail";
                        break;
                    default:
                        msg = "UnknownError";
                }
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "This is a fatal error and we will not recover. Reason: %s. [EV_INTERNAL_FAIL]\n", msg);
                break;
            }
        default:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "New Event has just arrived on %u with untreated code: %x\n", obj, e->Code);
    }

    return ksSuccess;
}

void Kstdcall khomp_audio_listener (int32 deviceid, int32 objectid, byte * read_buffer, int32 read_size)
{
    KhompPvt * pvt = KhompPvt::khompPvt(deviceid, objectid);
    /*TODO: write to the switch_buffer_t member, which will be read by channel_read_frame */
        
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
