//
//  @file User.cpp
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.21.
//  Copyright 2021 Sergii Oryshchenko. All rights reserved.
//

#include "Process.h"
#include <Windows.h>

uid_t GetUserID(const CStdString &sUserInfo)
{
	uid_t nUserID = EMPTY_UID;
#if defined(WIN32)
	SID_NAME_USE eSIDNameUse = SidTypeUser;
	DWORD nReturned = 0, nDomainName = MAX_PATH;
	nUserID.resize(1024);
	CHAR lpDomainName[MAX_PATH];
	LookupAccountNameA(0, sUserInfo.data(), 0, &nReturned, lpDomainName, &nDomainName, &eSIDNameUse);
	if (LookupAccountNameA(0, sUserInfo.data(), nUserID.data(), &nReturned, lpDomainName, &nDomainName, &eSIDNameUse))
		nUserID.resize(nReturned);
	else
		nUserID.resize(0);
#endif
	return nUserID;
}

CStdString GetUserInfo(const uid_t &nUserID)
{
	CStdString sUserInfo;
#if defined(WIN32)
	SID_NAME_USE eSIDNameUse = SidTypeUser;
	DWORD nUserInfo = MAX_PATH, nUserDomain = MAX_PATH;
	char lpUserInfo[MAX_PATH] = { 0 }, lpUserDomain[MAX_PATH] = { 0 };
	if(LookupAccountSidA(0, (PSID)nUserID.data(), lpUserInfo, &nUserInfo, lpUserDomain, &nUserDomain, &eSIDNameUse))
	{
		if(*lpUserDomain)
			sUserInfo = CStdString(lpUserDomain) + "\\";
		sUserInfo += lpUserInfo;
	}
	else
		sUserInfo = "SYSTEM";
#else
	passwd *pUser = getpwuid(nUserID);
	sUserInfo = pUser && pUser->pw_name ? pUser->pw_name : "";
#endif
	return sUserInfo;
}

