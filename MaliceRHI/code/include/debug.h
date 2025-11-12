#pragma once

#include <iostream>
#include <fstream>
#include <string>

#ifdef NDEBUG
// UNUSABLE IF NOT IN DEBUG MODE
#define LOG_RHI(_str, ...) { }

// UNUSABLE IF NOT IN DEBUG MODE
#define LOG_THROW(_str, ...) { }

// UNUSABLE IF NOT IN DEBUG MODE
#define LOG_DEBUG(_str, ...) { }

// UNUSABLE IF NOT IN DEBUG MODE
#define LOG_CLEAN(_str, ...) { }
#else
// Used for : MACRO DEBUG_LOG (RHI logs)
#define LOG_RHI(_str, ...) { std::cout << "[RHI LOG] : " << MaliceRHI::Debug::Log::GetInstance()->PrintRHI_Log(_str, __VA_ARGS__) << std::endl; }

// Used for : MACRO THROW EXCEPTION (RHI logs)
#define LOG_THROW(_str, ...) { std::string thrownText = MaliceRHI::Debug::Log::GetInstance()->PrintRHI_Throw(_str, __VA_ARGS__); std::cout << "\n\n" << __FILE__ << " (l." << __LINE__ << ") " << "[RHI EXCEPTION] : " << thrownText << std::endl; throw std::runtime_error(thrownText); }

// Used for : MACRO DEBUG_LOG (user logs)
#define LOG_DEBUG(_str, ...) { std::cout << "[USER DEBUG LOG] : " << MaliceRHI::Debug::Log::GetInstance()->PrintDebug(_str, __VA_ARGS__) << std::endl; }

// Used for : MACRO DEBUG_LOG (user logs)
#define LOG_CLEAN(_str, ...) { std::cout << MaliceRHI::Debug::Log::GetInstance()->PrintClean(_str, __VA_ARGS__) << std::endl; }
#endif

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

