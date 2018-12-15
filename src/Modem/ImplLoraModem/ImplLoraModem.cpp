#include "ImplLoraModem.h"
#include <iostream>

std::unique_ptr<ILoraModem> LoraModemBuilder::CreateModem()
{
    
    return std::make_unique<ImplLoraModem>();
}

ImplLoraModem::ImplLoraModem()
{
    
}

ImplLoraModem::~ImplLoraModem()
{
    
}


bool ImplLoraModem::Start(const Configuration& configuration, LoraModemObserver& observer)
{
    std::cout<<"Impl Starting" << std::endl;
    return true;

}

bool ImplLoraModem::SendPacket(const nlohmann::json& json)
{
    
}
