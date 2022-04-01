//
//  @file UtilsWin32.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.21.
//  Copyright 2021 Sergii Oryshchenko. All rights reserved.
//

#ifndef UTILSWIN32_H
#define UTILSWIN32_H

#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
	#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <Windows.h>

UINT PrivilegeEnable(DWORD nAttrs, UINT nCount, ...);
BOOL PrivilegeEnable(LPCTSTR lpName, DWORD nAttrs, HANDLE hToken = 0);

#endif
