/*
 Name:		DebugTools.cpp
 Created:	8/25/2021 12:24:24 PM
 Author:	civel
 Editor:	http://www.visualmicro.com
*/

#include "DebugTools.h"

#ifdef  _DEBUG
#include "cstring"

#ifdef _USE_WDT

#ifdef ARDUINO_ARCH_MEGAAVR
#include "avr/wdt.h"


#if !defined(WDTO_4S)

#define WDTO_4S WDTO_1S

#endif // !defined(WDTO_4S)

#if !defined(WDTO_8S)

#define WDTO_8S WDTO_1S

#endif // !defined(WDTO_8S)

void watchdogEnable(int ms)
{
    uint8_t wdt_timeout = 0;

    switch (ms)
    {
    case 15:
    {
        wdt_timeout = WDTO_15MS;
        break;
    }
    case 30:
    {
        wdt_timeout = WDTO_30MS;
        break;
    }
    case 60:
    {
        wdt_timeout = WDTO_60MS;
        break;
    }
    case 120:
    {
        wdt_timeout = WDTO_120MS;
        break;
    }
    case 250:
    {
        wdt_timeout = WDTO_250MS;
        break;
    }
    case 500:
    {
        wdt_timeout = WDTO_500MS;
        break;
    }
    case 1000:
    {
        wdt_timeout = WDTO_1S;
        break;
    }
    case 2000:
    {
        wdt_timeout = WDTO_2S;
        break;
    }
    case 4000:
    {
        wdt_timeout = WDTO_4S;
        break;
    }
    case 8000:
    {
        wdt_timeout = WDTO_8S;
        break;
    }
    }

    wdt_enable(wdt_timeout);
}

void watchdogReset()
{
    wdt_reset();
}

#endif // ARDUINO_ARCH_MEGAAVR
#ifdef ARDUINO_ARCH_SAM
#include "watchdog.h"
#endif // ARDUINO_ARCH_SAM
#ifdef ARDUINO_ARCH_SAMD
#include <Sodaq_wdt.h>
#define watchdogReset() sodaq_wdt_reset()

#if !defined(WDTO_4S)

#define WDTO_4S WDTO_1S

#endif // !defined(WDTO_4S)

#if !defined(WDTO_8S)

#define WDTO_8S WDTO_1S

#endif // !defined(WDTO_8S)

void watchdogEnable(int ms)
{
    wdt_period wdt_timeout = WDT_PERIOD_8X;

    if (ms <= 16) wdt_timeout = WDT_PERIOD_1DIV64;   // 16 cycles   = 1/64s
    else if (ms <= 31) wdt_timeout = WDT_PERIOD_1DIV32;   // 32 cycles   = 1/32s
    else if (ms <= 63) wdt_timeout = WDT_PERIOD_1DIV16;   // 64 cycles   = 1/16s
    else if (ms <= 125) wdt_timeout = WDT_PERIOD_1DIV8;   // 128 cycles  = 1/8s
    else if (ms <= 250) wdt_timeout = WDT_PERIOD_1DIV4;   // 256 cycles  = 1/4s
    else if (ms <= 500) wdt_timeout = WDT_PERIOD_1DIV2;   // 512 cycles  = 1/2s
    else if (ms <= 1000) wdt_timeout = WDT_PERIOD_1X;   // 1024 cycles = 1s
    else if (ms <= 2000) wdt_timeout = WDT_PERIOD_2X;   // 2048 cycles = 2s
    else if (ms <= 4000) wdt_timeout = WDT_PERIOD_4X;   // 4096 cycles = 4s
    else wdt_timeout = WDT_PERIOD_8X;   // 8192 cycles = 8s

    sodaq_wdt_enable(wdt_timeout);
}
#endif // ARDUINO_ARCH_SAMD
#endif // _USE_WDT

Stream* DebugStream{ &Serial };

#ifdef _USE_WDT
void watchdogSetup(void) {/*** watchdogDisable (); ***/ }
#endif // _USE_WDT

#ifdef ARDUINO_SAM_DUE
const size_t MaxMem = 98304;
#elif defined(ARDUINO_AVR_NANO_EVERY)
const size_t MaxMem = 6144;
#endif


void DebugToolsClass::SetDebugOutput(Stream* output)
{
    DebugStream = output;
}

Stream* DebugToolsClass::GetDebugOutput()
{
    return DebugStream;
}

typedef void (*func_t)();

func_t ClearMemFunc;

size_t* stopPoint;

#define PTRContains(value, ptr, size) ((uint8_t*)ptr <= (uint8_t*)value && (uint8_t*)value < (uint8_t*)(ptr + size))


inline void ClearMemory()
{
    DebugStream->print("Last stopPoint: ");
    DebugStream->println(*stopPoint);
    DebugStream->println("Clearing");
    DebugStream->flush();
    uint8_t* ptr = nullptr;
    digitalWrite(LED_BUILTIN, HIGH);

    for (ptr = (uint8_t*)(MaxMem); ptr > 0; ptr--)
    {
        *stopPoint = (size_t)ptr;
        if (!PTRContains(ptr, &MaxMem, sizeof(size_t)) && !PTRContains(ptr, &ptr, sizeof(uint8_t*)) && !PTRContains(ptr, &ClearMemFunc, 32) && !PTRContains(ptr, &stopPoint, sizeof(size_t)) && !PTRContains(ptr, stopPoint, sizeof(size_t))
#ifdef ARDUINO_AVR_NANO_EVERY
            && !PTRContains(ptr, 1670, 4)
            && !PTRContains(ptr, 2053, 4)
            && !PTRContains(ptr, 2085, 4)
            && !PTRContains(ptr, 2117, 4)
            && !PTRContains(ptr, 2242, 4)
            && !PTRContains(ptr, 2320, 64)
            && !PTRContains(ptr, 4195 - 3, 4)
#endif
            )
        {
            *ptr = 0;
        }
#ifdef _USE_WDT
        watchdogReset();
#endif // _USE_WDT
    }
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
}

void DebugToolsClass::CleanMemory()
{
    //atexit(ClearMemory);
    DebugStream->println("Wiping memory!");
    DebugStream->flush();
    delay(100);
    ClearMemFunc = ClearMemory;
    ClearMemFunc();
    exit(0);
}

void DebugToolsClass::NotifyMemoryAllocation(int size)
{
    m_allocated += size;
}

void DebugToolsClass::NotifyMemoryFree(int size)
{
    m_allocated -= size;
}

int DebugToolsClass::GetAllocatedMemory()
{
    return m_allocated;
}

void DebugToolsClass::PrintCurrentlyAllocatedMemory()
{
    DebugStream->print("Memory: ");
    DebugStream->print(m_allocated);
    DebugStream->println(" bytes");
    DebugStream->flush();
}

size_t DebugToolsClass::GetFreeMem()
{
    size_t mem = 0;
    for (uint8_t* ptr = (uint8_t*)1; ptr < (uint8_t*)MaxMem; ++ptr)
        if (*ptr == 0) ++mem;
    return mem;
}

void DebugToolsClass::FunctionEnter(uint16_t line, const char* fileName, const char* funcName)
{
#ifdef _USE_STACK
    DebugInfo debugInfo{ (DebugInfoTypeFlags)(DebugInfoTypeFlags::HasFunc | DebugInfoTypeFlags::IsTrace) };
    char* file_ptr = (char*)fileName;
    debugInfo.Line = { line };
    debugInfo.MemLeft = GetAllocatedMemory();

    for (char* c = file_ptr; *c != 0; c++)
    {
        if (*c == '\\') file_ptr = c;
    }

    strlcpy(debugInfo.FileName, file_ptr, DEBUG_TOOLS_FILE_NAME_LENGTH);
    strlcpy(debugInfo.FunctionName, funcName, DEBUG_TOOLS_STEP_NAME_LENGTH);
    *m_debugInfo = DebugInfo(debugInfo);
    m_callStack->push_back(debugInfo);
#endif // _USE_STACK
}

void DebugToolsClass::FunctionExit(int memLeft, bool suppressWarning, int memAllowed)
{
#ifdef _USE_STACK
    auto endMem = GetAllocatedMemory();
    auto delta = endMem - memLeft; // + means allocated, - means freed
    int allowed = delta - memAllowed;
    if (!suppressWarning && allowed)
    {
        auto back = m_callStack->operator[](m_callStack->size() != 0 ? m_callStack->size() - 1 : 0);
        if (allowed > 0) // Less mem free now than before - allocated memory
        {
            DebugStream->println(4);
            DebugStream->print("[Warning] Possible memory leak in '");
            DebugStream->print(back.FunctionName);
            DebugStream->print("', bytes leaked: ");
            DebugStream->print(allowed);
            DebugStream->print(" over leak budget of: ");
            DebugStream->println(memAllowed);
            DebugStream->flush();
        }
        if (allowed < 0) // More mem free than before - freed memory
        {
            DebugStream->println(4);
            DebugStream->print("[Warning] Possible unwanted freeing of memory in '");
            DebugStream->print(back.FunctionName);
            DebugStream->print("', bytes freed: ");
            DebugStream->print(-allowed);
            DebugStream->print(" over freeing budget of: ");
            DebugStream->println(-memAllowed);
            DebugStream->flush();
        }
    }
    m_callStack->pop_back();
    *m_debugInfo = m_callStack->back();
#endif // _USE_STACK
}

#ifdef _USE_WDT


void DebugToolsClass::SetupWatchdog(uint32_t ms)
{
    watchdogEnable(ms);
    //m_array = new Alloc_t(DEBUG_TOOLS_STACK_LENGTH, {});
    *m_callStack = Stack_t(DEBUG_TOOLS_STACK_LENGTH);
    m_debugInfo->DisplayOnNextRun = true;
}

void DebugToolsClass::ResetWatchdog(uint16_t line, const char* fileName, const char* funcName, const char* stepName)
{
    watchdogReset();

    m_debugInfo->Line = { line };
    m_debugInfo->TypeFlags = (DebugInfoTypeFlags)(DebugInfoTypeFlags::HasFunc | DebugInfoTypeFlags::HasName);

    char* file_ptr = (char*)fileName;

    for (char* c = file_ptr; *c != 0; c++)
    {
        if (*c == '\\') file_ptr = c;
    }

    strlcpy(m_debugInfo->FileName, file_ptr, DEBUG_TOOLS_FILE_NAME_LENGTH);
    strlcpy(m_debugInfo->FunctionName, funcName, DEBUG_TOOLS_STEP_NAME_LENGTH);
    strlcpy(m_debugInfo->StepName, stepName, DEBUG_TOOLS_STEP_NAME_LENGTH);

    if ((m_debugInfo->TypeFlags & DebugInfoTypeFlags::IsTrace) != DebugInfoTypeFlags::IsTrace && strcmp(m_debugInfo->StepName, stepName))
    {
        m_debugInfo->Count++;
        m_debugInfo->TypeFlags = (DebugInfoTypeFlags)(m_debugInfo->TypeFlags | DebugInfoTypeFlags::HasCount);
        m_debugInfo->Line = line;
    }
    m_callStack->back().MemLeft = GetAllocatedMemory();
}

bool DebugToolsClass::WasLastResetFromWatchdog()
{
#ifdef ARDUINO_ARCH_SAM
    uint32_t status = (RSTC->RSTC_SR & RSTC_SR_RSTTYP_Msk) >> 8;
    return status == 0b010;
#else
#ifdef _USE_STACK
    return m_debugInfo->DisplayOnNextRun;
#else
    return false;
#endif // _USE_STACK
#endif
}
#endif // _USE_WDT



static void PrintIndent(Print& print, int indent)
{
    for (size_t i = 0; i < indent; i++)
    {
        print.print('\t');
    }
}

#ifdef _USE_STACK
static void PrintTrace(Print& print, DebugInfo* it, int indent, int count)
{
#ifdef _USE_WDT
    watchdogReset();
#endif // _USE_WDT

    if (count > DEBUG_TOOLS_STACK_LENGTH) count = DEBUG_TOOLS_STACK_LENGTH;
    auto v = *it;
    if (indent < count - 1)
    {
        PrintIndent(print, indent); print.print('<');
        v.Display(print, DebugInfoDisplayFlags::MentionStack);
        print.println('>');
        PrintTrace(print, ++it, indent + 1, count);
        PrintIndent(print, indent); print.print("</");
        v.Display(print, DebugInfoDisplayFlags::MentionMinimal);
        print.println('>');
    }
    else
    {
        PrintIndent(print, indent);
        v.Display(print, DebugInfoDisplayFlags::MentionStack);
        print.println();
    }
#ifdef _USE_WDT
    watchdogReset();
#endif // _USE_WDT
}

void DebugToolsClass::PrintStack()
{
    m_debugInfo->DisplayOnNextRun = false;
    PrintTrace(*DebugStream, m_callStack->begin(), 0, m_callStack->size());
    DebugStream->println();
#ifdef _USE_WDT
    watchdogReset();
#endif // _USE_WDT
    DebugStream->flush();
    m_debugInfo->DisplayOnNextRun = true;
}


void DebugToolsClass::PrintDebugInfo(DebugInfoDisplayFlags displayFlags)
{
    m_debugInfo->DisplayOnNextRun = false;
    m_debugInfo->Display(*DebugStream, displayFlags);
    DebugStream->println();
    DebugStream->flush();
    m_debugInfo->DisplayOnNextRun = true;
}
#endif // _USE_STACK

void DebugToolsClass::DisplayMemory(size_t columns)
{
    uint8_t* ptr = nullptr;

    DebugStream->print("   |");
    for (size_t i = 0; i < columns; i++)
    {
        DebugStream->print(i);
        DebugStream->print(
            i >= 100 ? "|" :
            i >= 10 ? " |" :
            "  |");
    }
    DebugStream->println();

    size_t column = 0;
    size_t row = 0;

    DebugStream->print("0  |");
    for (ptr = (uint8_t*)0; ptr < (uint8_t*)MaxMem; ptr++)
    {
        if (*ptr == 0) DebugStream->print("   |");
        else DebugStream->print(" X |");
        column++;
        if (column >= columns)
        {
            column = 0;
            row++;
            DebugStream->println();
            DebugStream->print(row);
            DebugStream->print(
                row >= 100 ? "|" :
                row >= 10 ? " |" :
                "  |");
            DebugToolsStep("Display memory");
        }
    }
    DebugStream->flush();
}

void DebugToolsClass::DisplayMemoryContent(size_t columns)
{
    uint8_t* ptr = nullptr;

    DebugStream->print("   |");
    for (size_t i = 0; i < columns; i++)
    {
        DebugStream->print(i);
        DebugStream->print(
            i >= 100 ? "|" :
            i >= 10 ? " |" :
            "  |");
    }
    DebugStream->println();

    size_t column = 0;
    size_t row = 0;

    DebugStream->print("0  |");
    for (ptr = (uint8_t*)0; ptr < (uint8_t*)MaxMem; ptr++)
    {
        auto x = (int)*ptr;
        DebugStream->print(x);
        DebugStream->print(
            x >= 100 ? "|" :
            x >= 10 ? " |" :
            "  |");
        column++;
        if (column >= columns)
        {
            column = 0;
            row++;
            DebugStream->println();
            DebugStream->print(row);
            DebugStream->print(
                row >= 100 ? "|" :
                row >= 10 ? " |" :
                "  |");
            DebugToolsStep("Display memory");
        }
    }
    DebugStream->flush();
}

#ifdef _USE_STACK
void DebugInfo::Display(Print& print, DebugInfoDisplayFlags displayFlags)
{
    print.print("Debug info { ");

    int flags = (DebugInfoDisplayFlags)(TypeFlags & displayFlags);
    if ((flags & DebugInfoDisplayFlags::MentionFile) == DebugInfoDisplayFlags::MentionFile)
    {
        print.print("File: '");
        print.print(FileName);
        print.print("' ");
    }

    if ((flags & DebugInfoDisplayFlags::MentionLine) == DebugInfoDisplayFlags::MentionLine)
    {
        print.print("Line: '");
        print.print(Line);
        print.print("' ");
    }

    if ((flags & DebugInfoDisplayFlags::MentionFunc) == DebugInfoDisplayFlags::MentionFunc)
    {
        print.print("Func: '");
        print.print(FunctionName);
        print.print("' ");
    }

    if ((flags & DebugInfoDisplayFlags::MentionName) == DebugInfoDisplayFlags::MentionName)
    {
        print.print("Step name: '");
        print.print(StepName);
        print.print("' ");
    }

    if ((flags & DebugInfoDisplayFlags::MentionCount) == DebugInfoDisplayFlags::MentionCount)
    {
        print.print("Count: '");
        print.print(Count);
        print.print("' ");
    }

    if ((flags & DebugInfoDisplayFlags::MentionMemory) == DebugInfoDisplayFlags::MentionMemory)
    {
        print.print("Memory: '");
        print.print(MemLeft);
        print.print("' ");
    }

    print.print("}");
}
#endif // _USE_STACK

void OnExit()
{
    DebugStream->println("Program exitted!");
    DebugTools.PrintDebugInfo();
    DebugStream->println();
    DebugTools.PrintStack();
}

DebugToolsClass::DebugToolsClass()
{
    //watchdogDisable();
#ifdef _USE_STACK
    m_debugInfo = (DebugInfo*)malloc(sizeof(DebugInfo));
    m_callStack = (Stack_t*)malloc(sizeof(Stack_t));
#endif // _USE_STACK
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(14, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    stopPoint = (size_t*)malloc(sizeof(size_t));
    //NotifyMemoryAllocation(sizeof(DebugInfo));
    //m_callStack = (Stack_t*)malloc(sizeof(Stack_t));
    //NotifyMemoryAllocation(sizeof(Stack_t));
}



void DebugToolsClass::Flash()
{

    if (millis() < m_nextFlash) return;
    digitalWrite(14, m_LEDState);
    m_nextFlash = millis() + 250;
    m_LEDState != m_LEDState;
}
#endif
DebugToolsClass DebugTools;

