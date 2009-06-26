#include <string>

#include <k3l.h>

/* if using full k3l.h (for softpbx), version already defined. */
#ifndef k3lApiMajorVersion
# include <k3lVersion.h>
#endif

#ifdef __GNUC_PREREQ
#if __GNUC_PREREQ(4,3)
#include <cstring>
#endif
#endif

#ifndef INCLUDED_K3LAPI_HPP
#define INCLUDED_K3LAPI_HPP

struct K3LAPI
{
    /* exceptions */

    struct start_failed
    {
        start_failed(const char * _msg) : msg(_msg) {};
        std::string msg;
    };
    
    struct failed_command
    {
        failed_command(int32 _code, unsigned short _dev, unsigned short _obj, int32 _rc)
        : code(_code), dev(_dev), obj(_obj), rc(_rc) {};
        
        int32           code;
        unsigned short  dev;
        unsigned short  obj;
        int32           rc;
    };

    struct failed_raw_command
    {
        failed_raw_command(unsigned short _dev, unsigned short _dsp, int32 _rc)
        : dev(_dev), dsp(_dsp), rc(_rc) {};
        
        unsigned short  dev;
        unsigned short  dsp;
        int32           rc;
    };

	struct invalid_session
	{
		invalid_session(unsigned int _device, unsigned int _channel)
		: device(_device), channel(_channel) {};
		
		unsigned int device;
        unsigned int channel;
	};
    
    struct invalid_device
    {
        invalid_device(int32 _device)
        : device(_device) {};
        
        int32 device;
    };

    struct invalid_channel
    {
        invalid_channel(int32 _device, int32 _channel)
        : device(_device), channel(_channel) {};

        int32 device, channel;
    };

    struct invalid_link
    {
        invalid_link(unsigned int _device, unsigned int _link)
        : device(_device), link(_link) {};

        int32 device, link;
    };

    struct get_param_failed
    {
        get_param_failed(std::string _name, int32 _rc)
        : name(_name), rc((KLibraryStatus)_rc) {};

        std::string name;
        KLibraryStatus rc;
    };
    
    typedef K3L_DEVICE_CONFIG          device_conf_type;
    typedef K3L_CHANNEL_CONFIG        channel_conf_type;
    typedef K3L_CHANNEL_CONFIG *  channel_ptr_conf_type;
    typedef K3L_LINK_CONFIG              link_conf_type;
    typedef K3L_LINK_CONFIG *        link_ptr_conf_type;

    /* constructors/destructors */
    
    K3LAPI();
    ~K3LAPI();

    /* (init|final)ialize the whole thing! */
    
    void start(void);
    void stop(void);

    /* verificacao de intervalos */
    
    inline bool valid_device(int32 dev)
    {
        return (dev >= 0 && dev < ((int32)_device_count));
    }

    inline bool valid_channel(int32 dev, int32 obj)
    {
        return (valid_device(dev) && obj >= 0 && obj < ((int32)_channel_count[dev]));
    }

    inline bool valid_link(int32 dev, int32 obj)
    {
        return (valid_device(dev) && obj >= 0 && obj < ((int32)_link_count[dev]));
    }

    /* identificadores alto-nivel de objectos */
    struct target
    {
        typedef enum { DEVICE, CHANNEL, MIXER, LINK } target_type;

        target(K3LAPI & k3lapi, target_type type_init, int32 device_value, int32 object_value)
            : type(type_init), 
            device((unsigned short)device_value), 
            object((unsigned short)object_value)
        {
            switch (type_init)
            {
                case DEVICE:
                    if (!k3lapi.valid_device(device))
                        throw invalid_device(device);
                    break;

                case CHANNEL:
                case MIXER:
                    if (!k3lapi.valid_channel(device, object))
                        throw invalid_channel(device, object);
                    break;

                case LINK:
                    if (!k3lapi.valid_link(device, object))
                        throw invalid_link(device, object);
                    break;
            }

        };

        target_type type;

        unsigned short device;
        unsigned short object;
    };

    /* envio de comandos para placa (geral) */

    void raw_command(int32 dev, int32 dsp, std::string & str);
    void raw_command(int32 dev, int32 dsp, const char * cmds, int32 size);

    /* obter dados 'cacheados' (geral) */
    
    inline unsigned int device_count(void)
    {
        return _device_count;
    }

    /* envio de comandos para placa (sem identificadores) */
    
    void mixer(int32 dev, int32 obj, byte track, KMixerSource src, int32 index);
    void mixerCTbus(int32 dev, int32 obj, byte track, KMixerSource src, int32 index);

    void command (int32 dev, int32 obj, int32 code, std::string & str);
    void command (int32 dev, int32 obj, int32 code, const char * parms = NULL);

    /* obter dados 'cacheados' (sem identificadores) */

    inline unsigned int channel_count(int32 dev)
    {
        if (!valid_device(dev))
            throw invalid_device(dev);

        return _channel_count[dev];
    }

    inline unsigned int link_count(int32 dev)
    {
        if (!valid_device(dev))
            throw invalid_device(dev);

        return _link_count[dev];
    }

    KDeviceType device_type(int32 dev)
    {
        if (!valid_device(dev))
            throw invalid_device(dev);
        
        return _device_type[dev];
    }
    

    K3L_DEVICE_CONFIG & device_config(int32 dev)
    {
        if (!valid_device(dev))
            throw invalid_device(dev);

        return _device_config[dev];
    }

    K3L_CHANNEL_CONFIG & channel_config(int32 dev, int32 obj)
    {
        if (!valid_channel(dev, obj))
            throw invalid_channel(dev, obj);

        return _channel_config[dev][obj];
    }

    K3L_LINK_CONFIG & link_config(int32 dev, int32 obj)
    {
        if (!valid_channel(dev, obj))
            throw invalid_channel(dev, obj);

        return _link_config[dev][obj];
    }

    /* envio de comandos para placa (com identificadores) */
    
    void mixer(target & tgt, byte track, KMixerSource src, int32 index)
    {
        mixer((int32)tgt.device, (int32)tgt.object, track, src, index);
    }

    void mixerCTbus(target & tgt, byte track, KMixerSource src, int32 index)
    {
        mixerCTbus((int32)tgt.device, (int32)tgt.object, track, src, index);
    }

    void command (target & tgt, int32 code, std::string & str)
    {
        command((int32)tgt.device, (int32)tgt.object, code, str);
    };

    void command (target & tgt, int32 code, const char * parms = NULL)
    {
        command((int32)tgt.device, (int32)tgt.object, code, parms);
    };

    /* obter dados 'cacheados' (com indentificadores) */
    
    inline unsigned int channel_count(target & tgt)
    {
        return _channel_count[tgt.device];
    }

    inline unsigned int link_count(target & tgt)
    {
        return _link_count[tgt.device];
    }

    KDeviceType device_type(target & tgt)
    {
        return _device_type[tgt.device];
    }
    

    K3L_DEVICE_CONFIG & device_config(target & tgt)
    {
        return _device_config[tgt.device];
    }

    K3L_CHANNEL_CONFIG & channel_config(target & tgt)
    {
        return _channel_config[tgt.device][tgt.object];
    }

    K3L_LINK_CONFIG & link_config(target & tgt)
    {
        return _link_config[tgt.device][tgt.object];
    }

    /* pega valores em strings de eventos */

    KLibraryStatus get_param(K3L_EVENT *ev, const char *name, std::string &res);
    std::string get_param(K3L_EVENT *ev, const char *name);

    /* inicializa valores em cache */

    void init(void);

 protected:

    unsigned int           _device_count;
    unsigned int *        _channel_count;
    unsigned int *           _link_count;
    
         device_conf_type *   _device_config;
    channel_ptr_conf_type *  _channel_config;
       link_ptr_conf_type *     _link_config;
              KDeviceType *     _device_type;
};

#endif /* INCLUDED_K3LAPI_HPP */
