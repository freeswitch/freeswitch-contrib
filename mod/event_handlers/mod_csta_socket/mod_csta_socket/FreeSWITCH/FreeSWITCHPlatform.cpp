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

#include "FreeSWITCHPlatform.h"
#include "switch.h"


FreeSWITCHPlatform::FreeSWITCHPlatform(void)
{
}

FreeSWITCHPlatform::~FreeSWITCHPlatform(void)
{
}

void FreeSWITCHPlatform::Trace(ci_trace_level loglevel, const char *fmt, ...)
{
	switch_log_level_t fsloglevel=SWITCH_LOG_ALERT;
	switch (loglevel)
	{
	case CI_LLV_FATAL: fsloglevel=SWITCH_LOG_ALERT;break;
	case CI_LLV_ERROR: fsloglevel=SWITCH_LOG_ERROR;break;
	case CI_LLV_WARN: fsloglevel=SWITCH_LOG_WARNING;break;
	case CI_LLV_INFO: fsloglevel=SWITCH_LOG_NOTICE;break;
	case CI_LLV_DEBUG: fsloglevel=SWITCH_LOG_DEBUG;break;
	}
	
	va_list ap;
	va_start(ap, fmt);
	

	switch_log_vprintf(SWITCH_CHANNEL_LOG, fsloglevel, fmt, ap);	
	va_end(ap);
}