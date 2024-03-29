#include <fstream>
#include <iostream>
#ifdef _WIN32
    #include <conio.h>
#endif

#include "asio2/tcp/tcp_server.hpp"
#include "nlohmann/json.hpp"
#include "PositionLogger.h"
#include "StateManager.h"
#include "dto.h"

const std::string CurrentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

int main(int argc, char const *argv[]) {
    int port = 9099;
    int reportLogFrequency = 2000;
    int reportSendFrequency = 250;
    long recievedMessageCount = 0;
    int positionLogFrequency = 1000;
    std::string positionLogFile = "positions.db";


    if (argc > 2) {
        positionLogFile = std::string(argv[2]);
    }
    else {
        std::cout << "Position log file path not supplyed as a prameter. Choosing '" << positionLogFile << "'" << std::endl;
    }

    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    else {
        std::cout << "Port not supplyed as a prameter. Choosing " << port << std::endl;
    }

    asio2::tcp_server server;
    StateManager stateManager;
    PositionLogger positionLogger(positionLogFile);

    server.start_timer(123456789, std::chrono::milliseconds(reportSendFrequency), [&server, &stateManager]() {
        if (server.session_count() > 0) {
            nlohmann::json report = stateManager.getStateReport();
            std::string serializedReport = report.dump() + "\n";

            server.foreach_session([&serializedReport](std::shared_ptr<asio2::tcp_session> & session_ptr){
                session_ptr->async_send(serializedReport);
            });
        }
    });

    server.start_timer(123456788, std::chrono::milliseconds(reportLogFrequency), [&server, &stateManager]() {
        if (server.session_count() > 0) {
            nlohmann::json report = stateManager.getStateReport();

            if (report.size() > 0) {
                std::string serializedReport = report.dump();

                printf("%s\tcurrent report: %s\n", CurrentDateTime().c_str(), serializedReport.c_str());
            }
        }
    });

    server.start_timer(123456787, std::chrono::milliseconds(positionLogFrequency), [&stateManager, &positionLogger]() {
        positionLogger.persist(stateManager.getStateReport());
    });


    server.bind_recv([&server, &stateManager, &recievedMessageCount](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s) {
        session_ptr->no_delay(true);

        try {
            DTO::ClientState clientState = nlohmann::json::parse(s).get<DTO::ClientState>();
            stateManager.putClientState(session_ptr->hash_key(), clientState);
            recievedMessageCount++;
        }
        catch(const std::exception& e)
        {
            std::cerr << CurrentDateTime() << "\tError while processing client state: " << e.what() << std::endl;
        }
    })
    .bind_connect([&server](std::shared_ptr<asio2::tcp_session> & session_ptr) {
        printf("%s\tclient enter: %s %u %s %u\n", CurrentDateTime().c_str(),
            session_ptr->remote_address().c_str(), session_ptr->remote_port(),
            session_ptr->local_address().c_str(), session_ptr->local_port());
    })
    .bind_disconnect([&server, &stateManager](std::shared_ptr<asio2::tcp_session> & session_ptr) {
        stateManager.clearSession(session_ptr->hash_key());

        printf("%s\tclient leave: %s %u %s\n", CurrentDateTime().c_str(),
            session_ptr->remote_address().c_str(),
            session_ptr->remote_port(), asio2::last_error_msg().c_str());
    })
    .bind_start([&](asio::error_code ec) {
        if (ec.value() == 0) {
            printf("%s\tlistening on: %s:%u\n", CurrentDateTime().c_str(),
                server.listen_address().c_str(), server.listen_port());
        }
        else {
            printf("%s\tlistening on: %s:%u (%d: %s)\n", CurrentDateTime().c_str(),
                server.listen_address().c_str(), server.listen_port(),
                ec.value(), ec.message().c_str());
        }
	})
    .bind_stop([&](asio::error_code ec) {
		printf("%s\tstop: %d %s\n", CurrentDateTime().c_str(), ec.value(), ec.message().c_str());
	});

    try {
        std::cout << CurrentDateTime() << "\tStarting server" << std::endl;

        server.start("0.0.0.0", port);

        while (true) {
#ifdef _WIN32
            char input = _getch_nolock();

            if (input == 'r') {
                nlohmann::json reportJson = stateManager.getStateReport();
                printf("%s\tcurrent report: %s\n", CurrentDateTime().c_str(), reportJson.dump().c_str());
            }
            else if (input == 's') {
                printf("%s\t%zd connected clients: ", CurrentDateTime().c_str(), server.session_count());
                server.foreach_session([](std::shared_ptr<asio2::tcp_session> & session_ptr){
                    printf("%zd, ", session_ptr->hash_key());
                });
                puts("");
            }
            else if (input == 'c') {
                printf("%s\t%zd clients with reports\n", CurrentDateTime().c_str(), stateManager.getReportCount());
            }
            else if (input == 'm') {
                printf("%s\t%ld recieved updates\n", CurrentDateTime().c_str(), recievedMessageCount);
            }
#endif
        }

        return 0;
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
