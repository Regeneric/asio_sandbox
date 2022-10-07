#pragma once 

#include "commons.hpp"
#include "CommMessage.hpp"
#include "CommConnection.hpp"
#include "CommTSQ.hpp"

namespace comm {
    template<typename T>
    class Client {
        public:
            // Client() : cSocket(cContext) {}
            Client() {}
            virtual ~Client() {disconnect();}

            void send(const Message<T> &msg) {
                if(status()) cConnection->send(msg);
            }

            bool connect(const std::string &host, const Word port) {                
                try {
                    asio::ip::tcp::resolver resolver(cContext);
                    asio::ip::tcp::resolver::results_type cEndpoints = resolver.resolve(host, std::to_string(port));

                    cConnection = std::make_unique<Connection<T>>(
                        Connection<T>::owner::client,
                        cContext,
                        asio::ip::tcp::socket(cContext),
                        cMessageIn
                    );

                    cConnection->connect(cEndpoints);
                    contextThread = std::thread([this](){cContext.run();});

                } catch(std::exception &e) {
                    std::cerr << "[CLIENT] Exception: " << e.what() << std::endl;
                    return false;
                }; 
                
                return true;
            }

            void disconnect() {
                if(status()) cConnection->disconnect();
                
                cContext.stop();
                if(contextThread.joinable()) contextThread.join();

                cConnection.release();
            }

            bool status() {
                if(cConnection) return cConnection->status();
                else return false;
            }

            tsq<MsgOwned<T>> &incoming() {
                return cMessageIn;
            }

        protected:
            asio::io_context cContext;
            std::thread contextThread;

            // asio::ip::tcp::socket cSocket;
            std::unique_ptr<Connection<T>> cConnection;

        private:
            tsq<MsgOwned<T>> cMessageIn;
    };
};