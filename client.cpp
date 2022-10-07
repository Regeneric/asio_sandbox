#include "headers/CommLib.hpp"

enum class MsgTypes : Dword {
    ServerAccept,
    ServerDeny,
    ServerPing,
    CallAll,
    Call,
};

class CustomClient : public comm::Client<MsgTypes> {
public:
	void ping() {
		comm::Message<MsgTypes> msg;
		msg.header.id = MsgTypes::ServerPing;

		// Caution with this...
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		

		msg << timeNow;
		send(msg);
	}

	void callAll() {
		comm::Message<MsgTypes> msg;
		msg.header.id = MsgTypes::CallAll;		
		send(msg);
	}
};

int main() {
	CustomClient c;
	c.connect("127.0.0.1", 60000);

    bool quit = false;
    while(!quit) {

        // Here comes the client logic - now empty

        if (c.status()) {
            if (!c.incoming().empty()) {
                auto msg = c.incoming().pop_front().msg;

                switch(msg.header.id) {
                    case MsgTypes::ServerAccept: {			
                        std::cout << "Server accepted connection" << std::endl;
                    } break;

                    case MsgTypes::ServerPing: {
                        // Server has responded to a ping request
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        msg >> timeThen;
                        std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << std::endl;
                    } break;

                    case MsgTypes::Call: {
                        // Server has responded to a ping request	
                        Dword clientID;
                        msg >> clientID;
                        std::cout << "Hello from [" << clientID << "]" << std::endl;
                    } break;
                }
            }
        } else {
            std::cout << "Server Down" << std::endl;
            quit = true;
        }
    
    }

	return 0;
}