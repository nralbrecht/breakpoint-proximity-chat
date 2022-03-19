#pragma once

#include "dto.h"
#include "sqlite3.h"


class PositionLogger
{
private:
    sqlite3 *database;
public:
    PositionLogger();
    ~PositionLogger();

    void persist(DTO::ServerStateReport report);
};
