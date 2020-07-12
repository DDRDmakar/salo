/*
*
*  Copyright ©, 2015-2020. All Rights Reserved.
*
*    Autors:
*    Motylenok Mikhail
*    Makarevich Nikita
*
*    This code is privately owned and is a commercial secret. We do not provide
*    code to anyone without the written agreement. Copying, publication, use
*    for commercial or non-commercial purposes without the consent
*    of the authors is a violation of applicable law.
*
*/

#include <iostream>
#include <stack>
#include <string.h>
#include <stdlib.h>
#include <cmath>
#include <string>

#include "headers/mathematics.h"
#include "headers/log.h"


/*

Возвращает приоритет операций.

*/

int priority(char c)
{
	switch(c)
	{
		case '(': return 0; break;
		case ')': return 0; break;
		case '+': return 1; break;
		case '-': return 1; break;
		case '*': return 2; break;
		case '/': return 2; break;
		case '^': return 2; break;
	}

	return 1;
}

/*

Является цифрой 

*/
bool isDigit(char c)
{
	if(c >= '0' && c <= '9')
	{
		return true;
	}
	
	return false;
}


/*

Проверка длины 

*/

bool isValidLength(const std::string& number)
{

	if(number.length() > 14)
	{
		return false;
	}	
	else
	{
		return true;
	}

}


/*

Вычисляет результат, из обратной польской записи

*/

double calc_it(const char* opz, int N)
{
	std::stack<double> numbers;
	char* buffer = new char[N];
	int k = 0;
	double result = 0;

	for(int i = 0; i < strlen(opz); i++)
	{
		if((opz[i] > 47 && opz[i] < 58) || (opz[i] == '.' || opz[i] == ',')) //Если это цифра
		{
			buffer[k] = opz[i];
			k++;
		}
		else //Если это не цифра
		{
			if(k > 0) //Запушить в стек число из буфера
				{
					buffer[k] = 0; 
					k = 0; 
					if(!isValidLength(std::string(buffer))) { buffer[13] = 0; } //throw 2;
					double dd = atof(buffer);
					numbers.push(dd);
				}
			
			//Если это знак операции, то достать из стека последние числа и посчитать их
			if(opz[i] == '+' || opz[i] == '-' || opz[i] == '*' || opz[i] == '/' || opz[i] == '^') 
			{
				if(numbers.size() < 2)
				{
					throw(1);
				}

				double a = numbers.top(); numbers.pop();
				double b = numbers.top(); numbers.pop();

				switch(opz[i])
				{
					case '+': numbers.push(a + b); break;
					case '-': numbers.push(b - a); break;
					case '*': numbers.push(a * b); break;
					case '/': numbers.push(b / a); break;
					case '^': numbers.push(pow(b, a)); break;
				} 
			}
		}
	}

	if(numbers.empty()) { throw(1); }

	result = numbers.top(); //В конце результат на вершине стека
	delete[] buffer;
	return result;
}


/*

Преобразование из обычной записи в обратную польскую

*/

char* preproc(const char* data, int N)
{
	std::stack<char> operators;
	char* result = new char[N * 3];
	int size = strlen(data);
	int j = 0;

	for(int i = 0; i < size; i++)
		{
			//Цифры и один пробел подряд сразу идут в результирующую строку
			if((data[i] > 47 && data[i] < 58) || (data[i] == '.' || data[i] == ',') || (data[i] == ' ' && result[j - 1] != ' ')) 
			{
				result[j] = data[i];
				j++;
			}
			else 
			{
				//Если встретился знак операции
				if((data[i] > 39 && data[i] < 44) || data[i] == 45 || data[i] == 47 || data[i] == 94)
				{
					result[j] = ' '; j++;

				    if(data[i] != ')')
					{
					if(!operators.empty() && data[i] != '(')
						{
						//Знаки в стеке, с большим приоритетом чем текущий, записать в результат
						while(priority(operators.top()) >= priority(data[i])) 
							{
							result[j] = ' ';
							result[j + 1] = operators.top();
							j += 2;
							operators.pop();
							if(operators.empty()) { break; }
							}
						}
						
					operators.push(data[i]); //текущий знак поместить в стек
					}
					else //По закрывающей скобке в результат вернуть все знаки до открывающей
					{
						if(operators.empty()) { throw 1; }
						char CH = operators.top();
						while(CH != '(')
							{
							result[j] = ' ';
							result[j + 1] = CH;
							j += 2;
							operators.pop();
							if(operators.empty()) { throw 1; }
							CH = operators.top();
							}
						operators.pop();
					}
				}
			}
		}
	
	while(!operators.empty()) //Все знаки оставшиеся в стеке дописать в конец
		{
			result[j] = ' ';
			result[j + 1] = operators.top();
			j += 2;
			operators.pop();
		}

	result[j] = 0; //Конец самодельной строки
	return result;
}


/*

Фильтрует строку от лишних символов

*/

char* filter(const char* input)
{
	char* output = new char[strlen(input) * 3];
	int j = 0;

	int ln = 0;

	for(int i = 0; i < strlen(input); i++)
	{
		if((input[i] > 47 && input[i] < 58) || (input[i] == '.') || (input[i] == ','))
		{
			ln++;
			if(ln > 12) { continue; }

			if(i > 0 && i < (strlen(input)-1))
			{
				if(input[i] == '.' || input[i] == ',')
				{
					if(isDigit(input[i-1]) && isDigit(input[i+1]))
					{
						output[j] = '.'; //locale settings sensible
						j++;
					}
				}
			}

			if(input[i] == '.' || input[i] == ',')
			{
				//output[j] = ',';
				//j++;
			}
			else
			{
				output[j] = input[i];
				j++;
			}
		}

		if((input[i] > 39 && input[i] < 44) || input[i] == 45 || input[i] == 47 || input[i] == 94)
		{
			output[j] = input[i];
			j++;
		}

		if(!isDigit(input[i])) 
		{
			ln = 0;
		}

	}

	output[j] = 0;
	return output;
}


/*

Решает пример по строке. Возвращает строку

*/

std::string getMathematicsResult(const std::string& expression)
{

	logs->CommitGeneric(F, L, "getMathematicsResult called, expression = " + expression);

	std::string res;
	char* filtered_str;
	char* ready_str;

	try
	{
		int N = expression.length();
		filtered_str = filter(expression.c_str());
		ready_str = preproc(filtered_str, N); 
		res = std::string(filtered_str) + " = ";
		
		double r = calc_it(ready_str, N);

		if (r == (int)r)
		{
			res += std::to_string((int)r);
		} 
		else
		{
			res += std::to_string(r);
		}
		
		if(filtered_str != NULL) delete[] filtered_str;
		if(filtered_str != NULL) delete[] ready_str;
	}
	catch(int e)
	{
		
		logs->CommitGeneric(F, L, "EXEPTION IN CALC - " + std::to_string(e));
			//if(filtered_str != NULL) delete[] filtered_str;
			//if(filtered_str != NULL) delete[] ready_str;
		return std::string("Хреновый пример :(");
	}


	return res;

}
