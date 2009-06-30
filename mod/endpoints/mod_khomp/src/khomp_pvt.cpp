#include "khomp_pvt.h"

KhompPvt::KhompPvtVector KhompPvt::_pvts;
switch_mutex_t * KhompPvt::_pvts_mutex;

KhompPvt * KhompPvt::find_channel(char* allocation_string, switch_core_session_t * new_session, switch_call_cause_t * cause)
{

    char *argv[3] = { 0 };
    int argc = 0;
    
    int board_low = 0;
    int board_high = 0;

    int channel_low = 0;
    int channel_high = 0;
    
    bool first_channel_available = true;
    bool reverse_first_board_available = false;
    bool reverse_first_channel_available = false;

    *cause = SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;

    KhompPvt * pvt = NULL;
    
    /* Let's setup our own vars on tech_pvt */
    if ((argc = switch_separate_string(allocation_string, '/', argv, (sizeof(argv) / sizeof(argv[0])))) < 3)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
                "Invalid dial string (%s). Should be on the format:[Khomp/BoardID (or A for first free board)/CHANNEL (or A for first free channel)]\n",
                allocation_string);
        return NULL;
    }

    if(*argv[0] == 'A' || *argv[0] == 'a')
    {
        board_low = 0;
        board_high = Globals::_k3lapi.device_count();
        // Lets make it reverse...
        if(*argv[0] == 'a')
        {
            reverse_first_board_available = true;
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Doing reverse board lookup!\n");
        }
    }
    else
    {
        board_low = atoi(argv[0]);
        board_high = board_low;
    }

    if(*argv[1] == 'A' || *argv[1] == 'a')
    {
        first_channel_available = true;
        // Lets make it reverse...
        if(*argv[1] == 'a')
        {
            reverse_first_channel_available = true;
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Doing reverse channel lookup!\n");
        }
    }
    else
    {
        first_channel_available = false;
        channel_low = atoi(argv[0]);
        channel_high = board_low;
    }

    /* Sanity checking */
    if(board_low < 0 || board_high > Globals::_k3lapi.device_count())
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid board selection (%d-%d) !\n", board_low, board_high);
        return NULL;            
    }

    switch_mutex_lock(_pvts_mutex);

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Channel selection: board (%d-%d), channel (%d-%d)!\n", board_low, board_high, channel_low, channel_high);
    
    int board = board_low;
    for ((reverse_first_board_available == false ? board = board_low : board = board_high-1);
            (reverse_first_board_available == false ? board <= board_high : board >= board_low);
            (reverse_first_board_available == false ? board++ : board--))
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Checking board %d\n", board);
        
        if(pvt != NULL)
            break;
        
        if(first_channel_available)
        {
            channel_low = 0;
            channel_high = Globals::_k3lapi.channel_count(board);
        }
        else
        {
            if(channel_low < 0 || channel_high > Globals::_k3lapi.channel_count(board))
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid channel selection (%d-%d) !\n", channel_low, channel_high);
                switch_mutex_unlock(_pvts_mutex);
                return NULL;
            }
        }

        int channel = channel_low;
        for ((reverse_first_channel_available == true ? channel = channel_high-1 : channel = channel_low);
                (reverse_first_channel_available == true ? channel >= channel_low : channel <= channel_high);
                (reverse_first_channel_available == true ? channel-- : channel++)) 
        {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Checking if (%d-%d) is free\n", board, channel);
            try 
            {
                K3L_CHANNEL_CONFIG channelConfig;
                channelConfig = Globals::_k3lapi.channel_config( board, channel );
            }
            catch (...)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Exception while retrieving channel config for board %d, channel%d!\n", board, channel);
                switch_mutex_unlock(_pvts_mutex);
                return NULL;
            }
            K3L_CHANNEL_STATUS status;
            if (k3lGetDeviceStatus( board, channel + ksoChannel, &status, sizeof(status) ) != ksSuccess)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "k3lGetDeviceStatus failed to retrieve channel config for board %d, channel%d!\n", board, channel);
                switch_mutex_unlock(_pvts_mutex);
                return NULL;
            }
            if(status.CallStatus == kcsFree)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Channel (%d-%d) is free, let's check if the session is available ...\n", board, channel);
                pvt = KhompPvt::khompPvt(board, channel);
                if(pvt != NULL && pvt->session() == NULL)
                {
                    pvt->session(new_session);
                    break;
                }
                pvt = NULL;
            }
        }
    }

    if(pvt != NULL)
    {
        *cause = SWITCH_CAUSE_SUCCESS;
    }
    else
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No channels available\n");
        *cause = SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION;
    }
    
    switch_mutex_unlock(_pvts_mutex);
    return pvt;
}

/* Helper functions - based on code from chan_khomp */

int32 KhompPvt::get_audio_dsp()
{
    switch (Globals::_k3lapi.device_type(_KDeviceId))
    {
        case kdtFXO:
        case kdtFXOVoIP:
        case kdtGSM:
        case kdtGSMSpx:
            return 0;
        default:
            return 1;
    }
}

bool KhompPvt::obtain_both(void)
{
	/* estes buffers *NAO PODEM SER ESTATICOS*! */
    char cmd1[] = { 0x3f, 0x03, 0xff, 0x00, 0x05, 0xff };
    char cmd2[] = { 0x3f, 0x03, 0xff, 0x01, 0x00, 0xff };

    cmd1[2] = cmd1[5] = cmd2[2] = cmd2[5] = _KChannelId;

    int32 val = get_audio_dsp();

    try
    {
	    Globals::_k3lapi.raw_command(_KDeviceId, val, cmd1, 6);
	    Globals::_k3lapi.raw_command(_KDeviceId, val, cmd2, 6);
    }
    catch(...)
    {
        return false;
    }
        
	return true;
}

bool KhompPvt::obtain_rx(bool with_delay)
{
	/* estes buffers *NAO PODEM SER ESTATICOS*! */
    char cmd1[] = { 0x3f, 0x03, 0xff, 0x00, (with_delay ? 0x05 : 0x0a), 0xff };
    char cmd2[] = { 0x3f, 0x03, 0xff, 0x01, 0x09, 0x0f };

    cmd1[2] = cmd1[5] = cmd2[2] = _KChannelId;

    int32 val = get_audio_dsp();

    try
    {
	    Globals::_k3lapi.raw_command(_KDeviceId, val, cmd1, 6);
	    Globals::_k3lapi.raw_command(_KDeviceId, val, cmd2, 6);
    }
    catch(...)
    {
        return false;
    }

	return true;
}

bool KhompPvt::obtain_tx(void)
{
	/* estes buffers *NAO PODEM SER ESTATICOS*! */
    char cmd1[] = { 0x3f, 0x03, 0xff, 0x00, 0x00, 0xff };
    char cmd2[] = { 0x3f, 0x03, 0xff, 0x01, 0x09, 0x0f };

	cmd1[2] = cmd1[5] = cmd2[2] = _KChannelId;

    int32 val = get_audio_dsp();

    try
    {
	    Globals::_k3lapi.raw_command(_KDeviceId, val, cmd1, 6);
	    Globals::_k3lapi.raw_command(_KDeviceId, val, cmd2, 6);
    }
    catch(...)
    {
        return false;
    }

	return true;
}

bool KhompPvt::start_stream(void)
{
    if(switch_test_flag(this, TFLAG_STREAM))
        return true;
    
    try
    {
        Globals::_k3lapi.mixer(_KDeviceId, _KChannelId, 0, kmsPlay, _KChannelId);
        Globals::_k3lapi.command(_KDeviceId, _KChannelId, CM_START_STREAM_BUFFER);
    }
    catch(...)
    {
		return false;
    }

    switch_set_flag_locked(this, TFLAG_STREAM);
    
	return true;
}

bool KhompPvt::stop_stream(void)
{
    if(!switch_test_flag(this, TFLAG_STREAM))
        return true;
    
    try
    {
        Globals::_k3lapi.mixer(_KDeviceId, _KChannelId, 0, kmsGenerator, kmtSilence);
	    Globals::_k3lapi.command(_KDeviceId, _KChannelId, CM_STOP_STREAM_BUFFER);
    }
    catch(...)
    {
        return false;
    }

    switch_clear_flag_locked(this, TFLAG_STREAM);
    
	return true;
}

bool KhompPvt::start_listen(bool conn_rx)
{
    if(switch_test_flag(this, TFLAG_LISTEN))
        return true;
    
	const size_t buffer_size = KHOMP_PACKET_SIZE;

    if (conn_rx)
    {
	    if (!obtain_rx(false)) // no delay, by default..
			return false;
	}

    try
    {
        Globals::_k3lapi.command(_KDeviceId, _KChannelId, CM_LISTEN, (const char *) &buffer_size);
    }
    catch(...)
    {
        return false;
    }

    switch_set_flag_locked(this, TFLAG_LISTEN);
    
	return true;
}

bool KhompPvt::stop_listen(void)
{
    if(!switch_test_flag(this, TFLAG_LISTEN))
        return true;
    
    try
    {
        Globals::_k3lapi.command(_KDeviceId, _KChannelId, CM_STOP_LISTEN);
    }
    catch(...)
    {
        return false;
    }

	switch_clear_flag_locked(this, TFLAG_LISTEN);

	return true;
}

