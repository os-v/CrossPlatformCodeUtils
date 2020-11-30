//
//  @file SyncNamedMutex.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 15.10.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#ifndef SYNCNAMEDMUTEX_H
#define SYNCNAMEDMUTEX_H

#if defined(WIN32)

#include <windows.h>

class CSyncNamedMutex
{
public:

	CSyncNamedMutex() : m_hMutex(0), m_fCreated(false)
	{
	}

	CSyncNamedMutex(const char *lpName) : m_hMutex(0), m_fCreated(false)
	{
		Create(lpName);
	}

	~CSyncNamedMutex()
	{
		Destroy();
	}

	bool Create(const char *lpName)
	{
		m_hMutex = CreateMutexA(0, false, lpName);
		if(!m_hMutex)
			return false;
		m_fCreated = GetLastError() != ERROR_ALREADY_EXISTS;
		return true;
	}

	void Destroy()
	{
		if(m_hMutex)
		{
			CloseHandle(m_hMutex);
			m_hMutex = 0;
		}
	}

	void Reset()
	{

	}

	bool Lock(uint32_t nTimeout = -1)
	{
		return m_hMutex ? WaitForSingleObject(m_hMutex, nTimeout) == WAIT_OBJECT_0 : false;
	}

	void Unlock()
	{
		ReleaseMutex(m_hMutex);
	}

	bool IsReady()
	{
		return m_hMutex != 0;
	}

	bool IsOpened()
	{
		return m_hMutex && !m_fCreated;
	}

protected:

	HANDLE m_hMutex;
	bool m_fCreated;

};

#else

#include "SyncSharedMemory.h"
#include <pthread.h>

class CSyncNamedMutex
{
public:

	CSyncNamedMutex() : m_pMutex(0)
	{
	}

	CSyncNamedMutex(const char *lpName) : m_pMutex(0)
	{
		Create(lpName);
	}

	~CSyncNamedMutex()
	{
		Destroy();
	}

	bool Create(const char *lpName)
	{
		if(!m_pMemory.Create(lpName, sizeof(pthread_mutex_t)))
			return false;
		m_pMutex = (pthread_mutex_t*)(m_pMemory.Data());
		if(!m_pMemory.IsOpened())
			Reset();
		return true;
	}

	void Destroy()
	{
		if(m_pMutex)
		{
			pthread_mutex_destroy(m_pMutex);
			m_pMutex = 0;
		}
		m_pMemory.Destroy();
	}

	void Reset()
	{
		if(!m_pMutex)
			return;
		pthread_mutexattr_t pAttr;
		pthread_mutexattr_init(&pAttr);
		pthread_mutexattr_setpshared(&pAttr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(m_pMutex, &pAttr);
		pthread_mutexattr_destroy(&pAttr);
	}

	bool Lock(uint32_t nTimeout = -1)
	{

		if(!m_pMutex)
			return false;

		if(nTimeout == -1)
		{
			pthread_mutex_lock(m_pMutex);
			return true;
		}

		/*timespec pTimeout;
		clock_gettime(CLOCK_REALTIME, &pTimeout);
		pTimeout.tv_sec += nTimeout / 1000;
		pTimeout.tv_nsec += (nTimeout % 1000) * 1000000;
		int nResult = pthread_mutex_timedlock(m_pMutex, &pTimeout);*/
		//need replacement with elapsed time
		for(uint32_t nWait = 0; nWait < nTimeout; nWait += 10)
		{
			if(!pthread_mutex_trylock(m_pMutex))
				return true;
			usleep(10 * 1000);
		}

		return false;
	}
	
	void Unlock()
	{
		pthread_mutex_unlock(m_pMutex);
	}

	bool IsReady()
	{
		return m_pMutex != 0;
	}

	bool IsOpened()
	{
		return m_pMemory.IsOpened();
	}

protected:

	CSyncSharedMemory m_pMemory;

	pthread_mutex_t *m_pMutex;

};

#endif

#endif
