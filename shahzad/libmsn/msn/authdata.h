#ifndef __msn_authdata_h__
#define __msn_authdata_h__

/*
 * authdata.h
 * libmsn
 *
 * Created by Mark Rowe on Mon Mar 22 2004.
 * Refactored by Tiago Salem Herrmann on 08/2007.
 * Copyright (c) 2004 Mark Rowe. All rights reserved.
 * Copyright (c) 2007 Tiago Salem Herrmann. All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string>
#include <msn/passport.h>

#include "libmsn_export.h"

namespace MSN
{
    class LIBMSN_EXPORT AuthData
    {
public:
        Passport username;
        
        AuthData(Passport username_) : username(username_) {};
        virtual ~AuthData() {};
    };
}
#endif
