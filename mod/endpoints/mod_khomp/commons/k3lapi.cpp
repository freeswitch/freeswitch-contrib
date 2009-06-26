#include <k3lapi.hpp>

K3LAPI::K3LAPI()
: _device_count(0),  _channel_count(0),  _link_count(0),
  _device_config(0), _channel_config(0), _link_config(0)
{}

K3LAPI::~K3LAPI()
{
	_device_count = 0;
		
	if (_device_type)    { delete[] _device_type;       _device_type = NULL; }
	if (_device_config)  { delete[] _device_config;   _device_config = NULL; }
	if (_channel_config) { delete[] _channel_config; _channel_config = NULL; }
	if (_link_config)    { delete[] _link_config;       _link_config = NULL; }
	if (_channel_count)  { delete[] _channel_count;   _channel_count = NULL; }
	if (_link_count)     { delete[] _link_count;         _link_count = NULL; }
}

/* initialize the whole thing! */
	
void K3LAPI::start(void)
{
	/* tie the used k3l to the compiled k3l version */
	char *ret = k3lStart(2, 0, 0); //k3lApiMajorVersion, k3lApiMinorVersion, 0); //k3lApiBuildVersion);
	
	if (ret && *ret)
		throw start_failed(ret);
			
	/* call init automagically */
	init();
}

void K3LAPI::stop(void)
{
	k3lStop();
}

/* envio de comandos para placa */
	
void K3LAPI::mixer(int32 dev, int32 obj, byte track, KMixerSource src, int32 index)
{
   	KMixerCommand mix;

    mix.Track = track;
   	mix.Source = src;
    mix.SourceIndex = index;

   	command(dev, obj, CM_MIXER, (const char *) &mix);
}

void K3LAPI::mixerCTbus(int32 dev, int32 obj, byte track, KMixerSource src, int32 index)
{
   	KMixerCommand mix;

    mix.Track = track;
   	mix.Source = src;
    mix.SourceIndex = index;

   	command(dev, obj, CM_MIXER_CTBUS, (const char *) &mix);
}

void K3LAPI::command(int32 dev, int32 obj, int32 code, std::string & str)
{
	command(dev, obj, code, str.c_str());
}

void K3LAPI::command (int32 dev, int32 obj, int32 code, const char * parms)
{
    K3L_COMMAND cmd;

   	cmd.Cmd = code;
    cmd.Object = obj;
   	cmd.Params = (byte *)parms;

	int32 rc = k3lSendCommand(dev, &cmd);
		
	if (rc != ksSuccess)
		throw failed_command(code, dev, obj, rc);
}

void K3LAPI::raw_command(int32 dev, int32 dsp, std::string & str)
{
	raw_command(dev, dsp, str.data(), str.size());
}

void K3LAPI::raw_command(int32 dev, int32 dsp, const char * cmds, int32 size)
{
    std::string str(cmds, size);

	int32 rc = k3lSendRawCommand(dev, dsp, (void *)cmds, size);

	if (rc != ksSuccess)
		throw failed_raw_command(dev, dsp, rc);
}

KLibraryStatus K3LAPI::get_param(K3L_EVENT *ev, const char *name, std::string &res)
{
	char tmp_param[256];
	memset((void*)tmp_param, 0, sizeof(tmp_param));

	int32 rc = k3lGetEventParam (ev, (sbyte *)name, (sbyte *)tmp_param, sizeof(tmp_param)-1);
		
	if (rc != ksSuccess)
		return (KLibraryStatus)rc;

	res.append(tmp_param, strlen(tmp_param));
	return ksSuccess;
}
	
std::string K3LAPI::get_param(K3L_EVENT *ev, const char *name)
{
	std::string res;

	KLibraryStatus rc = get_param(ev, name, res);

	if (rc != ksSuccess)
		throw get_param_failed(name, rc);
			
	return res;
}

void K3LAPI::init(void)
{
	if (_device_count != 0) return;

	_device_count = k3lGetDeviceCount();

	_device_type	= new KDeviceType[_device_count];
	_device_config  = new device_conf_type[_device_count];
	_channel_config = new channel_ptr_conf_type[_device_count];
	_KChannel       = new KChannel_ptr_t[_device_count];
	_link_config    = new link_ptr_conf_type[_device_count];
	_channel_count	= new unsigned int[_device_count];
	_link_count		= new unsigned int[_device_count];
		
	for (unsigned int dev = 0; dev < _device_count; dev++)
	{
		_device_type[dev] = (KDeviceType) k3lGetDeviceType(dev);
			
		/* caches each device config */
		if (k3lGetDeviceConfig(dev, ksoDevice + dev, &(_device_config[dev]), sizeof(_device_config[dev])) != ksSuccess)
			throw start_failed("k3lGetDeviceConfig(device)");

		/* adjust channel/link count for device */
		_channel_count[dev] = _device_config[dev].ChannelCount;
		_link_count[dev] = _device_config[dev].LinkCount;
			
		/* caches each channel config */
		_channel_config[dev] = new channel_conf_type[_channel_count[dev]];
        _KChannel[dev] = new KChannel_t[_channel_count[dev]];

		for (unsigned int obj = 0; obj < _channel_count[dev]; obj++)
		{
			if (k3lGetDeviceConfig(dev, ksoChannel + obj, &(_channel_config[dev][obj]), 
									sizeof(_channel_config[dev][obj])) != ksSuccess)
            {
				throw start_failed("k3lGetDeviceConfig(channel)");
            }
		}

		/* adjust link count for device */
		_link_count[dev] = _device_config[dev].LinkCount;
			
		/* caches each link config */
		_link_config[dev] = new link_conf_type[_link_count[dev]];

		for (unsigned int obj = 0; obj < _link_count[dev]; obj++)
		{
			if (k3lGetDeviceConfig(dev, ksoLink + obj, &(_link_config[dev][obj]), 
									sizeof(_link_config[dev][obj])) != ksSuccess)
				throw start_failed("k3lGetDeviceConfig(link)");
		}
	}
}
