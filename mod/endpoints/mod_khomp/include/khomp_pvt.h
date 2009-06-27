#ifndef _KHOMP_PVT_H_
#define _KHOMP_PVT_H_

#include "globals.h"

struct KhompPvt
{

    typedef std::vector < KhompPvt * >        PvtVectorType;
    typedef std::vector < PvtVectorType >   PvtVector2Type;
    typedef PvtVector2Type KhompPvtVector;
    
    KhompPvt(K3LAPI::target & target)
    : _target(target), _session(NULL) {};

    K3LAPI::target target()
    {
        return _target;
    }
    
    void session(switch_core_session_t * newSession)
    {
        _session = newSession;
    }

    switch_core_session_t * session()
    {
        return _session;
    }

    static KhompPvt * khompPvt(int32 device, int32 object)
    {
        if (!Globals::_k3lapi.valid_channel(device, object))
            throw K3LAPI::invalid_channel(device, object);

        return _pvts[device][object];
    }

    static KhompPvt * khompPvt(K3LAPI::target & target)
    {
        return _pvts[target.device][target.object];
    }

    static void initialize(void)
    {
        for (unsigned dev = 0; dev < Globals::_k3lapi.device_count(); dev++)
        {
           // KhompPvt * tech_pvt;
            //tech= (KhompPvt *) switch_core_session_alloc(*new_session, sizeof(KhompPvt));
            _pvts.push_back(std::vector<KhompPvt*>());

            for (unsigned obj = 0; obj < Globals::_k3lapi.channel_count(dev); obj++)
            {
                K3LAPI::target tgt(Globals::_k3lapi, K3LAPI::target::CHANNEL, dev, obj);
                KhompPvt * pvt = new KhompPvt(tgt);
                _pvts.back().push_back(pvt);
            }
        }
    }

    K3LAPI::target          _target;
    switch_core_session_t * _session;

    unsigned int flags; //TODO: Alterar o nome depois

    switch_codec_t _read_codec;
    switch_codec_t _write_codec;

    switch_frame_t _read_frame;

    unsigned char _databuf[SWITCH_RECOMMENDED_BUFFER_SIZE];

    switch_caller_profile_t *_caller_profile;

    switch_mutex_t *_mutex;
    switch_mutex_t *flag_mutex; //TODO: Alterar o nome depois

    unsigned int _KDeviceId;    // Represent de board we are making the call from
    unsigned int _KChannel;   // Represent the channel we are making the call from
    
    /* static stuff */    
    static KhompPvtVector _pvts;
};



#endif /* _KHOMP_PVT_H_*/
