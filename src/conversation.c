#include "conversation.h"
#include "sound_system.h"
#include "console.h"
#include "globals.h"
#include "log.h"
#include "inf_message_system.h"
#include "infection.h"
#include "extra_genesis.h"
#include "subtitles.h"

int currentInstruction=0;
char scriptFile[300] ="";

char levelFile[300] ="";
FILE* currentFile =0;
geVec3d *person=0;
char personName[200] = "";
float timeLeft=0.0f;
float conversationSqRadius=555000.0f;
geBoolean allowMiddleEnd = GE_FALSE;
char endScript[300] ="";
geBoolean ignore = GE_TRUE;
float min = 4.0f;

// talkSounds is cleared when killing the game
// this function is also called when the player leaves the level
// so this should only stop the current sound (if there is anyone)
// and kill the file handle and default all values
void almost_endGame(){
	if( currentFile ){
		fclose(currentFile);
		currentFile = 0;
	}
	person = 0;
	currentInstruction = 0;
	timeLeft = 0.0f;
	conversationSqRadius = 120.0f;
	allowMiddleEnd = GE_FALSE;
	strcpy(endScript , "");
	strcpy(levelFile , "");
	strcpy(scriptFile , "");
	min = 4.0f;
	ignore = GE_TRUE;
}

void conversation_endGame(){
	soundsys_conversation_stopTalkiing();
	almost_endGame();
	subtitle_newGame();
}

// returns true if it shouldn't continue to read
int processInput(geWorld *world, char* input, float timePassed){
	char c;
	char processingString = 0;
	int stringPos=0;
	int commandPos=0;
	int inputPos=0;
	char str[200];

	char command[10][200];

	if( is_string_null(input) ){
		currentInstruction++;
		return 0;
	}

	for( c= 0; c<10; c++)
		memset(&(command[c]), 0, 200 );
	
	sprintf(str, "Recieved conversation command: %s.\n", input);
	printLog(str);
	
	// stor input in the command string array
	while( 1 )
	{
		c = input[inputPos];
		inputPos++;
		
		if( c== 0 )
		{
			command[ commandPos ][ stringPos ] = 0;
			break;
		}
		
		if( !processingString )
		if( stringPos != 0 && (c == ' ' || c == '\t'))
		{
			command[ commandPos ][ stringPos ] = 0;
			commandPos ++;
			stringPos = 0;
			if( commandPos == 10 ) break;
		}

		if( (c != ' ' && c != '\t') || processingString)
		{
			if( c=='*' )
			{
				processingString = 1;
				continue;
			}
			command[commandPos][stringPos] = c;
			stringPos++;

			if( stringPos == 99 )
			{
				command[commandPos][stringPos] = 0;
				stringPos = 0;
				commandPos++;
				if( commandPos == 10 ) break;
			}
		}
	}

	if( _stricmp(command[0], "AllowMiddleEnds")==0 ){
		allowMiddleEnd = GE_TRUE;
	} else if( _stricmp(command[0], "ConversationSquareRadius")==0 ){
		conversationSqRadius = (float)atof(command[1]);
	} else if( _stricmp(command[0], "ScriptOnConversationEnd")==0 ){
		strcpy(endScript, command[1]);
	} else if( _stricmp(command[0], "DoNotIgnore")==0 ){
		ignore = GE_FALSE;
	} else if( _stricmp(command[0], "MinimumDistance")==0 ){
		min= (float)atof(command[1]);
	} else if( _stricmp(command[0], "Wait")==0 ){
		// unimplemented
	} else if( _stricmp(command[0], "Subtitle")==0 ){
		subtitle_setSubtitle(command[1]);
	} else if( _stricmp(command[0], "EndConversation")==0 ){
		subtitle_newGame();
	} else if( _stricmp(command[0], "TalkingTo")==0 ){
		strcpy(personName, command[1]);
		person = findLocationByName(world, command[1] );
		if(! person ){
			char str[200];
			sprintf(str, "System can not find anyone or anything named %s", command[1] );
			system_message(str);
		}
		strcpy(personName, command[1]);
	} else if( _stricmp(command[0], "ThroughRadioWith")==0 ){
		person = 0;
		strcpy(personName, command[1]);
		allowMiddleEnd = GE_FALSE;
		currentInstruction++;
		ignore = GE_FALSE;
	} else if( _stricmp(command[0], "PersonSay")==0 ){
		char fileName[300];
		sprintf(fileName, ".\\levels\\%s\\Conversation\\%s\\%s.wav", levelFile, personName, command[1]);
		soundsys_conversation_personSay(fileName, person, min, ignore);
		currentInstruction++;
		return GE_TRUE;
	} else if( _stricmp(command[0], "YouSay")==0 ){
		char fileName[300];
		sprintf(fileName, ".\\levels\\%s\\Conversation\\hero\\%s.wav", levelFile, command[1]);
		soundsys_conversation_heroSay(fileName);
		currentInstruction++;
		return GE_TRUE;
	} else {
		execute_command(input);
	}

	return GE_FALSE;
}

void execute_endScript(){
	if( ! is_string_null(endScript) ){
		execute_scriptFile(endScript);
		strcpy(endScript, "");
	} else {
		printLog("End script is considered null");
	}
}

void conversation_iterate(geWorld* world, geVec3d *playerPos, float timePassed){
	char input[200];
	float distance = 0.0f;
	if(! currentFile ) return;
	if( soundsys_conversation_isTalking() ){
		//playerPos
		//soundsys_conversation_updatePersonSound(person, min, ignore); // deprecated function, don't needed to be called
		return;
	}

	fgetln(input, currentFile);
	if( allowMiddleEnd ){ // some very slight improvement - we don't need to check the distance if the conversation doesn't depend on distance
		if(! isWithinSquaredRange( person, playerPos, conversationSqRadius ) ) {
			char temp[300];
			strcpy(temp, endScript);
			conversation_endGame();
			strcpy(endScript, temp);
			execute_endScript();
			return;
		}
	}
	processInput(world, input, timePassed);

	if( feof(currentFile) ){
		almost_endGame();
	}
}


// set's up and does the file checking/loading
// the iterate function takes care of executing all the commands
// this function also kills the previous sounds if needed
void startConversation(const char* theLevelFile, const char* fileName, geBoolean stopThisIfPrevious){
	char filePath[300];
	console_message("Conversation initiated");
	if( soundsys_conversation_isTalking() ){
		if( stopThisIfPrevious == GE_TRUE ) return;
		soundsys_conversation_stopTalkiing();
		conversation_endGame();
	}
	sprintf(filePath, ".\\levels\\%s\\Conversation\\%s", theLevelFile, fileName);
	currentFile = fopen(filePath, "r");
	if(! currentFile ){
		char str[200];
		sprintf(str, "File not found: %s", fileName);
		strcpy(endScript, "");
		console_message(str);
		error(str);
		conversation_endGame();
		console_message("Conversation aborted");
		return;
	}
	strcpy(levelFile, theLevelFile);
	strcpy(scriptFile , fileName);
	console_message("Conversation has begun");
}