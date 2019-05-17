#include "ILoraModem.h"
#include <iostream>



int main()
{
    auto modem = LoraModemBuilder::CreateModem();
    modem->Start(ILoraModem::Configuration());
    
    while (1)
    {
        std::string payload;
        ILoraModem::Packet packet;
        if (modem->ReceiveNextPacket(packet))
        {
            std::cout << payload;
        }
        else
        {
            std::cout << "Error while receiving next Packet";
            
        }
    }
    
    return 0;
    
}
