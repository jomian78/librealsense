#include "types.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <ctime>

rs_log_severity rsimpl::minimum_log_severity = RS_LOG_SEVERITY_NONE;
static rs_log_severity minimum_console_severity = RS_LOG_SEVERITY_NONE;
static rs_log_severity minimum_file_severity = RS_LOG_SEVERITY_NONE;
static std::ofstream log_file;
static std::mutex log_mutex;

void rsimpl::log(rs_log_severity severity, const std::string & message)
{
    std::lock_guard<std::mutex> lock(log_mutex);

    if(static_cast<int>(severity) < minimum_log_severity) return;

    std::time_t t = std::time(nullptr); char buffer[20] = {}; const tm* time = std::localtime(&t);
    if (nullptr!=time)
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time);

    if(severity >= minimum_file_severity)
    {
        switch(severity)
        {
        case RS_LOG_SEVERITY_DEBUG: log_file << buffer << " DEBUG: " << message << "\n"; break;
        case RS_LOG_SEVERITY_INFO:  log_file << buffer << " INFO: "  << message << "\n"; break;
        case RS_LOG_SEVERITY_WARN:  log_file << buffer << " WARN: "  << message << "\n"; break;
        case RS_LOG_SEVERITY_ERROR: log_file << buffer << " ERROR: " << message << "\n"; break;
        case RS_LOG_SEVERITY_FATAL: log_file << buffer << " FATAL: " << message << "\n"; break;
        default: throw std::logic_error("not a valid severity for log message");
        }
    }

    if(severity >= minimum_console_severity)
    {
        switch(severity)
        {
        case RS_LOG_SEVERITY_DEBUG: std::cout << "rs.debug: " << message << "\n"; break;
        case RS_LOG_SEVERITY_INFO:  std::cout << "rs.info: "  << message << "\n"; break;
        case RS_LOG_SEVERITY_WARN:  std::cout << "rs.warn: "  << message << "\n"; break;
        case RS_LOG_SEVERITY_ERROR: std::cout << "rs.error: " << message << "\n"; break;
        case RS_LOG_SEVERITY_FATAL: std::cout << "rs.fatal: " << message << "\n"; break;
        default: throw std::logic_error("not a valid severity for log message");
        }
    }
}

void rsimpl::log_to_console(rs_log_severity min_severity)
{
    minimum_console_severity = min_severity;
    rsimpl::minimum_log_severity = std::min(minimum_console_severity, minimum_file_severity);
}

void rsimpl::log_to_file(rs_log_severity min_severity, const char * file_path)
{
    minimum_file_severity = min_severity;
    log_file.open(file_path, std::ostream::out | std::ostream::app);
    rsimpl::minimum_log_severity = std::min(minimum_console_severity, minimum_file_severity);
}
