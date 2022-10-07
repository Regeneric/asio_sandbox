#include "headers/CommLib.hpp"
#include "headers/MsgTypes.hpp"

#include <iostream>
#include <unordered_map>

class MessageHandler : public comm::Server<MsgTypes> {
public:
	MessageHandler(Word port) : comm::Server<MsgTypes>(port) {}

	std::unordered_map<Dword, MessengerDesc> users;

protected:
	bool onClientConnect(std::shared_ptr<comm::Connection<MsgTypes>> client) override {
		return true;
	}

	void onClientValidate(std::shared_ptr<comm::Connection<MsgTypes>> client) override {
		comm::Message<MsgTypes> msg;
		msg.header.id = MsgTypes::ClientAccept;
		client->send(msg);
	}

	void onClientDisconnect(std::shared_ptr<comm::Connection<MsgTypes>> client) override {

	}


	void onCall(std::shared_ptr<comm::Connection<MsgTypes>> client, comm::Message<MsgTypes> &msg) override {
		switch(msg.header.id) {
			case MsgTypes::ClientRegister: {
				MessengerDesc md;
				msg >> md;
				md.ID = client->id();
				users.insert_or_assign(md.ID, md);

				comm::Message<MsgTypes> msgID;
				msgID.header.id = MsgTypes::ClientID;
				msgID << md.ID;
				callClient(client, msgID);

				comm::Message<MsgTypes> msgAdd;
				msgAdd.header.id = MsgTypes::AddUser;
				msgAdd << md;
				callAll(msgAdd);

				for(const auto &u : users) {
					comm::Message<MsgTypes> msgAddAll;
					msgAddAll.header.id = MsgTypes::AddUser;
					msgAddAll << u.second;
					callClient(client, msgAddAll);
				}
			} break;

			case MsgTypes::ClientUnregister: {

			} break;

			case MsgTypes::UpdateUser: {
				callAll(msg, client);
			} break;
		}
	}
};

auto main() -> int {
	MessageHandler server(60000);
	server.start();

	while(1) {
		server.update(-1, true);
	}

	return 0;
}



// #include "headers/CommLib.hpp"
// #include "headers/MsgTypes.hpp"


// class MessageHandler : public comm::Server<MsgTypes> {
// public:
// 	MessageHandler(Word nPort) : comm::Server<MsgTypes>(nPort) {}

// protected:
// 	bool onClientConnect(std::shared_ptr<comm::Connection<MsgTypes>> client) override {
// 		return true;
// 	}

// 	void onClientValidate(std::shared_ptr<comm::Connection<MsgTypes>> client) override {
// 		comm::Message<MsgTypes> msg;
// 		msg.header.id = MsgTypes::ServerAccept;
// 		client->send(msg);
// 	}

// 	void onClientDisconnect(std::shared_ptr<comm::Connection<MsgTypes>> client) override {
// 		std::cout << "Removing client [" << client->id() << "]" << std::endl;
// 	}

// 	void onCall(std::shared_ptr<comm::Connection<MsgTypes>> client, comm::Message<MsgTypes> &msg) override {
// 		switch (msg.header.id) {
//             case MsgTypes::ServerPing: {
//                 std::cout << "[" << client->id() << "] Server ping" << std::endl;

//                 client->send(msg);
//             } break;

//             case MsgTypes::CallAll: {
//                 std::cout << "[" << client->id() << "] Call all" << std::endl;

//                 comm::Message<MsgTypes> msg;
//                 msg.header.id = MsgTypes::Call;
//                 msg << client->id();
//                 callAll(msg, client);
//             } break;
// 		}
// 	}
// };


// int main() {
// 	MessageHandler server(60000); 
// 	server.start();

// 	while(1) server.update(-1, true);

// 	return 0;
// }