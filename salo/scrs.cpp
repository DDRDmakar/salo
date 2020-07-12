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
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

#include "headers/misc.h"
#include "headers/scrs.h"
#include "headers/network.h"
#include "headers/message.h"
#include "headers/log.h"

network* pScrsNetwork;
std::mutex SCRS_MUTEX;

class scrsJob
{
public:
	std::thread* threadObject;
	std::mutex threadStatusMutex;
	bool threadRunning;

	//Было бы правильно использовать для синхронизации std::atomic<bool> но при его
	//использовании начинаются проблемы с конструкторами класса
	//возможно баг GCC 6, на версиях 5.x.x такого не было 
};

std::vector<scrsJob*> jobs;


/*

Инициализирует сеть для работы с SCRS

*/

void scrs_connect()
{
	std::cout << "SCRS: ";
	pScrsNetwork = new network();
	std::cout << std::endl;
	jobs.clear();
}


/*

Отправляет сообщение в ВК

*/

inline void asyncSendMessage(const Answer& a, const ResponseData& d)
{
	d.targetInterface->asyncAnswerQueueLocker.lock();
	d.targetInterface->asyncAnswerQueue.push(a);
	d.targetInterface->asyncAnswerQueueLocker.unlock();
}


/*

Помечает поток как завершенный 

*/

inline void asyncMarkAsEnded(scrsJob* status)
{
	status->threadStatusMutex.lock();
	status->threadRunning = false;
	status->threadStatusMutex.unlock();
}


/*

Функция обращающаяся к синтезатору речи, расположенному на сервере SCRS1

*/
 
void synth_worker(std::string task, ResponseData d, scrsJob* status)
{
	SCRS_MUTEX.lock();

	Answer a;
	a.HaveMessage = true;
	a.userID = d.targetPeer;

	task = miscFilterLetters(task);

	//std::cout << task << std::endl;

	if(task.length() > 1800 || task.length() < 5)
	{
		a.messageText = "Сообщение должно быть длиной от 3 до 900 значащих символов, не содержать кавычек и смайликов!";

		asyncSendMessage(a, d);
		asyncMarkAsEnded(status);
		SCRS_MUTEX.unlock();
		return;
	}


	std::string key = miscExecSystem("python3 ../scrsmisc/access_client.py");
	key = key.substr(0,32);
	//std::cout << key << std::endl;
	
	std::string taskd = pScrsNetwork->urlencode(task);
	std::string resturl = "http://212.232.73.210:666/?task="+taskd+"&engine=synth_set&access=" + key;

	NetworkResult res = pScrsNetwork->network_HTTPS_GET(resturl.c_str(), "(none)", 0, 60);

	std::string job_id = miscFindAndCopyString(res.DATA.c_str(), "COMMAND SET OK: task id=\"", "\"");

	if(job_id == std::string())
	{
		AlertDevelopers("SCRS ERROR: synth_set failed!!! \n\nTask was: " + task + "\n\nSCRS says: " + res.DATA);
		
		a.messageText = "Произошла ошибка! Пожалуйста, свяжитесь с разработчиками!";

		asyncSendMessage(a, d);
		asyncMarkAsEnded(status);
		SCRS_MUTEX.unlock();
		return;
	}

	std::string doc;

	for(int i = 0; i < 20; i++)
	{
		key = miscExecSystem("python3 ../scrsmisc/access_client.py");
		key = key.substr(0,32);
		resturl = "http://212.232.73.210:666/?task="+job_id+"&engine=synth_get&access=" + key;
		res = pScrsNetwork->network_HTTPS_GET(resturl.c_str());

		if(res.DATA != "Unresulted.")
		{
			doc = res.DATA;
			break;
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	//std::cout << doc << std::endl;
	logs->CommitGeneric(F, L, doc);
	
	if(doc.substr(0, 5) == "Error" || doc.length() < 5 || doc.substr(0, 3) != "doc")
	{
		
		/*
		//err -------------
		key = miscExecSystem("python3 ../scrsmisc/access_client.py");
		key = key.substr(0,32);
		resturl = "http://212.232.73.210:666/?task="+job_id+"&engine=synth_err&access=" + key;
		NetworkResult res1 = pScrsNetwork->network_HTTPS_GET(resturl.c_str());

		//AlertDevelopers(res.DATA);
		//std::cout << res.DATA << std::endl;

		std::string errtxt;

		if (res1.DATA.length() > 1700)
			errtxt = res1.DATA.substr(res1.DATA.length() - 1700);
		else
			errtxt = res1.DATA;

		//AlertDevelopers(errtxt); 
		//err -------------

		//AlertDevelopers("SCRS ERROR: synth_get failed!!! \n\nTask was: " + task + "\n\nSCRS says: " + res.DATA 
		//	+ "\n\n ------ STDERR AND STDOUT DUMP: ------ \n" + errtxt);
		*/

		a.messageText = "Невозможно синтезировать голос. Убедитесь, что в сообщении содержатся произносимые русские или английские буквы!";

		asyncSendMessage(a, d);
		asyncMarkAsEnded(status);
		SCRS_MUTEX.unlock();
		return;
	}

	a.messageAttachment = doc;

	asyncSendMessage(a, d);
	asyncMarkAsEnded(status);
	SCRS_MUTEX.unlock();
}


/*

Функция обращающаяся к обработчику картинок krapiva, расположенному на сервере SCRS1

*/
 
void krapiva_worker(std::string task, ResponseData d, scrsJob* status)
{
	SCRS_MUTEX.lock();

	Answer a;
	a.HaveMessage = true;
	a.userID = d.targetPeer;


	//получение ключа SCRS
	std::string key = miscExecSystem("python3 ../scrsmisc/access_client.py");
	key = key.substr(0,32);
	//std::cout << key << std::endl;
	
	//формирование дополнительных данных и шифрование
	std::string dt = d.krapivaInputPhotoId + " " + d.krapivaFingerprint;
	std::string multidata = miscExecSystem(std::string("../libcryptm/netcryptm --encrypt \"" + dt + "\"").c_str());


	std::string taskd = pScrsNetwork->urlencode(task);
	std::string resturl = "http://212.232.73.210:666/?task="+taskd+"&engine=krapiva_set&access=" + key + "&multimedia=" + multidata;

	NetworkResult res = pScrsNetwork->network_HTTPS_GET(resturl.c_str(), "(none)", 0, 60);

	std::string job_id = miscFindAndCopyString(res.DATA.c_str(), "COMMAND SET OK: task id=\"", "\"");

	if(job_id == std::string())
	{
		AlertDevelopers("SCRS ERROR: krapiva_set failed!!! \n\nTask was: " + task + "\n\nSCRS says: " + res.DATA);
		
		a.messageText = "Произошла ошибка! Пожалуйста, свяжитесь с разработчиками! (set failed)";

		asyncSendMessage(a, d);
		asyncMarkAsEnded(status);
		SCRS_MUTEX.unlock();
		return; 
	}

	std::string doc;

	for(int i = 0; i < 20; i++)
	{
		key = miscExecSystem("python3 ../scrsmisc/access_client.py");
		key = key.substr(0,32);
		resturl = "http://212.232.73.210:666/?task="+job_id+"&engine=krapiva_get&access=" + key;
		res = pScrsNetwork->network_HTTPS_GET(resturl.c_str());

		if(res.DATA != "Unresulted.")
		{
			doc = res.DATA;
			break;
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	//std::cout << doc << std::endl;
	logs->CommitGeneric(F, L, doc);

	if(doc.substr(0, 5) == "Error" || doc.length() < 5 || (doc.substr(0, 5) != "photo" && doc.substr(0, 3) != "doc"))
	{
		//err -------------
		key = miscExecSystem("python3 ../scrsmisc/access_client.py");
		key = key.substr(0,32);
		resturl = "http://212.232.73.210:666/?task="+job_id+"&engine=krapiva_err&access=" + key;
		NetworkResult res1 = pScrsNetwork->network_HTTPS_GET(resturl.c_str());

		std::string errtxt;

		if (res1.DATA.length() > 1700)
			errtxt = res1.DATA.substr(0, 1700);
		else
			errtxt = res1.DATA;

		//AlertDevelopers(errtxt);
		//err -------------

		AlertDevelopers("SCRS ERROR: krapiva_get failed!!! \n\nTask was: " + task + "\n\nSCRS says: " + res.DATA 
			+ "\n\n ------ STDERR AND STDOUT DUMP: ------ \n" + errtxt);
		
		a.messageText = "Произошла ошибка! Пожалуйста, свяжитесь с разработчиками! (get failed)";

		asyncSendMessage(a, d);
		asyncMarkAsEnded(status);
		SCRS_MUTEX.unlock();
		return;
	}

	a.messageAttachment = doc;

	asyncSendMessage(a, d);
	asyncMarkAsEnded(status);
	SCRS_MUTEX.unlock();
}


/*

Запрашивает изменение шапки сообщества Сала

*/

void infoheader_worker(std::string task, scrsJob* status)
{
	SCRS_MUTEX.lock();

	//получение ключа SCRS	
	std::string key = miscExecSystem("python3 ../scrsmisc/access_client.py");
	key = key.substr(0,32);
	//запрос на выполнение
	std::string taskd = pScrsNetwork->urlencode(task);
	std::string resturl = "http://212.232.73.210:666/?task="+taskd+"&engine=infoheader_set&access=" + key;
	NetworkResult res = pScrsNetwork->network_HTTPS_GET(resturl.c_str(), "(none)", 0, 60);

	std::string job_id = miscFindAndCopyString(res.DATA.c_str(), "COMMAND SET OK: task id=\"", "\"");

	if(job_id == std::string())
	{
		AlertDevelopers("SCRS ERROR: infoheader_set failed!!! \n\nTask was: " + task + "\n\nSCRS says: " + res.DATA);
		
		//Завершение
		asyncMarkAsEnded(status);
		SCRS_MUTEX.unlock();
		return;
	}


	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	//очистка
	key = miscExecSystem("python3 ../scrsmisc/access_client.py");
	key = key.substr(0,32);
	resturl = "http://212.232.73.210:666/?task="+job_id+"&engine=infoheader_get&access=" + key;
	res = pScrsNetwork->network_HTTPS_GET(resturl.c_str());

	//завершение задачи
	asyncMarkAsEnded(status);
	SCRS_MUTEX.unlock();
	return;
}


/*

Основная функция, вызывающаяся из OnCommand по требованию

*/

void scrs(const std::string& engine, const std::string& task, const ResponseData& dat)
{
	//cleanup
	for(int i = 0; i < jobs.size(); i++)
	{
		jobs[i]->threadStatusMutex.lock();

		if(!jobs[i]->threadRunning)
		{
			logs->CommitGeneric(F, L, "Deleting old async SCRS job... ");
			//std::cout << "Deleting old async SCRS job... ";
			jobs[i]->threadObject->join();
			jobs[i]->threadStatusMutex.unlock();
			delete jobs[i]->threadObject;
			delete jobs[i];
			jobs.erase(jobs.begin() + i);
			logs->CommitGeneric(F, L, "Done.");
			//std::cout << "Done." << std::endl;
		}
		else
		{
			jobs[i]->threadStatusMutex.unlock();
		}
	}

	//engines

	//synth engine
	if(engine == "synth")
	{
		scrsJob* j = new scrsJob();

		j->threadRunning = true;
		std::thread* t = new std::thread(synth_worker, task, dat, j);
		j->threadObject = t;

		jobs.push_back(j);
		//synth_worker(task);
	}

	//krapiva engine
	if(engine == "krapiva")
	{
		scrsJob* j = new scrsJob();

		j->threadRunning = true;
		std::thread* t = new std::thread(krapiva_worker, task, dat, j);
		j->threadObject = t;

		jobs.push_back(j);
		//synth_worker(task);
	}

	//infoheader engine
	if(engine == "infoheader")
	{
		scrsJob* j = new scrsJob();

		j->threadRunning = true;
		std::thread* t = new std::thread(infoheader_worker, task, j);
		j->threadObject = t;

		jobs.push_back(j);
	}

	return;
}