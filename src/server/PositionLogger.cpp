#include "PositionLogger.h"


PositionLogger::PositionLogger() {
    int rc = sqlite3_open("positions.db", &database);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(database));
        sqlite3_close(database);
        database = nullptr;
    }

    char *zErrMsg = nullptr;
    rc = sqlite3_exec(database,
        "CREATE TABLE positions (timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, uuid TEXT NOT NULL, x REAL NOT NULL, y REAL NOT NULL, z REAL NOT NULL);",
        nullptr, 0, &zErrMsg);

    if(rc != SQLITE_OK) {
        fprintf(stderr, "Database could not be created or already exists: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

PositionLogger::~PositionLogger() {
    sqlite3_close(database);
}


void PositionLogger::persist(DTO::ServerStateReport report) {
    char *zErrMsg = nullptr;

    for (const DTO::ClientState& client : report) {
        int rc = sqlite3_exec(database,
            ("INSERT into positions (uuid, x, y, z) VALUES ('" + client.uuid + "', '" + std::to_string(client.x) + "', '" + std::to_string(client.y) + "', '" + std::to_string(client.z) + "')").c_str(),
            nullptr, 0, &zErrMsg);

        if(rc != SQLITE_OK) {
            fprintf(stderr, "could not insert position: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }
}
