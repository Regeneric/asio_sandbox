#include "headers/CommLib.hpp"
#include "headers/MsgTypes.hpp"

#include <iostream>
#include <unordered_map>

class Messenger : public comm::Client<MsgTypes> {
public:
    Messenger() {}

    MessengerDesc messenger;
    bool waitForConn = true;

    std::unordered_map<Dword, MessengerDesc> msgObjects;
    Dword msgID = 0;
};


auto main() -> int {
    Messenger client;
    client.connect("127.0.0.1", 60000);

    bool quit = false;
    while(!quit) {
        if(client.status()) {
            while(!client.incoming().empty()) {
                auto msg = client.incoming().pop_front().msg;

                switch(msg.header.id) {
                    case MsgTypes::ClientAccept: {
                        std::cout << "Connection accepted" << std::endl;

                        comm::Message<MsgTypes> msg;
                        msg.header.id = MsgTypes::ClientRegister;

                        client.messenger.msgHistory.resize(0);
                        msg << client.messenger;

                        client.send(msg);
                    } break;
                
                    case MsgTypes::ClientID: {
                        msg >> client.msgID;
                        std::cout << "Client ID assigned: " << client.msgID << std::endl;
                    } break;

                    case MsgTypes::AddUser: {
                        MessengerDesc md;
                        msg >> md;

                        client.msgObjects.insert_or_assign(md.ID, md);
                        if(md.ID == client.msgID) client.waitForConn = false;

                    } break;

                    case MsgTypes::RemoveUser: {
                        Dword remID = 0;
                        msg >> remID;

                        client.msgObjects.erase(remID);
                    } break;

                    case MsgTypes::UpdateUser: {
                        MessengerDesc md;
                        msg >> md;

                        client.msgObjects.insert_or_assign(md.ID, md);
                    } break;
                }
            }
        } 
        
        // if(client.waitForConn) std::cout << "Waiting for connection..." << std::endl;
    
        comm::Message<MsgTypes> msg;
        msg.header.id = MsgTypes::UpdateUser;
        msg << client.msgObjects[client.msgID];
        
        client.send(msg);
    }

    return 0;
}




// #include "headers/CommLib.hpp"
// // #include "headers/MsgTypes.hpp"

// enum class MsgTypes : Dword {
//     ServerAccept,
//     ServerDeny,
//     ServerPing,
//     CallAll,
//     Call,
// };

// class CustomClient : public comm::Client<MsgTypes> {
// public:
// 	void ping() {
// 		comm::Message<MsgTypes> msg;
// 		msg.header.id = MsgTypes::ServerPing;

// 		// Caution with this...
// 		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		

// 		msg << timeNow;
// 		send(msg);
// 	}

// 	void callAll() {
// 		comm::Message<MsgTypes> msg;
// 		msg.header.id = MsgTypes::CallAll;		
// 		send(msg);
// 	}
// };

// int main() {
// 	CustomClient c;
// 	c.connect("127.0.0.1", 60000);

//     bool quit = false;
//     while(!quit) {

//         // Here comes the client logic - now empty

//         if (c.status()) {
//             if (!c.incoming().empty()) {
//                 auto msg = c.incoming().pop_front().msg;

//                 switch(msg.header.id) {
//                     case MsgTypes::ServerAccept: {			
//                         std::cout << "Server accepted connection" << std::endl;
//                     } break;

//                     case MsgTypes::ServerPing: {
//                         // Server has responded to a ping request
//                         std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
//                         std::chrono::system_clock::time_point timeThen;
//                         msg >> timeThen;
//                         std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << std::endl;
//                     } break;

//                     case MsgTypes::Call: {
//                         // Server has responded to a ping request	
//                         Dword clientID;
//                         msg >> clientID;
//                         std::cout << "Hello from [" << clientID << "]" << std::endl;
//                     } break;
//                 }
//             }
//         } else {
//             std::cout << "Server Down" << std::endl;
//             quit = true;
//         }
    
//     }

// 	return 0;
// }