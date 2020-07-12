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


#include <chrono>
#include "headers/vkcom.h"
#include "../headers/message.h"
#include "../headers/misc.h"


//init
networking_vkcom::networking_vkcom(void* botptr, const std::string& simnam, const std::string& snapname, bool igb, InterfaceSettings preSetter)
{
	INTERFACE_SIMPLE_NAME = simnam;
	thisBot = botptr;
	SnapshotName = snapname;
	LoggedIn = false;
	relogin_procedure = false;
	force_relogin = false;
	fetchCounter = 0;
	onTick = false;
	TX = false;
	debugFetcher = false;
	isOnPromotionStreak = false;
	std::cout << std::endl << "VK: ";
	pNetwork = new network();
	floodprot = new flood_protection();
	if(preSetter.useCallbackApiConnector /*&& preSetter.official*/)
		connector = new CallbackApi(botptr, selfPageId);
	else
		connector = new Fetcher(botptr, pNetwork, this->VKONTAKTE_ACCESS_TOKEN, igb);
	ErrorTick = false;
	enable_idlebreaker = false;
	idle_timer = 0;
	bdater_timer = 128;
	promotion_timer = 0;
	promotion_sent = 0;
	isGroupBot = igb;

	showAuthForm();
}

//destr
networking_vkcom::~networking_vkcom()
{
	std::cout << "Shutting down vk interface" << std::endl;
	auth_logout();
	delete pNetwork;
	delete floodprot;
	delete connector;
}


/*

Основная функция обновления вк

*/

void networking_vkcom::Tick()
{
	std::cout << miscFormatted(miscFormatted("[SPAWN]", 2), 1) 
				<< " VK: Thread started. \n";

	int unloginAttempts = 0;

	while(ServerAlive && *state)
	{
		if(this->LoggedIn)
		{
			int latency = 350;
			latency -= iStatistics->iStorage.msg_avg_time;

			if(latency <= 0)
				latency = 0;

			std::this_thread::sleep_for(std::chrono::milliseconds(latency)); //0 ~ 350 ms sleep
				
				this->iStatistics->iTiming.StartCounting();
			
					this->OnUpdate();
			
				this->iStatistics->iTiming.EndCounting();
			
			if(this->TX)
			{
				this->iStatistics->iStorage.TX_TIMINGS += this->iStatistics->iTiming.getPeriod();
				this->iStatistics->iStorage.TX_TIMINGS /= 2; 
				//statistics->IncrementInRAM(GENERAL_TX_TIMING, INTERFACE_VKCOM, statistics->vkcomGeneralTimings->getPeriod()); 
			}
			else
			{
				this->iStatistics->iStorage.RX_TIMINGS += this->iStatistics->iTiming.getPeriod();
				this->iStatistics->iStorage.RX_TIMINGS /= 2; 
				//statistics->IncrementInRAM(GENERAL_RX_TIMING, INTERFACE_VKCOM, statistics->vkcomGeneralTimings->getPeriod()); 
			}
			
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			unloginAttempts++;

			if(unloginAttempts > 10)
			{
				break;
			}	
		}
	}

	std::cout << miscFormatted(miscFormatted("[KILL]", 3), 1) 
				<< " VK: Update function reached end: Thread terminated. \n";

	thisState = false;
}



/*

Вызывается из основной функции обновления с некоторым интервалом.

*/

void networking_vkcom::OnUpdate()
{
	//std::cout << "Update function called\n";
	if(!isGroupBot)
	{
		checkTimings();
		if(force_relogin)
		{
			auth_relogin();
			force_relogin = false;
		}
	}


	//Alertdevelopers


	AnswerError tmp;

	if(!ErrorStack.empty() && this->interfaceSettings.alertdevelopersEnabled)
	{
		tmp = ErrorStack.top();
	
		if(tmp.HaveError && !this->interfaceAnswer.HaveMessage)
		{
			ErrorStack.pop();
			this->interfaceAnswer.HaveError = false;
			this->interfaceAnswer.HaveMessage = true;
			this->interfaceAnswer.userID = tmp.errorID;

			std::string tmpstr = tmp.errorText;

			for(int i = 0; i < tmpstr.length(); i++)
			{
				if(tmpstr[i] == '.') tmpstr[i] = ' ';
			}

			this->interfaceAnswer.messageText = tmpstr;

			TX = true;
			pushNewMessages();
			floodprot->Tick(true);
			onTick = true;
			idle_timer = 0;
		}
	}


	//AlertDevelopers


	if(!this->interfaceAnswer.HaveMessage && idle_timer > 5000)
	{
		//idlebreaker();
		idle_timer = 0;
	}

	if(this->interfaceAnswer.HaveMessage)
	{
		if(floodprot->floodprotection_status() && floodprot->cooldown_enabled)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(3700));
		}

		pushNewMessages();
		TX = true;
		floodprot->Tick(true);
		onTick = true;
		idle_timer = 0;
	}
	else
	{
		TX = false;

		if(!asyncAnswerQueue.empty())
		{
			asyncAnswerQueueLocker.lock();
			this->interfaceAnswer = asyncAnswerQueue.front();
			asyncAnswerQueue.pop();
			asyncAnswerQueueLocker.unlock();
		}

		if(fetchCounter == 20)
		{
			if(!isGroupBot && this->interfaceSettings.ignoreFriendsReq == 0) autoAcceptFriends();
			fetchCounter = 0;
		}
		else
		{
			bool res = fetchNewMessages();
			
			if(res == false)
			{
				floodprot->Tick(false);
				idle_timer++;
				onTick = false;

				if(bdater_timer >= 192)
				{
					if(!isGroupBot) bdater();
					bdater_timer = 0;
				}
				else
				{
					bdater_timer++;
				}
			}
			else
			{
				ErrorTick = false;
			}

			fetchCounter++;
		}
	}
}
