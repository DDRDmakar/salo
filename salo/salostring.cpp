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

#include "headers/salostring.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <sstream>


/*

Отладочная функция - выводит порядок битов на экран


void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    }
}

*/


/*

Берет один бит, помещает его в младший разряд

*/

char getBit(char byte, int bitNum)
{
	return ((byte >> bitNum) & 0x01);
}


/*

Ставит один бит

*/

char setBit(char val, int num, bool bitval)
{
    return (val & ~(1<<num)) | (bitval << num);
}


/*

Клиппер

*/

std::string clipper(const std::string& multi)
{
	std::string res;

	std::wstring wide = MultibyteToWideString(multi);
	std::wstring wideacc = std::wstring();

	for(int i=0; i < wide.length(); i++)
	{
		if(wide[i] < 0x000007FF)
		{
			wideacc.push_back(wide[i]);
		}
	}

	res = WideToMultibyteString(wideacc);

	return res;
}


/*

STL Вайдчар -> мультибайт

*/

std::string WideToMultibyteString(const std::wstring& wide)
{
	std::string r;
	
	//итерируемся по каждому широкому символу
	for(int i = 0; i < wide.length(); i++)
	{
		int bytesNum = 0;
		char currentChar1 = 0;
		char currentChar2 = 0;
		char currentChar3 = 0;
		char currentChar4 = 0; //TODO: не допустить "открытого байта"
		

		//значение умещается в 7 бит (ASCII-совместимость)
		if(0x00000000 <= wide[i] && 0x0000007F >= wide[i])
		{
			bytesNum = 1;
			currentChar4 = wide[i];
			//continue;
		}

		//2-ух байтовая последовательность
		if(0x00000080 <= wide[i] && 0x000007FF >= wide[i])
		{
			bytesNum = 2;
			
			currentChar3 = 0xC0; //паттерн 110XXXXX первого байта
			currentChar4 = 0x80; //паттерн 10XXXXXX второго байта

			//первый байт
			unsigned wchar_t byte3 = wide[i] >> 6;
			byte3 = byte3 & 0x1F; 
			currentChar3 = currentChar3 | byte3;

			//второй байт
			unsigned wchar_t byte4 = wide[i] & 0x3F;
			currentChar4 = currentChar4 | byte4;

		}
		
		//3-х байтовая последовательность
		if(0x00000800 <= wide[i] && 0x0000FFFF >= wide[i])
		{
			bytesNum = 3;

			currentChar2 = 0xE0; //паттерн 1110XXXX первого байта 
			currentChar3 = 0x80; //паттерн 10XXXXXX второго байта
			currentChar4 = 0x80; //паттерн 10XXXXXX третьего байта
			
			//первый байт
			unsigned wchar_t byte2 = wide[i] >> 12;
			byte2 = byte2 & 0xF;
			currentChar2 = currentChar2 | byte2;

			//второй байт
			unsigned wchar_t byte3 = wide[i] >> 6;
			byte3 = byte3 & 0x3F; 
			currentChar3 = currentChar3 | byte3;

			//третий байт
			unsigned wchar_t byte4 = wide[i] & 0x3F;
			currentChar4 = currentChar4 | byte4;
		}


		//4-х байтовая последовательность
		if(0x00010000 <= wide[i] && 0x001FFFFF >= wide[i])
		{
			bytesNum = 4;

			currentChar1 = 0xF0; //паттерн 11110XXX первого байта 
			currentChar2 = 0x80; //паттерн 10XXXXXX второго байта
			currentChar3 = 0x80; //паттерн 10XXXXXX третьего байта
			currentChar4 = 0x80; //паттерн 10XXXXXX четвертого байта

			//первый байт
			unsigned wchar_t byte1 = wide[i] >> 18;
			byte1 = byte1 & 0x7;
			currentChar1 = currentChar1 | byte1;

			//второй байт
			unsigned wchar_t byte2 = wide[i] >> 12;
			byte2 = byte2 & 0x3F;
			currentChar2 = currentChar2 | byte2;

			//третий байт
			unsigned wchar_t byte3 = wide[i] >> 6;
			byte3 = byte3 & 0x3F; 
			currentChar3 = currentChar3 | byte3;

			//четвертый байт
			unsigned wchar_t byte4 = wide[i] & 0x3F;
			currentChar4 = currentChar4 | byte4;

		}

		/*
		std::cout << i << ") -- [";
		printBits(sizeof(wide[i]), &wide[i]);
		std::cout << "] -- [";
		printBits(sizeof(currentChar1), &currentChar1);
		std::cout << "] [";
		printBits(sizeof(currentChar2), &currentChar2);
		std::cout << "] [";
		printBits(sizeof(currentChar3), &currentChar3);
		std::cout << "] [";
		printBits(sizeof(currentChar4), &currentChar4);
		std::cout << "] -- ";
		std::wcout << wide[i] << std::endl;
		*/

		if(bytesNum == 4 && currentChar1 != 0) r.push_back(currentChar1);
		if(bytesNum >= 3 && currentChar2 != 0) r.push_back(currentChar2);
		if(bytesNum >= 2 && currentChar3 != 0) r.push_back(currentChar3);
		if(bytesNum >= 1) r.push_back(currentChar4);
		
	}

	return r;
}


/*

Вайдчар -> мультибайт
Совместимая версия.

*/

char* WideToMultibyte(const wchar_t* wide)
{
	std::string rr = WideToMultibyteString(std::wstring(wide));

	char* res = new char[rr.length() + 1];
	strncpy(res, rr.c_str(), rr.length());
	res[rr.length()] = L'\0';

	//std::cout << "[WC -> MB] (" << rr << ") \n";

	return res;	
}


/*{
	unsigned int size = wcslen(wide) * 4 + 1;
	char* multi = new char[size]; //Выделение для наихудшего случая
	
	unsigned int real_size = std::wcstombs(multi, wide, size); // strlen(multi);
	char* multibyte = new char[real_size + 1];
	strncpy(multibyte, multi, real_size);
	
	multibyte[real_size] = 0;

	delete[] multi;
	return multibyte;
}
*/


/*

STL Мультибайт -> вайдчар
TODO: Error correction

*/

std::wstring MultibyteToWideString(const std::string& multi)
{
	std::wstring r;
	int slaveBytes = 0;
	wchar_t currentChar = 0;

	//проходимся по каждому символу мультибайта
	for(unsigned int i=0; i < multi.length(); i++)
	{
		//однобайтные ASCII-последовательности. Не нуждаются в сборке.
		if(getBit(multi[i], 7) == (char)0) 
		{
			//Перед обработкой нового ASCII-байта
			//нужно проверить, и сохранить предыдущий сивол 
			//если это необходимо
			if(currentChar != 0)
			{
				//сохранение рабочего символа
				r.push_back(currentChar);
				//зануление рабочего символа 
				currentChar = 0;
			}
			
			currentChar = multi[i];

			//сохранение рабочего символа
			r.push_back(currentChar);
			//зануление рабочего символа 
			currentChar = 0;
			continue; 
		} 
		
		//мультибайтовые UTF-8 последовательности
		//Лидирующие байты последовательности обозначил как MASTER
		//Ведомые байты последовательности как SLAVE
		if(slaveBytes > 0)
		{
			if(getBit(multi[i], 7) == (char)1 && getBit(multi[i], 6) == (char)0)
			{
				//байт часть уже начатой UTF-8 последовательности
				//сдвигаем рабочий символ влево и дописываем младшие 6 бит из SLAVE-байта
				currentChar = currentChar << 6;
				currentChar = currentChar | (multi[i] & 0x3F);

				slaveBytes--;
				continue;
			}
		}
		else
		{
			if(slaveBytes == 0)
			{
				//байт - ведущий в UTF-8 последовательности
				//необходимо вычислить количество ведомых байтов
				//мультибайтовой последовательности
				//а также скопировать первые биты составного символа в буфер

				//Перед обработкой нового MASTER-байта
				//нужно проверить, и сохранить предыдущий символ 
				//если это необходимо
				if(currentChar != 0)
				{
					//сохранение рабочего символа
					r.push_back(currentChar);
					//зануление рабочего символа 
					currentChar = 0;
				}

				int sequence = 0;
				int dropoff_bit = 0;
				int j = 7;

				//вычисляем колечество SLAVE-байтов
				while(sequence < 5 && dropoff_bit == 0 && j >= 0)
				{
					char bit = getBit(multi[i], j);
					if(bit == (char)1) 
						sequence++; 
					else
						dropoff_bit++;
					j--;
				}

				slaveBytes = sequence-1; //SLAVE-байтов на 1 меньше чем всего байт в последовательности
				int specialBits = sequence+1; //служебных бит на 1 больше, по биту на каждый ведомый байт и конечный 0
				unsigned char masterContent = 0; //копия текущего байта мультибайта

				//Удаляем служебные биты
				masterContent = multi[i] << specialBits;
				masterContent = masterContent >> specialBits;
		
				currentChar = masterContent;
				continue;
			}
		}
	}

	//дописываем остаток в строку
	if(currentChar != 0) r.push_back(currentChar);
		
	return r;
}


/*

Мультибайт -> вайдчар
Совместимая версия

*/

wchar_t* MultibyteToWide(const char* multi)
{
	std::wstring rr = MultibyteToWideString(std::string(multi));

	wchar_t* res = new wchar_t[rr.length() + 1];
	wcsncpy(res, rr.c_str(), rr.length());
	res[rr.length()] = L'\0';

	return res;
}


/*{
	unsigned int size = strlen(multi);
	wchar_t* wide = new wchar_t[size + 4];

	unsigned int real_size = std::mbstowcs(wide, multi, size); //wcslen(wide);
	wchar_t* widestring = new wchar_t[real_size + 1];
	wcsncpy(widestring, wide, real_size);
	
	widestring[real_size] = L'\0';

	delete[] wide;
	return widestring;
}
*/



/*

Переводит шестнадцатеричный код символа в строке в десятичный в инте

*/

unsigned int hexstr_todecint(const std::string& var)
{
	std::stringstream str;
	str << var;
	int value;
	str >> std::hex >> value;
	return value;
}


/*

Раскодирование последовотельностей UTF-16 в готовые 4-байтные символы и запись их в мультибайт

*/

std::string getUnicodeFromCodes(const std::string& str)
{
	std::string res;

	for(int i = 0; i < str.length(); i++)
	{
		if(str[i] == '\\' && str[i + 1] == 'u')
		{
			int surrogate = 0;

			std::string code;
			code.push_back(str[i+2]);
			code.push_back(str[i+3]);
			code.push_back(str[i+4]);
			code.push_back(str[i+5]);

			wchar_t symbol;

			unsigned int LEADING_UTF16_WORD = hexstr_todecint(code);

			if(LEADING_UTF16_WORD < 0xD800 || LEADING_UTF16_WORD > 0xDFFF) 
			{
				symbol = LEADING_UTF16_WORD;
			}
			else 
			{
				if(LEADING_UTF16_WORD >= 0xDC00)
				{
					i += 11;
					continue;
				}

				surrogate = 1;

				std::string sec_byte;

				sec_byte.push_back(str[i+8]);
				sec_byte.push_back(str[i+9]);
				sec_byte.push_back(str[i+10]);
				sec_byte.push_back(str[i+11]);

				unsigned int TRAILING_UTF16_WORD = /*71680 +*/ hexstr_todecint(sec_byte);

				if(TRAILING_UTF16_WORD < 0xDC00 || TRAILING_UTF16_WORD > 0xDFFF)
				{
					i += 11;
					continue;
				}

				unsigned int unicode = LEADING_UTF16_WORD & 0x3FF;
				unicode = unicode << 10;
				unsigned int second = TRAILING_UTF16_WORD & 0x3FF;
				unicode = unicode | second;
				unicode += 0x10000;
				symbol = unicode;
			}

			wchar_t* symbolstr = new wchar_t[2];
			symbolstr[0] = symbol;
			symbolstr[1] = L'\0';

			char* multibyte = WideToMultibyte(symbolstr);
			std::string multistr = std::string(multibyte);

			res += multibyte;

			delete[] multibyte;
			delete[] symbolstr;

			if(surrogate)
			{
				i += 11;
			}
			else
			{
				i += 5;
			}
		}
		else
		{
			res.push_back(str[i]);			
		}
	}

	return res;
}