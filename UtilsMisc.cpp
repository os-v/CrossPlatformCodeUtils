//
//  @file UtilsMisc.cpp
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#include "UtilsMisc.h"
#include <string.h>
#include <errno.h>
#include "Logger.h"

#ifdef WIN32
	#include <windows.h>
	#include <direct.h>
	#define PATH_SYM		'\\'
#else
	#include <unistd.h>
	#define PATH_SYM		'/'
#endif

DefLogScope(Common)

void DetachProcess()
{
#if defined(__unix__)
	//signal(SIGHUP, SIG_IGN);

	pid_t nPID = fork();
	if(nPID < 0)
		exit(EXIT_FAILURE);
	else if(nPID > 0)
		exit(EXIT_SUCCESS);

	setsid();
#endif
}

CStdString GetCurPath()
{
	CStdString sResult;
	sResult.resize(1024);
#ifdef WIN32
	for(const char *lpResult = 0; !(lpResult = _getcwd(&sResult[0], (int)sResult.size())) && errno == ERANGE; sResult.resize(sResult.size() + 1024));
#else
	for (const char *lpResult = 0; !(lpResult = getcwd(&sResult[0], (int)sResult.size())) && errno == ERANGE; sResult.resize(sResult.size() + 1024));
#endif
	sResult.resize(strlen(sResult.c_str()));
	return sResult;
}

CStdString GetFullPath(const CStdString &sPath)
{
#ifdef WIN32
	if (sPath.length() >= 2 && sPath[1] == ':')
		return sPath;
#else
	if (sPath.length() && sPath[0] == PATH_SYM)
		return sPath;
#endif
	if (sPath.length() >= 2 && sPath[0] == '.' && (sPath[1] == '\\' || sPath[1] == '/'))
		return ConcatPath(GetCurPath(), sPath.substr(2));
	return ConcatPath(GetCurPath(), sPath);
}

CStdString ConcatPath(const CStdString &sPathA, const CStdString &sPathB)
{
	CStdString sPath = sPathA;
	if(sPath.length() && sPath[sPath.length() - 1] != PATH_SYM)
		sPath += PATH_SYM;
	sPath += sPathB;
	return sPath;
}

bool LoadFile(const char *sPath, CStdString &sText)
{
	char *pData = 0;
	size_t nSize = 0;
	if(!LoadFile(sPath, pData, nSize))
		return false;
	sText.assign(pData, nSize);
	return true;
}

bool LoadFile(const char *sPath, char *&pData, size_t &nData, uint32_t nExtData)
{
	pData = 0;
	nData = 0;
	FILE *pFile = fopen(sPath, "rb");
	if(!pFile)
		return false;
	fseek(pFile, 0, SEEK_END);
	size_t nFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	if(nFileSize > 0)
	{
		pData = new char[nFileSize + nExtData];
		nData = (uint32_t)fread(pData, 1, nFileSize, pFile);
	}
	fclose(pFile);
	return nData > 0;
}

bool SaveFile(const char *sPath, const char *pData, size_t nData)
{
	FILE *pFile = fopen(sPath, "wb");
	if(!pFile)
		return false;
	size_t nWrited = fwrite(pData, 1, nData, pFile);
	fclose(pFile);
	return nWrited == nData;
}

CStdString CStdStrFormat(const char *lpFmt, ...)
{

	CStdString sText;

	va_list args;
	va_start(args, lpFmt);
	int nLength = vsnprintf(0, 0, lpFmt, args);
	va_end(args);

	if(nLength)
	{
		sText.resize(nLength + 1);
		va_start(args, lpFmt);
		vsnprintf(&sText[0], nLength + 1, lpFmt, args);
		va_end(args);
		sText.resize(nLength);
	}

	return sText;
}

CStdStringArray StrSplit(const char *lpString, char cDelimiter, char cQuote, bool fSkipEmpty)
{
	CStdStringArray pResult;
	for( ; lpString && *lpString; )
	{
		const char *lpStartPtr = lpString;
		lpString = StrSkipQuote(lpString, cQuote);
		const char *lpEndPtr = strchr(lpString, cDelimiter);
		if(!lpEndPtr)
		{
			pResult.push_back(lpStartPtr);
			break;
		}
		if(lpEndPtr != lpStartPtr || (lpEndPtr == lpStartPtr && !fSkipEmpty))
			pResult.push_back(CStdString(lpStartPtr, lpEndPtr));
		lpString = lpEndPtr + 1;
	}
	return pResult;
}

const char *StrSkipQuote(const char *lpString, char cQuote)
{
	if(*lpString == cQuote)
	{
		lpString++;
		for( ; *lpString; lpString++)
		{
			if(*lpString == cQuote)
			{
				lpString++;
				if(*lpString != cQuote)
					break;
			}
		}
	}
	return lpString;
}

bool StrStartsWith(const char *lpText, const char *lpStartsWith)
{
	return !strncmp(lpText, lpStartsWith, strlen(lpStartsWith));
}

int System(const char *lpFmt, ...)
{

	va_list args;
	va_start(args, lpFmt);
	int nLength = vsnprintf(0, 0, lpFmt, args);
	va_end(args);

	char *lpCmd = new char[nLength + 1];
	char *lpCmdFmt = lpCmd;

	va_start(args, lpFmt);
	vsnprintf(lpCmdFmt, nLength + 1, lpFmt, args);
	va_end(args);

	int nResult = SystemInvoke(lpCmd);

	delete[] lpCmd;

	return nResult;
}

int SystemInvoke(const char *lpCommand)
{

#ifndef WIN32
	LogMessage("System(%s)", lpCommand);
	int nResult = system(lpCommand);
#else
	CStdString sCommand = CStdString("cmd.exe /C ") + lpCommand;
	lpCommand = sCommand.c_str();
	LogMessage("System(%s)", lpCommand);
	int nResult = -1;
	STARTUPINFOA pStartupInfo;
	PROCESS_INFORMATION pProcessInfo;
	memset(&pStartupInfo, 0, sizeof(STARTUPINFO));
	memset(&pProcessInfo, 0, sizeof(PROCESS_INFORMATION));
	pStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	pStartupInfo.wShowWindow = SW_HIDE;
	if(CreateProcessA(0, (char*)lpCommand, 0, 0, TRUE, 0, 0, 0, &pStartupInfo, &pProcessInfo))
	{
		DWORD nExitCode = 0;
		WaitForSingleObject(pProcessInfo.hProcess, INFINITE);
		GetExitCodeProcess(pProcessInfo.hProcess, &nExitCode);
		CloseHandle(pProcessInfo.hThread);
		CloseHandle(pProcessInfo.hProcess);
		nResult = nExitCode;
	}
#endif

	LogMessage("System() -> %d", nResult);

	return nResult;
}

int SystemOutput(const CStdString &sPath, bool fAppend, CStdString *pOutput, const char *lpFmt, ...)
{

	CStdString sText;

	va_list args;
	va_start(args, lpFmt);
	int nLength = vsnprintf(0, 0, lpFmt, args);
	va_end(args);

	if(!nLength)
		return -1;

	sText.resize(nLength + 1);
	va_start(args, lpFmt);
	vsnprintf(&sText[0], nLength + 1, lpFmt, args);
	va_end(args);

	sText.resize(nLength);

	sText += CStdString(fAppend ? " >> \"" : " > \"") + sPath + "\"";

	int nResult = SystemInvoke(sText.c_str());

	if(pOutput)
		LoadFile(sPath.c_str(), *pOutput);

	return nResult;
}
