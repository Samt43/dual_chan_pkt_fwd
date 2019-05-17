#include "ImplLoraModem.h"
#include "sx127x.h"
#include <sys/ioctl.h>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


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


bool ImplLoraModem::Start(const Configuration& configuration)
{
    std::cout<<"Impl Starting" << std::endl;
	fd = open("/dev/sx127x0", O_RDWR);

	if (fd < 0)
    {
		std::cout<<"failed to open device"<<std::endl;
        return false;
    }
    
	if (ioctl(fd, SX127X_IOCTL_CMD_SETPAOUTPUT, SX127X_PA_PABOOST) != 0) {
		std::cout<<"failed to set pa output"<<std::endl;
        return false;
	}

	if (ioctl(fd, SX127X_IOCTL_CMD_SETMODULATION, SX127X_MODULATION_LORA)
			!= 0) {
		std::cout<<"failed to set modulation"<<std::endl;
        return false;
	}

	if (ioctl(fd, SX127X_IOCTL_CMD_SETCARRIERFREQUENCY, 868100000) != 0) {
		std::cout<<"failed to set carrier frequency"<<std::endl;
        return false;
	}

    if (ioctl(fd, SX127X_IOCTL_CMD_SETSYNCWORD, 0x34) != 0) {
        std::cout<<"failed to set LoraWan Sync Word"<<std::endl;
        return false;
    }

	if (ioctl(fd, SX127X_IOCTL_CMD_SETSF, 7) != 0) {
		std::cout<<"failed to set spreading factor"<<std::endl;
        return false;
	}

	if (ioctl(fd, SX127X_IOCTL_CMD_SETOPMODE, SX127X_OPMODE_CAD) != 0) {
		std::cout<<"failed to set opmode"<<std::endl;
        return false;
	}

    
    return true;

}

bool ImplLoraModem::ReceiveNextPacket(Packet& packet)
{
    bool ret= false;
    uint8_t buff[1025];

    if (read(fd, buff, sizeof(size_t)) &&
    read(fd, buff + sizeof(size_t), *((size_t*) buff)))
    {
        struct sx127x_pkt* pkt = (sx127x_pkt*)(buff);
        uint8_t* payload = buff + pkt->hdrlen;

        printf("payloadlen: %zd bytes, snr: %ddB, rssi: %ddBm\n",
                        pkt->payloadlen, (int) pkt->snr, (int) pkt->rssi);
        //printf("%s\n", payload);

        if ((pkt->payloadlen + pkt->hdrlen) < sizeof (buff))
        {
            packet.SetPayload(
                        7,
                        pkt->snr,
                        pkt->rssi,
                        pkt->payloadlen,
                        (size_t*)payload);
            ret = true;
        }
        else {
            std::cout<<"not enough bits allocated !"<<std::endl;
        }
    }

    return ret;
}

bool ImplLoraModem::SendPacket(const nlohmann::json& json)
{
    
}
