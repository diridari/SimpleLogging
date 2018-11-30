/*
 * Logging.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: Sebastian Balz
 */

#include "logging.h"
#include <fstream>
#include <mutex>

using namespace std::chrono;

static LogLevel level = Error;
static LogLevel logFileLevel = None; // create no log file if not requested
static fstream *logWriter;
static bool loggerIsOpen = false;
static bool firstTimeStamp = true;
static string logfile = "log.log";
static system_clock::time_point startTime;
static mutex locker;
static loggerPrivateConfig config =  loggerPrivateConfig();
static unsigned int pointsOfInterest = 0;
static LogLevel printLineIf = Error;

static string getTimeStamp()
{
    if (firstTimeStamp)
    {
        firstTimeStamp = false;
        startTime = chrono::high_resolution_clock::now();
    }
    auto current_time = std::chrono::high_resolution_clock::now();
    string s = to_string(std::chrono::duration_cast<std::chrono::seconds>(current_time - startTime).count()) + ":";
    s += to_string(std::chrono::duration_cast<std::chrono::milliseconds>(current_time - startTime).count() % 1000);
    while (s.length() < 6)
        s += " "; // Append blanks for a better appearance
    s += "\t";
    return s;

}


void openLogWriter(fstream **writer) {
    *writer = new fstream();
    (*writer)->open(logfile, ofstream::trunc | ofstream::out);
    if (level != None) {
        cout << "write logfile to " << logfile<< endl;
    }
    loggerIsOpen = true;
}



void Log::log_(string src, string message, LogLevel l,string name, int line, unsigned int interest) {
    if(isInPointOfInterest(interest)) {
        locker.lock();
        src = config.handleSrc(src);
        string log;
        if(l != UserInfo)
            log  = getTimeStamp() + logLevelToString(l) + src + message;
        else
            log = message;

        if (l <= printLineIf && l != LogLevel::UserInfo)
            log += "\n\t\t\t\t from :\t " + (name) + ":" + to_string(line);
        // write to logfile
        if (l <= logFileLevel) {
            if (!loggerIsOpen)
                openLogWriter(&logWriter);
            *logWriter << log << endl;
            logWriter->flush();
        }
        // write to cli

        if (l <= level) {
            highlight(l);
            cout << log << "\n";
            if (highlight) {
                resetCLIToDefault();
            }
        }
        locker.unlock();
    }
}

void Log::log_(string message, LogLevel l){
    log_("",message,l,"",-1);
}


void Log::setLogLevel(LogLevel cli, LogLevel file)
{
    level = cli;
    logFileLevel = file;
}

void Log::setLogLevel(LogLevel cliAndFile)
{
    setLogLevel(cliAndFile, cliAndFile);
}


void Log::setLogFileName(string fileName){
    locker.lock();
    if(loggerIsOpen) {
        loggerIsOpen = false;
        if (level != None) {
            *logWriter << "change Logfile to " << fileName << endl;
        }
        logWriter->close();
    }
    logfile = fileName;
    locker.unlock();
}
string Log::logLevelToString(LogLevel l)
{
    switch (l)
    {
        case CriticError:
            return ":CriticError  : ";
        case Error:
            return ":  Error      : ";
        case Info:
            return ":      info   : ";
        case Message:
            return ":    Message  : ";
        case UserInfo:
            return ":User_info    : ";
        case None:
            return "";
        case Debug:
            return ":      Debug  : ";
        case DebugL2:
            return ":    Debug L2 : ";
        case DebugL3:
            return ":     Debug L3: ";
        default	: return "unknown Level";
    }

}

void Log::resetCLIToDefault() {
#ifdef __linux__
    cout << DISABLE_CLI_HIGHLIGHT;
#elif __WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15 );
#endif
}

#ifdef __WIN32
void Log::highlight(LogLevel l) {
    if (config.isHighlight()) {
        switch (l) {
            case None:
                break;;
            case UserInfo:
                break;
            case CriticError:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 78);
                break;
            case Error:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
                break;
            case Message:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);
                break;
            case Info:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
                break;
            case Debug:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
                break;
            case DebugL2:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
                break;
            case DebugL3:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
                break;
        }
    }
}
#endif

#ifdef __linux__
void Log::highlight(LogLevel l) {
    if(config.isHighlight()) {
        switch (l) {
            case None: break;
            case UserInfo: break;
            case CriticError:
                cout<< "\033[1;31m";break;
            case Error:
                cout<< "\033[1;32m";break;
            case Message:
                cout<< "\033[0;33m";break;
            case Info:
                cout<< "\033[0;34m";break;
            case Debug:
                cout<< "\033[0;37m";break;
            case DebugL2:
                cout<< "\033[0;37m";break;
            case DebugL3:
                cout<< "\033[2;37m";break;
        }

    }
}
#endif


void Log::setLogLevel(int cli, int file) {
    setLogLevel(IntToLogLevel(cli),IntToLogLevel(file));
}

LogLevel Log::IntToLogLevel(int i) {
    switch (i){
        case    -1  : return None;
        case    0   : return UserInfo;
        case    1   : return CriticError;
        case    2   : return Error;
        case    3   : return Message;
        case    4   : return Info;
        case    5   : return Debug;
        case    6   : return DebugL2;
        case    7   : return DebugL3;
        default:
            cerr << "Log :: change Loglevel from int with un undefined value: allowed values ar <-1...7>  set logLevel to Debug"<< endl;
            return Debug;
    }
}

void Log::setLogLevel(int cliAndFile) {
    setLogLevel(IntToLogLevel(cliAndFile));

}



advancedConfiguration * Log::advancedConf() {
    return &config;
}

void Log::setLogLevel(string cli, string file) {
    setLogLevel(stringToLogLevel(cli),stringToLogLevel(file));
}

LogLevel Log::stringToLogLevel(string toConvert) {

    if(toConvert == "None")         return None;
    if(toConvert == "UserInfo")     return UserInfo;
    if(toConvert == "CriticError")  return CriticError;
    if(toConvert == "Error")        return Error;
    if(toConvert == "Message")      return Message;
    if(toConvert == "Info")         return Info;
    if(toConvert == "Debug")        return Debug;
    if(toConvert == "DebugL2")      return DebugL2;
    if(toConvert == "DebugL3")      return DebugL3;

    Log::log("unknown loglevel string "  + toConvert,CriticError);
    return DebugL3;
}

void Log::setLogLevel(string cliAndFile) {
    setLogLevel(stringToLogLevel(cliAndFile));
}

void Log::setPointOfInterest(unsigned int points) {
    pointsOfInterest = points;
}

bool Log::isInPointOfInterest(unsigned int toCheck) {
    return (toCheck & pointsOfInterest) >0 || toCheck == 0 || pointsOfInterest == 0;
}

void Log::printSrcLine(LogLevel l) {
    printLineIf = l;
}


