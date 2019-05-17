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
      GETSET(bool, AllSF)
   };

   ILoraModem()
   {
   }

   virtual ~ILoraModem()
   {
   }

   typedef struct Packet {
       Packet()
       {
       }

       bool SetPayload(
                  const int& aSF,
                  const int& aSnr,
                  const int& aRssi,
                  const unsigned int& aPayloadLen,
                  const char* aPayload)
       {
           payload = new char[aPayloadLen];
           memcpy(payload, aPayload, aPayloadLen);
           payloadLen = aPayloadLen;
           snr = aSnr;
           rssi = aRssi;
           sf = aSF;


           return true;

       }

       ~Packet()
       {
           delete [] payload;
       }

       int snr = 0;
       int rssi = 0;
       unsigned char sf = 0;
       size_t payloadLen = 0;
       char* payload = nullptr;
   } Packet;

   virtual bool Start(const Configuration& configuration) = 0;
   virtual bool SendPacket(const nlohmann::json& json) = 0;
   virtual bool ReceiveNextPacket(Packet& packet) = 0;

};

class LoraModemBuilder
{
   public:
    static std::unique_ptr<ILoraModem> CreateModem();
};

