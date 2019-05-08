#include <Modem/ILoraModem.h>
#include "Server.hpp"
#include <iostream>

// UDP Connection
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>


int main()
{
    Server serveur;
    serveur.Start(IPServer{"router.eu.staging.thethings.network",1700});
    
    // Todo : implement stop when SIG received
    
    return 0;
    
}

void Die(const char *s)
{
  perror(s);
  exit(1);
}

bool Server::Start(const IPServer& config)
{
    std::cout << "Server Starting" << std::endl;
    auto modem = LoraModemBuilder::CreateModem();
    modem->Start(ILoraModem::Configuration());

    m_pUDPConnection = std::make_unique<UDPConnection>(config);
    if (!m_pUDPConnection->Connect())
    {
        std::cout<<"Could not connect, failure";
        exit(-1);
    }

    while (1)
    {
        std::string packet;
        ILoraModem::PacketInfos infos;
        if (modem->ReceiveNextPacket(packet, infos))
        {
            std::cout << "Packet to be transmited : ";
            std::cout << packet << std::endl
                      << "RSSI : " << infos.rssi << std::endl
                      << "SNR : " <<infos.lsnr << std::endl;

            /*
            // OK got one
            bool ret = true;

            uint8_t value = ReadRegister(REG_PKT_SNR_VALUE, CE);

            if (value & 0x80) { // The SNR sign bit is 1
              // Invert and divide by 4
              value = ((~value + 1) & 0xFF) >> 2;
              SNR = -value;
            } else {
              // Divide by 4
              SNR = ( value & 0xFF ) >> 2;
            }

            rssicorr = sx1272 ? 139 : 157;

            printf("CE%i Packet RSSI: %d, ", CE, ReadRegister(0x1A, CE) - rssicorr);
            printf("RSSI: %d, ", ReadRegister(0x1B,CE) - rssicorr);
            printf("SNR: %li, ", SNR);
            printf("Length: %hhu Message:'", length);
            for (int i=0; i<length; i++) {
              char c = (char) message[i];
              printf("%c",isprint(c)?c:'.');
            }
            printf("'\n");

            char buff_up[TX_BUFF_SIZE]; /* buffer to compose the upstream packet
            int buff_index = 0;

            /* gateway <-> MAC protocol variables */
            //static uint32_t net_mac_h; /* Most Significant Nibble, network order */
            //static uint32_t net_mac_l; /* Least Significant Nibble, network order */

            /* pre-fill the data buffer with fixed fields
            buff_up[0] = PROTOCOL_VERSION;
            buff_up[3] = PKT_PUSH_DATA;

            /* process some of the configuration variables
            //net_mac_h = htonl((uint32_t)(0xFFFFFFFF & (lgwm>>32)));
            //net_mac_l = htonl((uint32_t)(0xFFFFFFFF &  lgwm  ));
            //*(uint32_t *)(buff_up + 4) = net_mac_h;
            //*(uint32_t *)(buff_up + 8) = net_mac_l;

            buff_up[4] = (uint8_t)ifr.ifr_hwaddr.sa_data[0];
            buff_up[5] = (uint8_t)ifr.ifr_hwaddr.sa_data[1];
            buff_up[6] = (uint8_t)ifr.ifr_hwaddr.sa_data[2];
            buff_up[7] = 0xFF;
            buff_up[8] = 0xFF;
            buff_up[9] = (uint8_t)ifr.ifr_hwaddr.sa_data[3];
            buff_up[10] = (uint8_t)ifr.ifr_hwaddr.sa_data[4];
            buff_up[11] = (uint8_t)ifr.ifr_hwaddr.sa_data[5];

            /* start composing datagram with the header
            uint8_t token_h = (uint8_t)rand(); /* random token
            uint8_t token_l = (uint8_t)rand(); /* random token
            buff_up[1] = token_h;
            buff_up[2] = token_l;
            buff_index = 12; /* 12-byte header

            // TODO: tmst can jump is time is (re)set, not good.
            struct timeval now;
            gettimeofday(&now, NULL);
            uint32_t tmst = (uint32_t)(now.tv_sec * 1000000 + now.tv_usec);

            // Encode payload.
            char b64[BASE64_MAX_LENGTH];
            bin_to_b64((uint8_t*)message, length, b64, BASE64_MAX_LENGTH);

            // Build JSON object.
            StringBuffer sb;
            Writer<StringBuffer> writer(sb);
            writer.StartObject();
            writer.String("rxpk");
            writer.StartArray();
            writer.StartObject();
            writer.String("tmst");
            writer.Uint(tmst);
            writer.String("freq");
            if (CE == 0) {
              writer.Double((double)freq / 1000000);
              writer.String("chan");
              writer.Uint(0);
            } else {
              writer.Double((double)freq_2 / 1000000);
              writer.String("chan");
              writer.Uint(1);
            }
            writer.String("rfch");
            writer.Uint(0);
            writer.String("stat");
            writer.Uint(1);
            writer.String("modu");
            writer.String("LORA");
            writer.String("datr");
            char datr[] = "SFxxBWxxx";
            snprintf(datr, strlen(datr) + 1, "SF%hhuBW%hu", sf, bw);
            writer.String(datr);
            writer.String("codr");
            writer.String("4/5");
            writer.String("rssi");
            writer.Int(ReadRegister(0x1A, CE) - rssicorr);
            writer.String("lsnr");
            writer.Double(SNR); // %li.
            writer.String("size");
            writer.Uint(length);
            writer.String("data");
            writer.String(b64);
            writer.EndObject();
            writer.EndArray();
            writer.EndObject();

            string json = sb.GetString();
            printf("rxpk update: %s\n", json.c_str());

            // Build and send message.
            memcpy(buff_up + 12, json.c_str(), json.size());
            SendUdp(buff_up, buff_index + json.size());

            */

        }
        else
        {
            std::cout << "Error while receiving next Packet";
            
        }
    }
    
}

void Server::Stop()
{
    // TODO;
    
}


void Server::SendUdp(const std::string& str)
{
    m_pUDPConnection->SendUdp(str);
}


void UDPConnection::SendUdp(const std::string &str)
{
    if (send(m_Socket, str.c_str(), str.length(), 0) == -1)
    {
        Die("sendto()");
    }
}

bool UDPConnection::Connect()
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo* p_result = nullptr;
    int ret = getaddrinfo(m_ServeurParams.address.c_str(), "1700", &hints, &p_result);
    if (ret != 0) {
        printf("ERROR: [down] getaddrinfo on address %s (port %i) returned: %s\n", m_ServeurParams.address.c_str(), m_ServeurParams.port, gai_strerror(ret));
    }

    /* try to open socket for downstream traffic */
    struct addrinfo* currentResult = nullptr;
    for (currentResult=p_result; currentResult!=nullptr; currentResult=currentResult->ai_next) {
        m_Socket = socket(currentResult->ai_family, currentResult->ai_socktype,currentResult->ai_protocol);
        if (m_Socket == -1) continue; /* try next field */
        else break; /* success, get out of loop */
    }
    if (currentResult == nullptr) {
        printf("ERROR: [down] failed to open socket to any of server %s addresses (port %i)\n", m_ServeurParams.address.c_str(), m_ServeurParams.port);
        return false;
    }
    /* connect so we can send/receive packet with the server only */
    ret = connect(m_Socket, currentResult->ai_addr, currentResult->ai_addrlen);
    if (ret != 0) {
        printf("ERROR: [down] connect address %s (port %i) returned: %s\n", m_ServeurParams.address.c_str(), m_ServeurParams.port, strerror(errno));
    }
    freeaddrinfo(currentResult);

    /* If we made it through to here, this server is live */
    printf("INFO: Successfully contacted server %s (port %i) \n", m_ServeurParams.address.c_str(), m_ServeurParams.port);

    return true;

}
