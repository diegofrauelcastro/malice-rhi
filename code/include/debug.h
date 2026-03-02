#pragma once

#include <iostream>
#include <fstream>
#include <string>

#ifdef NDEBUG
// Only write to the log file, without printing to the console.
#define LOG_RHI(_str, ...) { MaliceRHI::Debug::Log::GetInstance()->PrintRHI_Log(_str, __VA_ARGS__); }

// Only write to the log file, without printing to the console.
#define LOG_CLEAN(_str, ...) { MaliceRHI::Debug::Log::GetInstance()->PrintClean(_str, __VA_ARGS__); }
#else
// Used for : MACRO DEBUG_LOG (RHI logs)
#define LOG_RHI(_str, ...) { std::cout << "[RHI LOG] : " << MaliceRHI::Debug::Log::GetInstance()->PrintRHI_Log(_str, __VA_ARGS__) << std::endl; }

// Used for : MACRO DEBUG_LOG (user logs)
#define LOG_CLEAN(_str, ...) { std::cout << MaliceRHI::Debug::Log::GetInstance()->PrintClean(_str, __VA_ARGS__) << std::endl; }
#endif

// Used for : MACRO DEBUG_LOG (user logs)
#define LOG_DEBUG(_str, ...) { std::cout << "[USER DEBUG LOG] : " << MaliceRHI::Debug::Log::GetInstance()->PrintDebug(_str, __VA_ARGS__) << std::endl; }

// Used for : MACRO THROW EXCEPTION (RHI error logs)
#define LOG_THROW(_str, ...) { std::string thrownText = MaliceRHI::Debug::Log::GetInstance()->PrintRHI_Throw(_str, __VA_ARGS__); std::cout << "\n\n" << __FILE__ << " (l." << __LINE__ << ") " << "[RHI EXCEPTION] : \n" << thrownText << std::endl; throw std::runtime_error(thrownText); }


// Max size of a log.
#define LOG_MAX_SIZE 1024


namespace MaliceRHI
{

	namespace Debug
	{
		enum LogType
		{
			NONE,
			USER_LOG,
			RHI_LOG,
			RHI_THROW
		};

        class Log
		{
        private:
			Log(std::string _fileName="log.txt");

            static Log* singleton;
			std::ofstream f;
			bool bIsCurrentlyDestroying = false;
        public:
            static Log* GetInstance();

			void OpenFile(std::string _fileName);
			std::string PrintDebug(const char* _format, ...);
			std::string PrintRHI_Log(const char* _format, ...);
			std::string PrintRHI_Throw(const char* _format, ...);
			std::string PrintClean(const char* _format, ...);
			void WriteInFile(const char* _format, LogType _type, bool _showTimestamp = true);

            ~Log();
            void Destroy();
        };
	}
}

