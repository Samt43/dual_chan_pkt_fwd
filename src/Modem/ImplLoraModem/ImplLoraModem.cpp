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
	int fd = open("/dev/sx127x0", O_RDWR);

	if (fd < 0)
    {
		std::cout<<"failed to open device"<<std::end;
        return false;
    }
    
	if (ioctl(fd, SX127X_IOCTL_CMD_SETPAOUTPUT, SX127X_PA_PABOOST) != 0) {
		std::cout<<"failed to set pa output<<std::endl;
        return false;
	}

	if (ioctl(fd, SX127X_IOCTL_CMD_SETMODULATION, SX127X_MODULATION_LORA)
			!= 0) {
		std::cout<<"failed to set modulation<<std::endl;
        return false;
	}

	if (ioctl(fd, SX127X_IOCTL_CMD_SETCARRIERFREQUENCY, 868100000) != 0) {
		std::cout<<"failed to set carrier frequency<<std::endl;
        return false;
	}

        if (ioctl(fd, SX127X_IOCTL_CMD_SETSYNCWORD, 0x34) != 0) {
        std::cout<<"failed to set LoraWan Sync Word<<std::endl;
        return false;
        }

	if (ioctl(fd, SX127X_IOCTL_CMD_SETSF, 7) != 0) {
		cout<<"failed to set spreading factor<<std::endl;
        return false;
	}

	if (ioctl(fd, SX127X_IOCTL_CMD_SETOPMODE, opmode) != 0) {
		std::cout<<"failed to set opmode<<std::endl;
        return false;
	}

    
    return true;

}

bool ImplLoraModem::SendPacket(const nlohmann::json& json)
{
    
}
