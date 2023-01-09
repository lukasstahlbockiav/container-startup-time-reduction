/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#include "logging.h"
#include <mutex>

std::mutex coutMutex;

void createLogMessage(const logLevel& logLevel_, const std::string& msg_){
    const char* sLogLevel = 0;
    #define PROCESS_VAL(p) case(p): sLogLevel = #p; break;
        switch(logLevel_){
            PROCESS_VAL(info);
            PROCESS_VAL(debug);
            PROCESS_VAL(warning);
            PROCESS_VAL(error);
        }
    #undef PROCESS_VAL

    auto when_ = std::chrono::high_resolution_clock::now();
    auto its_time_t = std::chrono::high_resolution_clock::to_time_t(when_);
    auto its_time = std::localtime(&its_time_t);
    auto its_ns = (when_.time_since_epoch().count()) % 1000000000;
    coutMutex.lock();
    std::cout
        << std::dec << std::setw(4) << its_time->tm_year + 1900 << "-"
        << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_mon << "-"
        << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_mday << " "
        << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_hour << ":"
        << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_min << ":"
        << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_sec << "."
        << std::dec << std::setw(9) << std::setfill('0') << its_ns << " [" << sLogLevel << "] " << msg_ 
        << std::endl;
    coutMutex.unlock();
}