// ConsoleCommands.c
// This is where you add commands:
//		1. Add a protoype
//			static eCommandResult_T ConsoleCommandVer(const char buffer[]);
//		2. Add the command to mConsoleCommandTable
//		    {"ver", &ConsoleCommandVer, HELP("Get the version string")},
//		3. Implement the function, using ConsoleReceiveParam<Type> to get the parameters from the buffer.

#include <string.h>
#include <math.h>
#include "consoleCommands.h"
#include "console.h"
#include "consoleIo.h"
#include "version.h"
#include "main.h"

#define IGNORE_UNUSED_VARIABLE(x)     if ( &x == &x ) {}
#define WAVE_MAX_STR_LENGTH 12 //
#define PI 3.14159

extern Waveform_T Wave;
extern int16_t Amplitude;
extern int16_t Frequency;
extern uint32_t Delta_Phase;
extern int16_t Amplitude;
extern uint32_t lut[1024];
extern Events_T event;

static eCommandResult_T ConsoleCommandComment(const char buffer[]);
static eCommandResult_T ConsoleCommandVer(const char buffer[]);
static eCommandResult_T ConsoleCommandHelp(const char buffer[]);
static eCommandResult_T ConsoleCommandParamExampleInt16(const char buffer[]);
static eCommandResult_T ConsoleCommandSetWave(const char buffer[]);
static eCommandResult_T ConsoleCommandSetFrequency(const char buffer[]);
static eCommandResult_T ConsoleCommandSetAmplitude(const char buffer[]);
static eCommandResult_T ConsoleCommandParamExampleHexUint16(const char buffer[]);

static const sConsoleCommandTable_T mConsoleCommandTable[] =
{
    {";", &ConsoleCommandComment, HELP("Comment! You do need a space after the semicolon. ")},
    {"help", &ConsoleCommandHelp, HELP("Lists the commands available")},
    {"ver", &ConsoleCommandVer, HELP("Get the version string")},
    {"int", &ConsoleCommandParamExampleInt16, HELP("How to get a signed int16 from params list: int -321")},
	{"wave", &ConsoleCommandSetWave, HELP("Sets type of waveform (sine,square,sawtooth)")},
	{"freq", &ConsoleCommandSetFrequency, HELP("Sets frequency of waveform in Hz")},
	{"amp", &ConsoleCommandSetAmplitude, HELP("Sets amplitude of waveform in V")},
    {"u16h", &ConsoleCommandParamExampleHexUint16, HELP("How to get a hex u16 from the params list: u16h aB12")},
	CONSOLE_COMMAND_TABLE_END // must be LAST
};

static eCommandResult_T ConsoleCommandComment(const char buffer[])
{
	// do nothing
	IGNORE_UNUSED_VARIABLE(buffer);
	return COMMAND_SUCCESS;
}

static eCommandResult_T ConsoleCommandHelp(const char buffer[])
{
	uint32_t i;
	uint32_t tableLength;
	eCommandResult_T result = COMMAND_SUCCESS;

    IGNORE_UNUSED_VARIABLE(buffer);

	tableLength = sizeof(mConsoleCommandTable) / sizeof(mConsoleCommandTable[0]);
	for ( i = 0u ; i < tableLength - 1u ; i++ )
	{
		ConsoleIoSendString(mConsoleCommandTable[i].name);
#if CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
		ConsoleIoSendString(" : ");
		ConsoleIoSendString(mConsoleCommandTable[i].help);
#endif // CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
		ConsoleIoSendString(STR_ENDLINE);
	}
	return result;
}

static eCommandResult_T ConsoleCommandParamExampleInt16(const char buffer[])
{
	int16_t parameterInt;
	eCommandResult_T result;
	result = ConsoleReceiveParamInt16(buffer, 1, &parameterInt);
	if ( COMMAND_SUCCESS == result )
	{
		ConsoleIoSendString("Parameter is ");
		ConsoleSendParamInt16(parameterInt);
		ConsoleIoSendString(" (0x");
		ConsoleSendParamHexUint16((uint16_t)parameterInt);
		ConsoleIoSendString(")");
		ConsoleIoSendString(STR_ENDLINE);
	}
	return result;
}

static eCommandResult_T ConsoleCommandSetWave(const char buffer[])
{
	eCommandResult_T result;
	char wave_str[WAVE_MAX_STR_LENGTH];
	result = ConsoleReceiveString(buffer, 1, wave_str);
	if (strcmp(wave_str,"sine")==0)
	{
		Wave = SINE;
	}
	else if (strcmp(wave_str,"square")==0)
	{
		Wave = SQUARE;
	}
	else if (strcmp(wave_str,"sawtooth")==0)
	{
		Wave = SAWTOOTH;
	}
	else if (strcmp(wave_str,"dc")==0)
	{
		Wave = DC;
	}
	else
	{
		result = COMMAND_ERROR;
	}
	if ( COMMAND_SUCCESS == result )
	{
		event = CHANGE_WAVE;
		ConsoleIoSendString("wave is ");
		ConsoleIoSendString(wave_str);
		ConsoleIoSendString(STR_ENDLINE);
	}
	return result;
}

static eCommandResult_T ConsoleCommandSetFrequency(const char buffer[])
{
	int16_t frequencyInt;
	eCommandResult_T result;
	result = ConsoleReceiveParamInt16(buffer, 1, &frequencyInt);
	Frequency = frequencyInt;
	if ( COMMAND_SUCCESS == result )
	{
		event = CHANGE_FREQUENCY;
		ConsoleIoSendString("Frequency is ");
		ConsoleSendParamInt16(frequencyInt);
		ConsoleIoSendString(STR_ENDLINE);
	}
	return result;
}

static eCommandResult_T ConsoleCommandSetAmplitude(const char buffer[])
{
	int16_t amplitudeInt;
	eCommandResult_T result;
	result = ConsoleReceiveParamInt16(buffer, 1, &amplitudeInt);
	Amplitude = amplitudeInt;

	if ( COMMAND_SUCCESS == result )
	{
		event = CHANGE_AMPLITUDE;
		ConsoleIoSendString("Amplitude is ");
		ConsoleSendParamInt16(amplitudeInt);
		ConsoleIoSendString(STR_ENDLINE);
	}
	return result;
}
static eCommandResult_T ConsoleCommandParamExampleHexUint16(const char buffer[])
{
	uint16_t parameterUint16;
	eCommandResult_T result;
	result = ConsoleReceiveParamHexUint16(buffer, 1, &parameterUint16);
	if ( COMMAND_SUCCESS == result )
	{
		ConsoleIoSendString("Parameter is 0x");
		ConsoleSendParamHexUint16(parameterUint16);

		ConsoleIoSendString(" (");
		ConsoleSendParamInt16((int16_t)parameterUint16);
		ConsoleIoSendString(")");

		ConsoleIoSendString(STR_ENDLINE);
	}
	return result;
}

static eCommandResult_T ConsoleCommandVer(const char buffer[])
{
	eCommandResult_T result = COMMAND_SUCCESS;
    IGNORE_UNUSED_VARIABLE(buffer);
	ConsoleIoSendString(VERSION_STRING);
	ConsoleIoSendString(STR_ENDLINE);
	return result;
}

const sConsoleCommandTable_T* ConsoleCommandsGetTable(void)
{
	return (mConsoleCommandTable);
}


