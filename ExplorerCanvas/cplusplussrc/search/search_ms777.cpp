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

#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#define COBJMACROS

//#include <iostream>
//#include <new>  // std::nothrow
#include <windows.h>
//#include "ShellHelpers.h"
#include <stdio.h>
#include "shlwapi.h"
#include "shlobj.h"
#include "shobjidl.h" // for ICommDlgBrowser2
#include <ole2.h>
//#include "missing.h"
#include <Propvarutil.h>
#include "structuredquery.h"
#include "search.h"
#include "StringUtils.h"
#include "Log.h"

#define IDT_SEARCHSTART     1       // ID for the search timer
#define SEARCH_TIMER_DELAY  250     // 250 milliseconds timeout



struct
{
    PCWSTR pszPropertyName;
    PCWSTR pszSemanticType;
}
const g_rgGenericProperties[] =
{
    { L"System.Generic.String",          L"System.StructuredQueryType.String" },
    { L"System.Generic.Integer",         L"System.StructuredQueryType.Integer" },
    { L"System.Generic.DateTime",        L"System.StructuredQueryType.DateTime" },
    { L"System.Generic.Boolean",         L"System.StructuredQueryType.Boolean" },
    { L"System.Generic.FloatingPoint",   L"System.StructuredQueryType.FloatingPoint" }
};




//http://msdn.microsoft.com/en-us/library/windows/desktop/dd378457(v=vs.85).aspx
//const CLSID FOLDERID_PicturesLibrary = {0xA990AE9F, 0xA03B, 0x4E80,{0x94, 0xBC, 0x99, 0x12, 0xD7, 0x50, 0x41, 0x04}};

//FOLDERID_PublicPictures GUID{B6EBFB86-6907-413C-9AF7-4FC2ABF07CC5}
//const CLSID FOLDERID_PublicPictures = {0xB6EBFB86, 0x6907, 0x413C,{0x9A, 0xF7, 0x4F, 0xC2, 0xAB, 0xF0, 0x7C, 0xC5}};
//FOLDERID_Pictures GUID{33E28130-4E1E-4676-835A-98395C3BC3BB}
//const CLSID FOLDERID_Pictures = {0x33E28130, 0x4E1E, 0x4676,{0x83, 0x5A, 0x98, 0x39, 0x5C, 0x3B, 0xC3, 0xBB}};

HRESULT CreateQueryParser(IQueryParser **ppqp)
{

	*ppqp = NULL;
    IQueryParserManager *pqpm;

    //    HRESULT hr = CoCreateInstance(__uuidof(QueryParserManager), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pqpm));
	HRESULT hr = CoCreateInstance(CLSID_QueryParserManager, NULL, CLSCTX_INPROC_SERVER, IID_IQueryParserManager, reinterpret_cast< void **> (&pqpm));

    if (SUCCEEDED(hr))
    {
        IQueryParser *pqp;
        //        hr = pqpm->CreateLoadedParser(L"SystemIndex", LOCALE_USER_DEFAULT, IID_IQueryParser, reinterpret_cast< void **>(&pqp));
//        hr = pqpm->CreateLoadedParser(L"SystemIndex", LOCALE_SYSTEM_DEFAULT, IID_IQueryParser, reinterpret_cast< void **>(&pqp));
        //        hr = pqpm->CreateLoadedParser(L"SystemIndex", MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), IID_IQueryParser, reinterpret_cast< void **>(&pqp));
//        hr = pqpm->CreateLoadedParser(L"SystemIndex", MAKELANGID(LANG_SYSTEM_DEFAULT, SUBLANG_SYS_DEFAULT), IID_IQueryParser, reinterpret_cast< void **>(&pqp));
//        hr = pqpm->CreateLoadedParser(L"SystemIndex", MAKELANGID(LANG_INVARIANT, SUBLANG_NEUTRAL), IID_IQueryParser, reinterpret_cast< void **>(&pqp));
        hr = pqpm->CreateLoadedParser(L"SystemIndex", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), IID_IQueryParser, reinterpret_cast< void **>(&pqp));
        if (SUCCEEDED(hr))
        {
            // Initialize the query parser and set default search property types
            hr = pqpm->InitializeOptions(FALSE, TRUE, pqp);
            pqpm->SetOption(QPMO_PRELOCALIZED_SCHEMA_BINARY_PATH, FALSE);
            for (int i = 0; i < ARRAYSIZE(g_rgGenericProperties) && SUCCEEDED(hr); i++)
            {
                PROPVARIANT propvar;
                hr = InitPropVariantFromString(g_rgGenericProperties[i].pszPropertyName, &propvar);
                if (SUCCEEDED(hr))
                {
                    hr = pqp->SetMultiOption(SQMO_DEFAULT_PROPERTY, g_rgGenericProperties[i].pszSemanticType, &propvar);
                    PropVariantClear(&propvar);
                }
            }

            if (SUCCEEDED(hr))
            {
//                pqp->QueryInterface(IID_PPV_ARGS(ppqp));
                pqp->QueryInterface(IID_IQueryParser, (void**) ppqp);
            }
            pqp->Release();
        }
        pqpm->Release();
    }
    return hr;

}

HRESULT ParseStructuredQuery(PCWSTR pszString, IQueryParser *pqp, ICondition **ppc)
{
    *ppc = NULL;

    IQuerySolution *pqs;
    HRESULT hr = pqp->Parse(pszString, NULL, &pqs);
    if (SUCCEEDED(hr))
    {
        ICondition *pc;
        hr = pqs->GetQuery(&pc, NULL);
        if (SUCCEEDED(hr))
        {
            SYSTEMTIME st;
            GetLocalTime(&st);
            hr = pqs->Resolve(pc, SQRO_DONT_SPLIT_WORDS, &st, ppc);
            pc->Release();
        }
        pqs->Release();
    }
    return hr;

}

HRESULT AddStructuredQueryCondition(ISearchFolderItemFactory *psfif, IQueryParser *pqp, PCWSTR pszQuery) {
    ICondition *pc;
    HRESULT hr = ParseStructuredQuery(pszQuery, pqp, &pc);
    if (SUCCEEDED(hr))
    {
        hr = psfif->SetCondition(pc);
        pc->Release();
    }
    return hr;
}




HRESULT AddCustomCondition(ISearchFolderItemFactory *psfif)
{
    IConditionFactory *pcf;
     HRESULT hr = CoCreateInstance(CLSID_ConditionFactory, NULL, CLSCTX_INPROC_SERVER, IID_IConditionFactory, reinterpret_cast< void **>(&pcf));
    if (SUCCEEDED(hr))
    {
        // pv does not have to be freed
        PROPVARIANT pv;
        pv.vt = VT_LPWSTR;
        pv.pwszVal = L"*.jpg";
        ICondition *pc;
        hr = pcf->MakeLeaf(L"System.FileName", COP_DOSWILDCARDS, NULL, &pv, NULL, NULL, NULL, FALSE, &pc);
        if (SUCCEEDED(hr))
        {
            hr = psfif->SetCondition(pc);
            pc->Release();
        }
        pcf->Release();
    }
    return hr;
}


HRESULT GetQueryItem(REFIID riid, void **ppv, IQueryParser* _pqp, IShellItemArray *psia, PCWSTR displayname, PCWSTR searchstring)
{
   	LOG(logDEBUG1) << "ExplorerBrowserSearch:GetQueryItem:start";


   	if (psia==NULL) {
			LOG(logDEBUG1) << "ExplorerBrowserSearch:GetQueryItem:SetScope:NULL";
   	} else {
   		DWORD pdwCount;
   		psia->GetCount(&pdwCount);
   		LOG(logDEBUG1) << "ExplorerBrowserSearch:GetQueryItem:SetScope:count: " << pdwCount;

   		for (int i=0; i < (int) pdwCount; i++) {
   			LPWSTR ppszName = 0;
   			IShellItem * item;
   			psia->GetItemAt(i, &item);
   			HRESULT hr1 = item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &ppszName);
   			LOG(logDEBUG1) << "ExplorerBrowserSearch:GetQueryItem:SetScope:hr: " << (long) hr1 << ", item " << i << ": " << wstrtochar(ppszName);
   			CoTaskMemFree(ppszName);
   		}
   	}

    *ppv = NULL;

    ISearchFolderItemFactory *psfif;
    HRESULT hr = CoCreateInstance(CLSID_SearchFolderItemFactory, NULL, CLSCTX_INPROC, IID_ISearchFolderItemFactory, reinterpret_cast< void **>(&psfif));
   	LOG(logDEBUG1) << "ExplorerBrowserSearch:GetQueryItem:CLSID_SearchFolderItemFactory created: " << (long) hr;

	if (SUCCEEDED(hr))
	{
		hr = psfif->SetDisplayName(displayname);
	   	LOG(logDEBUG1) << "ExplorerBrowserSearch:SetDisplayName: " << (long) hr;
		if (SUCCEEDED(hr))
		{

		   	LOG(logDEBUG1) << "ExplorerBrowserSearch:before SetScope: " << (long) hr;
			if (psia!=NULL) hr = psfif->SetScope(psia);
		   	LOG(logDEBUG1) << "ExplorerBrowserSearch:SetScope: " << (long) hr;
            if (SUCCEEDED(hr))
            {
                // Creates a structured query from AQS
                // Other examples of AQS queries:
                //       "System.IsFolder:=FALSE"
                //       "Tags:trees"
                //       "DateTaken:earlier this month"
                // Multiple AQS query terms can be used together to build more complex queries
                //       "DateTaken:earlier this month Type:JPG Size:medium Name:w"
  //              hr = AddStructuredQueryCondition(psfif, _pqp, L"kind:picture");
                hr = AddStructuredQueryCondition(psfif, _pqp, searchstring);
  //               hr = AddStructuredQueryCondition(psfif, _pqp, L"kind:everything");
                if (SUCCEEDED(hr))
                {
                    // Here is another way to add a custom query condition
                    // hr = AddCustomCondition(psfif);
                    hr = psfif->GetShellItem(riid, ppv);
                   	LOG(logDEBUG1) << "ExplorerBrowserSearch:GetSampleQueryItem:GetShellItem: " << (long) hr;
                }
            }
        }

        psfif->Release();
    }
   	LOG(logDEBUG1) << "ExplorerBrowserSearch:GetQueryItem:end";

    return hr;
}

IShellItemArray* CreateShellItemArrayFromLPWSTRarray(LPWSTR path[], int size) {
	PIDLIST_ABSOLUTE pidlFull[size];
	IShellItemArray *psia;

	for (int i=0; i < size; i++) {
       	IShellItem2 *psi;
       	HRESULT hr1 = SHParseDisplayName(path[i], NULL, (ITEMIDLIST_ABSOLUTE**) &pidlFull[i], 0, NULL);
		LOG(logDEBUG1) << "ExplorerBrowserJni:CreateShellItemArrayFromLPWSTRarray, SHParseDisplayName: hr: " << (long) hr1;
		if (FAILED(hr1)) {
			LOG(logERROR) << "ExplorerBrowserJni:CreateShellItemArrayFromLPWSTRarray, could not SHParseDisplayName argument " << i;
			for (int j=0; j < i; j++) {
				ILFree((ITEMIDLIST_RELATIVE *) pidlFull[j]);
			}
			return NULL;
		}
		HRESULT hr2 = SHCreateShellItem(NULL, NULL, (ITEMID_CHILD*) pidlFull[i], (IShellItem**) &psi);
		LOG(logDEBUG1) << "ExplorerBrowserJni:doSearch, SHCreateShellItem: hr: " << (long) hr2;
		if (FAILED(hr2)) {
			LOG(logERROR) << "ExplorerBrowserJni:CreateShellItemArrayFromLPWSTRarray, could not SHCreateShellItem argument " << i;
			for (int j=0; j < i; j++) {
				ILFree((ITEMIDLIST_RELATIVE *) pidlFull[j]);
			}
			return NULL;
		}
	}
	HRESULT hr = SHCreateShellItemArrayFromIDLists(ARRAYSIZE(pidlFull), (const ITEMIDLIST**) pidlFull, &psia);
	LOG(logDEBUG1) << "ExplorerBrowserJni:doSearch, SHCreateShellItemArrayFromIDLists: hr: " << (long) hr;
	if (FAILED(hr)) {
		LOG(logERROR) << "ExplorerBrowserJni:CreateShellItemArrayFromLPWSTRarray, could not SHCreateShellItemArrayFromIDLists";
		for (int j=0; j < size; j++) {
			ILFree((ITEMIDLIST_RELATIVE *) pidlFull[j]);
		}
		return NULL;
	}
	return psia;
}

//HRESULT CreateShellItemArrayFromSampleScope(IShellItemArray *psia)
IShellItemArray* CreateShellItemArrayFromSampleScope()
{

	IShellItemArray *psia = NULL;
  	LOG(logDEBUG1) << "ExplorerBrowserSearch:CreateShellItemArrayFromSampleScope:start";

    // Set scope to the Pictures library.
    IShellItem *psi;
        // you can use SHGetKnownFolderItem instead of SHCreateItemInKnownFolder on Win7 and greater
//    HRESULT hr = SHCreateItemInKnownFolder(FOLDERID_PicturesLibrary, 0, NULL, IID_PPV_ARGS(&psi));
    HRESULT hr = SHCreateItemInKnownFolder(FOLDERID_PicturesLibrary, 0, NULL, FOLDERID_PicturesLibrary, reinterpret_cast< void **>(&psi));
    if (SUCCEEDED(hr))
    {
        hr = SHCreateShellItemArrayFromShellItem(psi, IID_PPV_ARGS(&psia));
        if (SUCCEEDED(hr))
        {
           return S_OK;
        }
        psi->Release();
    }
    else
    {
      	LOG(logDEBUG1) << "ExplorerBrowserSearch:CreateShellItemArrayFromSampleScope: no Pictures library";

    	// If no Pictures library is available (on Vista, for example),
        // set scope to the Pictures and Public Pictures folders
        PIDLIST_ABSOLUTE rgItemIDs[2];
        hr = SHGetKnownFolderIDList(FOLDERID_Pictures, 0, NULL, (ITEMIDLIST_ABSOLUTE**) &rgItemIDs[0]);
        if (SUCCEEDED(hr))
        {
            hr = SHGetKnownFolderIDList(FOLDERID_PublicPictures, 0, NULL, (ITEMIDLIST_ABSOLUTE**) &rgItemIDs[1]);
            if (SUCCEEDED(hr))
            {
               	hr = SHCreateShellItemArrayFromIDLists(ARRAYSIZE(rgItemIDs), (const ITEMIDLIST**) rgItemIDs, &psia);
                	if (SUCCEEDED(hr))
            	{

            		DWORD pdwCount;
            		psia->GetCount(&pdwCount);
            		LOG(logDEBUG1) << "ExplorerBrowserSearch:CreateShellItemArrayFromSampleScope:count: " << pdwCount;

            		for (DWORD i=0; i < pdwCount; i++) {
            			LPWSTR ppszName = 0;
            			IShellItem * item;
            			psia->GetItemAt(i, &item);
            			hr = item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &ppszName);
            			LOG(logDEBUG1) << "ExplorerBrowserSearch:CreateShellItemArrayFromSampleScope:hr: " << (long) hr << ", item " << i << ": " << wstrtochar(ppszName);
            			CoTaskMemFree(ppszName);
            		}


            		return psia;
            	}
            	ILFree((ITEMIDLIST_RELATIVE*)rgItemIDs[1]);
            }
            ILFree((ITEMIDLIST_RELATIVE*)rgItemIDs[0]);
        }

    }
    return NULL;
}

