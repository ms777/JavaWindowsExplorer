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

#include <windows.h>
#include "Log.h"

using namespace std;


TLogLevel Log::reportingLevel = logDEBUG4;

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const string Log::currentDateTime() {
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);
	char buf[80];
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", systemTime.wYear, systemTime.wMonth, systemTime.wDay,
			systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
	return buf;
}


const string Log::ToString(TLogLevel level) {

	switch(level) {
	case logERROR:   return "ERROR:  ";
	case logWARNING: return "WARNING:";
	case logINFO:    return "INFO:   ";
	case logDEBUG:   return "DEBUG:  ";
	case logDEBUG1:  return "DEBUG1: ";
	case logDEBUG2:  return "DEBUG2: ";
	case logDEBUG3:  return "DEBUG3: ";
	case logDEBUG4:  return "DEBUG4: ";
	}
	return "";
}

void Log::setReportingLevel(TLogLevel level) {
	reportingLevel = level;
}

TLogLevel Log::ReportingLevel() {
	return reportingLevel;
}


ostringstream& Log::Get(TLogLevel level) {
	this->level = level;
	os << currentDateTime() << " " << ToString(level) << " ";
	return os;
}

Log::Log() {
	level = logDEBUG4;
}

Log::~Log() {
	os << endl;
	Output(os, level);
}





