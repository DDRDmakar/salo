
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

#include <string>
#include <vector>
#include <algorithm>
#include <yaml-cpp/yaml.h>

#ifndef sequence_check_stats
#define sequence_check_stats

using namespace std;

namespace Stats
{
	void update_stats(const std::string &dbname = "", const std::string &botname = "");
}

#endif
