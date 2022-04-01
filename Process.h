//
//  @file Process.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.21.
//  Copyright 2021 Sergii Oryshchenko. All rights reserved.
//

#ifndef PROCESS_H
#define PROCESS_H

#include "User.h"

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>

#if defined(WIN32)
	#define pid_t		uint32_t
#endif

#ifndef tid_t
	#define tid_t		void*
#endif

#define CStdProcessArray				CStdArray<PROCESSINFO>

typedef struct PROCESSINFO {
	enum { FTID = 0x01, FUID = 0x02, FName = 0x04, FUser = 0x08, FPath = 0x10, FAll = -1 };
	pid_t PID;
	tid_t TID;
	uid_t UID;
	CStdString Name;
	CStdString User;
	CStdString Path;
} *PPROCESSINFO;

CStdString GetProcessName(pid_t nProcessID);
CStdString GetProcessPath(pid_t nProcessID);
uid_t GetProcessUID(pid_t nProcessID);
CStdArray<pid_t> GetProcessList(CStdStringArray &pProcName);
CStdProcessArray GetProcessList(uint32_t eInfo, const char *lpProcessName, const char *lpUserInfo);

#endif
