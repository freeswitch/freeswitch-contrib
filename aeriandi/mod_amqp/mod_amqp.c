/*
* FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
* Copyright (C) 2005-2012, Anthony Minessale II <anthm@freeswitch.org>
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
* Based on mod_skel by
* Anthony Minessale II <anthm@freeswitch.org>
*
* Contributor(s):
*
* Daniel Bryars <danb@aeriandi.com>
* Tim Brown <tim.brown@aeriandi.com>
* Anthony Minessale II <anthm@freeswitch.org>
*
* mod_amqp.c -- Sends FreeSWITCH events to an AMQP broker
*
*/

#include <switch.h>
#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_tcp_socket.h>
#include <apr_errno.h>
#include <sys/types.h>
#include <strings.h>
#include "amqp_settings.h"

#define MAX_LOG_MESSAGE_SIZE 1024

//If you change MAX_ROUTING_KEY_FORMAT_FIELDS then you must change the implementation of makeRoutingKey where it formats the routing key using sprintf
#define MAX_ROUTING_KEY_FORMAT_FIELDS 10
#define MAX_AMQP_ROUTING_KEY_LENGTH 255

#define TIME_STATS_TO_AGGREGATE 1024

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_amqp_shutdown);
SWITCH_MODULE_LOAD_FUNCTION(mod_amqp_load);
SWITCH_MODULE_DEFINITION(mod_amqp, mod_amqp_load, mod_amqp_shutdown, NULL);

// Function prototypes
static void * SWITCH_THREAD_FUNC mod_amqp_event_consumer_thread(switch_thread_t *thread, void *params);
static void * SWITCH_THREAD_FUNC mod_amqp_command_listener_thread(switch_thread_t *thread, void *params);

static void closeAmqpConnection(amqp_connection_state_t *);
static switch_event_types_t parse_fs_event_name(const char *event_name);
static const char * fs_event_get_name(switch_event_types_t event_type);
static void mod_AMQP_event_handler(switch_event_t* evt);

struct amqp_message_t {
    char routing_key[MAX_AMQP_ROUTING_KEY_LENGTH];
    char *pjson;
};

static struct {
    char *amqpHostnames;
    char *amqpVirtualHost;
    int   amqpPort;
    char *amqpUsername;
    char *amqpPassword;
    int   amqpHeartbeatSeconds;

    char *eventExchange;
    char *eventExchangeType;
    char *eventRoutingKeyFormat;
    char *eventRoutingKeyFormatFieldsCsv;

    char *commandExchange;
    char *commandExchangeType;
    char *commandBindingKey;

    const char *eventRoutingKeyFormatFields[MAX_ROUTING_KEY_FORMAT_FIELDS];
    switch_event_node_t *eventNode;
    int amqpSendQueueSize;

    int reconnect_interval_ms;
    int circuit_breaker_ms;
    switch_time_t circuit_breaker_reset_time;

    // Flag to indicate mod is running (Set by mod, read by thread)
    int running;

    // Rabbit connections are not thread safe so one connection per thread.
    // Communicate with sender thread using a queue
    switch_thread_t *eventSendThread;
    switch_queue_t *amqp_send_queue;
    amqp_connection_state_t amqpEventConn;

    // Listener thread
    switch_thread_t *commandListenerThread;
    amqp_connection_state_t amqpCommandConn;

    //Note: The AMQP channel is not reentrant this MUTEX serializes sending events.
    switch_mutex_t *amqp_connection_mutex;

    // Array to store the possible event subscriptions
    char *event_filter;
    unsigned int number_of_event_filters;
    switch_event_node_t *event_nodes[SWITCH_EVENT_ALL];
    switch_event_types_t event_ids[SWITCH_EVENT_ALL];
} globals;

static switch_xml_config_item_t xmlConfigStructure[] = {
    SWITCH_CONFIG_ITEM("amqpHostnames", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.amqpHostnames,
        "localhost", NULL, "machineName", "The AMQP Broker host to connect to."),
    SWITCH_CONFIG_ITEM("amqpVirtualHost", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.amqpVirtualHost,
        "/", NULL, "amqpVirtualHost", "The virtual host on the AMQP Broker host to connect to."),
    SWITCH_CONFIG_ITEM("amqpPort", SWITCH_CONFIG_INT, CONFIG_RELOADABLE, &globals.amqpPort,
        5672, NULL, "5672", "The tcp ip port for connecting to the AMQP Broker host."),
    SWITCH_CONFIG_ITEM("amqpUsername", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.amqpUsername,
        "guest", NULL, "guest", "The username for connecting to the AMQP Broker host."),
    SWITCH_CONFIG_ITEM("amqpPassword", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.amqpPassword,
        "amqpPassword", NULL, "amqpPassword", "The password for connecting to the AMQP Broker host."),
    SWITCH_CONFIG_ITEM("amqpHeartbeatSeconds", SWITCH_CONFIG_INT, CONFIG_RELOADABLE, &globals.amqpHeartbeatSeconds,
        0, NULL, "0", "The connection heartbeat in seconds, 0 means no heartbeat."),

    SWITCH_CONFIG_ITEM("eventExchange", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.eventExchange,
        "TAP.Events", NULL, "TAP.Events", "The name of the Exchange on the AMQP Broker host to which to send event messages."),
    SWITCH_CONFIG_ITEM("eventExchangeType", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.eventExchangeType,
        "topic", NULL, "topic", "The type of Exchange for events; if the exchange does not exist then we create the exchange with this type."),
    SWITCH_CONFIG_ITEM("eventRoutingKeyFormat", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.eventRoutingKeyFormat,
        "%s.%s.%s.%s", NULL, "MyKey.%d.%s", "A printf style format string which takes a list of arguments from the event being send using the routingKeyFormatFields config value."),
    SWITCH_CONFIG_ITEM("eventRoutingKeyFormatFields", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.eventRoutingKeyFormatFieldsCsv,
        "FreeSWITCH-Hostname,Event-Name,Event-Subclass,Unique-ID", NULL, "FreeSWITCH-Hostname,Event-Name,Event-Subclass,Unique-ID", "A comma seperated list "
        "of event header names to use to build an array of values to use when formatting the routing key. If the header is not present then an empty string is used."),
    SWITCH_CONFIG_ITEM("amqpSendQueueSize", SWITCH_CONFIG_INT, 0, &globals.amqpSendQueueSize,
        500, NULL, "0", "The size of the send buffer. When the buffer fills, new messages are discarded."),
    SWITCH_CONFIG_ITEM("amqpCircuitBreakerTimeout", SWITCH_CONFIG_INT, CONFIG_RELOADABLE, &globals.circuit_breaker_ms,
        10000, NULL, "10000", "The number of milliseconds to wait before trying to send meesages again after being throttled."),
    SWITCH_CONFIG_ITEM("eventFilter", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.event_filter,
        "SWITCH_EVENT_CHANNEL_CREATE,SWITCH_EVENT_CHANNEL_DESTROY,", NULL, "SWITCH_EVENT_ALL", "A comma delimited list of event types to send."),

    SWITCH_CONFIG_ITEM("commandExchange", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.commandExchange,
        "TAP.Commands", NULL, "TAP.Commands", "The name of the Exchange on the AMQP Broker host to which to receive command messages."),
    SWITCH_CONFIG_ITEM("commandExchangeType", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.commandExchangeType,
        "topic", NULL, "topic", "The type of Exchange for commands; if the exchange does not exist then we create the exchange with this type."),
    SWITCH_CONFIG_ITEM("commandBindingKey", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &globals.commandBindingKey,
        "TapCommands", NULL, "TapCommands", "The pattern to use when filtering command messages by routing key."),

    SWITCH_CONFIG_ITEM("amqpReconnectInterval", SWITCH_CONFIG_INT, CONFIG_RELOADABLE, &globals.reconnect_interval_ms,
        1000, NULL, "1000", "The number of milliseconds to wait before trying to reconnect after getting disconnected from AMQP."),

    SWITCH_CONFIG_ITEM_END()
};

static void logConfigValues()
{
    char logMessage[MAX_LOG_MESSAGE_SIZE];
    int i;
    char *cursor;

    snprintf(logMessage, MAX_LOG_MESSAGE_SIZE,
          "Amqp Settings:-\n amqpHostnames: %s\n amqpVirtualHost: %s\n"
          " amqpPort: %d\n amqpUsername: %s\n amqpHeartbeatSeconds: %d\n"
          " eventExchange: %s\n eventExchangeType: %s\n"
          " commandExchange: %s\n commandExchangeType: %s\n commandBindingKey: %s\n"
          " amqpSendQueueSize: %d\n circuit_breaker_ms: %d\n reconnect_interval_ms: %d\n"
          " eventFilter: " /* events appended later */,
        globals.amqpHostnames,
        globals.amqpVirtualHost,
        globals.amqpPort,
        globals.amqpUsername,
        globals.amqpHeartbeatSeconds,
        globals.eventExchange,
        globals.eventExchangeType,
        globals.commandExchange,
        globals.commandExchangeType,
        globals.commandBindingKey,
        globals.amqpSendQueueSize,
        globals.circuit_breaker_ms,
        globals.reconnect_interval_ms);

    /* Move to the end of the message */
    cursor = logMessage + strlen(logMessage);
    for (i = 0; i < globals.number_of_event_filters; ++i) {
        cursor += snprintf(cursor, logMessage + MAX_LOG_MESSAGE_SIZE - cursor - 2, "%s(%d) ", fs_event_get_name(globals.event_ids[i]), (int)globals.event_ids[i]);
    }

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "%s\n", logMessage);
}

static switch_status_t do_config(switch_bool_t reload)
{
    int i = 0;

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, reload ? "Reloading Config\n" : "Loading Config\n");

    if (SWITCH_STATUS_SUCCESS == switch_xml_config_parse_module_settings("amqp.conf", reload, xmlConfigStructure)) {
        const char *delims = ",";
        char *result = NULL;
        char *strtok_ptr;

        //Reset them to empty string
        for (i = 0; i < MAX_ROUTING_KEY_FORMAT_FIELDS; i++) {
            globals.eventRoutingKeyFormatFields[i] = NULL;
        }

        result = strtok_r(globals.eventRoutingKeyFormatFieldsCsv, delims, &strtok_ptr);
        i = 0;
        while (result != NULL) {

            if (i >= MAX_ROUTING_KEY_FORMAT_FIELDS) {

                switch_log_printf(SWITCH_CHANNEL_LOG,
                    SWITCH_LOG_CRIT,
                    "You can have only %d routing fields in the routing key.\n",
                    MAX_ROUTING_KEY_FORMAT_FIELDS);

                return SWITCH_STATUS_FALSE;
            }

            globals.eventRoutingKeyFormatFields[i++] = result;
            result = strtok_r(NULL, delims, &strtok_ptr);
        }

        // Parse new events
        result = strtok_r(globals.event_filter, delims, &strtok_ptr);
        for (globals.number_of_event_filters = 0;
            result != NULL && globals.number_of_event_filters < SWITCH_EVENT_ALL;
            ++globals.number_of_event_filters) {

            globals.event_ids[globals.number_of_event_filters] = parse_fs_event_name(result);
            if (globals.event_ids[globals.number_of_event_filters] < 0) {

                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "The swich event %s was not recognised.\n", result);
                    return SWITCH_STATUS_FALSE;
            }
            result = strtok_r(NULL, delims, &strtok_ptr);
        }

        logConfigValues();

    } else {

        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not parse amqp.conf\n");

        globals.amqpHostnames = "localhost";
        globals.amqpVirtualHost = "/";
        globals.amqpPort = 5672;
        globals.amqpUsername = "guest";
        globals.amqpPassword = "guest";
        globals.amqpHeartbeatSeconds = 0;
        globals.eventExchange = "TAP.Events";
        globals.eventExchangeType = "topic";
        globals.eventRoutingKeyFormat = "NoRoutingKeySpecified";
        globals.eventRoutingKeyFormatFieldsCsv = "";
        globals.commandExchange = "TAP.Commands";
        globals.commandExchangeType = "topic";
        globals.commandBindingKey = "#";
        globals.amqpSendQueueSize = 500;
        globals.circuit_breaker_ms = 10000;
        globals.reconnect_interval_ms = 1000;
        globals.event_ids[0] = SWITCH_EVENT_ALL;
        globals.number_of_event_filters = 1;

        for (i = 0; i < MAX_ROUTING_KEY_FORMAT_FIELDS; i++) {
            globals.eventRoutingKeyFormatFields[i] = NULL;
        }

        logConfigValues();
        return SWITCH_STATUS_FALSE;
    }

    // Unsubscribe all previous events
    switch_event_unbind_callback(mod_AMQP_event_handler);

    if (reload) {
        //TODO: Check if the connection settings to rabbit have changed, and ONLY THEN re-connect to rabbit.
        //(IF only the routing key stuff has changed, then we don't need to re connect to rabbit.)

        // Kill the current connection and signal the reconnection thread to connect using the new settings.
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Config triggering connection bounce\n");

        switch_mutex_lock(globals.amqp_connection_mutex);
        closeAmqpConnection(&globals.amqpEventConn);
        closeAmqpConnection(&globals.amqpCommandConn);
        switch_queue_interrupt_all(globals.amqp_send_queue);
        switch_mutex_unlock(globals.amqp_connection_mutex);
    }

    // Resubscribe new events
    for (i = 0; i < globals.number_of_event_filters; ++i) {

        if (switch_event_bind_removable("AMQP",
                                        globals.event_ids[i],
                                        SWITCH_EVENT_SUBCLASS_ANY,
                                        mod_AMQP_event_handler,
                                        NULL,
                                        &globals.event_nodes[i]) != SWITCH_STATUS_SUCCESS) {

            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
                              "Cannot bind to event handler %d!\n",
                              (int)globals.event_ids[i]);

            return SWITCH_STATUS_FALSE;
        }
    }

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_STANDARD_API(amqp_reload)
{
    do_config(SWITCH_TRUE);
    return SWITCH_STATUS_SUCCESS;
}

static void makeRoutingKey(char routingKey[MAX_AMQP_ROUTING_KEY_LENGTH],
                           switch_event_t* evt,
                           const char* routingKeyFormat,
                           const char* routingKeyEventHeaderNames[])
{
    int i;
    char fieldValues[MAX_ROUTING_KEY_FORMAT_FIELDS][128];

    for (i = 0; i < MAX_ROUTING_KEY_FORMAT_FIELDS; i++) {
        const char * headerName = routingKeyEventHeaderNames[i];

        if (headerName != NULL) {
            char *pCh;

            switch_event_header_t * header = switch_event_get_header_ptr(evt, headerName);
            strncpy(fieldValues[i], header ? header->value : "", sizeof(fieldValues[0]));
            // Note: these headers get freed when the evt gets freed by Freeswitch.

            // Replace dots with underscores so that the routing key does not get corrupted
            for (pCh = fieldValues[i]; *pCh; ++pCh)
                if (*pCh == '.') *pCh = '_';

        } else {
            // Null-terminate the string
            fieldValues[i][0] = '\0';
        }
    };

    //http://stackoverflow.com/questions/14803464/how-do-i-format-a-string-from-an-array-of-values-in-c
    snprintf(routingKey, MAX_AMQP_ROUTING_KEY_LENGTH, routingKeyFormat,
        fieldValues[0],
        fieldValues[1],
        fieldValues[2],
        fieldValues[3],
        fieldValues[4],
        fieldValues[5],
        fieldValues[6],
        fieldValues[7],
        fieldValues[8],
        fieldValues[9]);
}

static void mod_AMQP_event_handler(switch_event_t* evt)
{
    struct amqp_message_t *amqp_message;
    switch_time_t now = switch_time_now();
    static int write_log_message = 1;

    /* If the mod is disabled or the circuit breaker is active, ignore the event */
    if (globals.running == 0 || now < globals.circuit_breaker_reset_time)
        return;
    /* check if error state has changed */
    if ((write_log_message != 0) == (switch_queue_size(globals.amqp_send_queue) > (globals.amqpSendQueueSize / 2)))
    {
        if (write_log_message)
        {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING,
                "AMQP message queue is half full. (Queue capacity %d)",
                globals.amqpSendQueueSize);

        }
        write_log_message = !write_log_message;
    }

    switch_malloc(amqp_message, sizeof(struct amqp_message_t));

    switch_event_serialize_json(evt, &amqp_message->pjson);
    makeRoutingKey(amqp_message->routing_key, evt, globals.eventRoutingKeyFormat, globals.eventRoutingKeyFormatFields);

    /* Queue the message to be sent by the worker thread, errors are reported only once per circuit breaker interval */
    if (switch_queue_trypush(globals.amqp_send_queue, amqp_message) != SWITCH_STATUS_SUCCESS)
    {
        /* Trip the circuit breaker for a short period to stop recurring error messages (time is measured in uS) */
        globals.circuit_breaker_reset_time = now + globals.circuit_breaker_ms * 1000;

        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
                          "Could not queue AMQP message, queue full. Messages will be dropped for %.1fs! (Queue capacity %d)",
                          globals.circuit_breaker_ms / 1000.0,
                          globals.amqpSendQueueSize);
    }
}

// ------------------------------
//   Startup
// ------------------------------
SWITCH_MODULE_LOAD_FUNCTION(mod_amqp_load)
{
    switch_api_interface_t *api_interface;

    *module_interface = switch_loadable_module_create_module_interface(pool, modname);

    memset(&globals, 0, sizeof(globals));
    switch_mutex_init(&globals.amqp_connection_mutex, SWITCH_MUTEX_NESTED, pool);

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Moshi Moshi from mod_amqp!\n Version %s\n", reposha);

    // Subscribe to all events during config
    do_config(SWITCH_FALSE);

    SWITCH_ADD_API(api_interface, "amqp", "amqp API", amqp_reload, "syntax");

    // Create a bounded FIFO queue for sending messages
    if (switch_queue_create(&globals.amqp_send_queue, globals.amqpSendQueueSize, pool) != SWITCH_STATUS_SUCCESS) {

        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
                    "Cannot create send queue of size %d!\n",
                    globals.amqpSendQueueSize);
        return SWITCH_STATUS_GENERR;
    }

    // Setup the sync objects for our reconnection thread (a condition to wait on, and condition requires a mutex)
    globals.running = 1;

    // Start the worker threads
    {
        switch_threadattr_t *thd_attr = NULL;
        switch_threadattr_create(&thd_attr, pool);

        switch_threadattr_detach_set(thd_attr, 0);
        switch_threadattr_stacksize_set(thd_attr, SWITCH_THREAD_STACKSIZE);

        // Start the event send thread. This will set up the initial connection
        if (switch_thread_create(&globals.eventSendThread, thd_attr, mod_amqp_event_consumer_thread, NULL, pool)) {

            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Cannot create 'amqp event sender' thread!\n");
            return SWITCH_STATUS_GENERR;
        }

        // Start the command listener thread. This will set up a separate connection
        if (switch_thread_create(&globals.commandListenerThread, thd_attr, mod_amqp_command_listener_thread, NULL, pool)) {

            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Cannot create 'amqp command listener' thread!\n");
            return SWITCH_STATUS_GENERR;
        }
    }

    return SWITCH_STATUS_SUCCESS;
}

// ------------------------------
//   Shutdown
// ------------------------------
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_amqp_shutdown)
{
    // Unsubscribe from events
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Removing event listeners...\n");
    switch_event_unbind_callback(mod_AMQP_event_handler);

    // Flag the 'send' worker thread to exit, and wake it up
    globals.running = 0;
    switch_queue_interrupt_all(globals.amqp_send_queue);

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Waiting for worker threads to terminate...\n");

    // Join worker threads
    {
        switch_status_t ret;
        switch_thread_join(&ret, globals.eventSendThread);
        switch_thread_join(&ret, globals.commandListenerThread);
    }

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Mod finished shutting down\n");
    return SWITCH_STATUS_SUCCESS;
}

static switch_event_types_t parse_fs_event_name(const char *event_name)
{
    if (strcasecmp(event_name, "SWITCH_EVENT_CUSTOM") == 0) return SWITCH_EVENT_CUSTOM;
    if (strcasecmp(event_name, "SWITCH_EVENT_CLONE") == 0) return SWITCH_EVENT_CLONE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_CREATE") == 0) return SWITCH_EVENT_CHANNEL_CREATE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_DESTROY") == 0) return SWITCH_EVENT_CHANNEL_DESTROY;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_STATE") == 0) return SWITCH_EVENT_CHANNEL_STATE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_CALLSTATE") == 0) return SWITCH_EVENT_CHANNEL_CALLSTATE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_ANSWER") == 0) return SWITCH_EVENT_CHANNEL_ANSWER;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_HANGUP") == 0) return SWITCH_EVENT_CHANNEL_HANGUP;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_HANGUP_COMPLETE") == 0) return SWITCH_EVENT_CHANNEL_HANGUP_COMPLETE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_EXECUTE") == 0) return SWITCH_EVENT_CHANNEL_EXECUTE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_EXECUTE_COMPLETE") == 0) return SWITCH_EVENT_CHANNEL_EXECUTE_COMPLETE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_HOLD") == 0) return SWITCH_EVENT_CHANNEL_HOLD;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_UNHOLD") == 0) return SWITCH_EVENT_CHANNEL_UNHOLD;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_BRIDGE") == 0) return SWITCH_EVENT_CHANNEL_BRIDGE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_UNBRIDGE") == 0) return SWITCH_EVENT_CHANNEL_UNBRIDGE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_PROGRESS") == 0) return SWITCH_EVENT_CHANNEL_PROGRESS;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_PROGRESS_MEDIA") == 0) return SWITCH_EVENT_CHANNEL_PROGRESS_MEDIA;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_OUTGOING") == 0) return SWITCH_EVENT_CHANNEL_OUTGOING;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_PARK") == 0) return SWITCH_EVENT_CHANNEL_PARK;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_UNPARK") == 0) return SWITCH_EVENT_CHANNEL_UNPARK;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_APPLICATION") == 0) return SWITCH_EVENT_CHANNEL_APPLICATION;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_ORIGINATE") == 0) return SWITCH_EVENT_CHANNEL_ORIGINATE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_UUID") == 0) return SWITCH_EVENT_CHANNEL_UUID;
    if (strcasecmp(event_name, "SWITCH_EVENT_API") == 0) return SWITCH_EVENT_API;
    if (strcasecmp(event_name, "SWITCH_EVENT_LOG") == 0) return SWITCH_EVENT_LOG;
    if (strcasecmp(event_name, "SWITCH_EVENT_INBOUND_CHAN") == 0) return SWITCH_EVENT_INBOUND_CHAN;
    if (strcasecmp(event_name, "SWITCH_EVENT_OUTBOUND_CHAN") == 0) return SWITCH_EVENT_OUTBOUND_CHAN;
    if (strcasecmp(event_name, "SWITCH_EVENT_STARTUP") == 0) return SWITCH_EVENT_STARTUP;
    if (strcasecmp(event_name, "SWITCH_EVENT_SHUTDOWN") == 0) return SWITCH_EVENT_SHUTDOWN;
    if (strcasecmp(event_name, "SWITCH_EVENT_PUBLISH") == 0) return SWITCH_EVENT_PUBLISH;
    if (strcasecmp(event_name, "SWITCH_EVENT_UNPUBLISH") == 0) return SWITCH_EVENT_UNPUBLISH;
    if (strcasecmp(event_name, "SWITCH_EVENT_TALK") == 0) return SWITCH_EVENT_TALK;
    if (strcasecmp(event_name, "SWITCH_EVENT_NOTALK") == 0) return SWITCH_EVENT_NOTALK;
    if (strcasecmp(event_name, "SWITCH_EVENT_SESSION_CRASH") == 0) return SWITCH_EVENT_SESSION_CRASH;
    if (strcasecmp(event_name, "SWITCH_EVENT_MODULE_LOAD") == 0) return SWITCH_EVENT_MODULE_LOAD;
    if (strcasecmp(event_name, "SWITCH_EVENT_MODULE_UNLOAD") == 0) return SWITCH_EVENT_MODULE_UNLOAD;
    if (strcasecmp(event_name, "SWITCH_EVENT_DTMF") == 0) return SWITCH_EVENT_DTMF;
    if (strcasecmp(event_name, "SWITCH_EVENT_MESSAGE") == 0) return SWITCH_EVENT_MESSAGE;
    if (strcasecmp(event_name, "SWITCH_EVENT_PRESENCE_IN") == 0) return SWITCH_EVENT_PRESENCE_IN;
    if (strcasecmp(event_name, "SWITCH_EVENT_NOTIFY_IN") == 0) return SWITCH_EVENT_NOTIFY_IN;
    if (strcasecmp(event_name, "SWITCH_EVENT_PRESENCE_OUT") == 0) return SWITCH_EVENT_PRESENCE_OUT;
    if (strcasecmp(event_name, "SWITCH_EVENT_PRESENCE_PROBE") == 0) return SWITCH_EVENT_PRESENCE_PROBE;
    if (strcasecmp(event_name, "SWITCH_EVENT_MESSAGE_WAITING") == 0) return SWITCH_EVENT_MESSAGE_WAITING;
    if (strcasecmp(event_name, "SWITCH_EVENT_MESSAGE_QUERY") == 0) return SWITCH_EVENT_MESSAGE_QUERY;
    if (strcasecmp(event_name, "SWITCH_EVENT_ROSTER") == 0) return SWITCH_EVENT_ROSTER;
    if (strcasecmp(event_name, "SWITCH_EVENT_CODEC") == 0) return SWITCH_EVENT_CODEC;
    if (strcasecmp(event_name, "SWITCH_EVENT_BACKGROUND_JOB") == 0) return SWITCH_EVENT_BACKGROUND_JOB;
    if (strcasecmp(event_name, "SWITCH_EVENT_DETECTED_SPEECH") == 0) return SWITCH_EVENT_DETECTED_SPEECH;
    if (strcasecmp(event_name, "SWITCH_EVENT_DETECTED_TONE") == 0) return SWITCH_EVENT_DETECTED_TONE;
    if (strcasecmp(event_name, "SWITCH_EVENT_PRIVATE_COMMAND") == 0) return SWITCH_EVENT_PRIVATE_COMMAND;
    if (strcasecmp(event_name, "SWITCH_EVENT_HEARTBEAT") == 0) return SWITCH_EVENT_HEARTBEAT;
    if (strcasecmp(event_name, "SWITCH_EVENT_TRAP") == 0) return SWITCH_EVENT_TRAP;
    if (strcasecmp(event_name, "SWITCH_EVENT_ADD_SCHEDULE") == 0) return SWITCH_EVENT_ADD_SCHEDULE;
    if (strcasecmp(event_name, "SWITCH_EVENT_DEL_SCHEDULE") == 0) return SWITCH_EVENT_DEL_SCHEDULE;
    if (strcasecmp(event_name, "SWITCH_EVENT_EXE_SCHEDULE") == 0) return SWITCH_EVENT_EXE_SCHEDULE;
    if (strcasecmp(event_name, "SWITCH_EVENT_RE_SCHEDULE") == 0) return SWITCH_EVENT_RE_SCHEDULE;
    if (strcasecmp(event_name, "SWITCH_EVENT_RELOADXML") == 0) return SWITCH_EVENT_RELOADXML;
    if (strcasecmp(event_name, "SWITCH_EVENT_NOTIFY") == 0) return SWITCH_EVENT_NOTIFY;
    if (strcasecmp(event_name, "SWITCH_EVENT_SEND_MESSAGE") == 0) return SWITCH_EVENT_SEND_MESSAGE;
    if (strcasecmp(event_name, "SWITCH_EVENT_RECV_MESSAGE") == 0) return SWITCH_EVENT_RECV_MESSAGE;
    if (strcasecmp(event_name, "SWITCH_EVENT_REQUEST_PARAMS") == 0) return SWITCH_EVENT_REQUEST_PARAMS;
    if (strcasecmp(event_name, "SWITCH_EVENT_CHANNEL_DATA") == 0) return SWITCH_EVENT_CHANNEL_DATA;
    if (strcasecmp(event_name, "SWITCH_EVENT_GENERAL") == 0) return SWITCH_EVENT_GENERAL;
    if (strcasecmp(event_name, "SWITCH_EVENT_COMMAND") == 0) return SWITCH_EVENT_COMMAND;
    if (strcasecmp(event_name, "SWITCH_EVENT_SESSION_HEARTBEAT") == 0) return SWITCH_EVENT_SESSION_HEARTBEAT;
    if (strcasecmp(event_name, "SWITCH_EVENT_CLIENT_DISCONNECTED") == 0) return SWITCH_EVENT_CLIENT_DISCONNECTED;
    if (strcasecmp(event_name, "SWITCH_EVENT_SERVER_DISCONNECTED") == 0) return SWITCH_EVENT_SERVER_DISCONNECTED;
    if (strcasecmp(event_name, "SWITCH_EVENT_SEND_INFO") == 0) return SWITCH_EVENT_SEND_INFO;
    if (strcasecmp(event_name, "SWITCH_EVENT_RECV_INFO") == 0) return SWITCH_EVENT_RECV_INFO;
    if (strcasecmp(event_name, "SWITCH_EVENT_RECV_RTCP_MESSAGE") == 0) return SWITCH_EVENT_RECV_RTCP_MESSAGE;
    if (strcasecmp(event_name, "SWITCH_EVENT_CALL_SECURE") == 0) return SWITCH_EVENT_CALL_SECURE;
    if (strcasecmp(event_name, "SWITCH_EVENT_NAT") == 0) return SWITCH_EVENT_NAT;
    if (strcasecmp(event_name, "SWITCH_EVENT_RECORD_START") == 0) return SWITCH_EVENT_RECORD_START;
    if (strcasecmp(event_name, "SWITCH_EVENT_RECORD_STOP") == 0) return SWITCH_EVENT_RECORD_STOP;
    if (strcasecmp(event_name, "SWITCH_EVENT_PLAYBACK_START") == 0) return SWITCH_EVENT_PLAYBACK_START;
    if (strcasecmp(event_name, "SWITCH_EVENT_PLAYBACK_STOP") == 0) return SWITCH_EVENT_PLAYBACK_STOP;
    if (strcasecmp(event_name, "SWITCH_EVENT_CALL_UPDATE") == 0) return SWITCH_EVENT_CALL_UPDATE;
    if (strcasecmp(event_name, "SWITCH_EVENT_FAILURE") == 0) return SWITCH_EVENT_FAILURE;
    if (strcasecmp(event_name, "SWITCH_EVENT_SOCKET_DATA") == 0) return SWITCH_EVENT_SOCKET_DATA;
    if (strcasecmp(event_name, "SWITCH_EVENT_MEDIA_BUG_START") == 0) return SWITCH_EVENT_MEDIA_BUG_START;
    if (strcasecmp(event_name, "SWITCH_EVENT_MEDIA_BUG_STOP") == 0) return SWITCH_EVENT_MEDIA_BUG_STOP;
    if (strcasecmp(event_name, "SWITCH_EVENT_CONFERENCE_DATA_QUERY") == 0) return SWITCH_EVENT_CONFERENCE_DATA_QUERY;
    if (strcasecmp(event_name, "SWITCH_EVENT_CONFERENCE_DATA") == 0) return SWITCH_EVENT_CONFERENCE_DATA;
    if (strcasecmp(event_name, "SWITCH_EVENT_CALL_SETUP_REQ") == 0) return SWITCH_EVENT_CALL_SETUP_REQ;
    if (strcasecmp(event_name, "SWITCH_EVENT_CALL_SETUP_RESULT") == 0) return SWITCH_EVENT_CALL_SETUP_RESULT;
    if (strcasecmp(event_name, "SWITCH_EVENT_ALL") == 0) return SWITCH_EVENT_ALL;
    return -1;
}

static const char * fs_event_get_name(switch_event_types_t event_type)
{
    if (event_type == SWITCH_EVENT_CUSTOM) return "SWITCH_EVENT_CUSTOM";
    if (event_type == SWITCH_EVENT_CLONE) return "SWITCH_EVENT_CLONE";
    if (event_type == SWITCH_EVENT_CHANNEL_CREATE) return "SWITCH_EVENT_CHANNEL_CREATE";
    if (event_type == SWITCH_EVENT_CHANNEL_DESTROY) return "SWITCH_EVENT_CHANNEL_DESTROY";
    if (event_type == SWITCH_EVENT_CHANNEL_STATE) return "SWITCH_EVENT_CHANNEL_STATE";
    if (event_type == SWITCH_EVENT_CHANNEL_CALLSTATE) return "SWITCH_EVENT_CHANNEL_CALLSTATE";
    if (event_type == SWITCH_EVENT_CHANNEL_ANSWER) return "SWITCH_EVENT_CHANNEL_ANSWER";
    if (event_type == SWITCH_EVENT_CHANNEL_HANGUP) return "SWITCH_EVENT_CHANNEL_HANGUP";
    if (event_type == SWITCH_EVENT_CHANNEL_HANGUP_COMPLETE) return "SWITCH_EVENT_CHANNEL_HANGUP_COMPLETE";
    if (event_type == SWITCH_EVENT_CHANNEL_EXECUTE) return "SWITCH_EVENT_CHANNEL_EXECUTE";
    if (event_type == SWITCH_EVENT_CHANNEL_EXECUTE_COMPLETE) return "SWITCH_EVENT_CHANNEL_EXECUTE_COMPLETE";
    if (event_type == SWITCH_EVENT_CHANNEL_HOLD) return "SWITCH_EVENT_CHANNEL_HOLD";
    if (event_type == SWITCH_EVENT_CHANNEL_UNHOLD) return "SWITCH_EVENT_CHANNEL_UNHOLD";
    if (event_type == SWITCH_EVENT_CHANNEL_BRIDGE) return "SWITCH_EVENT_CHANNEL_BRIDGE";
    if (event_type == SWITCH_EVENT_CHANNEL_UNBRIDGE) return "SWITCH_EVENT_CHANNEL_UNBRIDGE";
    if (event_type == SWITCH_EVENT_CHANNEL_PROGRESS) return "SWITCH_EVENT_CHANNEL_PROGRESS";
    if (event_type == SWITCH_EVENT_CHANNEL_PROGRESS_MEDIA) return "SWITCH_EVENT_CHANNEL_PROGRESS_MEDIA";
    if (event_type == SWITCH_EVENT_CHANNEL_OUTGOING) return "SWITCH_EVENT_CHANNEL_OUTGOING";
    if (event_type == SWITCH_EVENT_CHANNEL_PARK) return "SWITCH_EVENT_CHANNEL_PARK";
    if (event_type == SWITCH_EVENT_CHANNEL_UNPARK) return "SWITCH_EVENT_CHANNEL_UNPARK";
    if (event_type == SWITCH_EVENT_CHANNEL_APPLICATION) return "SWITCH_EVENT_CHANNEL_APPLICATION";
    if (event_type == SWITCH_EVENT_CHANNEL_ORIGINATE) return "SWITCH_EVENT_CHANNEL_ORIGINATE";
    if (event_type == SWITCH_EVENT_CHANNEL_UUID) return "SWITCH_EVENT_CHANNEL_UUID";
    if (event_type == SWITCH_EVENT_API) return "SWITCH_EVENT_API";
    if (event_type == SWITCH_EVENT_LOG) return "SWITCH_EVENT_LOG";
    if (event_type == SWITCH_EVENT_INBOUND_CHAN) return "SWITCH_EVENT_INBOUND_CHAN";
    if (event_type == SWITCH_EVENT_OUTBOUND_CHAN) return "SWITCH_EVENT_OUTBOUND_CHAN";
    if (event_type == SWITCH_EVENT_STARTUP) return "SWITCH_EVENT_STARTUP";
    if (event_type == SWITCH_EVENT_SHUTDOWN) return "SWITCH_EVENT_SHUTDOWN";
    if (event_type == SWITCH_EVENT_PUBLISH) return "SWITCH_EVENT_PUBLISH";
    if (event_type == SWITCH_EVENT_UNPUBLISH) return "SWITCH_EVENT_UNPUBLISH";
    if (event_type == SWITCH_EVENT_TALK) return "SWITCH_EVENT_TALK";
    if (event_type == SWITCH_EVENT_NOTALK) return "SWITCH_EVENT_NOTALK";
    if (event_type == SWITCH_EVENT_SESSION_CRASH) return "SWITCH_EVENT_SESSION_CRASH";
    if (event_type == SWITCH_EVENT_MODULE_LOAD) return "SWITCH_EVENT_MODULE_LOAD";
    if (event_type == SWITCH_EVENT_MODULE_UNLOAD) return "SWITCH_EVENT_MODULE_UNLOAD";
    if (event_type == SWITCH_EVENT_DTMF) return "SWITCH_EVENT_DTMF";
    if (event_type == SWITCH_EVENT_MESSAGE) return "SWITCH_EVENT_MESSAGE";
    if (event_type == SWITCH_EVENT_PRESENCE_IN) return "SWITCH_EVENT_PRESENCE_IN";
    if (event_type == SWITCH_EVENT_NOTIFY_IN) return "SWITCH_EVENT_NOTIFY_IN";
    if (event_type == SWITCH_EVENT_PRESENCE_OUT) return "SWITCH_EVENT_PRESENCE_OUT";
    if (event_type == SWITCH_EVENT_PRESENCE_PROBE) return "SWITCH_EVENT_PRESENCE_PROBE";
    if (event_type == SWITCH_EVENT_MESSAGE_WAITING) return "SWITCH_EVENT_MESSAGE_WAITING";
    if (event_type == SWITCH_EVENT_MESSAGE_QUERY) return "SWITCH_EVENT_MESSAGE_QUERY";
    if (event_type == SWITCH_EVENT_ROSTER) return "SWITCH_EVENT_ROSTER";
    if (event_type == SWITCH_EVENT_CODEC) return "SWITCH_EVENT_CODEC";
    if (event_type == SWITCH_EVENT_BACKGROUND_JOB) return "SWITCH_EVENT_BACKGROUND_JOB";
    if (event_type == SWITCH_EVENT_DETECTED_SPEECH) return "SWITCH_EVENT_DETECTED_SPEECH";
    if (event_type == SWITCH_EVENT_DETECTED_TONE) return "SWITCH_EVENT_DETECTED_TONE";
    if (event_type == SWITCH_EVENT_PRIVATE_COMMAND) return "SWITCH_EVENT_PRIVATE_COMMAND";
    if (event_type == SWITCH_EVENT_HEARTBEAT) return "SWITCH_EVENT_HEARTBEAT";
    if (event_type == SWITCH_EVENT_TRAP) return "SWITCH_EVENT_TRAP";
    if (event_type == SWITCH_EVENT_ADD_SCHEDULE) return "SWITCH_EVENT_ADD_SCHEDULE";
    if (event_type == SWITCH_EVENT_DEL_SCHEDULE) return "SWITCH_EVENT_DEL_SCHEDULE";
    if (event_type == SWITCH_EVENT_EXE_SCHEDULE) return "SWITCH_EVENT_EXE_SCHEDULE";
    if (event_type == SWITCH_EVENT_RE_SCHEDULE) return "SWITCH_EVENT_RE_SCHEDULE";
    if (event_type == SWITCH_EVENT_RELOADXML) return "SWITCH_EVENT_RELOADXML";
    if (event_type == SWITCH_EVENT_NOTIFY) return "SWITCH_EVENT_NOTIFY";
    if (event_type == SWITCH_EVENT_SEND_MESSAGE) return "SWITCH_EVENT_SEND_MESSAGE";
    if (event_type == SWITCH_EVENT_RECV_MESSAGE) return "SWITCH_EVENT_RECV_MESSAGE";
    if (event_type == SWITCH_EVENT_REQUEST_PARAMS) return "SWITCH_EVENT_REQUEST_PARAMS";
    if (event_type == SWITCH_EVENT_CHANNEL_DATA) return "SWITCH_EVENT_CHANNEL_DATA";
    if (event_type == SWITCH_EVENT_GENERAL) return "SWITCH_EVENT_GENERAL";
    if (event_type == SWITCH_EVENT_COMMAND) return "SWITCH_EVENT_COMMAND";
    if (event_type == SWITCH_EVENT_SESSION_HEARTBEAT) return "SWITCH_EVENT_SESSION_HEARTBEAT";
    if (event_type == SWITCH_EVENT_CLIENT_DISCONNECTED) return "SWITCH_EVENT_CLIENT_DISCONNECTED";
    if (event_type == SWITCH_EVENT_SERVER_DISCONNECTED) return "SWITCH_EVENT_SERVER_DISCONNECTED";
    if (event_type == SWITCH_EVENT_SEND_INFO) return "SWITCH_EVENT_SEND_INFO";
    if (event_type == SWITCH_EVENT_RECV_INFO) return "SWITCH_EVENT_RECV_INFO";
    if (event_type == SWITCH_EVENT_RECV_RTCP_MESSAGE) return "SWITCH_EVENT_RECV_RTCP_MESSAGE";
    if (event_type == SWITCH_EVENT_CALL_SECURE) return "SWITCH_EVENT_CALL_SECURE";
    if (event_type == SWITCH_EVENT_NAT) return "SWITCH_EVENT_NAT";
    if (event_type == SWITCH_EVENT_RECORD_START) return "SWITCH_EVENT_RECORD_START";
    if (event_type == SWITCH_EVENT_RECORD_STOP) return "SWITCH_EVENT_RECORD_STOP";
    if (event_type == SWITCH_EVENT_PLAYBACK_START) return "SWITCH_EVENT_PLAYBACK_START";
    if (event_type == SWITCH_EVENT_PLAYBACK_STOP) return "SWITCH_EVENT_PLAYBACK_STOP";
    if (event_type == SWITCH_EVENT_CALL_UPDATE) return "SWITCH_EVENT_CALL_UPDATE";
    if (event_type == SWITCH_EVENT_FAILURE) return "SWITCH_EVENT_FAILURE";
    if (event_type == SWITCH_EVENT_SOCKET_DATA) return "SWITCH_EVENT_SOCKET_DATA";
    if (event_type == SWITCH_EVENT_MEDIA_BUG_START) return "SWITCH_EVENT_MEDIA_BUG_START";
    if (event_type == SWITCH_EVENT_MEDIA_BUG_STOP) return "SWITCH_EVENT_MEDIA_BUG_STOP";
    if (event_type == SWITCH_EVENT_CONFERENCE_DATA_QUERY) return "SWITCH_EVENT_CONFERENCE_DATA_QUERY";
    if (event_type == SWITCH_EVENT_CONFERENCE_DATA) return "SWITCH_EVENT_CONFERENCE_DATA";
    if (event_type == SWITCH_EVENT_CALL_SETUP_REQ) return "SWITCH_EVENT_CALL_SETUP_REQ";
    if (event_type == SWITCH_EVENT_CALL_SETUP_RESULT) return "SWITCH_EVENT_CALL_SETUP_RESULT";
    if (event_type == SWITCH_EVENT_ALL) return "SWITCH_EVENT_ALL";
    return "<unknown event type>";
}



/******************************************************************
 *  The functions below are called from only the consumer thread
 *  with one exception: closeConnection is called within a mutex
 *  when the config is reloaded to force a reconnect.
 */

static switch_status_t mod_amqp_send(struct amqp_message_t *msg, const char *exchange);

static int log_if_error(int x, char const *context)
{
    if (x < 0) {

        const char *errstr = amqp_error_string2(-x);

        char logMessage[MAX_LOG_MESSAGE_SIZE];
        snprintf(logMessage, MAX_LOG_MESSAGE_SIZE, "%s: %s\n", context, errstr);
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "%s\n", logMessage);

        return -1;

    } else
        return 0;
}

static int log_if_amqp_error(amqp_rpc_reply_t x, char const *context)
{
    char logMessage[MAX_LOG_MESSAGE_SIZE];

    switch (x.reply_type) {
    case AMQP_RESPONSE_NORMAL:
        return 0;

    case AMQP_RESPONSE_NONE:
        snprintf(logMessage, MAX_LOG_MESSAGE_SIZE, "%s: missing RPC reply type!\n", context);
        break;

    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
        snprintf(logMessage, MAX_LOG_MESSAGE_SIZE, "%s: %s\n", context, amqp_error_string2(x.library_error));
        break;

    case AMQP_RESPONSE_SERVER_EXCEPTION:
        switch (x.reply.id) {
        case AMQP_CONNECTION_CLOSE_METHOD: {
            amqp_connection_close_t *m = (amqp_connection_close_t *) x.reply.decoded;
            snprintf(logMessage, MAX_LOG_MESSAGE_SIZE, "%s: server connection error %d, message: %.*s\n",
                context,
                m->reply_code,
                (int) m->reply_text.len, (char *) m->reply_text.bytes);
            break;
                                           }
        case AMQP_CHANNEL_CLOSE_METHOD: {
            amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
            snprintf(logMessage, MAX_LOG_MESSAGE_SIZE, "%s: server channel error %d, message: %.*s\n",
                context,
                m->reply_code,
                (int) m->reply_text.len, (char *) m->reply_text.bytes);
            break;
                                        }
        default:
            snprintf(logMessage, MAX_LOG_MESSAGE_SIZE, "%s: unknown server error, method id 0x%08X\n", context, x.reply.id);
            break;
        }
        break;

    default:
            snprintf(logMessage, MAX_LOG_MESSAGE_SIZE, "%s: unknown reply_type: %d \n", context, x.reply_type);
            break;
    }

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "%s\n", logMessage);
    return -1;
}

// MUST BE CALLED IN THE MUTEX
static void closeAmqpConnection(amqp_connection_state_t *conn)
{
    amqp_connection_state_t amqpConn = *conn;
    *conn = NULL;

    if (amqpConn == NULL)
        return;

    log_if_amqp_error(amqp_channel_close(amqpConn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
    log_if_amqp_error(amqp_connection_close(amqpConn, AMQP_REPLY_SUCCESS), "Closing connection");
    log_if_error(amqp_destroy_connection(amqpConn), "Ending connection");
}

static switch_status_t openAmqpConnection(amqp_connection_state_t *connection,
                                          char *exchangeName,
                                          char *exchangeType)
{
    switch_status_t ret;

    amqp_boolean_t passive = 0;
    amqp_boolean_t durable = 1;
    int channel_max = 0;
    int frame_max = 131072;
    amqp_table_t loginProperties;
    amqp_table_entry_t loginTableEntries[5];
    char hostname[64];
    int bHasHostname;

    /* Set up meta data for connection */
    bHasHostname = gethostname(hostname, sizeof(hostname)) == 0;

    loginProperties.num_entries = sizeof(loginTableEntries)/sizeof(*loginTableEntries);
    loginProperties.entries = loginTableEntries;

    loginTableEntries[0].key = amqp_cstring_bytes("x_" CUSTOM_ATTR "HostMachineName");
    loginTableEntries[0].value.kind = AMQP_FIELD_KIND_BYTES;
    loginTableEntries[0].value.value.bytes = amqp_cstring_bytes(bHasHostname ? hostname : "(unknown)");

    loginTableEntries[1].key = amqp_cstring_bytes("x_" CUSTOM_ATTR "ProcessDescription");
    loginTableEntries[1].value.kind = AMQP_FIELD_KIND_BYTES;
    loginTableEntries[1].value.value.bytes = amqp_cstring_bytes("FreeSwitch");

    loginTableEntries[2].key = amqp_cstring_bytes("x_" CUSTOM_ATTR "ProcessType");
    loginTableEntries[2].value.kind = AMQP_FIELD_KIND_BYTES;
    loginTableEntries[2].value.value.bytes = amqp_cstring_bytes("TAP");

    loginTableEntries[3].key = amqp_cstring_bytes("x_" CUSTOM_ATTR "ProcessBuildVersion");
    loginTableEntries[3].value.kind = AMQP_FIELD_KIND_BYTES;
    loginTableEntries[3].value.value.bytes = amqp_cstring_bytes(reposha);

    loginTableEntries[4].key = amqp_cstring_bytes("x_" CUSTOM_ATTR "Liquid_ProcessBuildBornOn");
    loginTableEntries[4].value.kind = AMQP_FIELD_KIND_BYTES;
    loginTableEntries[4].value.value.bytes = amqp_cstring_bytes(__DATE__ " " __TIME__);

    ret = SWITCH_STATUS_GENERR;
    do // Try..
    {
        amqp_rpc_reply_t status;
        int connection_status = -1;
        amqp_socket_t *socket = NULL;
        const char *host;
        const char *host_delims = ",";
        char *strtok_ptr;

        amqp_connection_state_t newConnection = amqp_new_connection();
        socket = amqp_tcp_socket_new(newConnection);
        if (!socket) {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not create TCP socket\n");
            break;
        }

        // amqpHostnames is a comma separated list, try to connect to each host in turn until success
        host = strtok_r(globals.amqpHostnames, host_delims, &strtok_ptr);
        while (host != NULL) {

            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,
                "Trying to connect to AMQP broker %s:%d\n",
                host, globals.amqpPort);

            connection_status = amqp_socket_open(socket, host, globals.amqpPort);

            if (connection_status == 0) { // Success, no need to try other hostnames
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,
                    "Opened socket connection to AMQP broker %s:%d\n",
                    host, globals.amqpPort);
                break;

            } else {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING,
                    "Could not open socket connection to AMQP broker %s:%d\n",
                host, globals.amqpPort);
            }
            host = strtok_r(NULL, host_delims, &strtok_ptr);
        }
        if (connection_status != 0) { // Failure

            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT,
                "Could not connect to any AMQP brokers (%s) on port %d\n",
                globals.amqpHostnames, globals.amqpPort);
            break;
        }

        // We have a connection, now log in
        status = amqp_login_with_properties(newConnection,
                                            globals.amqpVirtualHost,
                                            channel_max,
                                            frame_max,
                                            globals.amqpHeartbeatSeconds,
                                            &loginProperties,
                                            AMQP_SASL_METHOD_PLAIN,
                                            globals.amqpUsername,
                                            globals.amqpPassword);

        if (log_if_amqp_error(status, "Logging in"))
            break;

        // Open a channel (1). This is fairly standard
        amqp_channel_open(newConnection, 1);
        if (log_if_amqp_error(amqp_get_rpc_reply(newConnection), "Opening channel"))
            break;

        // Ensure that the exchange exists, and is of the correct type
        amqp_exchange_declare(newConnection, 1,
                            amqp_cstring_bytes(exchangeName),
                            amqp_cstring_bytes(exchangeType),
                            passive,
                            durable,
                            amqp_empty_table);
        if (log_if_amqp_error(amqp_get_rpc_reply(newConnection), "Declaring exchange"))
            break;

        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE,
                    "Connected to AMQP broker - %s:%d\n",
                    globals.amqpHostnames, globals.amqpPort);

        // Swap in new connection; if there was an old connection, destroy it
        {
            amqp_connection_state_t oldConnection = *connection;
            *connection = newConnection;

            if (oldConnection) {
                amqp_destroy_connection(oldConnection);
            }
        }
        ret = SWITCH_STATUS_SUCCESS;
    }
    while (0); // ..finally

    return ret;
}

static switch_status_t bounce_connection()
{
    switch_status_t status;

    switch_mutex_lock(globals.amqp_connection_mutex);
    {
        if (globals.amqpEventConn != NULL)
            closeAmqpConnection(&globals.amqpEventConn);

        status = openAmqpConnection(&globals.amqpEventConn,
                                    globals.eventExchange,
                                    globals.eventExchangeType);
    }
    switch_mutex_unlock(globals.amqp_connection_mutex);

    return status;
}

static void * SWITCH_THREAD_FUNC mod_amqp_event_consumer_thread(switch_thread_t *thread, void *params)
{
    struct amqp_message_t *msg = NULL;

    // MAIN THREAD LOOP

    // While the module is loaded
    while (globals.running) {

        // Ensure we have an AMQP connection
        if (globals.amqpEventConn == NULL) {

            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Amqp no connection- reconnecting...\n");
            if (bounce_connection() != SWITCH_STATUS_SUCCESS) {

                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING,
                    "Amqp connection bounce failed, sleeping for %dms\n",
                    globals.reconnect_interval_ms);
                switch_sleep(globals.reconnect_interval_ms * 1000);
            }
            else
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Amqp reconnect successful- connected\n");

            continue;
        }

        // Ensure we have a message to send
        if (msg == NULL) {

            if (switch_queue_pop(globals.amqp_send_queue, (void**)&msg) != SWITCH_STATUS_SUCCESS) {

                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Amqp queue interrupted, checking status and waiting again\n");
                // Interrupted: check we're still running and connected
                continue;
            }
        }

        // Try to send the message (if we have one)
        if (msg != NULL) {

            long times[TIME_STATS_TO_AGGREGATE];
            static unsigned int thistime = 0;
            switch_time_t start = switch_time_now();

            switch (mod_amqp_send(msg, globals.eventExchange)) {

            case SWITCH_STATUS_SUCCESS:
                // Success: prepare for next message
                switch_safe_free(msg->pjson);
                switch_safe_free(msg);
                msg = NULL;
                break;

            case SWITCH_STATUS_NOT_INITALIZED:
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Send failed with 'not initialised'\n");
                break;

            case SWITCH_STATUS_SOCKERR:
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Send failed with 'socket error'\n");
                break;

            default:
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Send failed with a generic error\n");

                // Send failed and closed the connection; reconnect will happen at the beginning of the loop
                // NB: do we need a delay here to prevent a fast reconnect-send-fail loop?
                break;
            }

            times[thistime++] = switch_time_now() - start;
            if (thistime >= TIME_STATS_TO_AGGREGATE) {

                int i;
                long min_time, max_time, avg_time;

                /* Calculate aggregate times */
                min_time = max_time = avg_time = times[0];
                for (i = 1; i < TIME_STATS_TO_AGGREGATE; ++i) {

                    avg_time += times[i];
                    if (times[i] < min_time) min_time = times[i];
                    if (times[i] > max_time) max_time = times[i];
                }
                avg_time /= TIME_STATS_TO_AGGREGATE;
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Microseconds to send last %d messages: Min %ld  Max %ld  Avg %ld\n",
                    TIME_STATS_TO_AGGREGATE, min_time, max_time, avg_time);
                thistime = 0;
            }
        }
    }
    // Abort the current message
    if (msg != NULL) {

        switch_safe_free(msg->pjson);
        switch_safe_free(msg);
    }

    // SHUTTING DOWN THE THREAD

    // Disconnect
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Closing AMQP socket...\n");

    switch_mutex_lock(globals.amqp_connection_mutex);
    closeAmqpConnection(&globals.amqpEventConn);
    switch_mutex_unlock(globals.amqp_connection_mutex);

    // Empty the queue so as not to leak any remaining messages
    {
        switch_status_t status = APR_SUCCESS;

        // Popping more than queueSize msgs means a thread is still pushing - this is futile so give up
        // EAGAIN => queue is empty; EOF => queue was terminated
        int to_pop = globals.amqpSendQueueSize;
        while (to_pop > 0 && (status != APR_EAGAIN && status != APR_EOF)) {

            // trypop does not block, we want to hit the end of the queue
            status = switch_queue_trypop(globals.amqp_send_queue, (void**)&msg);

            if (status == APR_SUCCESS)
                --to_pop;

            if (msg != NULL) {

                switch_safe_free(msg->pjson);
                switch_safe_free(msg);
            }
            msg = NULL;
        }
    }

    // Terminate the thread
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Event sender thread stopped\n");
    switch_thread_exit(thread, SWITCH_STATUS_SUCCESS);
    return NULL;
}

static switch_status_t mod_amqp_send(struct amqp_message_t *msg, const char *exchange)
{
    switch_status_t ret = SWITCH_STATUS_GENERR;

    // We received something, so send it
    if (globals.amqpEventConn == NULL) {

        // we couldn't send the message.
        ret = SWITCH_STATUS_NOT_INITALIZED;

    } else {

        switch_mutex_lock(globals.amqp_connection_mutex);
        // MUTEX LOCKED ==========================================================================
        {
            amqp_table_entry_t messageTableEntries[1];
            int status;

            amqp_basic_properties_t props;
            props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG | AMQP_BASIC_TIMESTAMP_FLAG | AMQP_BASIC_HEADERS_FLAG;
            props.content_type = amqp_cstring_bytes("text/json");
            props.delivery_mode = 1; // non persistent delivery mode
            props.timestamp = (uint64_t)time(NULL);

            props.headers.num_entries = 1;
            props.headers.entries = messageTableEntries;

            messageTableEntries[0].key = amqp_cstring_bytes("x_Liquid_MessageSentTimeStamp");
            messageTableEntries[0].value.kind = AMQP_FIELD_KIND_TIMESTAMP;
            messageTableEntries[0].value.value.u64 = (uint64_t)switch_micro_time_now();

            status = amqp_basic_publish(
                    globals.amqpEventConn,
                    1,
                    amqp_cstring_bytes(exchange),
                    amqp_cstring_bytes(msg->routing_key),
                    0,
                    0,
                    &props,
                    amqp_cstring_bytes(msg->pjson));

            if (status < 0) {
                log_if_error(status, "Publishing");

                //This is bad, we couldn't send the message. Clear up any connection
                closeAmqpConnection(&globals.amqpEventConn);
                ret = SWITCH_STATUS_SOCKERR;

            } else {
                ret = SWITCH_STATUS_SUCCESS;
            }
        }
        switch_mutex_unlock(globals.amqp_connection_mutex);
        // MUTEX UNLOCKED ==========================================================================
    }

    return ret;
}


static void * SWITCH_THREAD_FUNC mod_amqp_command_listener_thread(switch_thread_t *thread, void *params)
{
    while (globals.running) {

        amqp_bytes_t queueName;

        // Ensure we have an AMQP connection
        if (globals.amqpCommandConn == NULL) {

            // reconnect and log in
            openAmqpConnection(&globals.amqpCommandConn,
                               globals.commandExchange,
                               globals.commandExchangeType);
        }

        // Ensure we have a queue
        {
            amqp_queue_declare_ok_t *r =
                amqp_queue_declare(globals.amqpCommandConn, // state
                                   1,                       // channel
                                   amqp_empty_bytes,        // queue name
                                   0, 0,                    // passive, durable
                                   0, 1,                    // exclusive, auto-delete
                                   amqp_empty_table);       // args

            if (log_if_amqp_error(amqp_get_rpc_reply(globals.amqpCommandConn), "Declaring queue")) {
                break;
            }

            queueName = amqp_bytes_malloc_dup(r->queue);

            if (queueName.bytes == NULL) {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
                                  "Out of memory while copying queue name");
                break;
            }
        }

        // Bind the queue to the exchange
        amqp_queue_bind(globals.amqpCommandConn,                       // state
                        1,                                             // channel
                        queueName,                                     // queue
                        amqp_cstring_bytes(globals.commandExchange),   // exchange
                        amqp_cstring_bytes(globals.commandBindingKey), // routing key
                        amqp_empty_table);                             // args

        if (log_if_amqp_error(amqp_get_rpc_reply(globals.amqpCommandConn), "Binding queue")) {
            break;
        }

        // Start a consumer
        amqp_basic_consume(globals.amqpCommandConn,         // state
                           1,                               // channel
                           queueName,                       // queue
                           amqp_empty_bytes,                // consumer tag
                           0, 1, 0,                         // no_local, no_ack, exclusive
                           amqp_empty_table);               // args

        if (log_if_amqp_error(amqp_get_rpc_reply(globals.amqpCommandConn), "Consuming")) {
            break;
        }

        while (globals.running) {

            amqp_rpc_reply_t res;
            amqp_envelope_t envelope;
            struct timeval timeout = {0};
            char command[1024];
            enum ECommandFormat {
                COMMAND_FORMAT_UNKNOWN,
                COMMAND_FORMAT_PLAINTEXT
            } commandFormat = COMMAND_FORMAT_PLAINTEXT;

            amqp_maybe_release_buffers(globals.amqpCommandConn);

            timeout.tv_usec = 500 * 1000;
            res = amqp_consume_message(globals.amqpCommandConn, &envelope, &timeout, 0);

            if (res.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION) {
                if (res.library_error == AMQP_STATUS_UNEXPECTED_STATE) {
                    // Unexpected frame. Discard it then continue
                    amqp_frame_t decoded_frame;
                    amqp_simple_wait_frame(globals.amqpCommandConn, &decoded_frame);
                }

                if (res.library_error == AMQP_STATUS_TIMEOUT) {
                    // nop
                }

                // Try again
                continue;
            }

            if (res.reply_type != AMQP_RESPONSE_NORMAL) {
                break;
            }

            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,
                              "Delivery:%u, exchange:%.*s routingkey:%.*s\n",
                              (unsigned) envelope.delivery_tag,
                              (int) envelope.exchange.len, (char *) envelope.exchange.bytes,
                              (int) envelope.routing_key.len, (char *) envelope.routing_key.bytes);

            if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {

                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,
                                  "Content-type: %.*s\n",
                                  (int) envelope.message.properties.content_type.len,
                                  (char *) envelope.message.properties.content_type.bytes);

                if (strncasecmp("text/plain", envelope.message.properties.content_type.bytes, strlen("text/plain")) == 0) {
                    commandFormat = COMMAND_FORMAT_PLAINTEXT;
                } else {
                    commandFormat = COMMAND_FORMAT_UNKNOWN;
                }
            }

            if (commandFormat == COMMAND_FORMAT_PLAINTEXT) {

                // Convert amqp bytes to c-string
                snprintf(command, sizeof(command), "%.*s",
                        (int) envelope.message.body.len,
                        (char *) envelope.message.body.bytes);

                // Execute the command
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Executing: %s\n", command);
                {
                    switch_stream_handle_t stream = { 0 }; // Collects the command output
                    SWITCH_STANDARD_STREAM(stream);

                    if (switch_console_execute(command, 0, &stream) != SWITCH_STATUS_SUCCESS) {
                        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING,
                                          "Remote command failed:\n%s\n",
                                          (char *) stream.data);
                    } else {
                        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,
                                          "Remote command succeeded:\n%s\n",
                                          (char *) stream.data);
                    }

                    switch_safe_free(stream.data);
                }
            }

            // Tidy up
            amqp_destroy_envelope(&envelope);
        }
        amqp_bytes_free(queueName);

        if (log_if_amqp_error(amqp_channel_close(globals.amqpCommandConn, 1, AMQP_REPLY_SUCCESS),
                              "Closing channel"))
            continue;

        if (log_if_amqp_error(amqp_connection_close(globals.amqpCommandConn, AMQP_REPLY_SUCCESS),
                              "Closing connection"))
            continue;

        log_if_error(amqp_destroy_connection(globals.amqpCommandConn),
                          "Ending connection");
    }

    // Terminate the thread
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Command listener thread stopped\n");
    switch_thread_exit(thread, SWITCH_STATUS_SUCCESS);
    return NULL;
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
