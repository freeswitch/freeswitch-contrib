#include <string>

#include <k3l.h>
#include <k3lVersion.h>

#ifdef __GNUC_PREREQ
#if __GNUC_PREREQ(4,3)
#include <cstring>
#endif
#endif

#ifndef INCLUDED_K3LAPI_HPP
#define INCLUDED_K3LAPI_HPP

/* FreeSWITCH include */
extern "C" {
    #include "switch.h"
}


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
		failed_command(int32 _code, unsigned int _dev, unsigned int _obj, int32 _rc)
		: code(_code), dev(_dev), obj(_obj), rc(_rc) {};
		
		int32         code;
		unsigned int  dev;
		unsigned int  obj;
		int32         rc;
	};

	struct failed_raw_command
	{
		failed_raw_command(unsigned int _dev, unsigned int _dsp, int32 _rc)
		: dev(_dev), dsp(_dsp), rc(_rc) {};
		
		unsigned int  dev;
		unsigned int  dsp;
		int32         rc;
	};

	struct invalid_device
	{
		invalid_device(unsigned int _device)
		: device(_device) {};
		
		unsigned int device;
	};

	struct invalid_channel
	{
		invalid_channel(unsigned int _device, unsigned int _channel)
		: device(_device), channel(_channel) {};

		unsigned int device, channel;
	};

	struct invalid_link
	{
		invalid_link(unsigned int _device, unsigned int _link)
		: device(_device), link(_link) {};

		unsigned int device, link;
	};

	struct get_param_failed
	{
		get_param_failed(std::string _name, int32 _rc)
		: name(_name), rc((KLibraryStatus)_rc) {};

		std::string name;
		KLibraryStatus rc;
	};

    struct KChannel {
        public:
            KChannel() : _session(NULL){};
            void setSession(switch_core_session_t * session)
            {
                _session = session;
            }
            switch_core_session_t * getSession()
            {
                return _session;
            }
        protected:
            switch_core_session_t * _session;
    };
	
	typedef K3L_DEVICE_CONFIG          device_conf_type;
	typedef K3L_CHANNEL_CONFIG        channel_conf_type;
	typedef K3L_CHANNEL_CONFIG *  channel_ptr_conf_type;
	typedef K3L_LINK_CONFIG              link_conf_type;
	typedef K3L_LINK_CONFIG *        link_ptr_conf_type;
    typedef KChannel                         KChannel_t;
    typedef KChannel_t *                 KChannel_ptr_t;

	/* constructors/destructors */
	
	K3LAPI();
	~K3LAPI();

	/* (init|final)ialize the whole thing! */
	
	void start(void);
	void stop(void);

	/* verificacao de intervalos */
	
	inline bool valid_device( unsigned int dev )
	{
    	return (dev < _device_count);
	}

	inline bool valid_channel( unsigned int dev, unsigned int obj )
	{
	    return (valid_device(dev) && obj < _channel_count[dev]);
	}

	inline bool valid_link( unsigned int dev, unsigned int obj )
	{
	    return (valid_device(dev) && obj < _link_count[dev]);
	}

	/* envio de comandos para placa */
	
	void mixer(int32 dev, int32 obj, byte track, KMixerSource src, int32 index);
	void mixerCTbus(int32 dev, int32 obj, byte track, KMixerSource src, int32 index);
	void command (int32 dev, int32 obj, int32 code, std::string & str);
	void command (int32 dev, int32 obj, int32 code, const char * parms = NULL);
	void raw_command(int32 dev, int32 dsp, std::string & str);
	void raw_command(int32 dev, int32 dsp, const char * cmds, int32 size);

	/* obter dados 'cacheados' */
	
	inline unsigned int device_count(void)
	{
		return _device_count;
	}

	inline unsigned int channel_count(unsigned int dev)
	{
		if (!valid_device(dev))
			throw invalid_device(dev);

		return _channel_count[dev];
	}

	inline unsigned int link_count(unsigned int dev)
	{
		if (!valid_device(dev))
			throw invalid_device(dev);

		return _link_count[dev];
	}

	KDeviceType device_type(unsigned int dev)
	{
		if (!valid_device(dev))
			throw invalid_device(dev);
		
		return _device_type[dev];
	}
	

	K3L_DEVICE_CONFIG & device_config(unsigned int dev)
	{
		if (!valid_device(dev))
			throw invalid_device(dev);

		return _device_config[dev];
	}

	K3L_CHANNEL_CONFIG & channel_config(unsigned int dev, unsigned int obj)
	{
		if (!valid_channel(dev, obj))
			throw invalid_channel(dev, obj);

		return _channel_config[dev][obj];
	}

	K3L_LINK_CONFIG & link_config(unsigned int dev, unsigned int obj)
	{
		if (!valid_channel(dev, obj))
			throw invalid_channel(dev, obj);

		return _link_config[dev][obj];
	}

	/* pega valores em strings de eventos */

	KLibraryStatus get_param(K3L_EVENT *ev, const char *name, std::string &res);
	std::string get_param(K3L_EVENT *ev, const char *name);

	/* inicializa valores em cache */

	void init(void);

    void setSession(unsigned int boardId, unsigned int chanId, switch_core_session_t * session)
    {
		if (!valid_channel(boardId, chanId))
			throw invalid_channel(boardId, chanId);
        _KChannel[boardId][chanId].setSession(session);
    }

    switch_core_session_t * getSession(unsigned int boardId, unsigned int chanId)
    {
		if (!valid_channel(boardId, chanId))
			throw invalid_channel(boardId, chanId);
        return _KChannel[boardId][chanId].getSession();
    }

 protected:

	unsigned int              _device_count;
	unsigned int *           _channel_count;
	unsigned int *              _link_count;

           KChannel_ptr_t *        _KChannel;
	     device_conf_type *   _device_config;
	channel_ptr_conf_type *  _channel_config;
	   link_ptr_conf_type *     _link_config;
	          KDeviceType *     _device_type;
};

#endif /* INCLUDED_K3LAPI_HPP */
