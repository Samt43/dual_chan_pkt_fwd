#pragma once
#include "ILoraModem.h"

class ImplLoraModem : public ILoraModem
{
   public :
      ImplLoraModem();
      virtual ~ImplLoraModem();

      virtual bool Start(const Configuration& configuration) override;
      virtual bool SendPacket(const nlohmann::json& json) override;
      virtual bool ReceiveNextPacket(std::string& payloadPacket, PacketInfos& infos) override;

};
