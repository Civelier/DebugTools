/*
 Name:		DebugToolsTests.ino
 Created:	1/8/2022 6:37:58 PM
 Author:	civel
*/

#include <Sodaq_wdt.h>
#include "DebugTools.h"

void Func1()
{
	DebugToolsFunctionBegin();
	DebugTools.PrintStack();
}

void Bar()
{
	DebugToolsFunctionBegin();
	DebugTools.PrintStack();
}

void Foo()
{
	DebugToolsFunctionBegin();
	Bar();
}

int* arr;

void IGoodAllocate()
{
	DebugToolsFunctionBeginAlloc(10 * sizeof(int));
	arr = DeclareNewArr(int, 10);
}

void IGoodFree()
{
	DebugToolsFunctionBeginAlloc(-10 * sizeof(int));
	DeclareDeleteArr(int, 10) arr;
}

void IBadAllocate()
{
	DebugToolsFunctionBegin();
	arr = DeclareNewArr(int, 10);
}

void IBadFree()
{
	DebugToolsFunctionBegin();
	DeclareDeleteArr(int, 10) arr;
}

void setup()
{
	Serial.begin(115200);
	while (!Serial);
	DebugTools.SetupWatchdog(500); // Setup a watchdog with a 500ms timeout
	DebugStream = &Serial; // Optionnal because 'Serial' is the default value
	
	if (DebugTools.WasLastResetFromWatchdog())
	{
		DebugError("Last execution failed!");
		DebugTools.PrintDebugInfo();
		DebugLvl(1, "\n\n");
	}

	Func1();
	Foo();
	IGoodAllocate();
	IGoodFree();
	IBadAllocate();
	IBadFree();
}

void loop()
{
	DebugToolsFunctionBeginNoWarn();
	DebugToolsStep("Inside of loop");

	for (size_t i = 0; i < 10; i++)
	{
		delay(400);
		DebugToolsStep("Looping");
	}

	DebugToolsStep("Before stalling");
	for (;;); // stall
	DebugToolsStep("After stalling");
}
