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
#ifndef CExplorerBrowser_H_
#define CExplorerBrowser_H_
#define STRICT_TYPED_ITEMIDS

#include <stdio.h> // C:\MinGW64_TDM\x86_64-w64-mingw32\include
#include <windows.h> // C:\MinGW64_TDM\x86_64-w64-mingw32\include
#include <ole2.h> // C:\MinGW64_TDM\x86_64-w64-mingw32\include
#include <shlobj.h> // C:\MinGW64_TDM\x86_64-w64-mingw32\include
#include <shobjidl.h> // C:\MinGW64_TDM\x86_64-w64-mingw32\include
#include <objbase.h> // C:\MinGW64_TDM\x86_64-w64-mingw32\include
#include <initguid.h> // C:\MinGW64_TDM\x86_64-w64-mingw32\include

#include <iostream>
#include <map>



using namespace std;

#define WM_APP_CREATE WM_APP+3
#define WM_APP_EXECUTE WM_APP+2
#define WM_APP_DESTROY WM_APP+1


class CExplorerBrowser: public IServiceProvider, public ICommDlgBrowser2, public IExplorerBrowserEvents  {
private:
	int Initialize(RECT r, int ViewMode, int fFlags);
	long _cRef;

protected:

public:
	// for IExplorerBrowserEvents
	static const int NAVIGATION_PENDING = 1;
	static const int NAVIGATION_COMPLETE = 2;
	static const int NAVIGATION_FAILED = 3;

	DWORD pdwCookieAdvise;
	HWND hwndJavaWindow;
	HWND hwndParentWindow;
	DWORD threadDescriptorMsgLoop;
	HANDLE hThreadMsgLoop;

	IShellView *pIShellView;

	LPCONTEXTMENU2 pIContextMenu2 = NULL;
	WNDPROC pShellWindowProcOld;
	char * contextMenuFocusedPath = NULL;



	long lExplorerBrowserOptions;
	long lFolderViewMode;
	long lFolderFlags;

	static DWORD WINAPI threadMsgLoop(LPVOID threadParams);

/*
 * ---------- The static functions to manage a list of CExplorerBrowsers and to return the right instance by hwnd
 */
	static map<HWND, CExplorerBrowser*> mapCanvasExplorerbrowser;
	static int removeExplorerBrowser(HWND lhwndJavaWindow);
	static void addExplorerBrowser(HWND lhwndJavaWindow, CExplorerBrowser* explorerBrowser);
	static CExplorerBrowser* getExplorerBrowserByHwnd(HWND lhwndJavaWindow);

	IExplorerBrowser* explorerBrowser;

	CExplorerBrowser(HWND hwndJavaWindow, long lExplorerBrowserOptions, long lFolderViewMode, long lFolderFlags);
	virtual ~CExplorerBrowser();

	IFACEMETHODIMP QueryInterface(REFIID iid, void **ppvObject);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	// IExplorerBrowserEvents
	IFACEMETHODIMP OnNavigationPending(PCIDLIST_ABSOLUTE pidlFolder);
	IFACEMETHODIMP OnViewCreated(IShellView *psv);
	IFACEMETHODIMP OnNavigationComplete(PCIDLIST_ABSOLUTE pidlFolder);
	IFACEMETHODIMP OnNavigationFailed(PCIDLIST_ABSOLUTE pidlFolder);


	// IServiceProvider
	IFACEMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

	// ICommDlgBrowser
	IFACEMETHODIMP OnDefaultCommand(IShellView * /* psv */);
	IFACEMETHODIMP OnStateChange(IShellView * /* psv */, ULONG uChange);
	IFACEMETHODIMP IncludeObject(IShellView * psv, PCUITEMID_CHILD pidl);

	// ICommDlgBrowser2
	IFACEMETHODIMP Notify(IShellView * /* ppshv */ , DWORD /* dwNotifyType */);
	IFACEMETHODIMP GetDefaultMenuText(IShellView * /* ppshv */, PWSTR /* pszText */, int /* cchMax */);
	IFACEMETHODIMP GetViewFlags(DWORD *pdwFlags);

	void create();
	int BrowseToIDList(LPCWSTR szPath);
	int BrowseToIDList(int iSBSBParentBackForward);
	void getSelectedItems(map<long, char *>* mapReturn, UINT svgio);

	HMENU CreateCustomPopupMenu();


	void OnNavigation(int iMode, PCIDLIST_ABSOLUTE pidlFolder);
	bool failedWithLog(HRESULT hr, const char * comment);
	bool failedExplorerNull(const char * comment);
	virtual void logMessage( char * s) = 0;
	virtual void navigationMessage( int iMode, const char * sRet) = 0;
	virtual void selectionMessage( DWORD eventobjectselection) = 0;
	virtual void getContextMenuCustomOptions(map<long, char *>* mapReturn, char* contextMenuFocusedPath) = 0;
	virtual void notifyContextMenuCustomOption(int iOption, char* contextMenuFocusedPath) = 0;
};

class ExplorerExecutor {
private:
	HANDLE mutexJni;
	HWND hwnd;
public:
	ExplorerExecutor(CExplorerBrowser* explorerBrowser);
	virtual ~ExplorerExecutor();
	virtual void execute( CExplorerBrowser* explorerBrowser) = 0;
	void release();
	void waitFor();
};


#endif


