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

#define STRICT_TYPED_ITEMIDS    // in case you do IDList stuff you want this on for better type safety

#define COBJMACROS

#include <iostream>

#include <stdio.h>
#include <windows.h>
#include <ole2.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <objbase.h>
#include <initguid.h>

#include <windowsx.h>

#include "CExplorerBrowser.h"
#include "search_ms777.h"
#include "StringUtils.h"
#include "Log.h"


using namespace std;

#define MIN_SHELL_ID 1
#define MAX_SHELL_ID 30000



map<HWND, CExplorerBrowser*> CExplorerBrowser::mapCanvasExplorerbrowser;


/*
 * -----------------------   ExplorerExecutor is used from ExplorerBrowserJni to execute something
 * -----------------------   in the WindowProc, called by the threadMsgLoop windows message loop
 */

ExplorerExecutor::ExplorerExecutor(CExplorerBrowser* explorerBrowser) {
	hwnd = explorerBrowser->hwndParentWindow;
	mutexJni = CreateEvent(
	        NULL,               // default security attributes
	        TRUE,               // manual-reset event
	        FALSE,              // initial state is nonsignaled
	        NULL  // object name
	        );
}
ExplorerExecutor::~ExplorerExecutor() {
	CloseHandle(mutexJni);
}
void ExplorerExecutor::waitFor() {
	PostMessage(hwnd, WM_APP_EXECUTE, 777, (LPARAM) this);

	DWORD dwWaitResult = WaitForSingleObject(mutexJni,5000);
	switch (dwWaitResult)  {
	case WAIT_OBJECT_0:
		return;
	case WAIT_ABANDONED:
   		LOG(logERROR) << "ExplorerExecutor:waitFor failed: WAIT_ABANDONED";
		return;
	case WAIT_TIMEOUT:
   		LOG(logERROR) << "ExplorerExecutor:waitFor failed: WAIT_TIMEOUT";
		return;
	}
	LOG(logERROR) << "ExplorerExecutor:waitFor failed: " << dwWaitResult;
}

void ExplorerExecutor::release() {
	SetEvent(mutexJni);
}

/*
 * -----------------------   the Windowproc for the hwndParent window
 * -----------------------   hwndParent window is generated and added as a child to the hwndJavaWindow
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	switch (msg) {
	case WM_PAINT: {
		LOG(logDEBUG4) << "WindowProc WM_PAINT, hwnd: " << (long long int) hwnd << ", wparam: " << (DWORD) wparam << ", lparam: " << lparam;
		CExplorerBrowser* explorerBrowser = (CExplorerBrowser*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
		RECT r;
		GetClientRect(hwnd, &r);

		HWND hwndParent = GetParent(hwnd);
		RECT rParent;
		GetClientRect(hwndParent, &rParent);
		if ((rParent.right != r.right)||(rParent.bottom != r.bottom)) {
			LOG(logDEBUG4) << "WindowProc WM_PAINT, hwnd: " << (long long int) hwnd << ", new size: " << rParent.right-rParent.left << "/" << rParent.bottom-rParent.top;
			SetWindowPos(hwnd, 0, 0, 0, rParent.right-rParent.left, rParent.bottom-rParent.top, 0);
			GetClientRect(hwnd, &r);
			explorerBrowser->explorerBrowser->SetRect(NULL, r);
		}

/*
		char s[244];
		sprintf(s, "WindowText: %li", (long) explorerBrowser->hwndJavaWindow);
		PAINTSTRUCT ps;
		HDC dc;
		dc = BeginPaint(hwnd, &ps);
		DrawText(dc, s, -1, &r,	DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		EndPaint(hwnd, &ps);
*/
		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}

	case WM_SIZE: {
		RECT r;
		GetClientRect(hwnd, &r);
		LOG(logDEBUG4) << "WindowProc WM_SIZE, hwnd: " << (long long int) hwnd << ", w/h: " << r.right-r.left << "/" << r.bottom-r.top;
		break;
	}

	case WM_PARENTNOTIFY: {
		LOG(logDEBUG4) << "WindowProc WM_PARENTNOTIFY, hwnd: " << (long long int) hwnd << ", wparam: " << LOWORD(wparam) << ", lparam: " << lparam;
		if (LOWORD(wparam) == WM_RBUTTONDOWN) {
			LOG(logDEBUG4) << "WindowProc WM_PARENTNOTIFY, hwnd: " << (long long int) hwnd << ", WM_RBUTTONDOWN";
			return FALSE;
		}
		break;
	}
	case WM_RBUTTONDOWN:
		LOG(logDEBUG4) << "WM_RBUTTONDOWN";
		break;

	case WM_CREATE: {
		LOG(logDEBUG4) << "WindowProc WM_CREATE, hwnd: " << (long long int) hwnd << ", wparam: " << (DWORD) wparam << ", lparam: " << lparam;
	    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) ((LPCREATESTRUCT)lparam)->lpCreateParams);
		CExplorerBrowser* explorerBrowser = (CExplorerBrowser*) ((LPCREATESTRUCT)lparam)->lpCreateParams;
		explorerBrowser->hwndParentWindow = hwnd;
		explorerBrowser->create();
		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}

	case WM_DESTROY: {
		LOG(logINFO) << "WindowProc WM_DESTROY, hwnd: " << (long long int) hwnd << ", wparam: " << (DWORD) wparam << ", lparam: " << lparam;
		CExplorerBrowser* explorerBrowser = (CExplorerBrowser*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		int iResult = PostThreadMessage(explorerBrowser->threadDescriptorMsgLoop, WM_APP_DESTROY, 0, lparam);
		LOG(logDEBUG1) << "WindowProc WM_DESTROY, PostThreadMessage result: " << iResult;
		delete explorerBrowser;
		LOG(logDEBUG1) << "WindowProc WM_DESTROY, explorer deleted";
//		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}

	case WM_APP_EXECUTE: {
		CExplorerBrowser* explorerBrowser = (CExplorerBrowser*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
		LOG(logDEBUG1) << "WindowProc WM_APP_EXECUTE start, hwnd: " << (long long int) hwnd << ", hwndJavaWindow: " << (long long int) explorerBrowser->hwndJavaWindow;
		ExplorerExecutor* explorerExecutor = (ExplorerExecutor*) lparam;
		explorerExecutor->execute(explorerBrowser);
		UpdateWindow(hwnd);
		LOG(logDEBUG1) << "WindowProc WM_APP_EXECUTE end, hwnd: " << (long long int) hwnd << ", hwndJavaWindow: " << (long long int) explorerBrowser->hwndJavaWindow;
		break;

	}

	case WM_SETCURSOR: {
		LOG(logDEBUG4) << "WM_SETCURSOR, hwnd: " << (long long int) hwnd << ", wapram: " << wparam << ", LOWORD(lparam): " << LOWORD(lparam) << ", HIWORD(lparam): " << HIWORD(lparam);
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	default:
		LOG(logDEBUG4) << "WM_DEFAULT, iMsg: " << msg << ", hwnd: " << (long long int) hwnd;
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}


static WNDCLASS wc = {
		wc.style = CS_HREDRAW | CS_VREDRAW,
		wc.lpfnWndProc = WindowProc,
		wc.cbClsExtra = 0,
		wc.cbWndExtra = 0,
		wc.hInstance = 0,
		wc.hIcon = 0,
		wc.hCursor = 0,
//		wc.hbrBackground = (HBRUSH) COLOR_WINDOWFRAME,
		wc.hbrBackground = NULL,
		wc.lpszMenuName = NULL,
		wc.lpszClassName = "classExplorerBrowser"
};

/*
 * -----------------------   the functions to maintain a map of CExplorerBrowser objects
 * -----------------------   it is accessed by hwndJavaWindow
 */

static HANDLE ghMutex = CreateMutex(
    NULL,              // default security attributes
    FALSE,             // initially not owned
    NULL);             // unnamed mutex

DWORD waitForMapMutex(const char* sFunction) {
	DWORD  dwWaitResult = WaitForSingleObject(ghMutex, INFINITE);

	switch (dwWaitResult)  {
	case WAIT_OBJECT_0:
		return 0;
	case WAIT_ABANDONED:
		LOG(logERROR) << "CExplorerBrowser::" << sFunction << ":WAIT_ABANDONED";
		return -1;
	case WAIT_TIMEOUT:
		LOG(logERROR) << "CExplorerBrowser::" << sFunction << ":WAIT_TIMEOUT";
		return -1;
	}
	LOG(logERROR) << "CExplorerBrowser::" << sFunction << ":ERROR " << dwWaitResult;
	return -1;
}


int CExplorerBrowser::removeExplorerBrowser(HWND lhwndJavaWindow) {
	if (waitForMapMutex("removeExplorerBrowser") == 0) {
		int iRemainingExplorerCount = -1;
		if (mapCanvasExplorerbrowser.count(lhwndJavaWindow) == 0) {
			iRemainingExplorerCount =  mapCanvasExplorerbrowser.size();
			LOG(logERROR) << "CExplorerBrowser::removeExplorerBrowser failed: " << iRemainingExplorerCount;
		} else {
			mapCanvasExplorerbrowser.erase(lhwndJavaWindow);
			iRemainingExplorerCount = mapCanvasExplorerbrowser.size();
			LOG(logDEBUG1) << "CExplorerBrowser::removeExplorerBrowser finished: " << lhwndJavaWindow;
		}
		ReleaseMutex(ghMutex);
		return  iRemainingExplorerCount;
	} else {
		return -1;
	}
}

void CExplorerBrowser::addExplorerBrowser(HWND lhwndJavaWindow, CExplorerBrowser* explorerBrowser) {
	if (waitForMapMutex("addExplorerBrowser") == 0) {
		mapCanvasExplorerbrowser[lhwndJavaWindow] = explorerBrowser;
		explorerBrowser->hThreadMsgLoop = CreateThread(NULL,   0,
				(LPTHREAD_START_ROUTINE) reinterpret_cast< void *> (&CExplorerBrowser::threadMsgLoop),
				(LPVOID) (explorerBrowser),
				0,
				&(explorerBrowser->threadDescriptorMsgLoop));

		ReleaseMutex(ghMutex);
	}
}


CExplorerBrowser* CExplorerBrowser::getExplorerBrowserByHwnd(HWND lhwndJavaWindow) {
	if (waitForMapMutex("getExplorerBrowserByHwnd") == 0) {
		CExplorerBrowser* explorerBrowser = NULL;
		if (mapCanvasExplorerbrowser.count(lhwndJavaWindow) > 0)
			explorerBrowser = mapCanvasExplorerbrowser[lhwndJavaWindow];
		ReleaseMutex(ghMutex);
		return explorerBrowser;
	} else {
		return NULL;
	}
}

CExplorerBrowser::CExplorerBrowser(HWND hwndJavaWindow, long lExplorerBrowserOptions, long lFolderViewMode, long lFolderFlags) {
	this->hwndJavaWindow = hwndJavaWindow;
	this->lExplorerBrowserOptions = lExplorerBrowserOptions;
	this->lFolderViewMode = lFolderViewMode;
	this->lFolderFlags = lFolderFlags;

	hwndParentWindow = 0;
	pIShellView = NULL;
	explorerBrowser = NULL;
	pdwCookieAdvise = 0;
	hThreadMsgLoop = 0;
	threadDescriptorMsgLoop = 0;
	LOG(logINFO) << "CExplorerBrowser::CExplorerBrowser start: hwndJavaWindow: " << (long long int) hwndJavaWindow;

// take care of the window class registering
	if (!RegisterClass(&wc)) {
		DWORD dwError = GetLastError();

		if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
			LOG(logERROR) << "CExplorerBrowser::CExplorerBrowser:could not register class";
			return;
		}
	}
	addExplorerBrowser(hwndJavaWindow, this);

}

CExplorerBrowser::~CExplorerBrowser() {
	LOG(logDEBUG1) << "CExplorerBrowser:~CExplorerBrowser";

	if (pdwCookieAdvise!=0) {
		LOG(logINFO) << "CExplorerBrowser:destroy:before Unadvise";
		explorerBrowser->Unadvise(pdwCookieAdvise);
		pdwCookieAdvise = 0;
		LOG(logINFO) << "CExplorerBrowser:destroy:after Unadvise";
	}
/*
	if (pIShellView) {
		pIShellView->Release();
	}
*/
	if (explorerBrowser != NULL) {
		HRESULT hr = explorerBrowser->Destroy();
		if (FAILED(hr)) {
			LOG(logERROR) << "CExplorerBrowser:destroy: hr: " << (long) hr;
		}
//		explorerBrowser->Release();
		explorerBrowser = NULL;
		LOG(logINFO) << "CExplorerBrowser:~CExplorerBrowser:explorerBrowser->Destroy() success";
	}

	if (hThreadMsgLoop != 0) {
		int iResult = CloseHandle(hThreadMsgLoop);
		if (iResult == 0) {
			LOG(logERROR) << "CExplorerBrowser:~CExplorerBrowser:CloseHandle hThreadMsgLoop is zero, hwndParent : " << (long long int) hwndJavaWindow;
		}
		hThreadMsgLoop = 0;
	}

	int iRemainingExplorerCount = removeExplorerBrowser(hwndJavaWindow);
	LOG(logDEBUG1) << "CExplorerBrowser:~CExplorerBrowser: iRemainingExplorerCount: " << iRemainingExplorerCount;
}




IFACEMETHODIMP CExplorerBrowser::QueryInterface(REFIID iid, void **ppvObject)
{
	LOG(logDEBUG4) << "CExplorerBrowser:QueryInterface: " << getGuidDescription(iid);
	if(iid == IID_IUnknown)  {
		AddRef();
		*ppvObject = this;
		return S_OK;
	} else if(iid == IID_IServiceProvider)  {
		AddRef();
		*ppvObject = static_cast<IServiceProvider *>(this);
		return S_OK;
	} else if(iid == IID_ICommDlgBrowser)  {
		AddRef();
		*ppvObject = static_cast<ICommDlgBrowser *>(this);
		return S_OK;
	} else if(iid == IID_ICommDlgBrowser2)  {
		AddRef();
		*ppvObject = static_cast<ICommDlgBrowser2 *>(this);
		return S_OK;
	} else
	{
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

IFACEMETHODIMP_(ULONG) CExplorerBrowser::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) CExplorerBrowser::Release()
{
    long cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
    {
        delete this;
    }
    return cRef;
}

// IExplorerBrowserEvents
IFACEMETHODIMP CExplorerBrowser::OnNavigationPending(PCIDLIST_ABSOLUTE pidlFolder) {
	OnNavigation(NAVIGATION_PENDING, pidlFolder);
	return S_OK;
};


LRESULT CALLBACK ShellWindowProcHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	CExplorerBrowser* explorerBrowser = (CExplorerBrowser*) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {

	case WM_CONTEXTMENU:
		LOG(logDEBUG1) << "CExplorerBrowser::WM_CONTEXTMENU, hwnd: " << (long long int) hwnd << ", explorerBrowser: " << (long long int) explorerBrowser->hwndParentWindow;
		HMENU hmenu;
		hmenu = explorerBrowser->CreateCustomPopupMenu();
		if (hmenu != NULL) {
			HWND hwndshell;
			explorerBrowser->pIShellView->GetWindow(&hwndshell);
			HRESULT hr = TrackPopupMenu(hmenu, TPM_RETURNCMD | TPM_LEFTALIGN, GET_X_LPARAM(lp), GET_Y_LPARAM(lp), 0, hwndshell, NULL);
			LOG(logDEBUG1) << "CExplorerBrowser::Notify: TrackPopupMenu(), hr: " << (long) hr << ", hwndShell: " << (long long int) hwndshell << ", hwndParent: " << (long long int) explorerBrowser->hwndParentWindow;
			CloseHandle(hmenu);

			if ((hr > 0) && (hr <= MAX_SHELL_ID)) {
				CMINVOKECOMMANDINFO ici;
				ZeroMemory(&ici, sizeof(ici));
				ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
				ici.lpVerb = MAKEINTRESOURCE((long) hr - MIN_SHELL_ID);
				ici.nShow = SW_SHOWNORMAL;

				HRESULT hr1 = explorerBrowser->pIContextMenu2->InvokeCommand(&ici);
				LOG(logDEBUG1) << "CExplorerBrowser::ShellWindowProcHook:InvokeCommand(), hr: " << (long) hr1;

			} else if (hr > MAX_SHELL_ID) {
				explorerBrowser->notifyContextMenuCustomOption((long) hr - MAX_SHELL_ID - 1, explorerBrowser->contextMenuFocusedPath);
				delete[] explorerBrowser->contextMenuFocusedPath;
				explorerBrowser->contextMenuFocusedPath = NULL;

				LOG(logDEBUG1) << "CExplorerBrowser::ShellWindowProcHook:CustomPopup, hr: " << ((long) hr - MAX_SHELL_ID - 1);
			}
			explorerBrowser->pIContextMenu2->Release();
			explorerBrowser->pIContextMenu2 = NULL;
		}
		return 0; // handled
		break;

	default:
		break;
	}
	return CallWindowProc(explorerBrowser->pShellWindowProcOld, hwnd, msg, wp, lp);
}



IFACEMETHODIMP CExplorerBrowser::OnViewCreated(IShellView *ppshv) {
	pIShellView = ppshv;
	HWND hwndshell;
	HRESULT hr = ppshv->GetWindow(&hwndshell);

	LOG(logDEBUG1) << "CExplorerBrowser::OnViewCreated, hr: " << (long) hr << ", hwndShell: " << (long long int) hwndshell << ", hwndParent: " << (long long int) hwndParentWindow;
	pShellWindowProcOld = (WNDPROC) SetWindowLongPtr (hwndshell, GWLP_WNDPROC, (LONG_PTR) ShellWindowProcHook);

    SetWindowLongPtr (hwndshell, GWLP_USERDATA, (UINT_PTR) (CExplorerBrowser*) this);

	return S_OK;
};
IFACEMETHODIMP CExplorerBrowser::OnNavigationComplete(PCIDLIST_ABSOLUTE pidlFolder) {
	OnNavigation(NAVIGATION_COMPLETE, pidlFolder);
	return S_OK;
};
IFACEMETHODIMP CExplorerBrowser::OnNavigationFailed(PCIDLIST_ABSOLUTE pidlFolder) {
	OnNavigation(NAVIGATION_FAILED, pidlFolder);
	return S_OK;
};

// IServiceProvider
IFACEMETHODIMP CExplorerBrowser::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
	LOG(logDEBUG4) << "CExplorerBrowser:QueryService: " << getGuidDescription(guidService);
    *ppv = NULL;

    HRESULT hr = E_NOINTERFACE;
    if (guidService == SID_SExplorerBrowserFrame)
    {
    	LOG(logDEBUG4) << "CExplorerBrowser:QueryService:QueryInterface: " << getGuidDescription(riid);
        hr = QueryInterface(riid, ppv);
    }
    return hr;
}

// ICommDlgBrowser
IFACEMETHODIMP CExplorerBrowser::OnDefaultCommand(IShellView * /* psv */)
{
	LOG(logDEBUG1) << "CExplorerBrowser::OnDefaultCommand";
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CExplorerBrowser::OnStateChange(IShellView * /* psv */, ULONG uChange)
{
	if (uChange == CDBOSC_SELCHANGE)
	{
		LOG(logDEBUG1) << "CExplorerBrowser::OnStateChange ";
		selectionMessage(0);

	}
	return S_OK;
}

IFACEMETHODIMP CExplorerBrowser::IncludeObject(IShellView * psv, PCUITEMID_CHILD pidl )
{
    return S_OK;
}

HMENU CExplorerBrowser::CreateCustomPopupMenu() {
	HRESULT hr;
	HWND hwndshell;
	hr = pIShellView->GetWindow(&hwndshell);

	IFolderView2 * ppifw;
	hr = pIShellView->QueryInterface(IID_IFolderView2, (void**) &ppifw);
	LOG(logDEBUG1) << "CExplorerBrowser::CreateCustomPopupMenu: QueryInterface(IID_IFolderView), hr: " << (long) hr << ", hwndShell: " << (long long int) hwndshell << ", hwndParent: " << (long long int) hwndParentWindow;

	int iItem = -1;
	hr = ppifw->GetFocusedItem(&iItem);
	if (!SUCCEEDED(hr)) {
		LOG(logERROR) << "CExplorerBrowser::CreateCustomPopupMenu: GetFocusedItem(), iItem: " << iItem << ", hr: " << (long) hr;
		ppifw->Release();
		return NULL;
	}

	PITEMID_CHILD ppidl;
	hr = ppifw->Item(iItem, &ppidl);
	if (!SUCCEEDED(hr)) {
		LOG(logERROR) << "CExplorerBrowser::CreateCustomPopupMenu: Item(), iItem: " << iItem << ", hr: " << (long) hr;
		ppifw->Release();
		return NULL;
	}


	IShellFolder * ppshf;
	hr = ppifw->GetFolder(IID_IShellFolder, (void**) &ppshf);
	if (!SUCCEEDED(hr)) {
		LOG(logERROR) << "CExplorerBrowser::CreateCustomPopupMenu: GetFolder(), iItem: " << iItem << ", hr: " << (long) hr;
		ppifw->Release();
		return NULL;
	}

	if (contextMenuFocusedPath!=NULL) {
		delete[] contextMenuFocusedPath;
		contextMenuFocusedPath = NULL;
	}
	contextMenuFocusedPath = GetDisplayNameOf(ppshf, ppidl, SHGDN_NORMAL + SHGDN_FORPARSING);

	hr = ppshf->GetUIObjectOf(NULL, 1, (PCUITEMID_CHILD_ARRAY) &ppidl, IID_IContextMenu, NULL, (void**) &pIContextMenu2);
	if (!SUCCEEDED(hr)) {
		LOG(logERROR) << "CExplorerBrowser::CreateCustomPopupMenu: GetUIObjectOf(), iItem: " << iItem << ", hr: " << (long) hr;
		ppifw->Release();
		ppshf->Release();
		return NULL;
	}

	HMENU hmenu = CreatePopupMenu();
	hr = pIContextMenu2->QueryContextMenu(hmenu, 0, MIN_SHELL_ID, MAX_SHELL_ID, CMF_EXPLORE);
	LOG(logDEBUG1) << "CExplorerBrowser::Notify: QueryContextMenu(), hr: " << (long) hr << ", hwndShell: " << (long long int) hwndshell << ", hwndParent: " << (long long int) hwndParentWindow;


	map<long, char *> mapReturn;

	getContextMenuCustomOptions( &mapReturn, contextMenuFocusedPath);

	if (!mapReturn.empty()) {
		hr = InsertMenuA(hmenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		for(map<long, char*>::iterator p = mapReturn.begin(); p!=mapReturn.end(); ++p)	{
			long lOption = p->first;
			const char * option = p->second;
			LOG(logDEBUG1) << "CExplorerBrowser::getPopupCustomOptions, lOption: " << lOption << ", : " << option;
			hr = InsertMenuA(hmenu, -1, MF_BYPOSITION | MF_STRING, (UINT_PTR) MAX_SHELL_ID + lOption +1, option);
		}
	}


	LOG(logDEBUG1) << "CExplorerBrowser::Notify: InsertMenu(), hr: " << (long) hr << ", focusedItemName: " << contextMenuFocusedPath;
	return hmenu;
}

// ICommDlgBrowser2
IFACEMETHODIMP CExplorerBrowser::Notify(IShellView * ppshv, DWORD dwNotifyType)
{
	if (dwNotifyType==CDB2N_CONTEXTMENU_START) {
		LOG(logDEBUG1) << "CExplorerBrowser::Notify: CDB2N_CONTEXTMENU_START";
	}
	if (dwNotifyType==CDB2N_CONTEXTMENU_DONE) {
		LOG(logDEBUG1) << "CExplorerBrowser::Notify: CDB2N_CONTEXTMENU_DONE";
	}
    return S_OK;
}

IFACEMETHODIMP CExplorerBrowser::GetDefaultMenuText(IShellView * /* ppshv */, PWSTR /* pszText */, int /* cchMax */)
{
    return E_NOTIMPL;
}

IFACEMETHODIMP CExplorerBrowser::GetViewFlags(DWORD *pdwFlags)
{
    // setting this flag is needed to avoid the poor perf of having
    // ICommDlgBrowser::IncludeObject() for every item when the result
    // set is large.
    *pdwFlags = CDB2GVF_NOINCLUDEITEM;
    return S_OK;
}


/*************************************************************************
 *
 * Call either IObjectWithSite_SetSite() or IInternetSecurityManager_SetSecuritySite() on
 * an object.
 *
 */
HRESULT WINAPI IUnknown_SetSite(
		IUnknown *obj,        /* [in]   OLE object     */
		IUnknown *site)       /* [in]   Site interface */
{
	HRESULT hr;
	IObjectWithSite *iobjwithsite;
	IInternetSecurityManager *isecmgr;

	if (!obj) return E_FAIL;

	hr = obj->QueryInterface(IID_IObjectWithSite, (LPVOID *)&iobjwithsite);
	if (SUCCEEDED(hr))
	{
		hr = iobjwithsite->SetSite(site);
		iobjwithsite->Release();
	}
	else
	{
		hr = obj->QueryInterface(IID_IInternetSecurityManager, (LPVOID *)&isecmgr);
		if (FAILED(hr)) return hr;

		hr = isecmgr->SetSecuritySite((IInternetSecurityMgrSite *)site);
		isecmgr->Release();
	}
	return hr;
}





void CExplorerBrowser::create() {
	CoInitialize(NULL);
	explorerBrowser = NULL;

	HRESULT hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC_SERVER, IID_IExplorerBrowser, reinterpret_cast< void **> (&explorerBrowser));
	if (FAILED(hr)) {
		LOG(logERROR) << "CExplorerBrowser:makeNew:failed, return code: " << (long) hr;
		CoUninitialize();
		return;
	}
	hr = IUnknown_SetSite(explorerBrowser, static_cast<IServiceProvider *>(this));
	if (FAILED(hr)) {
		LOG(logERROR) << "CExplorerBrowser:IUnknown_SetSite:failed,  return code: " << (long) hr;
		CoUninitialize();
		return;
	}

	explorerBrowser->SetOptions((EXPLORER_BROWSER_OPTIONS) lExplorerBrowserOptions); // no border, no wrapper window, must be before initialize

	RECT r;
	GetClientRect(hwndJavaWindow, &r);

	FOLDERSETTINGS foldersettings;
	foldersettings.ViewMode = lFolderViewMode;
	foldersettings.fFlags = lFolderFlags;
	LOG(logDEBUG1) << "CExplorerBrowser::Initialize, hwndParentWindow: " << (long long int) hwndParentWindow;

	if (failedWithLog( explorerBrowser->Initialize(hwndParentWindow, &r, &foldersettings), "CExplorerBrowser:Initialize")) return;

	explorerBrowser->Advise((IExplorerBrowserEvents*) this, &pdwCookieAdvise);

}

int CExplorerBrowser::BrowseToIDList(LPCWSTR szPath) {
	if (failedExplorerNull("BrowseToIDList")) return -1;

	LPITEMIDLIST pidlBrowse = NULL;
	// in shlobj.h SHParseDisplayName(LPCWSTR,IBindCtx*,LPITEMIDLIST,SFGAOF,SFGAOF*) => SHParseDisplayName(LPCWSTR,IBindCtx*,LPITEMIDLIST*,SFGAOF,SFGAOF*);
	if (failedWithLog( SHParseDisplayName(szPath, NULL, (ITEMIDLIST_ABSOLUTE**) &pidlBrowse, 0, NULL), "CExplorerBrowser:BrowseToIDList:could not parse display name")) {
		return -1;
	}

	if (pidlBrowse==NULL) {
		LOG(logERROR) << "CExplorerBrowser:BrowseToIDList:pidlBrowse is null";
		return -1;
	}

	if (failedWithLog( explorerBrowser->BrowseToIDList((PCUIDLIST_RELATIVE) pidlBrowse, 0), "CExplorerBrowser:BrowseToIDList:could not browse")) {
		ILFree((ITEMIDLIST_RELATIVE*) pidlBrowse);
		return -1;
	}

	ILFree((ITEMIDLIST_RELATIVE*) pidlBrowse);
	return 0;
}

int CExplorerBrowser::BrowseToIDList(int iSBSBParentBackForward) {
	if (failedExplorerNull("BrowseToIDList")) return -1;
	if (failedWithLog( explorerBrowser->BrowseToIDList((PCUIDLIST_RELATIVE) NULL, (UINT) iSBSBParentBackForward), "CExplorerBrowser:BrowseToIDList:could not browse")) {
		return -1;
	}
	return 0;
}

void CExplorerBrowser::OnNavigation(int iMode, PCIDLIST_ABSOLUTE pidlFolder){
	wchar_t szPath[MAX_PATH];
	BOOL fRet= SHGetPathFromIDListW( pidlFolder, szPath );
	if (fRet==true) {
		const char* res = wstrtochar(szPath);
		navigationMessage( iMode, res);
	}
}


bool CExplorerBrowser::failedWithLog(HRESULT hr, const char * comment) {
	if (FAILED(hr)) {
		LOG(logERROR) << * comment << ", return code: " << (long) hr;
		return true;
	} else {
		return false;
	}
}

bool CExplorerBrowser::failedExplorerNull(const char * comment) {
	if (explorerBrowser==NULL) {
		LOG(logERROR) << * comment << ", explorerbrowser is null";
		return true;
	}
	return false;
}

void CExplorerBrowser::getSelectedItems(map<long, char *>* mapReturn, UINT svgio){

	LOG(logDEBUG1) << "CExplorerBrowser::getSelectedItems start";

	IFolderView* folderView;
	if (failedWithLog(explorerBrowser->GetCurrentView(IID_IFolderView, reinterpret_cast< void **> (&folderView)),
			"GetCurrentView:IID_IFolderView")) {
		return;
	}

	IShellFolder* shellFolder;
	if (failedWithLog(folderView->GetFolder(IID_IShellFolder, reinterpret_cast< void **> (&shellFolder)),
			"GetFolder:IID_IShellFolder")) {
		folderView->Release();
		return;
	}

	int pcItems;
	if (failedWithLog(folderView->ItemCount(svgio, &pcItems), "ItemCount:SVGIO_SELECTION")) {
		folderView->Release();
		return;
	}

	LPENUMIDLIST enumIDList;
	if (failedWithLog(folderView->Items(svgio, IID_IEnumIDList, reinterpret_cast< void **> (&enumIDList)),
			"Items:SVGIO_SELECTION")) {
		folderView->Release();
		return;
	}

	long iCount = 0;
	LPITEMIDLIST pidl;
	DWORD Result = enumIDList->Next(1, (ITEMID_CHILD**) &pidl, 0);
	while (Result != S_FALSE) {
		if (Result != NOERROR)
			break;

		char* DisplayName = GetDisplayNameOf(shellFolder, pidl, SHGDN_NORMAL + SHGDN_FORPARSING);
		(*mapReturn)[iCount] = DisplayName;
		iCount++;
		CoTaskMemFree(pidl);
		Result = enumIDList->Next(1, (ITEMID_CHILD** ) &pidl, 0);
	}
	enumIDList->Release();
	folderView->Release();
}




DWORD WINAPI CExplorerBrowser::threadMsgLoop(LPVOID threadParams) {

	CExplorerBrowser* explorerBrowser = (CExplorerBrowser*) threadParams;
	LOG(logINFO) << "threadMsgLoop start, hwndJavaWindow : " << (long long int) explorerBrowser->hwndJavaWindow;
/*
	RECT rJavaWindow;
	GetClientRect(explorerBrowser->hwndJavaWindow, &rJavaWindow);
*/

    HWND hwndNew  = CreateWindow(wc.lpszClassName, NULL, WS_CHILD ,
//    		0, 0, rJavaWindow.right-rJavaWindow.left, rJavaWindow.bottom-rJavaWindow.top,
    		0, 0, 50, 50,
			explorerBrowser->hwndJavaWindow,
			NULL,
			wc.hInstance,
			(LPVOID) explorerBrowser);

	if (!hwndNew) {
		LOG(logERROR) << "threadMsgLoop: could not create new window, hwndJavaWindow : " << (long long int) explorerBrowser->hwndJavaWindow;
		return 0;
	}

	ShowWindow(hwndNew, SW_SHOW);
	UpdateWindow(hwndNew);

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);

		if (msg.message==WM_APP_DESTROY) {
			LOG(logDEBUG1) << "threadMsgLoop WM_APP_DESTROY start, hwndJavaWindow : " << (long long int) explorerBrowser->hwndJavaWindow;
//			delete explorerBrowser;
			break;
		} else {
			DispatchMessage(&msg);
		}

	}

	LOG(logINFO) << "threadMsgLoop stop, hwndJavaWindow : " << (long long int) explorerBrowser->hwndJavaWindow;
	return 0;
}

