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


#ifndef CI_FREE_SWITCH_SOCKET_H
#define CI_FREE_SWITCH_SOCKET_H

#include "CISocket.h"
#include "switch.h"

//! CSTA Inside CISocket interface realization for FreeSWITCH.
/*!
Provides FreeSWITCH platform specific socket manipulation methods required by CSTA Inside.
*/
class FreeSWITCHSocket : public CISocket
{
protected:
	switch_socket_t* m_pSocket;
	switch_mutex_t *m_mutex;
	switch_memory_pool_t* m_pool;
public:
	FreeSWITCHSocket(switch_socket_t* pSocket, switch_memory_pool_t *pool);

	virtual ~FreeSWITCHSocket();

	virtual int Receive(unsigned char *buf, size_t *len);	

	virtual int Send(const unsigned char *buf, size_t *len);

	virtual int Close();
};

#endif	//CI_FREE_SWITCH_SOCKET_H
