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

#include "jni.h"
#include <windows.h>

#include "CExplorerBrowser.h"
#include "StringUtils.h"
#include "GetHwndFromJawt.h"

#include <jawt.h>
#include "jawt_md.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "structuredquery.h"
#include "search_ms777.h"
#include "Log.h"


void Log::Output(ostringstream& os, TLogLevel level) {
	fprintf(stdout, "%s", os.str().c_str());
	fflush(stdout);
}




/*
 * -----------------------   module helpers
 */
static jmethodID methHashcode = NULL;
static jmethodID methLogmessage = NULL;
static jmethodID methSelectionchanged = NULL;
static jmethodID methNavigationOccured = NULL;
static jmethodID methGetContextMenuCustomOptions = NULL;
static jmethodID methNotifyContextMenuCustomOption = NULL;
static jfieldID fieldHwnd = NULL;



int getHashCode(JNIEnv *env, jobject thisobject) {
	return (int) env->CallIntMethod(thisobject, methHashcode);
}

HWND getHwndFromJavaField(JNIEnv *env, jobject thisobject) {
	return (HWND) env->GetLongField(thisobject, fieldHwnd);
}

void callLogMessage(JNIEnv *env, jobject object, char * message) {
	char buf[200];
	sprintf(buf, "native hwnd: %I64d, native mes: %s", (long long int) getHwndFromJavaField(env, object), message);
	jstring jString = env->NewStringUTF(buf);
	if (jString != NULL) {
		env->CallVoidMethod(object, methLogmessage, jString);
	}
}


JavaVM * g_vm; // initialized by JNI_OnLoad

static JNIEnv* getGlobalEnv(JavaVM* g_vm) {
	JNIEnv* g_env;
	// double check it's all ok
	int getEnvStat = g_vm->GetEnv((void **)&g_env, JNI_VERSION_1_6);
	if (getEnvStat == JNI_EDETACHED) {
		if (g_vm->AttachCurrentThread((void **) &g_env, NULL) != 0) {
			LOG(logERROR) << "ExplorerBrowserJni:getGlobalEnv: attach failed";
			return NULL;
		}
	} else if (getEnvStat == JNI_OK) {
		//
	} else if (getEnvStat == JNI_EVERSION) {
		LOG(logERROR) << "ExplorerBrowserJni:getGlobalEnv: version not supported";
		return NULL;
	}
	return g_env;
}

LPWSTR jstringtoLPWSTR(JNIEnv *env, jstring s) {
	jboolean bIsCopy = true;
	return chartowstr(env->GetStringUTFChars(s, &bIsCopy));
}



static bool bShouldStop = false;
static DWORD threadJniDescriptor;
static HANDLE hThreadJni = 0;



class CExplorerBrowserJni : public CExplorerBrowser {
public:
	jobject javaCanvas;

	CExplorerBrowserJni(HWND hwndJavaWindow, jobject javaCanvas, long lExplorerBrowserOptions, long lFolderViewMode, long lFolderFlags):
		CExplorerBrowser(hwndJavaWindow, lExplorerBrowserOptions, lFolderViewMode, lFolderFlags) {
		this->javaCanvas = javaCanvas;
	};
	~CExplorerBrowserJni() {
		JNIEnv* env = getGlobalEnv(g_vm);
		env->DeleteGlobalRef(javaCanvas);
		javaCanvas = NULL;
	};
	void logMessage( char * s) {
		LOG(logINFO) << "ExplorerBrowserJni:CExplorerBrowser:logMessage: " << s;
	};
	void navigationMessage( int iMode, const char * sRet) {
		JNIEnv* env = getGlobalEnv(g_vm);
		jstring jRet = env->NewStringUTF(sRet);
		env->CallVoidMethod(javaCanvas, methNavigationOccured, (jint) iMode, jRet);
	};

	void selectionMessage( DWORD eventobjectselection) {
		JNIEnv* env = getGlobalEnv(g_vm);
		env->CallVoidMethod(javaCanvas, methSelectionchanged);
	};

	void getContextMenuCustomOptions(map<long, char *>* mapReturn, char* contextMenuFocusedPath) {
		JNIEnv* env = getGlobalEnv(g_vm);
		jstring jcontextMenuFocusedPath = env->NewStringUTF(contextMenuFocusedPath);
		jobjectArray astrCustomOptions = (jobjectArray) env->CallObjectMethod(javaCanvas, methGetContextMenuCustomOptions, jcontextMenuFocusedPath);
		if (astrCustomOptions==NULL) {
			LOG(logDEBUG1) << "ExplorerBrowserJni:CExplorerBrowser:getPopupCustomOptions:astrCustomOptions==NULL";
			return;
		}
		int iCount = env->GetArrayLength(astrCustomOptions);
		LOG(logDEBUG1) << "ExplorerBrowserJni:CExplorerBrowser:getPopupCustomOptions:iCount: " << iCount;
		for (int i=0; i < iCount; i++) {
			jobject js = env->GetObjectArrayElement(astrCustomOptions, i);
			const char *str = env->GetStringUTFChars((jstring) js, 0);
			LOG(logDEBUG1) << "ExplorerBrowserJni:CExplorerBrowser:getPopupCustomOptions:option " << i << ": " << str;
			(*mapReturn)[i] = (char*) str;
		}
	}

	void notifyContextMenuCustomOption(int iOption, char* contextMenuFocusedPath) {
		JNIEnv* env = getGlobalEnv(g_vm);
		jstring jcontextMenuFocusedPath = env->NewStringUTF(contextMenuFocusedPath);
		env->CallVoidMethod(javaCanvas, methNotifyContextMenuCustomOption, iOption, jcontextMenuFocusedPath);
	}


};



/*
 * -----------------------   diagnostics (temporary)
 */



static DWORD WINAPI threadMapDump(void* threadParams) {

	LOG(logDEBUG1) << "ExplorerBrowserJni:threadMapDump start ";

	JNIEnv* g_env = getGlobalEnv(g_vm);
	if (g_env==NULL) return 0;

	int i = 0;
	while (!bShouldStop) {
		Sleep(50);
		i++;
		if (i==100) {
			i=0;

			map<HWND, CExplorerBrowser*>::iterator p;

			int k=0;
			int maxchars = 240;
			int charpos = 0;
			char buf[maxchars];
			charpos += snprintf(buf+charpos, maxchars, "mapdump: ");

			for(p = CExplorerBrowser::mapCanvasExplorerbrowser.begin(); p!=CExplorerBrowser::mapCanvasExplorerbrowser.end(); ++p)	{
				charpos += snprintf(buf+charpos, maxchars-charpos, "map[%i]: hwnd: %I64d, ", k, (long long int) p->first);
				k++;
			}
			LOG(logDEBUG1) << buf;
		}
		if (CExplorerBrowser::mapCanvasExplorerbrowser.size() ==0 ) {
			break;
		}
	}
	if (g_env->ExceptionCheck()) {
		g_env->ExceptionDescribe();
	}

	g_vm->DetachCurrentThread();
	LOG(logDEBUG1) << "ExplorerBrowserJni:threadMapDump stop ";
	return 0;
}

#define btoa(x) ((x)?"true":"false")


/*
 * -----------------------   jni interface routines
 */

void setLoglevel(JNIEnv *env, jclass clazz, jint iLevel) {
	Log::setReportingLevel((TLogLevel) iLevel);
}

//#define BUILT_DATE "2014-02-16 12:32:30";

jstring getBuiltDate(JNIEnv *env, jclass clazz) {
#ifndef BUILT_DATE
	return NULL;
#else
	const char* cp = BUILT_DATE;
	return env->NewStringUTF(cp);
#endif
}

jlong notifyCreated(JNIEnv *env, jclass clazz, jobject thisobject, jlong lExplorerBrowserOptions, jlong lFolderViewMode, jlong lFolderFlags) {
	// setting up the global variables
	if (methHashcode == NULL) {
		methHashcode = env->GetMethodID(clazz, "hashCode", "()I");
	}
	if (methLogmessage == NULL) {
		methLogmessage = env->GetMethodID(clazz, "logMessage", "(Ljava/lang/String;)V");
	}
	if (methSelectionchanged == NULL) {
		methSelectionchanged = env->GetMethodID(clazz, "selectionChanged", "()V");
	}
	if (methNavigationOccured == NULL) {
		methNavigationOccured = env->GetMethodID(clazz, "navigationOccurred", "(ILjava/lang/String;)V");
	}
	if (methGetContextMenuCustomOptions == NULL) {
		methGetContextMenuCustomOptions = env->GetMethodID(clazz, "getContextMenuCustomOptions", "(Ljava/lang/String;)[Ljava/lang/String;");
	}
	if (methNotifyContextMenuCustomOption == NULL) {
		methNotifyContextMenuCustomOption = env->GetMethodID(clazz, "notifyContextMenuCustomOption", "(ILjava/lang/String;)V");
	}

	if (fieldHwnd == NULL) {
		fieldHwnd = env->GetFieldID(clazz, "lhwnd", "J");
	}

	// if the parent hwnd is generated in Java prior to calling notifycreated
	HWND lHwndJavaParent = getHwndFromJavaField(env, thisobject);
	LOG(logDEBUG1) << "ExplorerBrowserJni:notifyCreated:hwnd from Java: " << lHwndJavaParent;

	if (lHwndJavaParent ==0) {
		lHwndJavaParent = getHwndAwt( env, thisobject);
		LOG(logDEBUG1) << "ExplorerBrowserJni:notifyCreated:hwnd from jawt: " << lHwndJavaParent;
	}

	// must be done here in order to not loose the javaCanvas reference
	jobject javaCanvas = env->NewGlobalRef(thisobject);
	new CExplorerBrowserJni(lHwndJavaParent, javaCanvas, (long) lExplorerBrowserOptions, (long) lFolderViewMode, (long) lFolderFlags);

	if (hThreadJni==0) {
		bShouldStop = false;
		LPTHREAD_START_ROUTINE lpstart = (LPTHREAD_START_ROUTINE) reinterpret_cast< void *> (&threadMapDump);
		hThreadJni = CreateThread(NULL,   0,  lpstart,  NULL, 0,   &threadJniDescriptor);
	}

	callLogMessage(env, thisobject, "ExplorerBrowserJni:notifyCreated");

	return (jlong) lHwndJavaParent;
}

class ExplorerExecutorGetSelectedItems: public ExplorerExecutor {
public:
	map<long, char *> *mapReturn;
	UINT svgio;
	ExplorerExecutorGetSelectedItems(CExplorerBrowser* eb, map<long, char *> *mapReturn, UINT svgio):ExplorerExecutor(eb) {
		this->mapReturn = mapReturn;
		this->svgio = svgio;
	}
	~ExplorerExecutorGetSelectedItems() {
	}
	void execute( CExplorerBrowser* explorerBrowser) {
		explorerBrowser->getSelectedItems(mapReturn, svgio);
		release();
	}
};

jobjectArray getSelectedPaths(JNIEnv *env, jclass clazz, jobject thisobject, jlong svgio) {
	CExplorerBrowser* explorerBrowser = CExplorerBrowser::getExplorerBrowserByHwnd(getHwndFromJavaField(env, thisobject));
	if (explorerBrowser==NULL) return NULL;

	LOG(logDEBUG1) << "ExplorerBrowserJni:getSelectedPaths start";

	map<long, char *> mapReturn;
	ExplorerExecutorGetSelectedItems *explorerExecutor = new ExplorerExecutorGetSelectedItems(explorerBrowser, &mapReturn, (UINT) svgio);
	explorerExecutor->waitFor();
	delete explorerExecutor;

	int iCount = mapReturn.size();

	jobjectArray  joaRet = env->NewObjectArray(iCount, env->FindClass("java/lang/String"), NULL);

	for(int i = 0; i < iCount; i++) {
		env->SetObjectArrayElement(joaRet, i, env->NewStringUTF(mapReturn.at((long) i)));
	}
	LOG(logDEBUG1) << "ExplorerBrowserJni:getSelectedPaths end";
	return joaRet;
}

class ExplorerExecutorGiveupFocus: public ExplorerExecutor {
public:
	ExplorerExecutorGiveupFocus(CExplorerBrowser* eb):ExplorerExecutor(eb) {
	}
	void execute( CExplorerBrowser* explorerBrowser) {
		LOG(logDEBUG1) << "ExplorerBrowserJni:ExplorerExecutorGiveupFocus";
		HWND hwnd = GetFocus();
		LOG(logDEBUG1) << "ExplorerBrowserJni:ExplorerExecutorDummy, GetFocus: " << (long long int) hwnd;
		HWND hr = SetFocus(NULL);
		LOG(logDEBUG1) << "ExplorerBrowserJni:ExplorerExecutorDummy, SetFocus: " << (long long int) hr;
		release();
	}
};

void giveupFocus(JNIEnv *env, jclass clazz, jobject thisobject) {
	LOG(logDEBUG1) << "ExplorerBrowserJni:giveupFocus start";
	CExplorerBrowser* explorerBrowser = CExplorerBrowser::getExplorerBrowserByHwnd(getHwndFromJavaField(env, thisobject));
	if (explorerBrowser==NULL) return;
	ExplorerExecutorGiveupFocus *explorerExecutor = new ExplorerExecutorGiveupFocus(explorerBrowser);
	explorerExecutor->waitFor();
	delete explorerExecutor;
	LOG(logDEBUG1) << "ExplorerBrowserJni:giveupFocus end";
}


class ExplorerExecutorCommand: public ExplorerExecutor {
public:
	int iCommand;
	ExplorerExecutorCommand(CExplorerBrowser* eb, int iCommand):ExplorerExecutor(eb) {
		this->iCommand = iCommand;
	}
	void execute( CExplorerBrowser* explorerBrowser) {
		LOG(logDEBUG1) << "ExplorerBrowserJni:ExplorerExecutorDummy, iCommand: " << iCommand;
		if (iCommand==1) {
		}
		release();
	}
};

void sendCommand(JNIEnv *env, jclass clazz, jobject thisobject, jint iCommand) {
	CExplorerBrowser* explorerBrowser = CExplorerBrowser::getExplorerBrowserByHwnd(getHwndFromJavaField(env, thisobject));
	if (explorerBrowser==NULL) return;
	int iCmd = (int) iCommand;
	if (iCmd>1) {
		LOG(logDEBUG1) << "ExplorerBrowserJni:sendCommand start, iCommand: " << iCmd;

		ExplorerExecutorCommand *explorerExecutor = new ExplorerExecutorCommand(explorerBrowser, iCmd);
		explorerExecutor->waitFor();
		delete explorerExecutor;
	} else {
		HWND hwnd2 = getHwndAwt(env, thisobject);
		LOG(logDEBUG1) << "ExplorerBrowserJni:sendCommand end, hwnd: " << (long long int) hwnd2;
	}
	LOG(logDEBUG1) << "ExplorerBrowserJni:sendCommand end, iCommand: " << iCmd;
}


class ExplorerExecutorBrowseTo: public ExplorerExecutor {
public:
	LPWSTR unicodestr;
	int iCommand;
	ExplorerExecutorBrowseTo(CExplorerBrowser* eb, LPWSTR unicodestr):ExplorerExecutor(eb) {
		this->unicodestr = unicodestr;
		this->iCommand = INT_MIN;
	}
	ExplorerExecutorBrowseTo(CExplorerBrowser* eb, int iCommand):ExplorerExecutor(eb) {
		this->unicodestr = NULL;
		this->iCommand = iCommand;
	}
	~ExplorerExecutorBrowseTo() {
		delete unicodestr;
	}
	void execute( CExplorerBrowser* explorerBrowser) {
		if (iCommand==INT_MIN) {
			if (unicodestr==NULL) {
				LOG(logDEBUG1) << "ExplorerExecutorBrowseTo: path is null";
//				explorerBrowser->explorerBrowser->FillFromObject(NULL, (EXPLORER_BROWSER_FILL_FLAGS) 0);
//				explorerBrowser->explorerBrowser->BrowseToObject(NULL, (EXPLORER_BROWSER_FILL_FLAGS) 0);
				explorerBrowser->explorerBrowser->RemoveAll();
			} else {
				explorerBrowser->BrowseToIDList(unicodestr);
			}
		} else {
			explorerBrowser->BrowseToIDList(iCommand);
		}
		release();
	}
};


void browseRelative(JNIEnv *env, jclass clazz, jobject thisobject, jint iCommand) {
	CExplorerBrowser* explorerBrowser = CExplorerBrowser::getExplorerBrowserByHwnd(getHwndFromJavaField(env, thisobject));
	if (explorerBrowser==NULL) return;
	LOG(logDEBUG1) << "ExplorerBrowserJni:browseRelative start, iCommand: " << (int) iCommand;

	ExplorerExecutorBrowseTo *explorerExecutor = new ExplorerExecutorBrowseTo(explorerBrowser, (int) iCommand);
	explorerExecutor->waitFor();
	delete explorerExecutor;
	LOG(logDEBUG1) << "ExplorerBrowserJni:browseRelative end, iCommand: " << (int) iCommand;
}



//void browseTo(JNIEnv *env, jobject thisobject, jstring path) {
void browseTo(JNIEnv *env, jclass clazz, jobject thisobject, jstring path) {
	CExplorerBrowser* explorerBrowser = CExplorerBrowser::getExplorerBrowserByHwnd(getHwndFromJavaField(env, thisobject));
	if (explorerBrowser==NULL) return;

	LOG(logDEBUG1) << "ExplorerBrowserJni:browseTo start";
	LPWSTR unicodestr = NULL;
	if (path==NULL) {
		LOG(logDEBUG1) << "browseTo, path is null ";
	} else {
		const char *nativeString = env->GetStringUTFChars(path, 0);
		unicodestr = chartowstr(nativeString);
		env->ReleaseStringUTFChars(path, nativeString);
		LOG(logDEBUG1) << "browseTo, path: " << wstrtochar(unicodestr);
	}

	ExplorerExecutorBrowseTo *explorerExecutor = new ExplorerExecutorBrowseTo(explorerBrowser, unicodestr);
	explorerExecutor->waitFor();
	delete explorerExecutor;

	callLogMessage(env, thisobject, "ExplorerBrowserJni:browseTo finished");
	LOG(logDEBUG1) << "ExplorerBrowserJni:browseTo end";
}

class ExplorerExecutorSetOptions: public ExplorerExecutor {
public:
	long lOptions;
	ExplorerExecutorSetOptions(CExplorerBrowser* eb, long lOptions):ExplorerExecutor(eb) {
		this->lOptions = lOptions;
	}
	void execute( CExplorerBrowser* explorerBrowser) {
		explorerBrowser->explorerBrowser->SetOptions((EXPLORER_BROWSER_OPTIONS)lOptions);
		release();
	}
};


void setOptions(JNIEnv *env, jclass clazz, jobject thisobject, jlong options) {
	CExplorerBrowser* explorerBrowser = CExplorerBrowser::getExplorerBrowserByHwnd(getHwndFromJavaField(env, thisobject));
	if (explorerBrowser==NULL) return;

	ExplorerExecutorSetOptions *explorerExecutor = new ExplorerExecutorSetOptions(explorerBrowser, (long) options);
	explorerExecutor->waitFor();
	delete explorerExecutor;

	callLogMessage(env, thisobject, "ExplorerBrowserJni:setOptions finished");
}

class ExplorerExecutorDoQuery: public ExplorerExecutor {
public:
   	LPWSTR* lpwstrScopePath;
   	int sizeScopePath;
	LPWSTR lpwstrQuery;
	LPWSTR lpwstrDisplayName;

	ExplorerExecutorDoQuery(CExplorerBrowser* eb, LPWSTR* lpwstrScopePath, int sizeScopePath, LPWSTR lpwstrQuery, LPWSTR lpwstrDisplayName):ExplorerExecutor(eb) {
		this->lpwstrScopePath = lpwstrScopePath;
		this->sizeScopePath = sizeScopePath;
		this->lpwstrQuery = lpwstrQuery;
		this->lpwstrDisplayName = lpwstrDisplayName;
	}
	void execute( CExplorerBrowser* explorerBrowser) {
		LOG(logDEBUG1) << "ExplorerExecutorDoQuery start";
		IQueryParser *_pqp;
		HRESULT hr = CreateQueryParser(&_pqp);
		if (SUCCEEDED(hr))
		{
			IShellItemArray *psia = NULL;
			LOG(logDEBUG1) << "ExplorerBrowserJni:ExplorerExecutorDoQuery 1: " << (long) hr;
//				psia = CreateShellItemArrayFromSampleScope();

			if ((lpwstrScopePath!=NULL) && (sizeScopePath>0)) {
				psia = CreateShellItemArrayFromLPWSTRarray(lpwstrScopePath, sizeScopePath);
			}

			LOG(logDEBUG1) << "ExplorerBrowserJni:ExplorerExecutorDoQuery 2: " << (LPVOID) psia;

			IShellItem2 *psi;
			LOG(logDEBUG1) << "ExplorerBrowserJni:ExplorerExecutorDoQuery 3";
			hr = GetQueryItem(IID_IShellItem, reinterpret_cast< void **>(&psi), _pqp, psia, lpwstrDisplayName, lpwstrQuery);
			LOG(logDEBUG1) << "ExplorerBrowserJni:ExplorerExecutorDoQuery 4";
			if (SUCCEEDED(hr)) {
				explorerBrowser->explorerBrowser->BrowseToObject(psi, 0);
				psi->Release();
			}
			if (psia != NULL) psia->Release();
			_pqp->Release();
		}
		release();
	}
};


jlong doSearch(JNIEnv *env, jclass clazz, jobject thisobject, jobjectArray asScope, jstring sQuery, jstring sDisplayName) {
	CExplorerBrowser* explorerBrowser = CExplorerBrowser::getExplorerBrowserByHwnd(getHwndFromJavaField(env, thisobject));
	if (explorerBrowser==NULL) return -1;

	LPWSTR lpwstrQuery = jstringtoLPWSTR(env, sQuery);
	LPWSTR lpwstrDisplayName = jstringtoLPWSTR(env, sDisplayName);
	ExplorerExecutorDoQuery *explorerExecutor;

	if (asScope == NULL) {
		explorerExecutor = new ExplorerExecutorDoQuery(explorerBrowser, NULL, 0, lpwstrQuery, lpwstrDisplayName);
	} else {
		int sizeScopePath = (int) env->GetArrayLength(asScope);

	   	LPWSTR lpwstrScopePath[sizeScopePath];
		for (int i=0; i < sizeScopePath; i++) {
			jstring sScope = (jstring) env->GetObjectArrayElement(asScope, i);
	       	lpwstrScopePath[i] = jstringtoLPWSTR(env, sScope);
		}

		explorerExecutor = new ExplorerExecutorDoQuery(explorerBrowser, lpwstrScopePath, sizeScopePath, lpwstrQuery, lpwstrDisplayName);
	}
	explorerExecutor->waitFor();
	delete explorerExecutor;

	callLogMessage(env, thisobject, "doSearch finished");

	return 0;
}
/*
 * -----------------------   jni load and unload
 */


static JNINativeMethod method_table[] = {
		{ "notifyCreated", 		"(Lms777/explorercanvas/ExplorerCanvas;JJJ)J", (void *) notifyCreated }
		,{ "setLoglevel",		"(I)V", (void *) setLoglevel }
		,{ "getBuiltDate",		"()Ljava/lang/String;", (void *) getBuiltDate }
		,{ "sendCommand",		"(Lms777/explorercanvas/ExplorerCanvas;I)V", (void *) sendCommand }
		,{ "browseRelative",	"(Lms777/explorercanvas/ExplorerCanvas;I)V", (void *) browseRelative }
		,{ "browseTo",			"(Lms777/explorercanvas/ExplorerCanvas;Ljava/lang/String;)V", (void *) browseTo }
		,{ "getSelectedPaths",	"(Lms777/explorercanvas/ExplorerCanvas;J)[Ljava/lang/String;", (void *) getSelectedPaths }
		,{ "setOptions",		"(Lms777/explorercanvas/ExplorerCanvas;J)V", (void *) setOptions }
		,{ "doSearch",			"(Lms777/explorercanvas/ExplorerCanvas;[Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)J", (void *) doSearch }
		,{ "giveupFocus",		"(Lms777/explorercanvas/ExplorerCanvas;)V", (void *) giveupFocus }
};

static int method_table_size = sizeof(method_table) / sizeof(method_table[0]);

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env;
	if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
		return JNI_ERR;
	} else {
		jclass clazz = env->FindClass("ms777/explorercanvas/ExplorerCanvas");
		if (clazz) {
			jint ret = env->RegisterNatives(clazz, method_table, method_table_size);
			env->DeleteLocalRef(clazz);
//			classString = env->NewGlobalRef(env->FindClass("java/lang/String"));
			env->GetJavaVM(&g_vm);

			return ret == 0 ? JNI_VERSION_1_6 : JNI_ERR;
		} else {
			return JNI_ERR;
		}
	}
}
jint JNI_OnUnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env;
	printf("JNI_OnUnLoad");
	fflush(stdout);
	if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
		return JNI_ERR;
	} else {
//		env->DeleteGlobalRef(classString);

		printf("JNI_OnUnLoad");
	}
	return JNI_VERSION_1_6;
}




#pragma GCC diagnostic pop
