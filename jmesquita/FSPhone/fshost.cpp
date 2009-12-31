#include "fshost.h"

FSHost::FSHost(QObject *parent) :
    QThread(parent)
{
    /* Initialize libs & globals */
    printf("Initializing globals...\n");
    switch_core_setrlimits();
    switch_core_set_globals();
}

FSHost::~FSHost()
{
    switch_status_t destroy_status;
    /* When the runtime loop exits, its time to shutdown */
    destroy_status = switch_core_destroy();
    wait();
}

void FSHost::run(void)
{
    switch_core_flag_t flags = SCF_USE_SQL;
    const char *err = NULL;
    switch_bool_t console = SWITCH_FALSE;

    /* If you need to override configuration directories, you need to change them in the SWITCH_GLOBAL_dirs global structure */
    printf("Initializing core...\n");
    /* Initialize the core and load modules, that will startup FS completely */
    if (switch_core_init_and_modload(flags, console, &err) != SWITCH_STATUS_SUCCESS) {
            fprintf(stderr, "Failed to initialize FreeSWITCH's core: %s\n", err);
            return;
    }

    printf("Everything OK, Entering runtime loop.\n");
    emit ready();
    /* Go into the runtime loop. If the argument is true, this basically sets runtime.running = 1 and loops while that is set
     * If its false, it initializes the libedit for the console, then does the same thing
     */
    switch_core_runtime_loop(!console);
    fflush(stdout);
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
