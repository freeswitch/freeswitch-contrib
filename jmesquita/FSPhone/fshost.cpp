#include "fshost.h"

/* Declare it globally */
FSHost g_FSHost;

FSHost::FSHost(QObject *parent) :
    QThread(parent)
{
    /* Initialize libs & globals */
    printf("Initializing globals...\n");
    switch_core_setrlimits();
    switch_core_set_globals();
}

void FSHost::run(void)
{
    switch_core_flag_t flags = SCF_USE_SQL;
    const char *err = NULL;
    switch_bool_t console = SWITCH_FALSE;
    switch_status_t destroy_status;

    /* If you need to override configuration directories, you need to change them in the SWITCH_GLOBAL_dirs global structure */
    printf("Initializing core...\n");
    /* Initialize the core and load modules, that will startup FS completely */
    if (switch_core_init_and_modload(flags, console, &err) != SWITCH_STATUS_SUCCESS) {
            fprintf(stderr, "Failed to initialize FreeSWITCH's core: %s\n", err);
            return;
    }

    printf("Everything OK, Entering runtime loop.\n");

    if (switch_event_bind("FSHost", SWITCH_EVENT_ALL, SWITCH_EVENT_SUBCLASS_ANY, eventHandlerCallback, NULL) != SWITCH_STATUS_SUCCESS) {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind!\n");
            printf("Something went really wrong while binding to events...");
    }

    emit ready();
    /* Go into the runtime loop. If the argument is true, this basically sets runtime.running = 1 and loops while that is set
     * If its false, it initializes the libedit for the console, then does the same thing
     */
    switch_core_runtime_loop(!console);
    fflush(stdout);


    switch_event_unbind_callback(eventHandlerCallback);
    /* When the runtime loop exits, its time to shutdown */
    destroy_status = switch_core_destroy();
    if (destroy_status == SWITCH_STATUS_SUCCESS)
    {
        printf("We have properly shutdown the core.\n");
    }
}

void FSHost::generalEventHandler(switch_event_t *event)
{
    switch(event->event_id) {
    case SWITCH_EVENT_CUSTOM:
        {
            if (strcmp(event->subclass_name, "portaudio::ringing") == 0)
            {
                emit ringing(switch_event_get_header_nil(event, "call_id"), switch_event_get_header_nil(event, "Caller-Caller-ID-Number"), switch_event_get_header_nil(event, "Caller-Caller-ID-Name"));
            }
            else
            {
                printf("We got a not treated custom event: %s\n", (!zstr(event->subclass_name) ? event->subclass_name : "NULL"));
            }
            break;
        }
    default:
        {
            printf("Untreated event: %s -> %s\n", switch_event_name(event->event_id), (!zstr(event->subclass_name) ? event->subclass_name : "NULL"));
        }
    }
}

switch_status_t FSHost::sendCmd(const char *cmd, const char *args, QString *res)
{
    switch_status_t status = SWITCH_STATUS_FALSE;
    switch_stream_handle_t stream = { 0 };
    SWITCH_STANDARD_STREAM(stream);
    status = switch_api_execute(cmd, args, NULL, &stream);
    *res = switch_str_nil((char *) stream.data);

    return status;
}
