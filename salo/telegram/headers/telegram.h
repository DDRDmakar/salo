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

#include "../../headers/network.h"
#include "../../headers/interface.h"
#include "../../headers/message.h"


class networking_telegram : public interface
{
private:
	std::string ACCESS_TOKEN;
	std::string API_STRING;
	network* ptNetwork;

	bool IsSuccessful(const std::string& response);
	std::string getUnicode(const std::string& str);
	std::string inlineToConveerCommands(const std::string& str);
	bool haveCorrectGreeting(const std::string& message);

	int Auth(const std::string& token);
	void showAuthForm();

	int offset;

	bool fetchMessage();
	void pushMessage();

	bool firstFetch = true;
public:
	networking_telegram(void* botptr, const std::string& simnam, const std::string& snapname);
	~networking_telegram();
	
	bool _ENABLED;
	bool TX;

	void OnUpdate();

	void Tick() override;
	AnswerTelegram interfaceAnswerTG;
};