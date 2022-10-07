#pragma once 

#define NO_DEBUG    // DEBUG / NO_DEBUG

#include "commons.hpp"
#include "CommTSQ.hpp"
#include "CommMessage.hpp"

namespace comm {
    // Forward declaration
    template<typename T>
    class Server;

    template<typename T>
    class Connection : public std::enable_shared_from_this<Connection<T>> {
        public:
            enum class owner {
                server,
                client
            };

            Connection(owner parent, asio::io_context &context, asio::ip::tcp::socket socket, tsq<MsgOwned<T>> &qIn) 
                : rContext(context), rSocket(std::move(socket)), rMessageIn(qIn) 
            {
                ownerType = parent;

                if(ownerType == owner::server) {
                    handshakeOut = Qword(std::chrono::system_clock::now().time_since_epoch().count());
                    handshakeCheck = scramble(handshakeOut);

                    #ifdef DEBUG
                        std::cout << "[DEBUG] CommConnection::Connection() - handshakeOut == " << handshakeOut << std::endl;
                        std::cout << "[DEBUG] CommConnection::Connection() - handshakeCheck == " << handshakeCheck << std::endl;
                    #endif
                } else {
                    handshakeIn = 0;
                    handshakeOut = 0;
                }
            } 
            
            virtual ~Connection() {}

            Dword id() const {return rID;}

            // Connect to client
            void invoke(comm::Server<T> *server, Dword uid = 0) {
                if(ownerType == owner::server) {
                    if(rSocket.is_open()) {
                        rID = uid;
                        // readHeader();
                        writeValidate();
                        readValidate(server);
                    }
                }
            }

            // Connect to server
            void connect(const asio::ip::tcp::resolver::results_type &endpoints) {
                if(ownerType == owner::client) {
                    #ifdef DEBUG
                        std::cout << "[DEBUG] Inside CommConnection:connect()" << std::endl;
                    #endif
                    
                    asio::async_connect(rSocket, endpoints,
                        [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                            // if(!ec) readHeader();
                            if(!ec) readValidate();
                        }
                    );
                }
            }
            
            void disconnect() {if(status()) asio::post(rContext, [this](){rSocket.close();});}
            bool status() const {return rSocket.is_open();}

            void send(const Message<T> &msg) {
                asio::post(rContext, 
                    [this, msg]() {
                        bool writing = !rMessageOut.empty();
                        rMessageOut.push_back(msg);
                        
                        if(!writing) writeHeader();
                    }
                );
            }

        protected:
            asio::ip::tcp::socket rSocket;
            asio::io_context &rContext;

            tsq<Message<T>> rMessageOut;
            tsq<MsgOwned<T>> &rMessageIn;
            Message<T> rTempIn;

            owner ownerType = owner::server;
            Dword rID = 0;

            Qword handshakeOut = 0;
            Qword handshakeIn = 0;
            Qword handshakeCheck = 0;

        private:
            void readHeader() {
                asio::async_read(rSocket, asio::buffer(&rTempIn.header, sizeof(MsgHeader<T>)),
                    [this](std::error_code ec, std::size_t length) {
                        if(!ec) {
                            if(rTempIn.header.size > 0) {
                                rTempIn.body.resize(rTempIn.header.size);
                                readBody();
                            } else addToInQu();
                        } else {
                            std::cout << "[" << rID << "] Read header fail!" << std::endl;
                            std::cout << "[" << rID << "] " << ec.message() << std::endl;
                            rSocket.close(); 
                        }


                        // if(ec) {
                        //     std::cout << "[" << rID << "] Read header fail!" << std::endl;
                        //     rSocket.close();
                        // }

                        // if(rTempIn.header.size <= 0) {
                        //     addToInQu();
                        // }

                        // rTempIn.body.resize(rTempIn.header.size);
                        // readBody();
                    }
                );
            }

            void writeHeader() {
                asio::async_write(rSocket, asio::buffer(&rMessageOut.front().header, sizeof(Message<T>)),
                    [this](std::error_code ec, std::size_t length) {
                        if(!ec) {
                            if(rMessageOut.front().body.size() > 0) writeBody();
                            else {
                                rMessageOut.pop_front();
                                if(!rMessageOut.empty()) writeHeader();
                            }
                        } else {
                            std::cout << "[" << rID << "] Write header fail!" << std::endl;
                            std::cout << "[" << rID << "] " << ec.message() << std::endl;
                            rSocket.close();
                        }



                        // if(ec) {
                        //     std::cout << "[" << rID << "] Write header fail!" << std::endl;
                        //     rSocket.close();
                        // }

                        // if(rMessageOut.front().body.size() <= 0) {
                        //     rMessageOut.pop_front();
                        //     if(rMessageOut.empty()) writeHeader();
                        // }

                        // writeBody();
                    }
                );
            }


            void readBody() {
                asio::async_read(rSocket, asio::buffer(rTempIn.body.data(), rTempIn.body.size()),
                    [this](std::error_code ec, std::size_t length) {
                        if(!ec) addToInQu();
                        else {
                            std::cout << "[" << rID << "] Read body fail!" << std::endl;
                            std::cout << "[" << rID << "] " << ec.message() << std::endl;
                            rSocket.close();
                        }


                        // if(ec) {
                        //     std::cout << "[" << rID << "] Read body fail!" << std::endl;
                        //     rSocket.close();
                        // }

                        // addToInQu();
                    }
                );
            }

            void writeBody() {
                asio::async_write(rSocket, asio::buffer(rMessageOut.front().body.data(), rMessageOut.front().body.size()),
                    [this](std::error_code ec, std::size_t length) {
                        if(!ec) {
                            rMessageOut.pop_front();
                            if(!rMessageOut.empty()) writeHeader();
                        } else {
                            std::cout << "[" << rID << "] Write body fail!" << std::endl;
                            std::cout << "[" << rID << "] " << ec.message() << std::endl;
                            rSocket.close();
                        }


                        // if(ec) {
                        //     std::cout << "[" << rID << "] Write body fail!" << std::endl;
                        //     rSocket.close();
                        // }

                        // rMessageOut.pop_front();
                        // if(rMessageOut.empty()) writeHeader();
                    }
                );
            }


            void addToInQu() {
                if(ownerType == owner::server) rMessageIn.push_back({this->shared_from_this(), rTempIn});
                else rMessageIn.push_back({nullptr, rTempIn});

                readHeader();
            }

            Qword scramble(Qword input) {
                // Qword out = input ^ 0xDFBEFDACABFEDEFA;
                // out = (out & 0x1337AF7A0420BDBA) >> 4 | (out & 0x4206921371337ABC) << 4;
                // return out ^ 0xABCDEF123456AB12;

                Qword out = input ^ 0xDEADBEEFC0DECAFE;
				out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
				return out ^ 0xC0DEFACE12345678;
            }


            void writeValidate() {
                asio::async_write(rSocket, asio::buffer(&handshakeOut, sizeof(Qword)),
                    [this](std::error_code ec, std::size_t length) {
                        if(!ec) {
                            if(ownerType == owner::client) readHeader();
                        } else rSocket.close();
                    }
                );
            }

            void readValidate(comm::Server<T> *server = nullptr) {
                #ifdef DEBUG
                    std::cout << "[DEBUG] Inside CommConnection:readValidate()" << std::endl;
                #endif

                asio::async_read(rSocket, asio::buffer(&handshakeIn, sizeof(Qword)),
                    [this, server](std::error_code ec, std::size_t length) {
                        if(!ec) {
                            if(ownerType == owner::server) {
                                #ifdef DEBUG
                                    std::cout << "[DEBUG] CommConnection::readValidate() - owner::server" << std::endl;
                                    std::cout << "[DEBUG] CommConnection::readValidate() - handshakeCheck == " << handshakeCheck << std::endl;
                                    std::cout << "[DEBUG] CommConnection::readValidate() - handshakeIn == " << handshakeIn << std::endl;
                                #endif

                                if(handshakeIn == handshakeCheck) {
                                    #ifdef DEBUG
                                        std::cout << "[DEBUG] CommConnection::readValidate() - handshakeIn == handshakeOut" << std::endl;
                                    #endif

                                    std::cout << "[" << rID << "] Client validated (handshake validation success)" << std::endl;
                                    server->onClientValidate(this->shared_from_this());

                                    readHeader();
                                } else {
                                    std::cout << "[" << rID << "] Client disconnected (handshake validation fail)" << std::endl;
                                    rSocket.close();
                                }
                            } else {
                                #ifdef DEBUG
                                    std::cout << "[DEBUG] CommConnection::readValidate() - owner::client" << std::endl;
                                #endif

                                handshakeOut = scramble(handshakeIn);
                                
                                #ifdef DEBUG
                                    std::cout << "[DEBUG] CommConnection::readValidate() - handshakeOut == " << handshakeOut << std::endl;
                                    std::cout << "[DEBUG] CommConnection::readValidate() - handshakeIn == " << handshakeIn << std::endl;
                                #endif

                                writeValidate();
                            }
                        } else {
                            std::cout << "[" << rID << "] Client disconnected (handshake validation fail)" << std::endl;
                            rSocket.close();
                        }
                    }
                );
            }
    };
};