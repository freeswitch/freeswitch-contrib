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
  \mainpage Khomp board Endpoint Module
  
  \section Introduction
  This module has been developed to make a nice, affordable brazilian board
  called Khomp (http://www.khomp.com.br) compatible with FreeSWITCH.
  It's main features are MFC-R2 native compatibility and the GSM boards.
  This module is supported by the manufacturer.

  \section Contributors
  \li João Mesquita <jmesquita (at) gmail.com>
  \li Raul Fragoso <raulfragoso (at) gmail.com>
  \li Khomp development team
  */

/**
 * @file mod_khomp.h
 * @brief Khomp Endpoint Module
 * @see mod_khomp
 */
#ifndef _MOD_KHOMP_H_
#define _MOD_KHOMP_H_

#include "globals.h"
#include "khomp_pvt.h"
#include "opt.h"

/*!
 \brief Callback generated from K3L API for every new event on the board.
 \param[in] obj Object ID (could be a channel or a board, depends on device type) which generated the event.
 \param[in] e The event itself.
 \return ksSuccess if the event was treated
 \see K3L_EVENT Event specification
 */
static int32 Kstdcall khomp_event_callback(int32 obj, K3L_EVENT * e);

/*!
 \brief Callback generated from K3L API everytime audio is available on the board.
 @param[in] deviceid Board on which we get the event
 @param[in] objectid The channel we are getting the audio from
 @param[out] read_buffer The audio buffer itself (RAW)
 @param[in] read_size The buffer size, meaning the amount of data to be read
 \return ksSuccess if the event was treated
 */
static void Kstdcall khomp_audio_listener(int32 deviceid, int32 objectid,
                                          byte * read_buffer, int32 read_size);

/*!
 \brief Defined by mod_reference, defines statuses for the switch_channel
 \param TFLAG_IO Enables the IO state of a channel
 \param TFLAG_INBOUND Sets a channel as inbound
 \param TFLAG_OUTBOUND Sets a channel as outbound
 */
typedef enum
{
    TFLAG_IO = (1 << 0),
    TFLAG_INBOUND = (1 << 1),
    TFLAG_OUTBOUND = (1 << 2),
    TFLAG_DTMF = (1 << 3),
    TFLAG_VOICE = (1 << 4),
    TFLAG_HANGUP = (1 << 5),
    TFLAG_LINEAR = (1 << 6),
    TFLAG_CODEC = (1 << 7),
    TFLAG_BREAK = (1 << 8)
}
TFLAGS;

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
static switch_status_t channel_on_init(switch_core_session_t *session);
static switch_status_t channel_on_routing(switch_core_session_t *session);
static switch_status_t channel_on_execute(switch_core_session_t *session);
static switch_status_t channel_on_hangup(switch_core_session_t *session);
static switch_status_t channel_on_exchange_media(
        switch_core_session_t *session);
static switch_status_t channel_on_soft_execute(switch_core_session_t *session);
static switch_status_t channel_on_destroy(switch_core_session_t *session);

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
static switch_call_cause_t channel_outgoing_channel(
        switch_core_session_t *session, 
        switch_event_t *var_event,
        switch_caller_profile_t *outbound_profile,
        switch_core_session_t **new_session, 
        switch_memory_pool_t **pool, 
        switch_originate_flag_t flags);
static switch_status_t channel_read_frame(switch_core_session_t *session, 
        switch_frame_t **frame, 
        switch_io_flag_t flags, 
        int stream_id);
static switch_status_t channel_write_frame(switch_core_session_t *session, 
        switch_frame_t *frame, 
        switch_io_flag_t flags, 
        int stream_id);
static switch_status_t channel_kill_channel(switch_core_session_t *session, 
        int sig);
static switch_status_t channel_send_dtmf(switch_core_session_t *session, 
        const switch_dtmf_t *dtmf);
static switch_status_t channel_receive_message(switch_core_session_t *session, 
        switch_core_session_message_t *msg);
static switch_status_t channel_receive_event(switch_core_session_t *session, 
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
static void printSystemSummary(switch_stream_handle_t* stream);
/*!
 \brief Print link status. [khomp show links]
 */
static void printLinks(switch_stream_handle_t* stream, unsigned int device, 
        unsigned int link);
/*!
 \brief Print board channel status. [khomp show channels]
 */
static void printChannels(switch_stream_handle_t* stream, unsigned int device, 
        unsigned int link);


#endif /* _MOD_KHOMP_H_ */
