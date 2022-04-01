//
//  @file ConfigFileInfo.h
//  @author Sergii Oryshchenko <sergii.orishchenko@gmail.com>
//  @see https://github.com/os-v/CrossPlatformCodeUtils/
//
//  Created on 25.08.19.
//  Copyright 2019 Sergii Oryshchenko. All rights reserved.
//

#ifndef CONFIGFILEINFO_H
#define CONFIGFILEINFO_H

#include <map>
#include <string>
#include <vector>
#include <string.h>

class CConfigFileInfo
{
public:

	static CConfigFileInfo &Instance() {
		static CConfigFileInfo pInstance;
		return pInstance;
	}
	
	CConfigFileInfo()
	{
	}

	~CConfigFileInfo()
	{
	}

	bool Load(const char *lpPath, size_t nLimit = 1024 * 1024)
	{
		m_sPath = lpPath;
		if (m_sPath.empty())
			return false;
		FILE *pFile = fopen(lpPath, "rb");
		if(!pFile)
			return false;
		fseek(pFile, 0, SEEK_END);
		size_t nResult = ftell(pFile);
		if(nResult != (size_t)-1 && nResult <= nLimit)
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
		if (!*lpPath)
			return false;
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
		m_pItems.clear();
		std::string sGroup = "";
		for(const char *lpPtr = lpData; lpPtr && *lpPtr; )
		{
			size_t iSep;
			const char *lpEnd = strchr(lpPtr, '\n');
			std::string sLine(lpPtr, lpEnd ? lpEnd - lpPtr - (lpEnd > lpPtr && *(lpEnd - 1) == '\r' ? 1 : 0) : strlen(lpPtr));
			lpPtr = lpEnd ? lpEnd + 1 : 0;
			if (!sLine.length() || sLine[0] == ';')
				continue;
			else if (sLine.length() >= 2 && sLine[0] == '[' && sLine[sLine.length() - 1] == ']')
			{
				std::string sName = sLine.substr(1, sLine.length() - 2);
				if (sName != sGroup)
					m_pItems[sGroup = sName] = CVarMap();
			}
			else if ((iSep = sLine.find('=')) != -1)
			{
				std::string sName = sLine.substr(0, iSep);
				std::string sValue = sLine.substr(iSep + 1);
				CVarMap& pGroup = m_pItems[sGroup];
				pGroup[sName] = sValue;
			}
		}
	}

	char *SaveData(size_t *pSize = 0)
	{

		char *pData = 0;
		size_t nSize = 0;

		for (CVarGroup::const_iterator pGroup = m_pItems.begin(); pGroup != m_pItems.end(); pGroup++)
		{
			nSize += pGroup->first.length() + 3 * sizeof(char);
			for (CVarMap::const_iterator pItem = pGroup->second.begin(); pItem != pGroup->second.end(); pItem++)
				nSize += pItem->first.length() + sizeof(char) + pItem->second.length() + sizeof(char);
		}

		nSize++;
		pData = new char[nSize];

		char *pDataPtr = pData;
		for (CVarGroup::const_iterator pGroup = m_pItems.begin(); pGroup != m_pItems.end(); pGroup++)
		{
			*pDataPtr++ = '[';
			strcpy(pDataPtr, pGroup->first.c_str());
			pDataPtr += pGroup->first.length();
			*pDataPtr++ = ']';
			*pDataPtr++ = '\n';
			for (CVarMap::const_iterator pItem = pGroup->second.begin(); pItem != pGroup->second.end(); pItem++)
			{
				strcpy(pDataPtr, pItem->first.c_str());
				pDataPtr += pItem->first.length();
				*pDataPtr++ = '=';
				strcpy(pDataPtr, pItem->second.c_str());
				pDataPtr += pItem->second.length();
				*pDataPtr++ = '\n';
			}
		}

		*pDataPtr++ = 0;
		if(pSize)
			*pSize = nSize;

		return pData;
	}

	void SetStr(const char *lpGroup, const char *lpName, const char *lpValue)
	{
		CVarMap &pGroup = m_pItems[lpGroup];
		pGroup[lpName] = lpValue;
	}

	void SetInt(const char *lpGroup, const char *lpName, int nValue)
	{
		char pValue[256];
		sprintf(pValue, "%d", nValue);
		SetStr(lpGroup, lpName, pValue);
	}

	void SetBool(const char *lpGroup, const char *lpName, bool fValue)
	{
		SetStr(lpGroup, lpName, fValue ? "true" : "false");
	}

	const char *GetStr(const char *lpGroup, const char *lpName, const char *lpDefault = "")
	{
		CVarGroup::iterator pGroup = m_pItems.find(lpGroup);
		if (pGroup == m_pItems.end())
			return lpDefault;
		CVarMap::iterator pIterator = pGroup->second.find(lpName);
		return pIterator == pGroup->second.end() ? lpDefault : pIterator->second.c_str();
	}

	int GetInt(const char *lpGroup, const char *lpName, int nDefault = 0)
	{
		CVarGroup::iterator pGroup = m_pItems.find(lpGroup);
		if (pGroup == m_pItems.end())
			return nDefault;
		CVarMap::iterator pIterator = pGroup->second.find(lpName);
		return pIterator == pGroup->second.end() ? nDefault : atoi(pIterator->second.c_str());
	}
	
	bool GetBool(const char *lpGroup, const char *lpName, bool fDefault = false)
	{
		CVarGroup::iterator pGroup = m_pItems.find(lpGroup);
		if (pGroup == m_pItems.end())
			return fDefault;
		CVarMap::iterator pIterator = pGroup->second.find(lpName);
		return pIterator == pGroup->second.end() ? fDefault : !strcmp(pIterator->second.c_str(), "true");
	}

	std::vector<std::string> EnumGroups()
	{
		std::vector<std::string> pResult;
		for (CVarGroup::const_iterator pGroup = m_pItems.begin(); pGroup != m_pItems.end(); pGroup++)
			pResult.push_back(pGroup->first);
		return pResult;
	}

	std::vector<std::string> EnumItems(const char *lpGroup)
	{
		std::vector<std::string> pResult;
		CVarGroup::iterator pGroup = m_pItems.find(lpGroup);
		if(pGroup != m_pItems.end())
			for (CVarMap::const_iterator pVar = pGroup->second.begin(); pVar != pGroup->second.end(); pVar++)
				pResult.push_back(pVar->first);
		return pResult;
	}

	std::vector<std::string> GetItemsArray(const char *lpGroup)
	{
		std::vector<std::string> pResult;
		CVarGroup::iterator pGroup = m_pItems.find(lpGroup);
		if(pGroup != m_pItems.end())
			for (CVarMap::const_iterator pVar = pGroup->second.begin(); pVar != pGroup->second.end(); pVar++)
				pResult.push_back(pVar->second);
		return pResult;
	}

	bool FindValue(const char *lpGroup, const char *lpName, std::string &sValue)
	{
		CVarGroup::iterator pGroup = m_pItems.find(lpGroup);
		if (pGroup == m_pItems.end())
			return false;
		CVarMap::iterator pIterator = pGroup->second.find(lpName);
		if(pIterator == pGroup->second.end())
			return false;
		sValue = pIterator->second;
		return true;
	}

	bool IsEmpty() {
		return m_pItems.empty();
	}

private:

	typedef std::map<std::string, std::string> CVarMap;
	typedef std::map<std::string, CVarMap> CVarGroup;
	CVarGroup m_pItems;

	std::string m_sPath;

};

#endif
