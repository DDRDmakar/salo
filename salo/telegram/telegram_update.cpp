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
#include "headers/telegram.h"
#include "../headers/message.h"
#include "../headers/misc.h"

//constr
networking_telegram::networking_telegram(void* botptr, const std::string& simnam, const std::string& snapname)
{
	INTERFACE_SIMPLE_NAME = simnam;
	thisBot = botptr;
	SnapshotName = snapname;
	offset = 0;
	std::cout << std::endl << "TELEGRAM: ";
	ptNetwork = new network();
	TX = false;
	showAuthForm();
}

//destr
networking_telegram::~networking_telegram()
{
	std::cout << "Telegram interface is shutting down now" << std::endl;
	delete ptNetwork;
}



void networking_telegram::Tick()
{
	std::cout << miscFormatted(miscFormatted("[SPAWN]", 2), 1) 
				<< " TELEGRAM: Thread started. \n";

	while(ServerAlive && this->_ENABLED && *state)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(750)); 

		this->iStatistics->iTiming.StartCounting();
			
			this->OnUpdate();

		this->iStatistics->iTiming.EndCounting();

		if(this->TX)
			{
				this->iStatistics->iStorage.TX_TIMINGS += this->iStatistics->iTiming.getPeriod();
				this->iStatistics->iStorage.TX_TIMINGS /= 2; 
				//statistics->IncrementInRAM(GENERAL_TX_TIMING, INTERFACE_TELEGRAM, statistics->telegramGeneralTimings->getPeriod()); 
			}
			else
			{
				this->iStatistics->iStorage.RX_TIMINGS += this->iStatistics->iTiming.getPeriod();
				this->iStatistics->iStorage.RX_TIMINGS /= 2; 
				//statistics->IncrementInRAM(GENERAL_RX_TIMING, INTERFACE_TELEGRAM, statistics->telegramGeneralTimings->getPeriod()); 
			}
	}

	std::cout << miscFormatted(miscFormatted("[KILL]", 3), 1) 
				<< " TELEGRAM: Update function reached end: Thread terminated. \n";

	thisState = false;
}



/*

Главная функция обновления Telegram

*/

void networking_telegram::OnUpdate()
{
	//std::cout << "Telegram Update" << std::endl;
	AnswerError tmp;

	if(!ErrorStack.empty() && this->interfaceSettings.alertdevelopersEnabled)
	{
		tmp = ErrorStack.top();
	}


	if(tmp.HaveError && !this->interfaceAnswerTG.HaveMessage)
	{
		ErrorStack.pop();
		this->interfaceAnswerTG.HaveError = false;
		this->interfaceAnswerTG.HaveMessage = true;
		this->interfaceAnswerTG.attachtype = 0;
		this->interfaceAnswerTG.chatID = tmp.errorID;
		this->interfaceAnswerTG.messageText = tmp.errorText;

		TX = true;
		pushMessage();
	}
	else
	{
		TX = false;
		fetchMessage();
	}

	//if(this->interfaceAnswerTG.HaveMessage)
	//{
	//	pushMessage();
	//}
	//else
	//{
	//
	//}

	if(firstFetch)
	{
		AlertDevelopers("Telegram fetcher enabled.");
		std::cout << "Telegram fetcher enabled." << std::endl;
		firstFetch = false;
	}

}