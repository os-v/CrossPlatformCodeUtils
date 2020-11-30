//
//  @file SyncSharedMemory.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 15.10.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#ifndef SYNCSHAREDMEMORY_H
#define SYNCSHAREDMEMORY_H

#if defined(WIN32)

class CSyncSharedMemory
{
public:

	CSyncSharedMemory() : m_hMap(0), m_pMem(0), m_nSize(0), m_fCreated(false)
	{
	}

	CSyncSharedMemory(const char *lpName, uint64_t nSize) : m_hMap(0), m_pMem(0), m_nSize(0), m_fCreated(false)
	{
		Create(lpName, nSize);
	}

	~CSyncSharedMemory()
	{
		Destroy();
	}

	bool Create(const char *lpName, uint64_t nSize)
	{
		m_hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, ((PULARGE_INTEGER)&nSize)->HighPart, ((PULARGE_INTEGER)&nSize)->LowPart, lpName);
		if(!m_hMap)
			return false;
		m_nSize = nSize;
		m_fCreated = GetLastError() != ERROR_ALREADY_EXISTS;
		m_pMem = MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, (SIZE_T)nSize);
		return m_pMem != 0;
	}

	void Destroy()
	{
		if(m_pMem)
		{
			UnmapViewOfFile(m_pMem);
			m_pMem = 0;
		}
		if(m_hMap)
		{
			CloseHandle(m_hMap);
			m_hMap = 0;
		}
	}

	bool IsReady()
	{
		return m_pMem != 0;
	}

	bool IsOpened()
	{
		return !m_fCreated;
	}

	char *Data()
	{
		return (char*)m_pMem;
	}

	uint64_t Size()
	{
		return m_nSize;
	}

protected:

	uint64_t m_nSize;
	bool m_fCreated;

	HANDLE m_hMap;
	void *m_pMem;

};

#else

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class CSyncSharedMemory
{
public:

	CSyncSharedMemory() : m_lpName(0), m_hMem(-1), m_fCreated(false), m_pMem(0)
	{
	}

	CSyncSharedMemory(const char *lpName, uint64_t nSize) : m_lpName(0), m_hMem(-1), m_fCreated(false), m_pMem(0)
	{
		Create(lpName, nSize);
	}

	~CSyncSharedMemory()
	{
		Destroy();
	}

	bool Create(const char *lpName, uint64_t nSize)
	{
		m_lpName = lpName;
#if defined(__ANDROID__)
		return false;
#else
		m_fCreated = true;
		m_hMem = shm_open(lpName, O_RDWR | O_CREAT | O_TRUNC/*O_EXCL*/, 0666/*S_IRWXU*/);
		if(m_hMem != -1)
			ftruncate(m_hMem, nSize);
		else
		{
			m_hMem = shm_open(lpName, O_RDWR, 0666);
			if(m_hMem == -1)
				return false;
			m_fCreated = false;
		}
#endif
		fstat(m_hMem, &m_pStat);
		if(!m_pStat.st_size)
			return false;
		m_pMem = mmap(0, m_pStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_hMem, 0);
		return true;
	}

	void Destroy()
	{
		if(m_pMem)
		{
			munmap(m_pMem, m_pStat.st_size);
			m_pMem = 0;
		}
		if(m_hMem != -1 && m_fCreated)
		{
#if !defined(__ANDROID__)
			shm_unlink(m_lpName);
#endif
			m_hMem = -1;
		}
	}

	bool IsReady()
	{
		return m_pMem != 0;
	}

	bool IsOpened()
	{
		return !m_fCreated;
	}

	char *Data()
	{
		return (char*)m_pMem;
	}

	uint64_t Size()
	{
		return m_pMem ? m_pStat.st_size : 0;
	}

protected:

	const char *m_lpName;

	int m_hMem;
	bool m_fCreated;

	void *m_pMem;
	struct stat m_pStat;

};

#endif

#endif

