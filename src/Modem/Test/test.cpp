#include "ILoraModem.h"

class Observer : public LoraModem::LoraModemObserver
{
    
    virtual void OnPacketReceived(const nlohmann::json& json) override
    {
        
        
    }
    
};


int main()
{
    Observer obs;
    auto modem = LoraModemBuilder::CreateModem();
    modem->Start(LoraModem::Configuration(), obs);
    
    return 0;
    
}
