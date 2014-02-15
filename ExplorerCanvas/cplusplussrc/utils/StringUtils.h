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

#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include <string>
#include <windows.h>
#include <winuser.h>
#include <shobjidl.h>

using namespace std;


string wstrtostr(const wstring &wstr);
char* wstrtochar(const wstring &wstr);
char* wchartochar(const WCHAR* wstr);

LPWSTR chartowstr(const char *nativeString);

char* GetDisplayNameOf(	IShellFolder* shellFolder, LPITEMIDLIST pidl, SHGDNF uFlags);
string getGuidDescription(REFGUID guidService);



#endif /* STRINGUTILS_H_ */
