//
//  @file Logger.cpp
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#include "Logger.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>

#ifdef LOG_SYNC
	#include "SyncNamedMutex.h"
#endif

#ifdef WIN32
	#include <windows.h>
	#include <process.h>
	#include <direct.h>
	#include <time.h>
#else
	#include <pthread.h>
	#include <sys/stat.h>
	#include <unistd.h>
#endif

DefLogScope(Common)

const char *GProcName = "";
char GLogFile[256] = "";
int GLogLevel = 0;
bool GLogDebug = true;
bool GLogStd = false;

uint64_t GetCurrentProcessID()
{
#ifdef WIN32
	int pid = _getpid();
#else
	pid_t pid = getpid();
#endif
	return pid;
}

uint64_t GetCurrentThreadID()
{
	uint64_t tid = 0;
#if defined(WIN32)
	tid = GetCurrentThreadId();
#elif defined(__APPLE__)
	//tid = (uint64_t)pthread_self();
	pthread_threadid_np(0, &tid);
#elif defined(__ANDROID__)
//#elif defined(__linux__)
#elif defined(__unix__)
	tid = (uint64_t)pthread_self();
#endif
	return tid;
}

bool IsPathExists(const char *lpPath)
{

#ifdef WIN32
	DWORD nAttrs = GetFileAttributesA(lpPath);
	return nAttrs != INVALID_FILE_ATTRIBUTES;
#else
	if(!access(lpPath, F_OK))
		return true;
#endif

	return false;
}

void LogInitialize(const char *lpProcName, const char *lpLogPath, bool fLogStd, int nLevel, bool fDebug)
{

	if(lpLogPath)
#if defined(WIN32)
		ExpandEnvironmentStringsA(lpLogPath, GLogFile, sizeof(GLogFile));
#else
		strcpy(GLogFile, lpLogPath);
#endif

	char *lpSlash = strrchr(GLogFile, '/');
	if(!lpSlash)
		lpSlash = strrchr(GLogFile, '\\');
	if(lpSlash)
	{
		std::string sPath(GLogFile, lpSlash - GLogFile);
		if(!IsPathExists(sPath.c_str()))
#ifdef WIN32
			_mkdir(sPath.c_str());
#else
			mkdir(sPath.c_str(), 0777);
#endif
	}

	GProcName = lpProcName;
	GLogLevel = nLevel;
	GLogDebug = fDebug;

	if(fLogStd && lpLogPath)
	{
		GLogStd = fLogStd;
		freopen(GLogFile, "a+", stderr);
		freopen(GLogFile, "a+", stdout);
	}

}

const char *LogFilePath()
{
	return GLogFile;
}

int LogGetLevel()
{
	return GLogLevel;
}

bool LogIsLogStd()
{
	return GLogStd;
}

bool LogIsRequired(int nLevel)
{
	return GLogLevel && nLevel <= GLogLevel;
}

void LogAMessage(const char *lpScope, const char *lpFmt, va_list args)
{

#ifndef LOG_DYN
	static char pMessage[LOG_MAX_MESSAGE + 1];
#endif

	tm pTime;
#ifdef WIN32
	__int64 nTime = 0;
	_time64(&nTime);
	gmtime_s(&pTime, &nTime);
	long nTimeMS = GetTickCount() % 1000;
#else
	timespec pTimeSpec;
	clock_gettime(CLOCK_REALTIME, &pTimeSpec);
	gmtime_r(&pTimeSpec.tv_sec, &pTime);
	long nTimeMS = pTimeSpec.tv_nsec / 1000000;
#endif

	uint64_t pid = GetCurrentProcessID();
	uint64_t tid = GetCurrentThreadID();

#ifdef LOG_DYN
	va_list pArgs;
	va_copy(pArgs, args);
	int nMaxLength = snprintf(0, 0, "%.2d.%.2d.%.2d %.2d-%.2d-%.2d.%.3d: [%d] [%d] [%s] [%s] ", pTime.tm_year % 100, pTime.tm_mon + 1, pTime.tm_mday, pTime.tm_hour, pTime.tm_min, pTime.tm_sec, (int)nTimeMS, (int)pid, (int)tid, GProcName, lpScope);
	nMaxLength += vsnprintf(0, 0, lpFmt, pArgs) + 1;
	va_end(pArgs);
	char *pMessage = (char*)malloc(nMaxLength);
#else
	int nMaxLength = LOG_MAX_MESSAGE;
#endif

#if defined(LOG_SYNC) && !(defined(__APPLE__) && (TARGET_OS_IPHONE == 1) || defined(__ANDROID__))
	static CSyncNamedMutex pLock("/CPCULoggerLock");
	bool fLock = pLock.Lock(250);
#endif

	int nLength = snprintf(pMessage, nMaxLength, "%.2d.%.2d.%.2d %.2d-%.2d-%.2d.%.3d: [%d] [%d] [%s] [%s] ", pTime.tm_year % 100, pTime.tm_mon + 1, pTime.tm_mday, pTime.tm_hour, pTime.tm_min, pTime.tm_sec, (int)nTimeMS, (int)pid, (int)tid, GProcName, lpScope);
	int nMessage = vsnprintf(pMessage + nLength, nMaxLength - nLength, lpFmt, args);
	nLength += (nMessage < 0 ? 0 : (nMessage >= nMaxLength - nLength ? nMaxLength - nLength - 1 : nMessage));

	*(pMessage + nLength) = '\n';

	long nLogSize = LogRMessage(pMessage, nLength + 1);

#if defined(LOG_SYNC) && !(defined(__APPLE__) && (TARGET_OS_IPHONE == 1) || defined(__ANDROID__))
	if(fLock)
		pLock.Unlock();
	else
		pLock.Reset();
#endif

#ifdef LOG_DYN
	delete pMessage;
#endif

}

long LogRMessage(char *lpText, int nLength)
{

	long nFileSize = 0;
	FILE *pFile = fopen(GLogFile, "a");
	if (pFile)
	{
#ifndef WIN32
		if(!ftell(pFile))
			chmod(GLogFile, 0666);
#endif
		fwrite(lpText, 1, nLength, pFile);
		fflush(pFile);
		nFileSize = ftell(pFile);
		fclose(pFile);
	}

	if(GLogDebug)
	{
		fwrite(lpText, 1, nLength, stdout);
		fflush(stdout);
	}

	return nFileSize;
}

