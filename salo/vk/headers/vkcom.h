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


#ifndef VKCOM 
#define VKCOM


#include <vector>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include "../../headers/message.h"
#include "../../headers/network.h"
#include "../../headers/log.h"
#include "vkcom_fetcher.h"
#include "vkcom_callbackapi.h"
#include "../../headers/interface.h"


std::string GetError(const std::string& unexpected);

struct vkLoginForm
{
	std::string _origin;
	std::string ip_h;
	std::string lg_h;
	std::string to;

	std::string email;
	std::string pass;

	std::string cookie_remixlhk;
};


class flood_protection
{
private:
	bool* load;

	int current_streak;
	int flood_lock;

	int times_enabled;
	int empty_ticks;


public:
	bool floodprotection_status();
	std::string load_status();
	void Tick(bool send);

	bool cooldown_enabled;

	flood_protection();
	~flood_protection();
};


class networking_vkcom : public interface
{
private:
	std::string VKONTAKTE_ACCESS_TOKEN;
	long TOKEN_LIFETIME;
	long SESSION_START;
	std::thread* updater;

	vkLoginForm FORM;

	void showAuthForm();
	void parseVkLoginPage(const char* pagedata, const char* headerdata);
	std::string findToken(const char* headerdata);
	std::string findPermissionUrl(const std::string& PermissionPage);
	std::string findPermissionCookies(const std::string& PermissionHeader);
	int findLifetime(const char* headerdata);
	int __auth_login(const std::string& login, const std::string& passwd);
	void checkTimings();
	
	int fetchCounter;
	bool fetchNewMessages();
	void pushNewMessages();
	void autoAcceptFriends();

	bool relogin_procedure;

	friend void vkban(const std::string& id);
	friend void vkunban(const std::string& id);

	bool ErrorTick;

	void bdater();
	void idlebreaker();
	std::vector<std::string> findBirthdateUsers(std::string userdata, std::string today, std::string tomonth);
	int idle_timer;
	int bdater_timer;

	std::string getNextPromotion();
	int getPromotionTimer(int timer);
	int promotion_timer;
	bool isOnPromotionStreak;
	std::vector<int> promotion_cache;
	unsigned int promotion_sent;

	bool captcha(const std::string& mdata);
	std::string captcha_answer;
	std::string captcha_sid;
	
	Connector* connector;

	bool isGroupBot = false;

public:
	flood_protection* floodprot;
	
	network* pNetwork;
	bool LoggedIn;
	bool force_relogin;
	bool enable_idlebreaker;
	bool onTick;
	bool TX;
	bool debugFetcher;

	int auth_login(const std::string& login, const std::string& passwd);
	int auth_logout();
	int auth_relogin();

	std::string printTimings();
	Connector* getInterfaceConnector() { return this->connector; }
	unsigned int getPromotionSentCounter() { return promotion_sent; }
	
	void Tick() override;
	void OnUpdate();

	int banAction(const std::string& id, bool toBan);
	std::string forceBanlistRefresh(const std::string& botname, const std::string& dbname);
	std::string forceBanlistRefreshPy(const std::string& botname, const std::string& dbname);
	std::string makeRepost(const std::string& postId, bool pin);

	Answer interfaceAnswer;

	//Дополнительная асинхронная очередь ответов
	std::queue<Answer> asyncAnswerQueue;
	std::mutex asyncAnswerQueueLocker;

	//fingerprint for SCRS
	std::string getSCRSfingerprint();

	//self vk page id
	std::string selfPageId;

	networking_vkcom(void* botptr, const std::string& simnam, const std::string& snapname, bool igb, InterfaceSettings preSetter);
	~networking_vkcom();
};

#endif