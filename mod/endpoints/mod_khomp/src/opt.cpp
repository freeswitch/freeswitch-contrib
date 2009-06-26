#include "opt.h"

#include "globals.h"


bool Opt::_debug;
char * Opt::_ip;
int Opt::_port;

char * Opt::_dialplan;
char * Opt::_codec_string;
char * Opt::_codec_order[SWITCH_MAX_CODECS];
int Opt::_codec_order_last;
char * Opt::_codec_rates_string;
char * Opt::_codec_rates[SWITCH_MAX_CODECS];
int Opt::_codec_rates_last;


SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_dialplan, Opt::_dialplan);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_codec_string, Opt::_codec_string);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_codec_rates_string, Opt::_codec_rates_string);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_ip, Opt::_ip);



void Opt::initialize(void) 
{ 
    Globals::_options.add(config_option("debug", _debug, false));
    //Globals::options.add(config_option("port", _port, ???));
    //Globals::options.add(config_option("ip", _ip, ???));
    //Globals::options.add(config_option("dialplan", _dialplan, ???));
    //Globals::options.add(config_option("ip", _ip, ???));
}

void Opt::obtain(void)
{
    load_configuration("khomp.conf", NULL);
}

void Opt::load_configuration(const char *file_name, const char **section, bool show_errors)
{
    switch_xml_t cfg, xml, settings, param;

    switch_mutex_init(&Globals::_mutex, SWITCH_MUTEX_NESTED, Globals::_module_pool);
    if (!(xml = switch_xml_open_cfg(file_name, &cfg, NULL))) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", file_name);
        return;
    }

    if ((settings = switch_xml_child(cfg, "settings"))) {
        for (param = switch_xml_child(settings, "param"); param; param = param->next) {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            if (!strcmp(var, "_debug")) {
                Globals::_options.process(var, val);
                //_debug = atoi(val);
            } else if (!strcmp(var, "port")) {
                _port = atoi(val);
            } else if (!strcmp(var, "ip")) {
                set_global_ip(val);
            //} else if (!strcmp(var, "codec-master")) {
                //if (!strcasecmp(val, "us")) {
                //    switch_set_flag(&globals, GFLAG_MY_CODEC_PREFS);
                //}
            } else if (!strcmp(var, "dialplan")) {
                set_global_dialplan(val);
            } else if (!strcmp(var, "codec-prefs")) {
                set_global_codec_string(val);
                _codec_order_last = switch_separate_string(_codec_string, ',', _codec_order, SWITCH_MAX_CODECS);
            } else if (!strcmp(var, "codec-rates")) {
                set_global_codec_rates_string(val);
                _codec_rates_last = switch_separate_string(_codec_rates_string, ',', _codec_rates, SWITCH_MAX_CODECS);
            }
        }
    }

    if (!_dialplan) {
        set_global_dialplan("default");
    }

    if (!_port) {
        _port = 4569;
    }

    switch_xml_free(xml);

}
void Opt::clean_configuration(void){}


