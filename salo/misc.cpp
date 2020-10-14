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


#include <iostream>
#include <fstream>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <memory>
#include "headers/misc.h"
#include "headers/salostring.h"


/*

Проверяет правильно ли запущен процесс
Чтобы избежать ошибки с рабочими каталогами

*/

void miscCheckExecName(const char* fname)
{
	if(std::string(fname) != "./salobin" & std::string(fname) != "salobin")
	{
		std::cout << "SALO Server binary name mismatch " << std::endl << "Restart this program CORRECTLY" << std::endl;
		exit(0);
	}
}


/*

Выводит файл в консоль

*/

void miscFileToConsole(const char* fname)
{
std::ifstream input(fname, std::ifstream::in);

if(input.fail())
	{
		std::cout << "Cant read file " << fname << std::endl;
	}

while(!input.eof())
	{
		std::string tmp;
		std::getline(input, tmp);

		if(!input) { break; }

		std::cout << tmp << std::endl;
	}

	input.close();
}


/*

Возвращает файл в виде строки

*/

std::string miscGetFile(const char* fname, int ignoreCRLF)
{
std::ifstream input(fname, std::ifstream::in);

if(input.fail())
	{
		return (std::string("Cant read file ") + std::string(fname));
	}

std::string res;

while(!input.eof())
	{
		std::string tmp;
		std::getline(input, tmp);

		if(!input) { break; }

		if(!ignoreCRLF)
			res += tmp + "\n";
		else
			res += tmp;
	}

	input.close();

	return res;
}


/*

Записывает строку в файл на диск. Если не существует - создает.

*/

void miscSetFile(const char* fname, const std::string& str)
{
	std::ofstream fl(fname);
	fl << str;
	fl.flush();
	fl.close();
}


/*

Проверяет файл на существование

*/

bool miscFileExists(const char* fname) 
{
    if (FILE* file = fopen(fname, "r")) 
    {
        fclose(file);
        return true;
    } 
    else 
    {
        return false;
    }   
}


/*

Функция для нахожения значений в заголовках ответа сервера

*/

char* miscFindAndCopy(const char* where, const char* what, const char* end, int offset)
{
	char* pSTART = strstr((char*)where, what);
	
	if (pSTART == NULL) 
		{ 
			return NULL; 
		}
	
	pSTART += strlen(what) + offset;

	char* pEND = strstr(pSTART, end);
	
	if(pEND == NULL) 
		{ 
			return NULL; 
		}
	
	int size = pEND - pSTART;
	
	char* copy = new char[size+1];
	strncpy(copy, pSTART, size);
	copy[size] = 0;
	
	return copy;
}



/*

Аналогичная функция, возвращает строку.

*/

std::string miscFindAndCopyString(const char* where, const char* what, const char* end, int offset)
{
	char* pSTART = strstr((char*)where, what);
	
	if (pSTART == NULL) 
		{ 
			return std::string();
			//return NULL; 
		}
	
	pSTART += strlen(what) + offset;

	char* pEND = strstr(pSTART, end);
	
	if(pEND == NULL) 
		{ 
			return std::string();
			//return NULL; 
		}
	
	int size = pEND - pSTART;
	
	char* copy = new char[size+1];
	strncpy(copy, pSTART, size);
	copy[size] = 0;
	
	std::string res(copy);

	delete[] copy;

	return res;
}


/*

Включает/выключает эхо вводимых символов в консоль
Необходимо для скрытия пароля при вводе

*/

void SetStdinEcho(bool enable = true)
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}


/*

Ввод пароля

*/

std::string miscInputPassword()
{
	SetStdinEcho(false);
	std::string password;
	std::cin >> password;
	SetStdinEcho(true);

	return password;
}


/*

Переводит секунды в часы

*/

int miscSecondsToHours(int seconds)
{
	return (seconds/60)/60;
}

/*

Переводит секунды в сутки, часы, минуты и секунды

*/

std::string miscSecondsToDHMS(int seconds)
{
	int DAYS = ((seconds / 60)/60)/24;
	seconds -= DAYS*24*60*60;
	int HOURS = (seconds/60)/60;
	seconds -= HOURS*60*60;
	int MINS = seconds/60;
	seconds -= MINS*60;
	
	std::string res = std::to_string(DAYS) + " Days, " + std::to_string(HOURS) + " Hours, " 
						+ std::to_string(MINS) + " Minutes, " + std::to_string(seconds) + " seconds";

	return res;
}


/*

Переводит секунды в сутки, часы, минуты 

*/

std::string miscSecondsToRusDHM(int seconds)
{
	int DAYS = ((seconds / 60)/60)/24;
	seconds -= DAYS*24*60*60;
	int HOURS = (seconds/60)/60;
	seconds -= HOURS*60*60;
	int MINS = seconds/60;
	seconds -= MINS*60;
	
	std::string res = std::to_string(DAYS) + " дней, " + std::to_string(HOURS) + " часов, " + std::to_string(MINS) + " минут ";

	return res;
}


/*

Безопасно выводит полученый токен на экран

*/

void miscPrintAccessToken(const std::string& token)
{
	char* tok = (char*)token.c_str();

	printf("Your access token (first 10 digits hidden for privacy): \n**********%s\n", tok+10);
}


/*

Возвращает строку прочитанную из файла
Нужно для загрузки настроек сервера

*/

std::string miscGetStringFromFile(const char* fname)
{

std::ifstream input(fname, std::ifstream::in);

if(input.fail())
	{
		std::cout << "Cant read file " << fname << std::endl;
	}

std::string tmp;
std::getline(input, tmp);

input.close();

return tmp;
}


/*

Получает юникс-время (количество секунд прошедних с 01.01.1970 00:00)

*/

long miscUnixTimeNow()
{
	std::time_t t = std::time(0);  
    
    return t;
}


/*

Возвращает строку с текстом даты/времени в текущей локали

*/

std::string miscGetCurrentDateTime()
{

	time_t mytime;
	mytime = time(NULL);
	tm* mtm = localtime(&mytime);
	char buff[70];
	strftime(buff, sizeof buff, "%d %B %Y %H:%M", mtm);

	return std::string(buff);
}


/*

Выполняет сторонню команду

*/

std::string miscExecSystem(const char* cmd)
{
	char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return NULL;
    while (!feof(pipe.get())) 
    {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}


/*

Добавляет Escape-последовательность в строку

	1 = bold
	2 = green
	3 = red
	4 = blue
	5 = magenta
	6 = black&bold
	7 = white background

*/

std::string miscFormatted(const std::string& what, int mode)
{	
	std::string a;
	std::string b = "\033[0m";

	switch(mode)
	{
		case 1: a = "\033[1m"; break;
		case 2: a = "\033[32m"; break;
		case 3: a = "\033[31m"; break;
		case 4: a = "\033[34m"; break;
		case 5: a = "\033[35m"; break;
		case 6: a = "\033[1;30m"; break;
		case 7: a = "\033[47m"; break;
	}

	return a + what + b;
}


/*

Апскейлит строку для правильной табуляции

*/

std::string miscUpscale(const std::string& what, int scale)
{
	std::string t = what;
	int scalefactor = scale - what.length();
	for(int i = 0; i < scalefactor; i++) 
		t.append(" ");
	return t;
}


/*

Обрезает дабл и переводит в строку

*/

std::string miscStringDouble(double val, int pers)
{
	std::string tt;
	char buf[64];
	char fmt[64];

	sprintf(fmt, "%%0.%df", pers); //Формируем строку формата
	sprintf(buf, fmt, val);

	tt = std::string(buf);

	return tt;
}


/*

Фильтрует буквы

*/

std::string miscFilterLetters(const std::string& src)
{
	std::wstring wsrc = MultibyteToWideString(src); 
	std::wstring wres;
	std::string res;
	int j = 0;

	for(int i = 0; i < wsrc.length(); i++)
	{
		if(wsrc[i] >= L'a' && wsrc[i] <= L'z')
			{ wres.push_back(wsrc[i]); j++; continue; }
		if(wsrc[i] >= L'A' && wsrc[i] <= L'Z')
			{ wres.push_back(wsrc[i]); j++; continue; }
		if(wsrc[i] >= L'а' && wsrc[i] <= L'я')
			{ wres.push_back(wsrc[i]); j++; continue; }
		if(wsrc[i] >= L'А' && wsrc[i] <= L'Я')
			{ wres.push_back(wsrc[i]); j++; continue; }
		if(wsrc[i] >= L'0' && wsrc[i] <= L'9')
			{ wres.push_back(wsrc[i]); j++; continue; }
		if(wsrc[i] == L'.' || wsrc[i] == L',' || wsrc[i] == L':' || wsrc[i] == L'!' || wsrc[i] == L'?')
			{ wres.push_back(wsrc[i]); j++; continue; }
		if(wsrc[i] == L'\n')
			{ wres += L". "; j++; continue; }
		wres.push_back(L' ');
	}

	if(j <= 3)
		return std::string();

	res = WideToMultibyteString(wres);
	return res;
}