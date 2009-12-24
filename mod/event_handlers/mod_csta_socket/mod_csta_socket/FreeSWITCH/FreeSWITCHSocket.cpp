/* 
 * CSTA interface for FreeSWICH 
 * Copyright (C) 2009, Szentesi Krisztian <sz.krisz@freemail.hu>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is CSTA interface for FreeSWICH   
 *
 * The Initial Developer of the Original Code is
 * Szentesi Krisztian <sz.krisz@freemail.hu>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Szentesi Krisztian <sz.krisz@freemail.hu>
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 */


#include "FreeSwitchSocket.h"
#include <apr_errno.h>

FreeSWITCHSocket::FreeSWITCHSocket(switch_socket_t* pSocket, switch_memory_pool_t *pool) : m_pSocket(pSocket), m_pool(pool)
{
	switch_mutex_init(&m_mutex, SWITCH_MUTEX_NESTED, m_pool);
	switch_socket_opt_set(m_pSocket, SWITCH_SO_TCP_NODELAY, TRUE);
	switch_socket_opt_set(m_pSocket, SWITCH_SO_NONBLOCK, FALSE);

	char remote_ip[50];
	switch_sockaddr_t *sa;

	switch_socket_addr_get(&sa, SWITCH_TRUE, m_pSocket);
	switch_get_addr(remote_ip, sizeof(remote_ip), sa);
	switch_port_t remote_port = switch_sockaddr_get_port(sa);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Connection Open from %s:%d\n", remote_ip, remote_port);
}

FreeSWITCHSocket::~FreeSWITCHSocket()
{
	switch_mutex_destroy(m_mutex);
}

int FreeSWITCHSocket::Receive(unsigned char *buf, size_t *len) 
{
	switch_status_t status=switch_socket_recv(m_pSocket, (char*)buf, len);
	if(status)
	{
		char errbuf[4096];
		switch_log_printf(SWITCH_CHANNEL_LOG,SWITCH_LOG_NOTICE,"Receive: socket terminated: %d (%s)",status, apr_strerror(status,errbuf,sizeof(errbuf)));
	}
	else
		switch_log_printf(SWITCH_CHANNEL_LOG,SWITCH_LOG_NOTICE,"%d bytes received:\n%s",*len, buf);
	return status;
}	

int FreeSWITCHSocket::Send(const unsigned char *buf, size_t *len) 
{
	switch_mutex_lock(m_mutex);
	switch_status_t status=switch_socket_send(m_pSocket, (const char*)buf, len);
	if(status)
	{
		char errbuf[4096];
		switch_log_printf(SWITCH_CHANNEL_LOG,SWITCH_LOG_NOTICE,"Send: socket terminated: %d (%s)",status, apr_strerror(status,errbuf,sizeof(errbuf)));
	}
	switch_mutex_unlock(m_mutex);
	return status;
}

int FreeSWITCHSocket::Close()
{
	switch_socket_shutdown(m_pSocket, SWITCH_SHUTDOWN_READWRITE);
	switch_socket_close(m_pSocket);	
	return 0;
}
