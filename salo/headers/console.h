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

#ifndef CONSOLE
#define CONSOLE

#include "message.h"

extern int ServerAlive; 

std::string onCommand(const std::string& command, bool internal, Person p, CONFIGURATION h, void* externalBotV = NULL, const std::string& externalInterfaceName = "default");
void server_console();

void botui();

#endif
