#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <cstdarg>
#include <ostream>
#include <sstream>
#include <stdio.h>

/**
 * Simple Logging framework
 *
 * Supports C-style logging using the LOG_<LEVEL> printf-style macros e. g.:
 *
 *     LOG_WARNING("Error running something: %s", message);
 *
 * C++ ostream logging uses the <LEVEL>_LOG macros e. g.;
 * 
 *     WARNING_LOG << "Error running something " << message;
 */


enum class LogLevel {
    BadLevel,
    Error,
    Warning,
    Notice,
    Info,
    Debug,
    Trace
};


/** The underpinning logfile object, a singleton. */
class LogBackend
{
    public :
        static LogBackend& getInstance();

        /** Set logfile path and dump buffered data onto it. */
        bool setLogfile(const char* path);

        /** Write some data to log or buffer it until there is a log file. */
        void write(const char* s);

        LogBackend(const LogBackend&) = delete;
        void operator=(const LogBackend&) = delete;

        ~LogBackend();

    private:
        LogBackend();
        FILE* f;
        std::stringstream buffer;
        bool isBuffering;
};

/** Transient logger class, instantiated by the *LOG* macros. */
class Logger
{
    public:
        Logger();
        ~Logger();
        void write(LogLevel level, const char* file, int line,
                   const char* fmt, ...);
        void write(LogLevel level, const char* file, int line,
                   wxString fmt, ...);
        std::ostream& get(LogLevel level = LogLevel::Info);
        std::ostream& get(LogLevel level, const char* path, int line);
        static LogLevel getLevel();
        static void setLevel(LogLevel level);
        static LogLevel string2level(const char* level);

    protected:
        std::stringstream os;

};


#define LOG_MESSAGE(level, fmt, ...)  { \
    if (level  <= Logger::getLevel()) { \
        Logger().write(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } \
}

#define LOG(level) \
    if (level > Logger::getLevel()) ;  \
    else Logger().get(level, __FILE__, __LINE__)


#define TRACE_LOG    LOG(LogLevel::Trace)
#define DEBUG_LOG    LOG(LogLevel::Debug)
#define INFO_LOG     LOG(LogLevel::Info)
#define NOTICE_LOG   LOG(LogLevel::Notice)
#define WARNING_LOG  LOG(LogLevel::Warning)
#define ERROR_LOG    LOG(LogLevel::Error)

#define LOG_TRACE(fmt, ...)   LOG_MESSAGE(LogLevel::Trace, fmt, ##__VA_ARGS__);
#define LOG_DEBUG(fmt, ...)   LOG_MESSAGE(LogLevel::Debug, fmt, ##__VA_ARGS__);
#define LOG_INFO(fmt, ...)    LOG_MESSAGE(LogLevel::Info, fmt, ##__VA_ARGS__);
#define LOG_NOTICE(fmt, ...)  LOG_MESSAGE(LogLevel::Notice, fmt, ##__VA_ARGS__);
#define LOG_WARNING(fmt, ...) LOG_MESSAGE(LogLevel::Warning, fmt, ##__VA_ARGS__);
#define LOG_ERROR(fmt, ...)   LOG_MESSAGE(LogLevel::Error, fmt, ##__VA_ARGS__);


#endif   // LOGGER_H
