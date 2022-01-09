/*
 Name:		DebugTools.h
 Created:	8/25/2021 12:24:24 PM
 Author:	civel
 Editor:	http://www.visualmicro.com
*/

#ifndef _DebugTools_h
#define _DebugTools_h


/*
 Defines:
	_DEBUG <lvl>:
		Allow debugging at the specified level (lvl)
		lvl:
			0  : nothing
			1  : Errors
			2  : Values
			>2 : Other
	_USE_WDT:
		Allow use of watchdog
	_USE_STACK
		Allow use of stack
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#if defined(_DEBUG) && defined(ARDUINO_ARCH_MEGAAVR)
#include "malloc.h"
#endif

#define DEBUG_TOOLS_FILE_NAME_LENGTH 32
#define DEBUG_TOOLS_STEP_NAME_LENGTH 32
#define DEBUG_TOOLS_FUNC_NAME_LENGTH 32

#ifdef ARDUINO_ARCH_SAM
#define DEBUG_TOOLS_STACK_LENGTH 32
#else
#define DEBUG_TOOLS_STACK_LENGTH 16
#endif

#ifndef _DEBUG
#undef _USE_WDT
#undef _USE_STACK
#endif


#define WDT_KEY (0xA5)

enum Timeout {
	TIMEOUT_15MS = 15,
	TIMEOUT_30MS = 30,
	TIMEOUT_60MS = 60,
	TIMEOUT_120MS = 120,
	TIMEOUT_250MS = 250,
	TIMEOUT_500MS = 500,
	TIMEOUT_1S = 1000,
	TIMEOUT_2S = 2000,
	TIMEOUT_4S = 4000,
	TIMEOUT_8S = 8000
};
enum DebugInfoDisplayFlags
{
	MentionFile = 0b00000001,
	MentionLine = 0b00000010,
	MentionName = 0b00000100,
	MentionFunc = 0b00001000,
	MentionCount = 0b00010000,
	MentionTrace = 0b00100000,
	MentionMemory = 0b01000000,
	MentionDefault = 0b01101111,
	MentionAll = 0b01111111,
	MentionMinimal = 0b00001110,
	MentionLoop = 0b01111100,
	MentionDetails = 0b01101100,
	MentionStack = 0b00011111,
};

enum DebugInfoTypeFlags
{
	HasName = 0b00000111,
	HasFunc = 0b00001011,
	HasCount = 0b00010011,
	IsTrace = 0b00100011, // If meant to be used as a call trace
};

#ifdef _USE_STACK
class ISizeable
{
public:
	virtual size_t Size() const = 0;
};



struct DebugInfo
{
	DebugInfoTypeFlags TypeFlags;
	uint16_t Line;
	uint16_t Count;
	int MemLeft;
	bool DisplayOnNextRun;

	char FileName[DEBUG_TOOLS_FILE_NAME_LENGTH];
	char StepName[DEBUG_TOOLS_STEP_NAME_LENGTH];
	char FunctionName[DEBUG_TOOLS_FUNC_NAME_LENGTH];

	void Display(Print& print, DebugInfoDisplayFlags displayFlags);
};

template <typename T>
class Stack
{
private:
	T* m_arr = nullptr;
	size_t m_capacity;
	size_t m_count = 0;
public:
	Stack(size_t maxLength);
	void push_back(const T& item);
	void pop_back();
	T& back();
	T& operator[](size_t index);
	size_t size();
	T* begin();
	T* end();
	~Stack();
};

#endif // _USE_STACK

extern const size_t MaxMem;


#ifdef _DEBUG
extern Stream* DebugStream;


#define DebugLvl(lvl, message) if (_DEBUG >= lvl) DebugStream->print(message)
#define DebugLvlln(lvl, message) if (_DEBUG >= lvl) DebugStream->println(message)

#if _DEBUG >= 1
#define DebugError(errorMessage) DebugStream->print("[ERROR]");\
DebugStream->print(__FUNCTION__);\
DebugStream->print(" Line ");\
DebugStream->print(__LINE__);\
DebugStream->print(" ");\
DebugStream->println(errorMessage);\
DebugStream->flush()
#else
#define DebugError(errorMessage)
#endif // _DEBUG >= 1

#if _DEBUG >= 2

#define DebugMemAddress(value) DebugStream->print("[MEM]");\
DebugStream->print(__FUNCTION__);\
DebugStream->print(" address ");\
DebugStream->print(#value);\
DebugStream->print(": ");\
DebugStream->println(reinterpret_cast<uint32_t>(value));\
DebugStream->flush()

#define DebugValue(value) DebugStream->print("[DEBUG]");\
DebugStream->print(__FUNCTION__);\
DebugStream->print(" ");\
DebugStream->print(#value);\
DebugStream->print(": ");\
DebugStream->println(value);\
DebugStream->flush()

#else
#define DebugMemAddress(value)
#define DebugValue(value)
#endif //_DEBUG >= 2
#else
#define DebugMemAddress(value)
#define DebugValue(value)
#define DebugError(errorMessage)
#endif //_DEBUG

#ifdef _USE_STACK
template<typename T>
inline Stack<T>::Stack(size_t maxLength)
{
	m_arr = new T[maxLength];
	for (size_t i = 0; i < maxLength; i++)
	{
		m_arr[i] = T();
	}
	m_capacity = maxLength;
}

template<typename T>
inline void Stack<T>::push_back(const T& item)
{
	if (m_count >= m_capacity)
	{
		DebugError("Stack full");
		exit(0);
		return;
	}
	m_arr[m_count] = item;
	m_count++;
}

template<typename T>
inline void Stack<T>::pop_back()
{
	--m_count;
}

template<typename T>
inline T& Stack<T>::back()
{
	return m_arr[m_count];
}

template<typename T>
inline T& Stack<T>::operator[](size_t index)
{
	return m_arr[index];
}

template<typename T>
inline size_t Stack<T>::size()
{
	return m_count;
}

template<typename T>
inline T* Stack<T>::begin()
{
	return m_arr;
}

template<typename T>
inline T* Stack<T>::end()
{
	return m_arr + m_count + 1;
}

template<typename T>
inline Stack<T>::~Stack()
{
	delete[] m_arr;
}
#endif //_USE_STACK

#ifdef _DEBUG


#define CONCAT_TOKEN1(x, y) x##y

#define CONCAT_TOKEN2(x, y) CONCAT_TOKEN1(x, y)

#ifdef _USE_WDT

//Call often to reset the watchdog and indicate a new step.
//stepName must be less than 32 characters.
#define DebugToolsStep(stepName) DebugTools.ResetWatchdog(__LINE__, __FILE__, __FUNCTION__, stepName)

#else

#define DebugToolsStep(stepName)

#endif //_USE_WDT

#ifdef _USE_STACK

#define DebugToolsFunctionBegin() auto CONCAT_TOKEN2(_reserved_var, __COUNTER__) = TraceObject(__LINE__, __FILE__, __FUNCTION__, false)

#define DebugToolsFunctionBeginAlloc(memoryAllowed) auto CONCAT_TOKEN2(_reserved_var, __COUNTER__) = TraceObject(__LINE__, __FILE__, __FUNCTION__, false, memoryAllowed)

#define DebugToolsFunctionBeginNoWarn() auto CONCAT_TOKEN2(_reserved_var, __COUNTER__) = TraceObject(__LINE__, __FILE__, __FUNCTION__, true)

#else

#define DebugToolsFunctionBegin()

#define DebugToolsFunctionBeginAlloc(memoryAllowed)

#define DebugToolsFunctionBeginNoWarn()

#endif // _USE_STACK

#define DeclareNew(type, ...) new type(__VA_ARGS__); DebugTools.NotifyMemoryAllocation(sizeof(type))

#define DeclareDelete(type) DebugTools.NotifyMemoryFree(sizeof(type)); delete 

#define DeclareDeleteSize(size) DebugTools.NotifyMemoryFree(size); delete 

#define DeclareNewArr(type, size) new type[size]{}; DebugTools.NotifyMemoryAllocation(sizeof(type) * size)

#define DeclareDeleteArr(type, size) DebugTools.NotifyMemoryFree(sizeof(type) * size); delete[] 

#define DEFAULT_IF_NO_DEBUG ;

#else

#define DebugLvl(lvl, message)
#define DebugLvlln(lvl, message)
#define DebugToolsStep(stepName)

#define DebugToolsFunctionBegin()

#define DebugToolsFunctionBeginAlloc(memoryAllowed)

#define DebugToolsFunctionBeginNoWarn()

#define DeclareNew(type, ...) new type(__VA_ARGS__)

#define DeclareDelete(type) delete 

#define DeclareDeleteSize(size) delete 

#define DeclareNewArr(type, size) new type[size]{}

#define DeclareDeleteArr(type, size) delete[] 

#define DEFAULT_IF_NO_DEBUG {}

#endif //_DEBUG

#ifdef _USE_WDT
#define DEFAULT_IF_NO_WDT ;
#else
#define DEFAULT_IF_NO_WDT {}
#endif // _USE_WDT

#ifdef _USE_STACK
#define DEFAULT_IF_NO_STACK ;
#else
#define DEFAULT_IF_NO_STACK {}
#endif // _USE_STACK

class DebugToolsClass
{
private:
#ifdef _USE_STACK
	using Stack_t = Stack<DebugInfo>;
	DebugInfo* m_debugInfo;
	Stack_t* m_callStack;
#endif // _USE_STACK
	int m_allocated;
	uint8_t m_LEDState = 0;
	uint32_t m_nextFlash = 0;
public:
	DebugToolsClass() DEFAULT_IF_NO_DEBUG
		void SetDebugOutput(Stream* output) DEFAULT_IF_NO_DEBUG
		Stream* GetDebugOutput()DEFAULT_IF_NO_DEBUG
		size_t GetFreeMem()DEFAULT_IF_NO_DEBUG
		void PrintCurrentlyAllocatedMemory()DEFAULT_IF_NO_DEBUG

		/// <summary>
		/// Warning, this will override atexit(), exit the program and wipe the entire ram (flash, aka program, will not be erased)
		/// </summary>
		void CleanMemory()DEFAULT_IF_NO_DEBUG
		void NotifyMemoryAllocation(int size)DEFAULT_IF_NO_DEBUG
		void NotifyMemoryFree(int size)DEFAULT_IF_NO_DEBUG
		int GetAllocatedMemory()DEFAULT_IF_NO_DEBUG

		void DisplayMemory(size_t columns)DEFAULT_IF_NO_DEBUG
		void DisplayMemoryContent(size_t columns)DEFAULT_IF_NO_DEBUG
		void Flash()DEFAULT_IF_NO_DEBUG

		/// <summary>
		/// Prints the last debug update from <see cref="ResetWatchdog"/>
		/// </summary>
		void PrintDebugInfo(DebugInfoDisplayFlags displayFlags = DebugInfoDisplayFlags::MentionDefault)DEFAULT_IF_NO_STACK
		void SetupWatchdog(uint32_t ms)DEFAULT_IF_NO_WDT
		void FunctionEnter(uint16_t line, const char* fileName, const char* funcName)DEFAULT_IF_NO_DEBUG
		void FunctionExit(int memLeft, bool suppressWarning = true, int memAllowed = 0)DEFAULT_IF_NO_DEBUG
		//void LogStep(uint32_t line, const char* fileName, );
		void ResetWatchdog(uint16_t line, const char* fileName, const char* funcName, const char* stepName)DEFAULT_IF_NO_WDT
		bool WasLastResetFromWatchdog()DEFAULT_IF_NO_WDT
		void PrintStack()DEFAULT_IF_NO_STACK

};

extern DebugToolsClass DebugTools;

#ifdef _USE_STACK
struct TraceObject
{
	int MemLeft;
	int MemAllowed;
	bool SuppressWarning;
	TraceObject(uint16_t line, const char* fileName, const char* funcName, bool suppressWarning = true, int memoryAllowed = 0)
	{
		DebugTools.FunctionEnter(line, fileName, funcName);
		MemLeft = DebugTools.GetAllocatedMemory();
		MemAllowed = memoryAllowed;
		SuppressWarning = suppressWarning;
	}

	~TraceObject()
	{
		DebugTools.FunctionExit(MemLeft, SuppressWarning, MemAllowed);
	}
};
#endif

#endif