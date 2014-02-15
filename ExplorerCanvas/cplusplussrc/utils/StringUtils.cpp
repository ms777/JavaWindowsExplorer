/*
 * Copyright 2014 Martin Schell

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <iostream>
#include <windows.h>
#include "StringUtils.h"
#include "Log.h"

using namespace std;

string wstrtostr(const wstring &wstr) {
	std::string strTo;

	int nUserNameLenUnicode = lstrlenW( wstr.c_str() );
    int nUserNameLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), nUserNameLenUnicode, NULL, 0, NULL, NULL );
	char *szTo = new char[nUserNameLen + 1];
    szTo[nUserNameLen] = '\0';
//    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, nUserNameLenUnicode, NULL, NULL);

    strTo = szTo;
    delete[] szTo;
    return strTo;
}

char* wstrtochar(const wstring &wstr) {
	std::string strTo;

	int nUserNameLenUnicode = lstrlenW( wstr.c_str() );
    int nUserNameLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), nUserNameLenUnicode, NULL, 0, NULL, NULL );
	char *szTo = new char[nUserNameLen + 1];
    szTo[nUserNameLen] = '\0';
//    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, nUserNameLenUnicode, NULL, NULL);

    return szTo;
}

char* wchartochar(const WCHAR* wstr) {
	std::string strTo;

	int nUserNameLenUnicode = lstrlenW( wstr );
    int nUserNameLen = WideCharToMultiByte(CP_ACP, 0, wstr, nUserNameLenUnicode, NULL, 0, NULL, NULL );
	char *szTo = new char[nUserNameLen + 1];
    szTo[nUserNameLen] = '\0';
//    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, szTo, nUserNameLenUnicode, NULL, NULL);

    return szTo;
}

LPWSTR chartowstr(const char *nativeString) {
	int lenA = lstrlenA(nativeString);
	int lenW;
	LPWSTR unicodestr;

	lenW = MultiByteToWideChar(CP_ACP, 0, nativeString, lenA, 0, 0);
	if (lenW > 0) {
		// Check whether conversion was successful
		unicodestr = new wchar_t[lenW+1];
		MultiByteToWideChar(CP_ACP, 0, nativeString, lenA, unicodestr, lenW);
	} else	{
		LOG(logERROR) << "chartowstr:string conversion error: lenA: " << lenA << ", lenW: " << lenW << ", string: " << nativeString;
	}
	unicodestr[lenW] = 0;
	return unicodestr;
}

char* GetDisplayNameOf(	IShellFolder* shellFolder, LPITEMIDLIST pidl, SHGDNF uFlags) {
	STRRET StrRet;
	StrRet.uType = STRRET_WSTR;

	HRESULT hr = shellFolder->GetDisplayNameOf((PCUITEMID_CHILD) pidl, SHGDN_NORMAL + SHGDN_FORPARSING, &StrRet);
	if (FAILED(hr)) {
		LOG(logERROR) << "StringUtils:GetDisplayNameOf:hr:  " << (long) hr;
		return NULL;
	}

	char* DisplayName;
	switch (StrRet.uType) {
	case STRRET_CSTR :
		LOG(logERROR) << "StringUtils:GetDisplayNameOf:unexpected return type:STRRET_CSTR :  " << StrRet.cStr;
		DisplayName = StrRet.cStr;
		break;
	case STRRET_WSTR :
		DisplayName = wstrtochar(StrRet.pOleStr);
		break;
	case STRRET_OFFSET :
		LOG(logERROR) << "StringUtils:GetDisplayNameOf:unexpected return type:STRRET_OFFSET :  " << (((char*)pidl) + StrRet.uOffset);
		DisplayName = ((char*)pidl) + StrRet.uOffset;
	}

	return DisplayName;
}

string getGuidDescription(REFGUID guidService) {
	ostringstream os;

	WCHAR szGuid[40]={0};

	int nCount = StringFromGUID2(guidService, szGuid, 40);
	szGuid[nCount] = 0;

	os << wchartochar(szGuid) << " ";

	HKEY hkResult;

	HRESULT hr1 = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"Interface", 0, KEY_READ, &hkResult);
	if (hr1 == ERROR_SUCCESS) {
		int SIZE = 200;
		WCHAR value[SIZE];
		unsigned long int value_length = SIZE;
		hr1 = RegGetValueW(hkResult, szGuid, NULL, RRF_RT_ANY, NULL, (LPBYTE) &value, &value_length);
		if (hr1 == ERROR_SUCCESS) {
			os << wchartochar(value);
		} else {
			os << "not found in registry";
		}
		RegCloseKey(hkResult);
	} else {
		os << "could not open registry: " << (long) hr1;
	}
	return os.str().c_str();
}

