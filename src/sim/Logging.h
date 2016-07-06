#pragma once

// These must be declared before we include easylogging++.h
#define ELPP_STL_LOGGING
#define ELPP_THREAD_SAFE
#define ELPP_NO_DEFAULT_LOG_FILE
#define ELPP_DISABLE_DEFAULT_CRASH_HANDLING

#include <easyloggingpp/easylogging++.h>
#include <QMap>
#include <QPair>

// To print and log something, simply provide the level and use printf-like syntax:
//
// L()->info("Message");
// Logging::mazeLogger()->warn("Message %v %v %v", 'c', "foo", 4);
//
// Note that we you should use %v for arguments of all types, and that the
// string must a string literal (no QStrings are allowed). You can write
// multiline logging statements as follows:
//
// Logging::mouseLogger()->debug(
//     "Message %v %v %v. This is really long and really should be on"
//     " at least two lines because it wouldn't fit on just a single line.",
//     'c', "foo", 4);
//
// The valid methods on the logger are debug, info, warn, and error. There are
// others, but those are the main ones you should use.

namespace sim {

// Wrapper for the static call to Logging::getSimLogger()
class Logging;
el::Logger* L();

class Logging {

public:

    // The logging class is not constructible
    Logging() = delete;

    // Accessors for our loggers 
    static el::Logger* getSimLogger();
    static el::Logger* getMazeLogger();
    static el::Logger* getMouseLogger();

    // Initializes all of the loggers, should only be called once
    static void initialize(const QString& runId);

private:

    // Used to determine part of the log file paths
    static QString m_runId;

    // The names of each of our loggers
    static QString m_simLoggerName;
    static QString m_mazeLoggerName;
    static QString m_mouseLoggerName;

    // A map of (loggerName) -> (path, numLogFiles)
    static QMap<QString, QPair<QString, int>> m_info;

    // Helper method for retrieving a particular logger
    static el::Logger* getLogger(const QString& loggerName);

    // Easy function for getting the next available log file name
    static QString getNextFileName(const char* filename);

    // Perform an action when files get too large
    static void rolloutHandler(const char* filename, std::size_t size);
};

} // namespace sim
