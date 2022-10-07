#pragma once

#include "commons.hpp"


namespace comm {
    template<typename T>
    struct MsgHeader {
        T id{};
        Dword size = 0;
    };

    template<typename T>
    struct Message {
        MsgHeader<T> header{};
        std::vector<Byte> body;


        template<typename DT>
        friend Message<T> &operator << (Message<T> &msg, const DT &data) {
            static_assert(std::is_trivially_copyable<DT>::value, "Data is too complex to be pushed into vector");
            
            size_t size = msg.body.size();
            msg.body.resize(msg.body.size() + sizeof(DT));

            std::memcpy(msg.body.data() + size, &data, sizeof(DT));
            msg.header.size = msg.size();

            return msg;
        }

        template<typename DT>
        friend Message<T> &operator >> (Message<T> &msg, DT &data) {
            static_assert(std::is_trivially_copyable<DT>::value, "Data is too complex to be pushed into vector");

            size_t size = msg.body.size() - sizeof(DT);

            std::memcpy(&data, msg.body.data() + size, sizeof(DT));
            msg.body.resize(size);
            msg.header.size = msg.size();

            return msg;
        }


        size_t size() const {
            return body.size();
        }

        friend std::ostream &operator << (std::ostream &os, const Message<T> &msg) {
            os << "ID: " << int(msg.header.id) << " Size: " << msg.header.size;
            return os;
        }
    };

    template<typename T>
    class Connection;

    template<typename T>
    struct MsgOwned {
        std::shared_ptr<Connection<T>> remote = nullptr;
        Message<T> msg;

        friend std::ostream &operator << (std::ostream &os, const MsgOwned<T> &msg) {
            os << msg.msg;
            return os;
        }
    };
}