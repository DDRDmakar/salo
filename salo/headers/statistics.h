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

#ifndef sequence_check_stat
#define sequence_check_stat null

#define INTERFACE_VKCOM 0
#define INTERFACE_WEBFRONTEND 1
#define INTERFACE_TELEGRAM 2
#define DATABASE 100
#define FETCHER 200

#define MESSAGES_COUNT 0
#define MESSAGES_TIME 1
#define MESSAGES_TIME_AVG 2
#define TRAFFIC_IN 3
#define TRAFFIC_OUT 4
#define MESSAGES_TOTAL 5
#define QUERYES_PER_MESSAGE 6
#define QUERYES_TOTAL 7 
#define QUERY_TIME_AVG 8
#define GENERAL_RX_TIMING 9
#define GENERAL_TX_TIMING 10
#define FETCHER_CACHE_USAGE 11
#define FETCHER_MESSAGES_LOAD 12
#define FETCHER_TIME 13

#include <chrono>

typedef unsigned long long int LONGINT;

struct Storage
{
	 LONGINT in_bytes;
	 LONGINT out_bytes;
	 LONGINT messages_count;
	 LONGINT msg_time;
	 double  msg_avg_time;
	 double RX_TIMINGS;
	 double TX_TIMINGS;
}; 

class Timing
{
public:
	std::chrono::time_point<std::chrono::system_clock> start, end;

	void StartCounting();
	void EndCounting();
	double getPeriod();
};

class Statistics
{
private:
	Storage vkcom_storage;
	Storage web_storage;
	Storage telegram_storage;

	//database
	LONGINT db_query_total;
	LONGINT db_query_per_msg;
	double  db_avg_time;

	LONGINT all_messages;

	unsigned int cache_usage;
	unsigned int messages_per_fetch;
	double fetcher_time;

public:
	Statistics();
	~Statistics();
	
	long int START; 
	Timing* conveerTimings;
	Timing* databaseTimings;
	Timing* vkcomGeneralTimings;
	Timing* telegramGeneralTimings;
	Timing* saloapiGeneralTimings;

	std::string getStatusString();
	std::string getStatisticsString(void* Bot = NULL);
	std::string getStressStatusString();
	std::string getFetcherStatusString();
	std::string getMessagesString(bool web, void* Bot = NULL);
	std::string getFullTimingsString();
	std::string getBotStats(void* Bot = NULL);
	long int getUptime();
	float getConveerAvgTime();

	void IncrementInRAM(int parameter, int interface, double value = 0);
};


extern Statistics* statistics; 



class interfaceStatistics
{
public:
	Storage iStorage;
	Timing iTiming;
	Timing tConveerTimings;
};
 


#endif