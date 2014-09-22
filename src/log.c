#include <stdio.h>
#include <time.h>


#include "infection.h"
#include "player.h"
#include "log.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		printLog
//			prints a string to the log file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void printLog(char *c)
{
	//Error output log
	FILE *log;

	if( !c ) return;

	if( !options.log ) return; // don't log if we shouldn't

	log = fopen("infection_flow.txt", "a");
	if(log)
	{
		fprintf(log, "%s", c); // print the data
		fflush(log); // flush it, we wan't emediate output
		fclose(log); // close it
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void printExtended(char* sz)
{
	/*
	I've done this way mainly because of two things:
	1.	If the extended log is on I am reserving memory of a "large" array.
		If I did it the other way, then I always would have
		reserved the memory, evry call of this function, even if I dodn't use it.
		This way I doesn't always reserve momory.
	2.	Forgot the other reason, but I'm sure it was a good one :)
	*/
	if( options.extended_log )
	{
		char str[300];
		sprintf(str, "Extended Log: %s", sz); // make sure we know it's a extended log
		printLog(str);
	}
}

void error(char* info)
{
	//Error output log
	FILE *log;
	//time varaibles
	struct tm *today;
	time_t ltime;

	log = fopen("infection_errors.txt", "a");
	if(log)
	{
		time(&ltime);
		today = localtime(&ltime);
		fprintf(log, "\n--------------Error--------------\n");
		fprintf(log, INFECTION_NAME_AND_VERSION); // give the name and the version to the log file
		fprintf(log, "\nBuild time: %s, %s.\n", __TIME__, __DATE__); // give the date of the error
		fprintf(log, "Error time: %d:%d:%d.\n", today->tm_hour, today->tm_min, today->tm_sec); // and the time of cource :)
		fprintf(log, "Info: %s\n", info); // print the info
		fprintf(log, "------------------------------------\n\n\n");
		fflush(log); // flush the log
		fclose(log); // close th file
	}
}

void errorx(char* info, char* reason){
		//Error output log
	FILE *log;
	//time varaibles
	struct tm *today;
	time_t ltime;

	log = fopen("infection_errors.txt", "a");
	if(log)
	{
		time(&ltime);
		today = localtime(&ltime);
		fprintf(log, "\n--------------Error--------------\n");
		fprintf(log, INFECTION_NAME_AND_VERSION); // give the name and the version to the log file
		fprintf(log, "\nBuild time: %s, %s.\n", __TIME__, __DATE__); // give the date of the error
		fprintf(log, "Error time: %d:%d:%d.\n", today->tm_hour, today->tm_min, today->tm_sec); // and the time of cource :)
		fprintf(log, "Info: %s\n", info); // print the info
		fprintf(log, "Reason: %s\n", reason); // print the info
		fprintf(log, "------------------------------------\n\n\n");
		fflush(log); // flush the log
		fclose(log); // close th file
	}
}



void cerror(char* info){
	error(info);
	console_message(info);
}

void cerrorx(char* info, char* reason){
	errorx(info, reason);
	console_message(info);
	console_message(reason);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		startLog
//			clear/creates the log file and write some basic information
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void startLog()
{
	FILE *log;
	//time varaibles
	struct tm *today;
	time_t ltime;

	time(&ltime);
	today = localtime(&ltime);

	//Open the error file:
	log = fopen("infection_flow.txt", "w");
	// print a nice header
	fprintf(log, "--------------------------\n");
	fprintf(log, "--- infection_flow.txt ---\n");
	fprintf(log, "--------------------------\n");
	fprintf(log, INFECTION_NAME_AND_VERSION); // print the name and the version
	fprintf(log, "\nBuild time: %s, %s.\n", __TIME__, __DATE__); // log build time and build date
	//log run time
	fprintf(log,"Run time: %d:%d:%d.\n\n\n", today->tm_hour, today->tm_min, today->tm_sec);
	fclose(log);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

