#pragma once
#include "ILoraModem.h"
#include <unistd.h>

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
    virtual bool ReceiveNextPacket(std::string& payloadPacket, PacketInfos& infos) override
    {
        sleep(2);
        payloadPacket = "AAAA";
        infos.lsnr = 45;
        infos.rssi = 102;
        return true;
    }
};

std::unique_ptr<ILoraModem> LoraModemBuilder::CreateModem()
{
    return std::make_unique<LoraModemStub>();
}
