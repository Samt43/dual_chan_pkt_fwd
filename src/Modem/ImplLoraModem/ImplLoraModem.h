#pragma once
#include "ILoraModem.h"

class ImplLoraModem : public ILoraModem
{
   public :
      ImplLoraModem();
      virtual ~ImplLoraModem();

      virtual bool Start(const Configuration& configuration) override;
      virtual bool SendPacket(const nlohmann::json& json) override;
      virtual bool ReceiveNextPacket(Packet& packet) override;

private:
      int fd;

};
