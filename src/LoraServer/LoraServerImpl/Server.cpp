#include <Modem/ILoraModem.h>
#include <sx127x/sx127x.h>

class Observer : public ILoraModem::LoraModemObserver
{
    
    virtual void OnPacketReceived(const nlohmann::json& json) override
    {
        
        
    }
    
};


int main()
{
    Observer obs;
    auto modem = LoraModemBuilder::CreateModem();
    modem->Start(ILoraModem::Configuration(), obs);
    
    return 0;
    
}
