#include "ILoraModem.h"
#include <iostream>



int main()
{
    auto modem = LoraModemBuilder::CreateModem();
    modem->Start(ILoraModem::Configuration());
    
    while (1)
    {
        std::string payload;
        ILoraModem::PacketInfos infos;
        if (modem->ReceiveNextPacket(payload, infos))
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
