#pragma once
#include "ILoraModem.h"
#include <unistd.h>

constexpr static const unsigned char fakeOTAA[]  = {0x00, 0x4e, 0x2c ,0x01, 0xd0, 0x7e, 0xd5, 0xb3, 0x70, 0xb0, 0xe7, 0xbe, 0xcb, 0x79, 0xe5, 0xe8, 0x00, 0x11, 0x98, 0x6e, 0x84, 0x73, 0xcb};

class LoraModemStub : public ILoraModem
{

    virtual bool Start(const Configuration& configuration) override
    {
        return true;
    }
    virtual bool SendPacket(const nlohmann::json& json) override
    {
        return true;
    }
    virtual bool ReceiveNextPacket(Packet& packet) override
    {
        sleep(2);
        packet.SetPayload(7, 6, -105, sizeof (fakeOTAA), (size_t*)fakeOTAA);
        return true;
    }
};

std::unique_ptr<ILoraModem> LoraModemBuilder::CreateModem()
{
    return std::make_unique<LoraModemStub>();
}
