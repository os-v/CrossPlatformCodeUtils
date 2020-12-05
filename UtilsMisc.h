//
//  @file UtilsMisc.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#ifndef UTILSMISC_H
#define UTILSMISC_H

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>

#if !defined(WIN32)
	#include <unistd.h>
#endif

#include "Logger.h"

#define ARRAY_SIZE(x)				(sizeof(x) / sizeof((x)[0]))

#define CStdString					std::string
#define CStdStringW					std::wstring
#define CStdArray					std::vector
#define CStdStringArray				std::vector<CStdString>
#define CStdStringWArray			std::vector<CStdStringW>

#define TOSTRW(s)					CStdStringW(s.begin(), s.end())

#if !defined(WIN32)
	#define Sleep(x)		usleep(x * 1000)
	#define stricmp	 	strcasecmp
#endif

#define SafeDelete(x)			if(x) { delete x; x = 0; }
#define SafeRelease(x)			if(x) { x->Release(); x = 0; }

void DetachProcess();

CStdString GetCurPath();
CStdString GetFullPath(const CStdString &sPath);
CStdString ConcatPath(const CStdString &sPathA, const CStdString &sPathB);

bool LoadFile(const char *sPath, CStdString &sText);
bool LoadFile(const char *sPath, char *&pData, size_t &nData, uint32_t nExtData = 0);
bool SaveFile(const char *sPath, const char *pData, size_t nData);

CStdString CStdStrFormat(const char *lpFmt, ...);
CStdStringArray StrSplit(const char *lpString, char cDelimiter, char cQuote, bool fSkipEmpty);
const char *StrSkipQuote(const char *lpString, char cQuote);
bool StrStartsWith(const char *lpText, const char *lpStartsWith);

int System(const char *lpFmt, ...);
int SystemInvoke(const char *lpCommand);
int SystemOutput(const CStdString &sPath, bool fAppend, CStdString *pOutput, const char *lpFmt, ...);

#endif
