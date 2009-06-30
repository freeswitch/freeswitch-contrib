/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2009, Anthony Minessale II <anthm@freeswitch.org>
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
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Joao Mesquita <mesquita (at) khomp.com.br>
 * Raul Fragoso <raulfragoso (at) gmail.com>
 *
 *
 * mod_khomp.c -- Khomp board Endpoint Module
 *
 */

/**
  \mainpage Khomp board Endpoint Module
  
  \section Introduction
  This module has been developed to make a nice, affordable brazilian board
  called Khomp (http://www.khomp.com.br) compatible with FreeSWITCH.
  It's main features are MFC-R2 native compatibility and the GSM boards.
  This module is supported by the manufacturer.

  \section Contributors
  \li João Mesquita <jmesquita (at) gmail.com>
  \li Raul Fragoso <raulfragoso (at) gmail.com>
  \li Khomp development team
  */

/**
 * @file mod_khomp.h
 * @brief Khomp Endpoint Module
 * @see mod_khomp
 */
#ifndef _MOD_KHOMP_H_
#define _MOD_KHOMP_H_

#include "globals.h"
#include "khomp_pvt.h"
#include "opt.h"

/*!
 \brief Defined by mod_reference, defines statuses for the switch_channel
 \param TFLAG_IO Enables the IO state of a channel
 \param TFLAG_INBOUND Sets a channel as inbound
 \param TFLAG_OUTBOUND Sets a channel as outbound
 */
typedef enum
{
    TFLAG_IO = (1 << 0),
    TFLAG_INBOUND = (1 << 1),
    TFLAG_OUTBOUND = (1 << 2),
    TFLAG_DTMF = (1 << 3),
    TFLAG_VOICE = (1 << 4),
    TFLAG_HANGUP = (1 << 5),
    TFLAG_LINEAR = (1 << 6),
    TFLAG_CODEC = (1 << 7),
    TFLAG_BREAK = (1 << 8),
    TFLAG_STREAM = (1 << 9),
    TFLAG_LISTEN = (1 << 10)
}
TFLAGS;

#endif /* _MOD_KHOMP_H_ */
