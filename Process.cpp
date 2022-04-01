//
//  @file Process.cpp
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 10.08.21.
//  Copyright 2021 Sergii Oryshchenko. All rights reserved.
//

#include "Process.h"
#include <string.h>
#include <errno.h>
#include <memory>

#if defined(__APPLE__) && defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_OS_MAC == 1 && TARGET_OS_IOS == 0
		#include <pthread.h>
		#include <sys/proc_info.h>
		#include <libproc.h>
		#include <pwd.h>
	#endif
#elif defined(__linux__)
	#include <sys/types.h>
	#include <dirent.h>
	#include <pwd.h>
#elif defined(WIN32)
	#include <windows.h>
	#include <tlhelp32.h>
	#include <psapi.h>
	#include <process.h>
	#include <direct.h>
	#include <time.h>
#endif

DefLogScope(ProcessList)

CStdString GetProcessName(pid_t nProcessID)
{
#if defined(__APPLE__) && TARGET_OS_IOS == 0
	char szName[PROC_PIDPATHINFO_MAXSIZE] = { 0 };
	proc_pidpath(nProcessID, szName, sizeof(szName));
	const char *lpName = strrchr(szName, '/');
	return lpName ? lpName + 1 : szName;
#elif defined(__unix__)
	CStdString sName;
	if(!LoadFile(CStdStrFormat("/proc/%lld/cmdline", nProcessID).c_str(), sName, 64 * 1024))
		LoadFile(CStdStrFormat("/proc/%lld/comm", nProcessID).c_str(), sName, 64 * 1024);
	sName = sName.substr(0, strlen(sName.c_str()));
	int iName = sName.rfind('/');
	sName = sName.size() && iName != -1 ? sName.substr(iName + 1) : sName;
	if(sName.size() && sName[sName.size() - 1] == '\n')
		sName = sName.substr(0, sName.size() - 1);
	return sName;
#elif defined(WIN32)
	CHAR szName[MAX_PATH] = { 0 };
	if (nProcessID == -1)
		::GetModuleFileNameA(0, szName, MAX_PATH);
	else
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, nProcessID);
		if (!hProcess)
			return "";
		GetProcessImageFileNameA(hProcess, szName, MAX_PATH);
		CloseHandle(hProcess);
	}
	const char *lpName = strrchr(szName, '\\');
	return lpName ? lpName + 1 : szName;
#endif
}

CStdString GetProcessPath(pid_t nProcessID)
{
	CStdString sResult;
#if defined(WIN32)
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, nProcessID);
	if(hProcess)
	{
		sResult.resize(1024);
		DWORD nResult = GetModuleFileNameExA(hProcess, 0, &sResult[0], sResult.size());
		//DWORD nResult = GetProcessImageFileNameA(hProcess, &sResult[0], sResult.size());
		sResult.resize(nResult);
		CloseHandle(hProcess);
	}
#endif
	return sResult;
}

uid_t GetProcessUID(pid_t nProcessID)
{
	uid_t nUserID = EMPTY_UID;
#if defined(WIN32)
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, nProcessID), hToken = 0;
	if(hProcess && OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
	{
		nUserID.resize(1024);
		DWORD nReturned = 0;
		if(GetTokenInformation(hToken, TokenUser, nUserID.data(), nUserID.size(), &nReturned))
		{
			char *pSID = (char*)(((PTOKEN_USER)nUserID.data())->User.Sid);
			nUserID = CStdByteArray(pSID, pSID + nReturned - sizeof(TOKEN_USER));
		}
		else
			nUserID = EMPTY_UID;
		CloseHandle(hToken);
		CloseHandle(hProcess);
	}
	else if(hProcess)
		CloseHandle(hProcess);
#elif defined(__APPLE__) && TARGET_OS_IOS == 0
	proc_bsdinfo pProcInfo;
	int nResult = proc_pidinfo(nProcessID, PROC_PIDTBSDINFO, 0, &pProcInfo, PROC_PIDTBSDINFO_SIZE);
	return nResult != PROC_PIDTBSDINFO_SIZE ? 0 : pProcInfo.pbi_uid;
#elif defined(__unix__)
	//LoadFile(StrFormat("/proc/%lld/loginuid", nPID).c_str());
	FILE *pFile = fopen(CStdStrFormat("/proc/%lld/status", nProcessID).c_str(), "r");
	CStdByteArray pLine(64 * 1024);
	for(; pFile && !nUserID && !feof(pFile) && fgets(&pLine[0], pLine.size(), pFile); )
	{
		CStdStringArray pFields = StrSplit(pLine.data(), '\t', 0, false);
		if(pFields.size() > 1 && !stricmp(pFields[0].c_str(), "uid:"))
			nUserID = atoi(pFields[1].c_str());
	}
	if(pFile)
		fclose(pFile);
#endif
	return nUserID;
}

CStdArray<pid_t> GetProcessList(CStdStringArray &pProcName)
{
	CStdArray<pid_t> pResult;
#if defined(WIN32)
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return pResult;
	for (BOOL fResult = Process32First(hSnapshot, &pEntry); fResult; fResult = Process32Next(hSnapshot, &pEntry))
		pResult.push_back(pEntry.th32ProcessID), pProcName.push_back(CStdString(pEntry.szExeFile, pEntry.szExeFile + wcslen(pEntry.szExeFile)));
	CloseHandle(hSnapshot);
#elif defined(__APPLE__) && TARGET_OS_IOS == 0
	int nProc = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
	CStdArray<pid_t> pProc(nProc);
	proc_listpids(PROC_ALL_PIDS, 0, pProc.data(), nProc * sizeof(pid_t));
	for(int iProc = 0; iProc < nProc; iProc++)
		pResult.push_back(pProc[iProc]), pProcName.push_back(GetProcessName(pResult.back()));
#elif defined(__unix__)
	DIR *pDir = opendir("/proc");
	for(dirent *pEntry = 0; pDir && (pEntry = readdir(pDir)); )
		pResult.push_back(atoi(pEntry->d_name)), pProcName.push_back(pResult.back());
	if(pDir)
		closedir(pDir);
#endif
	return pResult;
}

CStdProcessArray GetProcessList(uint32_t eInfo, const char *lpProcessName, const char *lpUserInfo)
{
	CStdStringArray pProcName;
	CStdProcessArray pResult;
	eInfo |= (lpProcessName ? PROCESSINFO::FName : 0) | (lpUserInfo ? PROCESSINFO::FUID : 0);
	uid_t nFindUserID = lpUserInfo ? GetUserID(lpUserInfo) : EMPTY_UID;
	CStdArray<pid_t> pProcList = GetProcessList(pProcName);
	for (int iProc = 0; iProc < pProcList.size(); iProc++)
	{
		pid_t nProcessID = pProcList[iProc];
		PROCESSINFO pProcess = { nProcessID, 0, EMPTY_UID, "", "" };
		if(eInfo & PROCESSINFO::FName)
		{
			pProcess.Name = pProcName[iProc].c_str();
			if(lpProcessName && stricmp(lpProcessName, pProcess.Name.c_str()))
				continue;
		}
		if(eInfo & PROCESSINFO::FUID || eInfo & PROCESSINFO::FUser)
		{
			pProcess.UID = GetProcessUID(pProcess.PID);
			if(lpUserInfo && (nFindUserID == EMPTY_UID || nFindUserID != pProcess.UID))
				continue;
		}
		if(eInfo & PROCESSINFO::FPath)
			pProcess.Path = GetProcessPath(pProcess.PID);
		if(eInfo & PROCESSINFO::FUser)
			pProcess.User = GetUserInfo(pProcess.UID);
		pResult.push_back(pProcess);
	}
	return pResult;
}

