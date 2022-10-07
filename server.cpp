#include "headers/CommLib.hpp"

enum class MsgTypes : Dword {
    ServerAccept,
    ServerDeny,
    ServerPing,
    CallAll,
    Call,
};


class CustomServer : public comm::Server<MsgTypes> {
public:
	CustomServer(Word nPort) : comm::Server<MsgTypes>(nPort) {}

protected:
	bool onClientConnect(std::shared_ptr<comm::Connection<MsgTypes>> client) override {
		// comm::Message<MsgTypes> msg;
		// msg.header.id = MsgTypes::ServerAccept;
		// client->send(msg);
		return true;
	}

	void onClientValidate(std::shared_ptr<comm::Connection<MsgTypes>> client) override {
		comm::Message<MsgTypes> msg;
		msg.header.id = MsgTypes::ServerAccept;
		client->send(msg);
	}

	void onClientDisconnect(std::shared_ptr<comm::Connection<MsgTypes>> client) override {
		std::cout << "Removing client [" << client->id() << "]" << std::endl;
	}

	void onCall(std::shared_ptr<comm::Connection<MsgTypes>> client, comm::Message<MsgTypes> &msg) override {
		switch (msg.header.id) {
            case MsgTypes::ServerPing: {
                std::cout << "[" << client->id() << "] Server ping" << std::endl;

                client->send(msg);
            } break;

            case MsgTypes::CallAll: {
                std::cout << "[" << client->id() << "] Call all" << std::endl;

                comm::Message<MsgTypes> msg;
                msg.header.id = MsgTypes::Call;
                msg << client->id();
                callAll(msg, client);
            } break;
		}
	}
};


int main() {
	CustomServer server(60000); 
	server.start();

	while(1) server.update(-1, true);

	return 0;
}