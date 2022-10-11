#pragma once

#include <cstdint>
#include <array>
#include <string>

using Byte = uint8_t;
using Word = uint16_t;
using Dword = uint32_t;
using Qword = uint64_t;

enum class MsgTypes : Dword {
    ServerStatus,
    ServerPing,

    ClientAccept,
    ClientID,
    ClientRegister,
    ClientUnregister,

    AddUser,
    RemoveUser,
    UpdateUser,

    MsgSent,
};

struct MessengerDesc {
    Dword ID = 0;
    Dword avatarID = 0;

    // char message[1024] = "";
};