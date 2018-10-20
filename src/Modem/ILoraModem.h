#pragma once
#include <memory>
#include "json.hpp"
#define GETSET(type, var) \
    public: \
       type Get##var() \
       {\
          return _##var; \
       }\
       auto& Set##var(type val) \
       {\
          _##var = val; \
          return *this;\
       } \
    private: \
       type _##var;

class ILoraModem
{
   public :
   struct Configuration
   {
      GETSET(int, ssPin)
      GETSET(int, dio0)
      GETSET(int, Rst)
      GETSET(int, ActivityLED)
      GETSET(int, SpiChannel)
   };

   class LoraModemObserver
   {
      virtual void OnPacketReceived(const nlohmann::json& json) = 0;
   };

   ILoraModem();

   virtual ~ILoraModem() = default;
   bool Start(const Configuration& configuration, LoraModemObserver& observer);
   bool SendPacket(const nlohmann::json& json);
};

class LoraModemBuilder
{
    std::unique_ptr<ILoraModem> CreateModem();
};

