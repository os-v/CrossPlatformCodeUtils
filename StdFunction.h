//
//  @file StdFunction.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 16.07.21.
//  Copyright 2021 Sergii Oryshchenko. All rights reserved.
//

#ifndef STDFUNCTION_H
#define STDFUNCTION_H

#include <functional>

#define CStdDefFunction(TFuncDef)		CStdFunction<std::remove_pointer<TFuncDef>::type>

template<typename TFuncDef>
class CStdFunction : public std::function<TFuncDef>
{
public:

	typedef TFuncDef TFuncType;

	CStdFunction() : std::function<TFuncDef>(), m_pFuncPtr(0)
	{
	}

	CStdFunction(TFuncDef pFunc) : std::function<TFuncDef>(pFunc), m_pFuncPtr(&pFunc)
	{
	}

	CStdFunction(void *pFunc) : std::function<TFuncDef>((TFuncDef*)pFunc), m_pFuncPtr((TFuncDef*)pFunc)
	{
	}

	TFuncDef *GetPtr() {
		return m_pFuncPtr;
	}

private:

	TFuncDef *m_pFuncPtr;

};

#endif
