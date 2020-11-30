//
//  @file SyncMutex.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 15.10.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#ifndef SYNCMUTEX_H
#define SYNCMUTEX_H

#if defined(WIN32)

#include "SyncNamedMutex.h"

class CSyncMutex : public CSyncNamedMutex
{
public:

	CSyncMutex() : CSyncNamedMutex(0)
	{
	}

	~CSyncMutex()
	{
	}

protected:

};

#else

#include <pthread.h>
#include "SyncSharedMemory.h"

class CSyncMutex
{
public:

	CSyncMutex()
	{
		pthread_mutex_init(&m_pMutex, 0);
	}

	~CSyncMutex()
	{
		pthread_mutex_destroy(&m_pMutex);
	}

	bool Lock(uint32_t nTimeout = -1)
	{

		if(nTimeout <= 0)
		{
			pthread_mutex_lock(&m_pMutex);
			return true;
		}

		/*timespec pTimeout;
		clock_gettime(CLOCK_REALTIME, &pTimeout);
		pTimeout.tv_sec += nTimeout / 1000;
		pTimeout.tv_nsec += (nTimeout % 1000) * 1000000;
		int nResult = pthread_mutex_timedlock(&m_pMutex, &pTimeout);*/
		//need replacement with elapsed time
		for(uint32_t nWait = 0; nWait < nTimeout; nWait += 10)
		{
			if(!pthread_mutex_trylock(&m_pMutex))
				return true;
			usleep(10 * 1000);
		}

		return false;
	}
	
	void Unlock()
	{
		pthread_mutex_unlock(&m_pMutex);
	}
	
protected:

	pthread_mutex_t m_pMutex;

};

#endif

#endif
