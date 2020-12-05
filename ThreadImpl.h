//
//  @file ThreadImpl.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 15.10.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#ifndef THREADIMPL_H
#define THREADIMPL_H

#include <stdint.h>

#if defined(WIN32)
	#include <windows.h>
#else
	#include <pthread.h>
	#include <signal.h>
#endif

#include "SyncMutex.h"

class CThreadImpl
{
public:

	typedef uint32_t (*OnThreadProcFunc)(CThreadImpl *pThread, void *pContext);

	static CThreadImpl *CreateThread(OnThreadProcFunc pExtFunc, void *pExtCtx)
	{
		CThreadImpl *pThread = new CThreadImpl();
		if(pThread->Create(pExtFunc, pExtCtx))
			return pThread;
		delete pThread;
		return 0;
	}

	CThreadImpl()
	{

		m_fAbort = false;

		m_pExtThreadCtx = 0;
		m_pExtThreadFunc = 0;

		m_hThread = 0;

	}

	~CThreadImpl()
	{

		Destroy();

	}

	bool Create(OnThreadProcFunc pExtFunc = 0, void *pExtCtx = 0)
	{
		m_pExtThreadCtx = pExtCtx;
		m_pExtThreadFunc = pExtFunc;
#if defined(WIN32)
		m_hThread = ::CreateThread(0, 0, SOnThreadProc, this, 0, 0);
		return m_hThread;
#else
		int nRet = pthread_create(&m_hThread, 0, SOnThreadProc, this);
		return !nRet;
#endif
	}

	void Destroy()
	{
		if(m_hThread)
		{
			Abort(-1);
			m_hThread = 0;
		}
	}

	bool Wait(uint32_t nTimeout)
	{
		return WaitThread(nTimeout);
	}

	bool Term()
	{
		return TermThread();
	}

	bool Abort(uint32_t nTimeout)
	{
		m_fAbort = true;
		return !nTimeout ? true : WaitThread(nTimeout);
	}

	bool IsAbort() {
		return m_fAbort;
	}

protected:

	volatile bool m_fAbort;

	void *m_pExtThreadCtx;
	OnThreadProcFunc m_pExtThreadFunc;

	CSyncMutex m_pMutex;

#if defined(WIN32)
	HANDLE m_hThread;
	static DWORD WINAPI SOnThreadProc(void *pContext) {
		return ((CThreadImpl*)pContext)->OnThreadProc();
	}
	bool WaitThread(uint32_t nTimeout) {
		return WaitForSingleObject(m_hThread, nTimeout) == WAIT_OBJECT_0;
	}
	bool TermThread() {
		return TerminateThread(m_hThread, 0);
	}
#else
	pthread_t m_hThread;
	static void *SOnThreadProc(void *pContext) {
		return (void*)((CThreadImpl*)pContext)->OnThreadProc();
	}
	bool WaitThread(uint32_t nTimeout) {
		int nRet = 0;
		if(nTimeout == -1)
			nRet = pthread_join(m_hThread, 0);
		else
		{
			if(!m_pMutex.Lock(nTimeout))
				return false;
			m_pMutex.Unlock();
			nRet = pthread_join(m_hThread, 0);
			/*timespec pTimeSpec;
			clock_gettime(CLOCK_REALTIME, &pTimeSpec);
			pTimeSpec.tv_sec += nTimeout / 1000;
			pTimeSpec.tv_nsec += (nTimeout % 1000) * 1000 * 1000;
			nRet = pthread_timedjoin_np(m_hThread, 0, &pTimeSpec);*/
		}
		return !nRet;
	}
	bool TermThread() {
		int nRet = pthread_kill(m_hThread, SIGTERM);
		return !nRet;
	}
#endif

	virtual uint32_t OnThreadProc()
	{
		uint32_t nReturn = 0;
		m_pMutex.Lock();
		if(m_pExtThreadFunc)
			nReturn = m_pExtThreadFunc(this, m_pExtThreadCtx);
		m_pMutex.Unlock();
		return nReturn;
	}

};

#endif

