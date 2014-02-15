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

// after http://www.drdobbs.com/cpp/logging-in-c/201804215?pgno=1

#ifndef LOG_H_
#define LOG_H_
#include <sstream>
#include "Log.h"

using namespace std;

const std::string currentDateTime();

enum TLogLevel {logERROR, logWARNING, logINFO, logDEBUG, logDEBUG1,
	logDEBUG2, logDEBUG3, logDEBUG4};

class Log {
public:
	Log();
	virtual ~Log();
	ostringstream& Get(TLogLevel level = logINFO);

	static TLogLevel ReportingLevel();
	static void setReportingLevel(TLogLevel level);
	static void Output(ostringstream& os, TLogLevel level);
	static const string ToString(TLogLevel level);
	static const string currentDateTime();
protected:
	ostringstream os;
private:
	static TLogLevel reportingLevel;
	TLogLevel level;
	Log(const Log&);
	Log& operator =(const Log&);
};

#define LOG(level) \
	if (level > Log::ReportingLevel()) ; \
	else Log().Get(level)



#endif /* LOG_H_ */
