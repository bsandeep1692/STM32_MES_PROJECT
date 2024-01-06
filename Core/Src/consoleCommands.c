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
#include "stm32f7xx_hal.h"
#include "string.h"


#define IGNORE_UNUSED_VARIABLE(x)     if ( &x == &x ) {}
#define WAVE_MAX_STR_LENGTH 12 //
#define PI 3.14159

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart3;
extern Waveform_T Wave;
extern int16_t Amplitude;
extern int16_t Frequency;
extern uint32_t Delta_Phase;
extern int16_t Amplitude;
extern uint32_t lut[1024];
extern Events_T event;
char adc_msg[20];
char wave_str[WAVE_MAX_STR_LENGTH] = "sine";

static eCommandResult_T ConsoleCommandComment(const char buffer[]);
static eCommandResult_T ConsoleCommandVer(const char buffer[]);
static eCommandResult_T ConsoleCommandHelp(const char buffer[]);
static eCommandResult_T ConsoleCommandSetWave(const char buffer[]);
static eCommandResult_T ConsoleCommandSetFrequency(const char buffer[]);
static eCommandResult_T ConsoleCommandSetAmplitude(const char buffer[]);
static eCommandResult_T ConsoleCommandGetAdcValue(const char buffer[]);

static const sConsoleCommandTable_T mConsoleCommandTable[] =
{
    {";", &ConsoleCommandComment, HELP("Comment! You do need a space after the semicolon. ")},
    {"help", &ConsoleCommandHelp, HELP("Lists the commands available")},
    {"ver", &ConsoleCommandVer, HELP("Get the version string")},
	{"wave", &ConsoleCommandSetWave, HELP("Sets type of waveform (sine,square,sawtooth)")},
	{"freq", &ConsoleCommandSetFrequency, HELP("Sets frequency of waveform in Hz")},
	{"amp", &ConsoleCommandSetAmplitude, HELP("Sets amplitude of waveform in V")},
	{"voltage", &ConsoleCommandGetAdcValue, HELP("How to get a hex u16 from the params list: u16h aB12")},
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

static eCommandResult_T ConsoleCommandSetWave(const char buffer[])
{
	eCommandResult_T result;
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
		ConsoleIoSendString("wave:");
		ConsoleIoSendString(wave_str);
		ConsoleIoSendString(" Amplitude:");
		ConsoleSendParamInt16(Amplitude);
		ConsoleIoSendString(" Frequency:");
		ConsoleSendParamInt16(Frequency);
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
		ConsoleIoSendString("wave:");
		ConsoleIoSendString(wave_str);
		ConsoleIoSendString(" Amplitude:");
		ConsoleSendParamInt16(Amplitude);
		ConsoleIoSendString(" Frequency:");
		ConsoleSendParamInt16(Frequency);
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
		ConsoleIoSendString("wave:");
		ConsoleIoSendString(wave_str);
		ConsoleIoSendString(" Amplitude:");
		ConsoleSendParamInt16(Amplitude);
		ConsoleIoSendString(" Frequency:");
		ConsoleSendParamInt16(Frequency);
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

static eCommandResult_T ConsoleCommandGetAdcValue(const char buffer[])
{
	eCommandResult_T result = COMMAND_SUCCESS;
	uint16_t raw =0;
	float voltage = 0;
	// Start ADC Conversion
	HAL_ADC_Start(&hadc1);
	// Poll ADC1 Perihperal & TimeOut = 1mSec
	HAL_ADC_PollForConversion(&hadc1, 100);
	// Read The ADC Conversion Result & Map It To PWM DutyCycle
	raw = HAL_ADC_GetValue(&hadc1);
	voltage = (float)raw * 3.3/4095.0;
	ConsoleIoSendString("ADC Voltage is ");
	ConsoleSendParamInt16((int16_t)round(voltage));
	ConsoleIoSendString(STR_ENDLINE);
	return result;

}

const sConsoleCommandTable_T* ConsoleCommandsGetTable(void)
{
	return (mConsoleCommandTable);
}


