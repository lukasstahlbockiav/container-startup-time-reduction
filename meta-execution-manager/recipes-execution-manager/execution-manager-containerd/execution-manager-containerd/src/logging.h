/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef LOGGING_H
#define LOGGING_H

#include <chrono>
#include <iostream>
#include <iomanip>

enum logLevel : uint8_t {info = 0, debug = 1, warning = 2, error = 3};

void createLogMessage(const logLevel& logLevel_, const std::string& msg_);

#endif /* LOGGING_H */