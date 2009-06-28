#ifndef _KHOMP_PVT_H_
#define _KHOMP_PVT_H_

#include "globals.h"

/*!
 \brief This struct holds a static linked list representing all the Khomp channels
        found in the host. It's also a place holder for session objects and some
        other opaque members used by the module.
 */
struct KhompPvt
{

    typedef std::vector < KhompPvt * >        PvtVectorType; /*!< Collection of pointers of KhompPvts */
    typedef std::vector < PvtVectorType >   PvtVector2Type;  /*!< Collection of PvtVectorType */
    typedef PvtVector2Type KhompPvtVector;                   /*!< A bidimensional array o KhompPvts, meaning [board][channel] */
    
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


    /*!
      \brief Lookup channels and boards when dialed.
      \param allocation_string The dialstring as put on Dialplan. [Khomp/[a|A|0-board_high]/[a|A|0-channel_high]/dest].
      \param new_session Session allocated for this call.
      \param[out] Cause returned. Returns NULL if suceeded if not, the proper cause.
      \return KhompPvt to be used on the call.
      */
    static KhompPvt * find_channel(char* allocation_string, switch_core_session_t * new_session, switch_call_cause_t * cause);

    static void initialize(void)
    {
        switch_mutex_init(&_pvts_mutex, SWITCH_MUTEX_NESTED, Globals::_module_pool);
        
        for (unsigned dev = 0; dev < Globals::_k3lapi.device_count(); dev++)
        {
            _pvts.push_back(std::vector<KhompPvt*>());

            for (unsigned obj = 0; obj < Globals::_k3lapi.channel_count(dev); obj++)
            {
                K3LAPI::target tgt(Globals::_k3lapi, K3LAPI::target::CHANNEL, dev, obj);
                KhompPvt * pvt = new KhompPvt(tgt);
                pvt->_KDeviceId = dev;
                pvt->_KChannelId = obj;
                pvt->_session = NULL;
                _pvts.back().push_back(pvt);
                Globals::_k3lapi.command(dev, obj, CM_DISCONNECT, NULL); 
            }
        }
    }

    static void terminate()
    {
        
    }

    K3LAPI::target          _target;
    switch_core_session_t * _session; /*!< The session to which this pvt is associated with */

    unsigned int flags; //TODO: Alterar o nome depois

    switch_codec_t _read_codec;
    switch_codec_t _write_codec;

    switch_frame_t _read_frame;

    switch_buffer_t * _audio_buffer;
    
    unsigned char _databuf[SWITCH_RECOMMENDED_BUFFER_SIZE];

    switch_caller_profile_t *_caller_profile;

    switch_mutex_t *_mutex;
    switch_mutex_t *flag_mutex; //TODO: Alterar o nome depois

    unsigned int _KDeviceId;    /*!< Represent de board we are making the call from */
    unsigned int _KChannelId;   /*!< Represent the channel we are making the call from */
    
    /* static stuff */
    static switch_mutex_t *_pvts_mutex;
    
    static KhompPvtVector _pvts; /*!< Static structure that contains all the pvts. Will be initialized by KhompPvt::initialize */
};



#endif /* _KHOMP_PVT_H_*/
