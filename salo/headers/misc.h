/*
*
* SaloIntellect project
* Copyright (C) 2015-2020 Motylenok Mikhail, Makarevich Nikita
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef MISC 
#define MISC

void miscFileToConsole(const char* fname);
void miscCheckExecName(const char* fname);
char* miscFindAndCopy(const char* where, const char* what, const char* end, int offset = 0);
std::string miscFindAndCopyString(const char* where, const char* what, const char* end, int offset = 0);
std::string miscInputPassword();
int miscSecondsToHours(int seconds);
void miscPrintAccessToken(const std::string& token); 
std::string miscGetStringFromFile(const char* fname);
long miscUnixTimeNow();
std::string miscSecondsToDHMS(int seconds);
std::string miscSecondsToRusDHM(int seconds);
std::string miscGetFile(const char* fname, int ignoreCRLF = 0);
void miscSetFile(const char* fname, const std::string& str);
bool miscFileExists(const char* fname);
std::string miscGetCurrentDateTime();
std::string miscExecSystem(const char*);
std::string miscFormatted(const std::string& what, int mode);
std::string miscUpscale(const std::string& what, int scale);
std::string miscStringDouble(double val, int pers);
std::string miscFilterLetters(const std::string& src);

#endif