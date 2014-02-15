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

#ifndef searchms777_H_
#define searchms777_H_

#include <windows.h>
#include <stdio.h>
#include "shlwapi.h"
#include "shlobj.h"
#include "shobjidl.h" // for ICommDlgBrowser2
#include <ole2.h>
#include <Propvarutil.h>
#include "structuredquery.h"

DEFINE_GUID (CLSID_QueryParserManager, 0x5088B39A, 0x29B4, 0x4d9d, 0x82, 0x45, 0x4E, 0xE2, 0x89, 0x22, 0x2f, 0x66);
DEFINE_GUID (CLSID_ConditionFactory,   0xE03E85B0, 0x7BE3, 0x4000, 0xBA, 0x98, 0x6C, 0x13, 0xDE, 0x9F, 0xA4, 0x86);
DEFINE_GUID (IID_IQueryParserManager,  0xA879E3C4, 0xAF77, 0x44fb, 0x8F, 0x37, 0xEB, 0xD1, 0x48, 0x7C, 0xF9, 0x20);
DEFINE_GUID (IID_IQueryParser,         0x2EBDEE67, 0x3505, 0x43f8, 0x99, 0x46, 0xEA, 0x44, 0xAB, 0xC8, 0xE5, 0xB0);
DEFINE_GUID (IID_IConditionFactory,    0xA5EFE073, 0xB16F, 0x474f, 0x9F, 0x3E, 0x9F, 0x8B, 0x49, 0x7A, 0x3E, 0x08);

HRESULT AddCustomCondition(ISearchFolderItemFactory *psfif);
HRESULT AddStructuredQueryCondition(ISearchFolderItemFactory *psfif, IQueryParser *pqp, PCWSTR pszQuery);
HRESULT ParseStructuredQuery(PCWSTR pszString, IQueryParser *pqp, ICondition **ppc);
HRESULT CreateQueryParser(IQueryParser **ppqp);

IShellItemArray* CreateShellItemArrayFromLPWSTRarray(LPWSTR path[], int size);
IShellItemArray* CreateShellItemArrayFromSampleScope();

HRESULT GetQueryItem(REFIID riid, void **ppv, IQueryParser* _pqp, IShellItemArray *psia, PCWSTR displayname, PCWSTR searchstring);


#endif
