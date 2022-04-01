//
//  @file User.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.21.
//  Copyright 2021 Sergii Oryshchenko. All rights reserved.
//

#ifndef USER_H
#define USER_H

#include "UtilsMisc.h"

#if defined(WIN32)
	#define uid_t		CStdByteArray
	#define EMPTY_UID	CStdByteArray()
#else
	#define EMPTY_UID	0
#endif

uid_t GetUserID(const CStdString &sUserInfo);
CStdString GetUserInfo(const uid_t &nUserID);

#endif
