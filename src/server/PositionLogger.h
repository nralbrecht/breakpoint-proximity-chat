#pragma once

#include <iostream>

#include "dto.h"
#include "sqlite3.h"


class PositionLogger
{
private:
    sqlite3 *database;
public:
    PositionLogger(std::string filename);
    ~PositionLogger();

    void persist(DTO::ServerStateReport report);
};
