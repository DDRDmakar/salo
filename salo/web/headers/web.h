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
#include <thread>
#include <mutex>

#include "../../headers/interface.h"
#include "../../headers/message.h"

class webinterface : public interface
{
private:
	bool ProcessMessage(const std::string& fname);
	void PushMessage();

	void updateStats();
	int k;

	std::thread spawn_updater() { return std::thread([this] { this->updateStats(); }); }
	std::thread serviceThread;

	std::mutex SERVICE_MUTEX;

public:
	webinterface();
	~webinterface();
	
	void OnUpdate();
	void Tick() override;

	bool enabled;

	AnswerWeb interfaceAnswerWeb;
};
