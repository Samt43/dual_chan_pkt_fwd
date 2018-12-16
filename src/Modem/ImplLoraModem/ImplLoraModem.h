#pragma once
#include "ILoraModem.h"

class ImplLoraModem : public ILoraModem
{
   public :
      ImplLoraModem();
      virtual ~ImplLoraModem();

      virtual bool Start(const Configuration& configuration, LoraModemObserver& observer) override;
      virtual bool SendPacket(const nlohmann::json& json) override;


};