#include <fstream>
#include <iostream>
#include <conio.h>

#include "asio2/tcp/tcp_server.hpp"
#include "nlohmann/json.hpp"
#include "StateManager.h"
#include "dto.h"


int main(int argc, char const *argv[]) {
    int port = 9002;

    if (argc < 2) {
        std::cout << "Port not supplyed as a prameter choosing " << port << std::endl;
    }

    asio2::tcp_server server;
    StateManager stateManager;

    server.start_timer(123456789, std::chrono::milliseconds(250), [&server, &stateManager]() {
        if (server.session_count() > 0) {
            nlohmann::json report = stateManager.getStateReport();
            std::string serializedReport = report.dump();

            server.foreach_session([&serializedReport](std::shared_ptr<asio2::tcp_session> & session_ptr){
                session_ptr->async_send(serializedReport);
            });
        }
    });

    server.start_timer(123456788, std::chrono::milliseconds(2000), [&server, &stateManager]() {
        nlohmann::json report = stateManager.getStateReport();
        std::string serializedReport = report.dump();

        printf("current report: %s\n", serializedReport.c_str());
    });


    server.bind_recv([&server, &stateManager](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s) {
        session_ptr->no_delay(true);

        try
        {
            /* code */
            DTO::ClientState clientState = nlohmann::json::parse(s).get<DTO::ClientState>();
            stateManager.putClientState(session_ptr->hash_key(), clientState);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error while processing client state: " << e.what() << std::endl;
        }
    })
    .bind_connect([&server](std::shared_ptr<asio2::tcp_session> & session_ptr) {
        printf("client enter: %s %u %s %u\n",
            session_ptr->remote_address().c_str(), session_ptr->remote_port(),
            session_ptr->local_address().c_str(), session_ptr->local_port());
    })
    .bind_disconnect([&server, &stateManager](std::shared_ptr<asio2::tcp_session> & session_ptr) {
        stateManager.clearSession(session_ptr->hash_key());

        printf("client leave: %s %u %s\n",
            session_ptr->remote_address().c_str(),
            session_ptr->remote_port(), asio2::last_error_msg().c_str());
    })
    .bind_start([&](asio::error_code ec) {
        if (ec.value() == 0) {
            printf("listening on: %s:%u\n",
                server.listen_address().c_str(), server.listen_port());
        }
        else {
            printf("listening on: %s:%u (%d: %s)\n",
                server.listen_address().c_str(), server.listen_port(),
                ec.value(), ec.message().c_str());
        }
	})
    .bind_stop([&](asio::error_code ec) {
		printf("stop: %d %s\n", ec.value(), ec.message().c_str());
	});

    try {
        std::cout << "Starting server" << std::endl;

        server.start("0.0.0.0", port);

        while (true) {
            char input = _getch_nolock();

            if (input == 's') {
                nlohmann::json reportJson = stateManager.getStateReport();
                printf("current report: %s", reportJson.dump().c_str());
            }
            else if (input == 'c') {
                printf("%zd connected clients: ", server.session_count());
                server.foreach_session([](std::shared_ptr<asio2::tcp_session> & session_ptr){
                    printf("%zd, ", session_ptr->hash_key());
                });
                puts("\n");
            }
        }

        return 0;
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
