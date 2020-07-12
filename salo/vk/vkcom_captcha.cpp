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

#include <time.h>
#include <chrono>
#include "headers/vkcom.h"
#include "headers/vkcom_captcha.h"
#include "../headers/bots.h"
#include "../headers/misc.h"
#include "../headers/message.h"
#include "../headers/network.h"

/*

Фильтрует адрес от экранирования вк

*/

std::string filterURL(const char* url)
{
	std::string res;

	for(int i = 0; i < strlen(url); i++)
	{
		if(url[i] != '\\')
		{
			res.push_back(url[i]);
		}
	}

	return res;
}


/*

Вызывается при выкидывании ошибки капчи

*/

bool networking_vkcom::captcha(const std::string& mdata)
{
	char* url = miscFindAndCopy(mdata.c_str(), "\"captcha_img\":\"", "\"");
	char* sid = miscFindAndCopy(mdata.c_str(), "\"captcha_sid\":\"", "\"");

	if(url == NULL || sid == NULL)
	{
		AlertDevelopers("Captcha error. Also error while trying to handle captcha url or sid. Please check the situation.");
		return false;
	}

	std::string urls = filterURL(url);
	std::string sids = std::string(sid);

	delete[] url;
	delete[] sid;

	//создаем рандомный путь для кэша антикапчи
	srand(time(NULL));
	int random = rand();
	std::string captcha_path = "captcha_cache/temp_"+std::to_string(random)+".jpg";
	//std::cout << "Captcha cache: " << captcha_path << std::endl;

	//скачивается текущая капча
	std::string download = "curl \"" + urls + "\" --create-dirs -s -o \""+captcha_path+"\"";
	std::string curlOutput = miscExecSystem(download.c_str());
	std::string b64_cpt = miscExecSystem(std::string(std::string("base64 -w 0 ") + captcha_path).c_str());
	system(std::string("rm -f " + captcha_path).c_str());

	if(b64_cpt.length() < 1)
	{
		AlertDevelopers("Captcha error. Captcha file downloading or validation failed (on Salo server side). Please check the situation.");
		return false;
	}

	//отсылается на антикапчу
	network* cNetwork = new network(true);

	//берем токен
	std::string TOKEN;	
	if(this->interfaceSettings.anticaptchaApiKey.length())
	{
		TOKEN = this->interfaceSettings.anticaptchaApiKey;
	}
	else
	{
		TOKEN = miscGetStringFromFile("resources/anticaptcha_token.conf");
	}
	
	std::string acurl = "http://anti-captcha.com/in.php";
	std::string post = "method=base64&key=" + TOKEN + "&body=" + cNetwork->urlencode(b64_cpt);

	NetworkResult rs = cNetwork->network_HTTPS_POST(acurl.c_str(), post.c_str(), "(none)", 0);

	char* errstr = miscFindAndCopy(rs.DATA.c_str(), "ERR", "_");
	if(errstr != NULL)
	{
		delete[] errstr;
		delete cNetwork; 
		AlertDevelopers("Captcha. \nANTI-CAPTCHA UPLOAD FAILED: " + rs.DATA + "\nTriggered by: " + INTERFACE_SIMPLE_NAME); 
		return false;
	}

	//std::cout << rs.DATA << std::endl;

	std::string ID = std::string(rs.DATA.c_str() + 3);
	//std::cout << ID << std::endl;

	//берем капчу назад
	bool success = false;
	int i = 0;
	std::string ready_solving;

	while(!success && i < 20)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));

		std::string geturl = "http://anti-captcha.com/res.php?key=" + TOKEN + "&id=" + ID + "&action=get";
		NetworkResult gt = cNetwork->network_HTTPS_GET(geturl.c_str());

		//std::cout << gt.DATA << std::endl;

		if(gt.DATA == "CAPCHA_NOT_READY")
		{
			i++;
			//уходим на следующую попытку
		}
		else
		{
			char* errstr1 = miscFindAndCopy(gt.DATA.c_str(), "ERR", "_");
			if(errstr != NULL)
			{
				delete[] errstr1;
				delete cNetwork; 
				AlertDevelopers("Captcha. \nANTI-CAPTCHA RESOLVER FAILED: " + gt.DATA + "\nTriggered by: " + INTERFACE_SIMPLE_NAME); 
				return false;
			}

			//если это не ошибка и не NOT READY значит это правильная капча
			success = true;
			ready_solving = std::string(gt.DATA.c_str() + 3);
		}
		
	}

	//если цикл промотал все 10 раз безуспешно
	if(!success)
	{
		delete cNetwork; 
		AlertDevelopers("Captcha. \nANTI-CAPTCHA TIMED OUT (60 sec)!\nTriggered by: " + INTERFACE_SIMPLE_NAME); 
		return false;
	}


	//Применяем
	//TryCaptcha(sids, ready_solving, pNetwork, true);
	captcha_answer = ready_solving;
	captcha_sid = sids;

	//ANTI-CAPTHA

	//AlertDevelopers("Anti-Captcha Done.\nMessage processing forced continue.");
	
	bot* Bot = static_cast<bot*>(this->thisBot);
	Bot->botsCaptchaCounter++;

	delete cNetwork;	
	return true;
}