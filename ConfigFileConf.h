//
//  @file ConfigFileConf.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 20.08.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#ifndef CONFIGFILECONF_H
#define CONFIGFILECONF_H

#include <map>
#include <string>
#include <vector>
#include <string.h>

#define CONFIGFILE_MAP

class CConfigFileConf
{
public:

	static CConfigFileConf &Instance() {
		static CConfigFileConf pInstance;
		return pInstance;
	}
	
	CConfigFileConf()
	{
	}

	~CConfigFileConf()
	{
	}

	bool Load(const char *lpPath, long nLimit = 1024 * 1024)
	{
		m_sPath = lpPath;
		FILE *pFile = fopen(lpPath, "rb");
		if(!pFile)
			return false;
		fseek(pFile, 0, SEEK_END);
		size_t nResult = ftell(pFile);
		if(nResult != -1 && nResult <= nLimit)
		{
			fseek(pFile, 0, SEEK_SET);
			char *pData = new char[nResult + 1];
			size_t nData = fread(pData, 1, nResult, pFile);
			pData[nResult] = 0;
			if(nData == nResult)
				LoadData(pData);
			delete[] pData;
		}
		fclose(pFile);
		return true;
	}

	bool Save(const char *lpPath = 0)
	{
		if(!lpPath)
			lpPath = m_sPath.c_str();
		FILE *pFile = fopen(lpPath, "wb");
		if(!pFile)
			return false;
		size_t nData = 0;
		char *pData = SaveData(&nData);
		size_t nResult = fwrite(pData, 1, nData - 1, pFile);
		delete pData;
		fclose(pFile);
		return nResult == nData;
	}

	void LoadData(const char *lpData)
	{

		m_pVar.clear();
		for(const char *lpPtr = lpData; lpPtr && *lpPtr; )
		{
			const char *lpEnd = strchr(lpPtr, '\n');
			std::string sLine(lpPtr, lpEnd ? lpEnd - lpPtr - (lpEnd > lpPtr && *(lpEnd - 1) == '\r' ? 1 : 0) : strlen(lpPtr));
			lpPtr = lpEnd ? lpEnd + 1 : 0;
			size_t iSep = sLine.find('=');
			if(iSep == -1)
				continue;
			std::string sName = sLine.substr(0, iSep);
			if(sName.length() && sName[0] == '#')
				continue;
			std::string sValue = sLine.substr(iSep + 1);
			if(sName.empty())
				continue;
			m_pVar[sName] = sValue;
		}
	}

	char *SaveData(size_t *pSize = 0)
	{

		char *pData = 0;
		size_t nSize = 0;

		for(CVarMap::const_iterator pItem = m_pVar.begin(); pItem != m_pVar.end(); pItem++)
			nSize += pItem->first.length() + sizeof(char) + pItem->second.length() + sizeof(char);

		nSize++;
		pData = new char[nSize];

		char *pDataPtr = pData;
		for(CVarMap::const_iterator pItem = m_pVar.begin(); pItem != m_pVar.end(); pItem++)
		{
			strcpy(pDataPtr, pItem->first.c_str());
			pDataPtr += pItem->first.length();
			*pDataPtr++ = '=';
			strcpy(pDataPtr, pItem->second.c_str());
			pDataPtr += pItem->second.length();
			*pDataPtr++ = '\n';
		}

		*pDataPtr++ = 0;
		if(pSize)
			*pSize = nSize;

		return pData;
	}

	void SetStr(const char *lpName, const char *lpValue)
	{
		m_pVar[lpName] = lpValue;
	}

	void SetInt(const char *lpName, int nValue)
	{
		char pValue[256];
		sprintf(pValue, "%d", nValue);
		SetStr(lpName, pValue);
	}

	void SetBool(const char *lpName, bool fValue)
	{
		SetStr(lpName, fValue ? "true" : "false");
	}

	const char *GetStr(const char *lpName, const char *lpDefault = "")
	{
		CVarMap::iterator pIterator = m_pVar.find(lpName);
		return pIterator == m_pVar.end() ? lpDefault : pIterator->second.c_str();
	}

	int GetInt(const char *lpName, int nDefault = 0)
	{
		CVarMap::iterator pIterator = m_pVar.find(lpName);
		return pIterator == m_pVar.end() ? nDefault : atoi(pIterator->second.c_str());
	}
	
	bool GetBool(const char *lpName, bool fDefault = false)
	{
		CVarMap::iterator pIterator = m_pVar.find(lpName);
		return pIterator == m_pVar.end() ? fDefault : !strcmp(pIterator->second.c_str(), "true");
	}

	bool FindValue(const char *lpName, std::string &sValue)
	{
#ifdef CONFIGFILE_MAP
		CVarMap::iterator pIterator = m_pVar.find(lpName);
		if(pIterator == m_pVar.end())
			return false;
		sValue = pIterator->second;
#else
		CVarVec::iterator pIterator = std::find(m_pName.begin(), m_pName.end(), lpName);
		if(pIterator == m_pName.end())
			return false;
		sValue = *pIterator;
#endif
		return true;
	}

	bool IsEmpty() {
#ifdef CONFIGFILE_MAP
		return m_pVar.empty();
#else
		return m_pName.empty()
#endif
	}

private:

#ifdef CONFIGFILE_MAP
	typedef std::map<std::string, std::string> CVarMap;
	CVarMap m_pVar;
#else
	std::vector<std::string> CVarVec;
	CVarVec m_pName;
	CVarVec m_pValue;
#endif

	std::string m_sPath;

};

#endif
