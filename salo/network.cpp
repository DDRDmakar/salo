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
#include <mutex>
#include "headers/network.h"
#include "headers/log.h"
#include "headers/misc.h"

//std::mutex NETWORK_MUTEX;

//init
network::network(bool silent)
{ 
	curl_global_init(CURL_GLOBAL_DEFAULT);  
	DebugMode = 0;
	OfflineMode = 0;
	silentStart = silent;
	if(!silentStart) std::cout << "Networking initialized..." << std::endl;
}

//destr
network::~network()
{
	if(!silentStart) std::cout << "Networking shutting down..." << std::endl;
	curl_global_cleanup();
}


/*

Функция инициализирует новый запрос

*/

void network::easyinit()
{
	curl = curl_easy_init();

	if(!curl) 
	{
		std::cout << "Err: CURL Lib failed to init" << std::endl; 
		ServerAlive = 0;
	}
}


/*

Возвращает строку из Cookie, полученых при последнем запросе на передваймом хендлере

*/

std::string cookies(CURL *curl)
{
	CURLcode res;
	struct curl_slist *cookies;
	struct curl_slist *nc;
	int i;
	 
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if(res != CURLE_OK) 
	{
		fprintf(stderr, "Curl: curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
		logs->CommitGeneric(F, L, "Curl: curl_easy_getinfo failed: " + std::string(curl_easy_strerror(res)));
		exit(1);
	}

	nc = cookies, i = 1;
  
	std::string rs;

	while(nc) 
	{
		//printf("[%d]: %s\n", i, nc->data);
		rs += nc->data;
		nc = nc->next;
		i++;
	}
  
	if (i == 1) 
	{
		rs = "(none)";
	}

	curl_slist_free_all(cookies);
	return rs;
}
 

/*

Функция для записи curl-ом в память, используется для данных и заголовков 

*/

size_t WriteToMemory(char* data, size_t size, size_t nmemb, std::string* buffer)
{
   int ReadBytes = 0;
	if (buffer != NULL)
	{
		buffer->append(data, size * nmemb);
		ReadBytes = size * nmemb;
	}

	return ReadBytes;
}



/*

	GET запрос по протоколу HTTPS, через libcurl, результат пишется в определенную в хедере спец. структуру
	Проверки SSL пропускаются, параметры get передаются с адресом  
	Последний аргумент определяет, будет ли крашится сервер при ошибке

*/

NetworkResult network::network_HTTPS_GET(const char* address, const std::string& cookiekeys, int critical, int timeout)
{
	//NETWORK_MUTEX.lock();
	easyinit();

	NetworkResult res;
	std::string memchunk;
	std::string hdrchunk;
	CURLcode curlres;

	if(DebugMode == 1)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}
 
	curl_easy_setopt(curl, CURLOPT_URL, address);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteToMemory);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToMemory); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&memchunk);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)&hdrchunk); 
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

	if(cookiekeys != "(none)")
	{
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookiekeys.c_str());
	}


	int success = 0;
	int attempt = 0;
	std::string neterr;

	while(!success)
	{
		if(attempt >= 30)
		{
			res.url = std::string(address);
			res.HEAD = "Networking failed!";
			res.hsize = 0;
			res.DATA = "Networking failed! Please check out generic.log for more information!";
			res.size = 0;    
			res.cookie = "Networking failed!";

			std::cout << "\n\n\n!!! ------------------------- NETWORKING CRITICAL ERROR ------------------------- !!!" << std::endl;
			std::cout << "Reason: " << neterr << std::endl;
			std::cout << "Failed to access URL: " << std::string(address) << std::endl; 
			std::cout << "Please check out logs on mark: " << miscGetCurrentDateTime() << std::endl;
			std::cout << "!!! ------------------------- NETWORKING CRITICAL ERROR ------------------------- !!!\n\n\n" << std::endl;

			logs->CommitGeneric(F, L, "\n\n\n!!! ------------------------- NETWORKING CRITICAL ERROR ------------------------- !!!");
			logs->CommitGeneric(F, L, "\n\nReason: " + neterr + "\n\n");
			logs->CommitGeneric(F, L, "\n\nFailed to access URL: " + std::string(address) + "\n\n");
			logs->CommitGeneric(F, L, "\n\nFailed cookiekeys: " + std::string(cookiekeys)+ "\n\n");
			logs->CommitGeneric(F, L, "\n\n\n!!! ------------------------- NETWORKING CRITICAL ERROR ------------------------- !!!\n\n\n");

			//NETWORK_MUTEX.unlock();
			return res;
		}

		if(OfflineMode == 1)
		{
			std::cout << "Curl failed, retrying in 2 sec\n" << "Retrying..." << std::endl;
			logs->CommitGeneric(F, L, "Curl failed, retrying in 2 sec");
			//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		}

		curlres = curl_easy_perform(curl);
		attempt++;

		if(curlres != CURLE_OK) 
		{
			OfflineMode = 1;
			fprintf(stderr, "Curl: %s\n", curl_easy_strerror(curlres));
			logs->CommitGeneric(F, L, "Curl: " + std::string(curl_easy_strerror(curlres)));
			neterr = std::string(curl_easy_strerror(curlres));

			if(critical)
			{
				std::cout << "Server shutting down due to critical network error" << std::endl;
				logs->CommitGeneric(F, L, "failed on " + std::string(address) + " " + cookiekeys);
				logs->CommitGeneric(F, L, "Server shutting down due to critical network error");
				exit(1);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(300)); 
		}
		else
		{
			OfflineMode = 0;
			success = 1;
		}
	}
	
	res.url = std::string(address);
	res.HEAD = hdrchunk;
	res.hsize = hdrchunk.size();
	res.DATA = memchunk;
	res.size = memchunk.size();    
	res.cookie = cookies(curl);

	curl_easy_cleanup(curl);

	if(DebugMode == 1)
	{
		std::cout << "HTTPS GET: recieved " << res.size + res.hsize << " bytes from server!" << std::endl;
	}

	//NETWORK_MUTEX.unlock();
	return res;
}


/*

	POST запрос по протоколу HTTPS, через libcurl, результат пишется в определенную в хедере спец. структуру
	Проверки SSL пропускаются, параметры POST передаются вторым аргументом  
	Последний аргумент определяет, будет ли крашится сервер при ошибке
	Допускает до 3 HTTP редиректов подряд.

*/

NetworkResult network::network_HTTPS_POST(const char* address, const char* postkeys, const std::string& cookiekeys, int critical)
{
	//NETWORK_MUTEX.lock();
	easyinit();

	NetworkResult res;
	std::string memchunk;
	std::string hdrchunk;
	CURLcode curlres;

	if(DebugMode == 1)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}

	curl_easy_setopt(curl, CURLOPT_URL, address);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postkeys);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteToMemory);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToMemory); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&memchunk);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)&hdrchunk); 
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3);
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

	if(cookiekeys != "(none)")
	{
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookiekeys.c_str());
	}

	int success = 0;
	int attempt = 0;
	std::string neterr;

	while(!success)
	{
		
		if(attempt >= 30)
		{
			res.url = std::string(address);
			res.HEAD = "Networking failed!";
			res.hsize = 0;
			res.DATA = "Networking failed! Please check out generic.log for more information!";
			res.size = 0;    
			res.cookie = "Networking failed!";

			std::cout << "\n\n\n!!! ------------------------- NETWORKING CRITICAL ERROR ------------------------- !!!" << std::endl;
			std::cout << "Reason: " << neterr << std::endl;
			std::cout << "Failed to access URL: " << std::string(address) << std::endl; 
			std::cout << "Please check out logs on mark: " << miscGetCurrentDateTime() << std::endl;
			std::cout << "!!! ------------------------- NETWORKING CRITICAL ERROR ------------------------- !!!\n\n\n" << std::endl;

			logs->CommitGeneric(F, L, "\n\n\n!!! ------------------------- NETWORKING CRITICAL ERROR ------------------------- !!!");
			logs->CommitGeneric(F, L, "\n\nReason: " + neterr + "\n\n");
			logs->CommitGeneric(F, L, "\n\nFailed to access URL: " + std::string(address) + "\n\n");
			logs->CommitGeneric(F, L, "\n\nFailed postkeys: " + std::string(postkeys) + "\n\n");
			logs->CommitGeneric(F, L, "\n\nFailed cookiekeys: " + std::string(cookiekeys)+ "\n\n");
			logs->CommitGeneric(F, L, "\n\n\n!!! ------------------------- NETWORKING CRITICAL ERROR ------------------------- !!!\n\n\n");

			//NETWORK_MUTEX.unlock();
			return res;
		}


		if(OfflineMode == 1)
		{
			std::cout << "Curl failed, retrying in 2 sec\n" << "Retrying..." << std::endl;
			logs->CommitGeneric(F, L, "Curl POST failed, retrying in 2 sec");
			//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		}

		curlres = curl_easy_perform(curl);
		attempt++;
	
		if(curlres != CURLE_OK) 
		{
			OfflineMode = 1;
			fprintf(stderr, "Curl: %s\n", curl_easy_strerror(curlres));
			logs->CommitGeneric(F, L, "Curl: " + std::string(curl_easy_strerror(curlres)));
			neterr = std::string(curl_easy_strerror(curlres));

			if(critical)
			{
				std::cout << "Server shutting down due to critical network error" << std::endl;
				logs->CommitGeneric(F, L, "failed on " + std::string(address) + " " + cookiekeys);
				logs->CommitGeneric(F, L, "Server shutting down due to critical network error");
				exit(1);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(300)); //2s sleep
		}
		else
		{
			OfflineMode = 0;
			success = 1;
		}
	}
	
	
	res.url = std::string(address);
	res.HEAD = hdrchunk;
	res.hsize = hdrchunk.size();
	res.DATA = memchunk;
	res.size = memchunk.size();
	res.cookie = cookies(curl);

	curl_easy_cleanup(curl);
	
	if(DebugMode == 1)
	{
		std::cout << "HTTPS POST: recieved " << res.size + res.hsize  << " bytes from server!" << std::endl;
	}

	//NETWORK_MUTEX.unlock();
	return res;
}


std::string network::urlencode(std::string str)
{
	char* result = curl_easy_escape(curl, str.c_str(), 0);
	std::string res(result);
	curl_free(result);
	return res;
}