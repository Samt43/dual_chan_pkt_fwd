#pragma once
#include <memory>
#include "json.hpp"
#define GETSET(type, var) \
    public: \
       type Get##var() \
       {\
          return _##var; \
       }\
       auto Set##var(type val) \
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

   ILoraModem()
   {
   }

   virtual ~ILoraModem()
   {
   }


   virtual bool Start(const Configuration& configuration, LoraModemObserver& observer) = 0;
   virtual bool SendPacket(const nlohmann::json& json) = 0;
};

class LoraModemBuilder
{
   public:
    static std::unique_ptr<ILoraModem> CreateModem();
};

