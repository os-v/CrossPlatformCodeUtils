//
//  @file Logger.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#ifndef LOGGER_H
#define LOGGER_H

//#define LOG_SYNC

#define LOG_MAX_MESSAGE			1024 * 1024

#include <stdarg.h>
#include <stdint.h>

enum { ELogLevelNone = 0, ELogLevelError, ELogLevelWarning, ELogLevelMessage, ELogLevelDetails };

#define ENABLE_LOG

#ifdef ENABLE_LOG
	#define DefLogFunction(LevelName, Scope)						\
		static inline void Log##LevelName(const char *lpFmt, ...)	\
		{															\
			if(!LogIsRequired(ELogLevel##LevelName))				\
				return;												\
			va_list args;											\
			va_start(args, lpFmt);									\
			LogAMessage(#Scope, lpFmt, args);					 	\
			va_end(args);											\
		}
	#define DefLogScope(Scope)										\
		DefLogFunction(Error, Scope);								\
		DefLogFunction(Warning, Scope);								\
		DefLogFunction(Message, Scope);								\
		DefLogFunction(Details, Scope);
#else
	#define DefLogScope(s)
	#define LogError
	#define LogWarning
	#define LogMessage
	#define LogDetails
#endif

uint64_t GetCurrentProcessID();
uint64_t GetCurrentThreadID();

bool IsPathExists(const char *lpPath);

void LogInitialize(const char *lpProcName, const char *lpLogPath = 0, bool fLogStd = false, int nLevel = ELogLevelMessage, bool fDebug = false);
const char *LogFilePath();
int LogGetLevel();
bool LogIsLogStd();
bool LogIsRequired(int nLevel);
void LogAMessage(const char *lpScope, const char *lpFmt, va_list args);
long LogRMessage(char *lpBuf, int nLength);

#endif
