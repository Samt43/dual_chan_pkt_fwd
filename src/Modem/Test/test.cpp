#include "ILoraModem.h"
#include <iostream>



int main()
{
    auto modem = LoraModemBuilder::CreateModem();
    modem->Start(ILoraModem::Configuration());
    
    while (1)
    {
        nlohmann::json json;
        if (modem->ReceiveNextPacket(json))
        {
            std::cout << json;
        }
        else
        {
            std::cout << "Error while receiving next Packet";
            
        }
    }
    
    return 0;
    
}
