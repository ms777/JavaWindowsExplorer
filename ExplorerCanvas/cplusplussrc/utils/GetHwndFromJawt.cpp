/*
 * GetHwndFromJawt.cpp
 *
 *  Created on: 15.02.2014
 *      Author: martin
 */

#include "jni.h"
#include <windows.h>
#include <winbase.h>
#include <psapi.h>
#include <jawt.h>
#include "jawt_md.h"


#include "CExplorerBrowser.h"
#include "StringUtils.h"
#include "Log.h"
#include "GetHwndFromJawt.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"


typedef jboolean(*PGET_AWT)(JNIEnv*, JAWT*);
static PGET_AWT pJAWT_GetAWT;

static void* jawt_handle = NULL;

#if defined(_WIN64)
#define METHOD_NAME "JAWT_GetAWT"
#else
#define METHOD_NAME "_JAWT_GetAWT@8"
#endif

static FARPROC w32_find_entry(JNIEnv* env, HANDLE handle, const char* funname) {
	FARPROC func = NULL;
	if (handle != GetModuleHandle(NULL)) {
		func = GetProcAddress((HMODULE) handle, funname);
	}
	else {
		HANDLE cur_proc = GetCurrentProcess();
		HMODULE *modules;
		DWORD needed, i;
		if (!EnumProcessModules(cur_proc, NULL, 0, &needed)) {
			LOG(logERROR) << "GetHwndFromJawt:w32_find_entry: EnumProcessModules 1 failed";
			return 0;
		}
		modules = (HMODULE*) alloca (needed);
		if (!EnumProcessModules(cur_proc, modules, needed, &needed)) {
			LOG(logERROR) << "GetHwndFromJawt:w32_find_entry: EnumProcessModules 2 failed";
			return 0;
		}
		for (i = 0; i < needed / sizeof (HMODULE); i++) {
			if ((func = GetProcAddress(modules[i], funname))) {
				break;
			}
		}
	}
	return func;
}



static jstring get_system_property(JNIEnv* env, const char* name) {
	jclass classSystem = env->FindClass("java/lang/System");
	if (classSystem != NULL) {
		jmethodID mid = env->GetStaticMethodID(classSystem, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
		if (mid != NULL) {
			jstring propname = env->NewStringUTF(name);
			return (jstring) env->CallStaticObjectMethod(classSystem, mid, propname);
		}
	}
	return NULL;
}




HWND getHwndAwt(JNIEnv *env, jobject component) {
	JAWT_DrawingSurface* ds;
	JAWT_DrawingSurfaceInfo* dsi;
	JAWT_Win32DrawingSurfaceInfo* dsi_win;
	JAWT awt;

	// NOTE: AWT/JAWT must be loaded prior to this code's execution
	// See http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6539705
	awt.version = JAWT_VERSION_1_4;

	// Java versions 1.5/1.6 throw UnsatisfiedLinkError when run headless
	// Avoid the issue by dynamic linking
	if (!pJAWT_GetAWT) {
		// Windows needs the full path to JAWT; calling System.loadLibrary("jawt")
		// from Java adds it to the path so that a simple LoadLibrary("jawt.dll")
		// works, but may cause other attempts to load that library from Java to
		// to get an UnsatisfiedLinkError, reporting that the library is already
		// loaded in a different class loader, since there is no way to force the
		// JAWT library by the system class loader.
		// Use Unicode strings in case the path to the library includes non-ASCII
		// characters.
		wchar_t* path = L"jawt.dll";
		jstring jprop = get_system_property(env, "java.home");
		if (jprop != NULL) {
			const char *nativeString = env->GetStringUTFChars(jprop, 0);
			const wchar_t* prop = chartowstr(nativeString);
			env->ReleaseStringUTFChars(jprop, nativeString);

			const wchar_t* suffix = L"/bin/jawt.dll";
			size_t len = wcslen(prop) + wcslen(suffix) + 1;
			path = (wchar_t*)alloca(len * sizeof(wchar_t));
			swprintf(path, L"%s%s", prop, suffix);
			free((void *)prop);
		}

		if ((jawt_handle = LoadLibraryExW(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)) == NULL) {
			LOG(logERROR) << "GetHwndFromJawt:LOAD_LIBRARY: jawt_handle is null, path: " << path;
			return (HWND) -1;
		}
		if ((pJAWT_GetAWT = (PGET_AWT)  w32_find_entry(env, jawt_handle, METHOD_NAME) ) == NULL) {
			LOG(logERROR) << "GetHwndFromJawt:FIND_ENTRY: pJAWT_GetAWT is null, METHOD_NAME: " << METHOD_NAME;
			return (HWND) -1;
		}
	}

	if (!pJAWT_GetAWT(env, &awt)) {
		LOG(logERROR) << "GetHwndFromJawt:JAWT_GetAWT: Can't load JAWT";
		return 0;
	}
	ds = awt.GetDrawingSurface(env, component);
	if(ds == NULL)	return (HWND) -1;
	// Lock the drawing surface
	ds->Lock(ds);

	// Get the drawing surface info
	dsi = ds->GetDrawingSurfaceInfo(ds);
	if(dsi == NULL)	return (HWND) -2;

	// Get the platform-specific drawing info
	dsi_win = (JAWT_Win32DrawingSurfaceInfo*)dsi->platformInfo;
	if(dsi_win == NULL)	return (HWND) -3;
	HWND hwnd = dsi_win->hwnd;

	// Free the drawing surface info
	ds->FreeDrawingSurfaceInfo(dsi);
	// Unlock the drawing surface
	ds->Unlock(ds);
	// Free the drawing surface
	awt.FreeDrawingSurface(ds);
	return hwnd;
}
