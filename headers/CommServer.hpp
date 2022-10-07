#pragma once 

#include "commons.hpp"
#include "CommTSQ.hpp"
#include "CommMessage.hpp"
#include "CommConnection.hpp"

namespace comm {
    template<typename T>
    class Server {
        public:
            Server(Word port)
             : sAcceptor(sContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {

            } virtual ~Server() {stop();}

            bool start() {
                try {
                    waitForClient();
                    contextThread = std::thread([this](){sContext.run();});
                } catch(std::exception &e) {
                    std::cerr << "[SERVER] Exception: " << e.what() << std::endl;
                    return false;
                }
            
                std::cout << "[SERVER] Started!" << std::endl;
                return true;
            }

            void stop() {
                sContext.stop();
                if(contextThread.joinable()) contextThread.join();

                std::cout << "[SERVER] Stopped!" << std::endl;
            }
            
            // ASYNC !!!
            void waitForClient() {
                sAcceptor.async_accept(
                    [this](std::error_code ec, asio::ip::tcp::socket socket) {
                        if(!ec) {
                            std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << std::endl;

                            std::shared_ptr<Connection<T>> conn =
                                std::make_shared<Connection<T>>(
                                    Connection<T>::owner::server,
                                    sContext, 
                                    std::move(socket),
                                    sMessageIn
                            );

                            if(onClientConnect(conn)) {
                                deqConnections.push_back(std::move(conn));

                                // connect() - connecto to server
                                // invoke()  - connect to client
                                deqConnections.back()->invoke(this, IDCounter++);

                                std::cout << "[" << deqConnections.back()->id() << "] Connection approved!" << std::endl;
                            } else std::cout << "[------] Connection denied" << std::endl;
                        } else std::cout << "[SERVER] New connection error: " << ec.message() << std::endl;

                        waitForClient();


                        // if(ec) std::cout << "[SERVER] New connection error: " << ec.message() << std::endl;
                        
                        // std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << std::endl;
                        // std::shared_ptr<Connection<T>> conn = 
                        //     std::make_shared<Connection<T>>(Connection<T>::owner::server,
                        //     sContext, std::move(socket), sMessageIn);

                        // if(onClientConnect(conn)) {
                        //     deqConnections.push_back(std::move(conn));
                        //     deqConnections.back()->invoke(IDCounter++);
                            
                        //     std::cout << "[" << deqConnections.back()->id() << "] connection approved" << std::endl;
                        // } else std::cout << "[------] Connection denied" << std::endl;

                        // waitForClient();
                    }
                );
            }

            void callClient(std::shared_ptr<Connection<T>> client, const Message<T> &msg) {
                if(client && client->status()) client->send(msg);
                else {
                    onClientDisconnect(client);
                    client.reset();

                    deqConnections.erase(
                        std::remove(deqConnections.begin(), deqConnections.end(), client),
                        deqConnections.end()
                    );
                }
            }

            void callAll(const Message<T> &msg, std::shared_ptr<Connection<T>> ignore = nullptr) {
                bool invalidClients = false;

                for(auto &client : deqConnections) {
                    if(client && client->status()) {
                        if(client != ignore) client->send(msg);
                    } else {
                        onClientDisconnect(client);
                        client.reset();
                        invalidClients = true;
                    }
                }

                if(invalidClients) {
                    deqConnections.erase(
                        std::remove(deqConnections.begin(), deqConnections.end(), nullptr),
                        deqConnections.end()
                    );
                }
            }

            // size_t is unsigned, so -1 returns maximum, possible value
            void update(size_t maxMsgs = -1, bool wait = false) {
                if(wait) sMessageIn.wait();

                size_t msgCnt = 0;
                while(msgCnt < maxMsgs && !sMessageIn.empty()) {
                    auto msg = sMessageIn.pop_front();
                    onCall(msg.remote, msg.msg);

                    msgCnt++;
                }
            }

            virtual void onClientValidate(std::shared_ptr<Connection<T>> client) {}

        protected:
            tsq<MsgOwned<T>> sMessageIn;
            std::deque<std::shared_ptr<Connection<T>>> deqConnections;

            asio::io_context sContext;
            std::thread contextThread;

            asio::ip::tcp::acceptor sAcceptor;
            Dword IDCounter = 10000;


            virtual bool onClientConnect(std::shared_ptr<Connection<T>> client) {
                return false;
            }

            virtual void onClientDisconnect(std::shared_ptr<Connection<T>> client) {

            }

            virtual void onCall(std::shared_ptr<Connection<T>> client, Message<T> &msg) {

            }
    };

}