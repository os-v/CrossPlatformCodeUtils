//
//  @file SharedLibrary.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 09.07.21.
//  Copyright 2021 Sergii Oryshchenko. All rights reserved.
//

#ifndef SHAREDLIBRARY_H
#define SHAREDLIBRARY_H

#ifdef _WIN32
	#include <windows.h>
	#define RESOLVELIBSYM(Lib, Name)		Name = GetProcAddress((HMODULE)Lib, #Name);
	#define RESOLVELIBFUNC(Lib, Name)		Name = (F##Name)GetProcAddress((HMODULE)Lib, #Name);
#else
	#include <dlfcn.h>
	#define RESOLVELIBSYM(Lib, Name)		Name = dlsym(Lib, #Name);
	#define RESOLVELIBFUNC(Lib, Name)		Name = (F##Name)dlsym(Lib, #Name);
#endif

#ifndef __stdcall
	#define __stdcall
#endif

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define DECLEXPORT		extern "C" __declspec(dllexport)
	#define DECLIMPORT		extern "C" __declspec(dllimport)
#else
	#define DECLEXPORT		extern "C" __attribute__((visibility("default")))
	#define DECLIMPORT		extern "C" __attribute__((visibility("default")))
#endif

#define SHAREDLIBRARY_NAME(sName)		bool Create() { return TSharedLibraryObjectClass::Create(sName); }

template<class TObject>
class CSharedLibraryObject
{
public:

	typedef CSharedLibraryObject<TObject> TSharedLibraryObjectClass;

	static TObject &Instance(const char *lpLibrary = 0) {
		static TObject pInstance;
		static bool fInitialized = false;
		if(!fInitialized && lpLibrary)// || !pInstance.m_pLibrary)
		{
			if(!(fInitialized = pInstance.Create(lpLibrary)))
				pInstance.Destroy();
		}
		return pInstance;
	}

	CSharedLibraryObject()
	{
		m_pLibrary = 0;
	}

	~CSharedLibraryObject()
	{
		Destroy();
	}

	bool Create(const char *lpLibraryPath)
	{
#ifdef _WIN32
		m_pLibrary = (void*)LoadLibraryA(lpLibraryPath);//, 0, LOAD_LIBRARY_SEARCH_USER_DIRS);
#else
		m_pLibrary = dlopen(lpLibraryPath, RTLD_LOCAL | RTLD_NOW);
#endif
		if(!m_pLibrary)
			return false;
		if(!((TObject*)this)->ResolveSymbols())
			return false;
		return true;
	}

	void Destroy()
	{
		if (m_pLibrary)
		{
#ifdef _WIN32
			FreeLibrary((HMODULE)m_pLibrary);
#else
			dlclose(m_pLibrary);
			dlclose(m_pLibrary);
#endif
			m_pLibrary = 0;
		}
		((TObject*)this)->ResetSymbols();
	}

	bool IsInitialized() {
		return m_pLibrary != 0;
	}

	char *GetError() {
#ifdef _WIN32
		return 0;
#else
		return dlerror();
#endif
	}

	void ResetSymbols()
	{
	}

protected:

	void *m_pLibrary;

};

#endif
