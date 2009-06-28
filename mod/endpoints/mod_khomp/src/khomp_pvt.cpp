#include "khomp_pvt.h"

KhompPvt::KhompPvtVector KhompPvt::_pvts;
switch_mutex_t * KhompPvt::_pvts_mutex;

KhompPvt * KhompPvt::find_channel(char* allocation_string, switch_core_session_t * new_session, switch_call_cause_t * cause)
{
    /* For now we support only the 'first-free' mode */

    char *argv[3] = { 0 };
    int argc = 0;
    
    int board_low = 0;
    int board_high = 0;

    int channel_low = 0;
    int channel_high = 0;
    
    bool first_channel_available = true;

    *cause = SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;

    KhompPvt * pvt = NULL;
    
    /* Let's setup our own vars on tech_pvt */
    if ((argc = switch_separate_string(allocation_string, '/', argv, (sizeof(argv) / sizeof(argv[0])))) < 3)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid dial string (%s). Should be on the format:[Khomp/BoardID (or A for first free board)/CHANNEL (or A for first free channel)]\n", allocation_string);
        return NULL;
    }

    if(*argv[0] == 'A' || *argv[0] == 'a')
    {
        board_low = 0;
        board_high = Globals::_k3lapi.device_count();
    }
    else
    {
        board_low = atoi(argv[0]);
        board_high = board_low;
    }

    if(*argv[1] == 'A' || *argv[1] == 'a')
    {
        first_channel_available = true;
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
    
    for (int board = board_low ; board <= board_high; board++)
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
        
        for (int channel = channel_low ; channel <= channel_high ; channel++) 
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