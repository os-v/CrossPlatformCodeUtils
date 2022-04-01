//
//  @file UtilsWin32.cpp
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.21.
//  Copyright 2021 Sergii Oryshchenko. All rights reserved.
//

#include "UtilsWin32.h"

UINT PrivilegeEnable(DWORD nAttrs, UINT nCount, ...)
{

	UINT nResult = 0;

	va_list pArgs;
	va_start(pArgs, nCount);

	for(UINT iArg = 0; iArg < nCount; iArg++)
	{
		LPCTSTR lpName = (LPCTSTR)va_arg(pArgs, LPCTSTR);
		if(PrivilegeEnable(lpName, nAttrs))
			nResult++;
	}

	va_end(pArgs);

	return nResult;
}

BOOL PrivilegeEnable(LPCTSTR lpName, DWORD nAttrs, HANDLE hToken)
{

	BOOL fOpenToken = !hToken;
	if (fOpenToken && !OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;

	TOKEN_PRIVILEGES pPriv;

	pPriv.PrivilegeCount = 1;
	pPriv.Privileges[0].Attributes = nAttrs;

	BOOL fResult = LookupPrivilegeValue(0, lpName, &pPriv.Privileges[0].Luid);
	if(fResult)
		fResult = AdjustTokenPrivileges(hToken, FALSE, &pPriv, sizeof(pPriv), 0, 0);

	if (fOpenToken)
		CloseHandle(hToken);

	return fResult;
}

