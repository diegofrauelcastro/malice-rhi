#include "debug.h"
using namespace MaliceRHI::Debug;

#include <stdio.h>
#include <stdarg.h>
#include <chrono>
#include <ctime>
#include <iomanip>

Log* Log::singleton = nullptr;

Log::Log(std::string fileName)
{
    OpenFile(fileName);
}

Log::~Log()
{
    Destroy();
}

void Log::Destroy()
{
    if (f.is_open())
    {
        std::cout << "[LOGGER] " << PrintClean("Closing log file...") << std::endl;
        f.close();
        f.clear();
    }

    if (singleton != nullptr && !bIsCurrentlyDestroying)
    {
        bIsCurrentlyDestroying = true;
        delete singleton;
        singleton = nullptr;
    }
}

Log* Log::GetInstance()
{
    if (singleton == nullptr)
        singleton = new Log();
    return singleton;
}

void Log::OpenFile(std::string _fileName)
{
    if (!f.is_open())
    {
        f.open(_fileName);
        if (!f.is_open()) std::cout << "Couldn't open file " << _fileName << "" << std::endl;
        else std::cout << "[LOGGER] " << PrintClean("Opened Log file.") << std::endl;
    }
    else
    {
        std::string s = "Tried to open file of name \"" + _fileName + "\", however it seems another program is currently using it.";
        std::cout << "[LOGGER] " << PrintClean(s.c_str()) << std::endl;
    }
}

std::string Log::PrintDebug(const char* _format, ...)
{
    char buffer[LOG_MAX_SIZE];
    va_list args;
    va_start(args, _format);
    vsnprintf(buffer, LOG_MAX_SIZE, _format, args);
    WriteInFile(buffer, LogType::USER_LOG);
    va_end(args);
    return buffer;
}

std::string Log::PrintRHI_Log(const char* _format, ...)
{
    char buffer[LOG_MAX_SIZE];
    va_list args;
    va_start(args, _format);
    vsnprintf(buffer, LOG_MAX_SIZE, _format, args);
    WriteInFile(buffer, LogType::RHI_LOG);
    va_end(args);
    return buffer;
}

std::string Log::PrintRHI_Throw(const char* _format, ...)
{
    char buffer[LOG_MAX_SIZE];
    va_list args;
    va_start(args, _format);
    vsnprintf(buffer, LOG_MAX_SIZE, _format, args);
    WriteInFile(buffer, LogType::RHI_THROW);
    va_end(args);
    return buffer;
}

std::string Log::PrintClean(const char* _format, ...)
{
    char buffer[LOG_MAX_SIZE];
    va_list args;
    va_start(args, _format);
    vsnprintf(buffer, LOG_MAX_SIZE, _format, args);
    WriteInFile(buffer, LogType::NONE, false);
    va_end(args);
    return buffer;
}

void Log::WriteInFile(const char* _format, LogType _type, bool _showTimestamp)
{
    if (!f.is_open()) return;

    std::string logType = "";
    // Write the given message with/without the correct header.
    switch (_type)
    {
    case MaliceRHI::Debug::USER_LOG:
        logType = "[USER DEBUG LOG] : ";
        break;
    case MaliceRHI::Debug::RHI_LOG:
        logType = "[RHI LOG] : ";
        break;
    case MaliceRHI::Debug::RHI_THROW:
        logType = "[RHI EXCEPTION] : ";
        f << "\n\n";
        break;
    case NONE:
    default:
        break;
    }

    // Write hour, minute, seconds and milliseconds.
    if (_showTimestamp)
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm;
        localtime_s(&now_tm, &now_time_t);
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        f << "[" << std::put_time(&now_tm, "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count() << "] ";
    }
    // Log type
    f << logType;
    // Text
    f << _format;
    // Apply line jump.
    f << std::endl;

    // Flush file.
    f.flush();
}

