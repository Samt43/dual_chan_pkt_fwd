#include <Modem/ILoraModem.h>
#include "Server.hpp"
#include "base64.h"
#include <bitset>
#include <sstream>
#include <iostream>
#include <string>

// UDP Connection
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>

#define BASE64_MAX_LENGTH 341
#define TX_BUFF_SIZE  2048
#define PROTOCOL_VERSION  1
#define PKT_PUSH_DATA 0
#define PKT_PUSH_ACK  1
#define PKT_PULL_DATA 2
#define PKT_PULL_RESP 3
#define PKT_PULL_ACK  4


#define COLWIDTH 8
#define OFFSET(r, c) ((r * COLWIDTH) + c)
#define REMAINING(r, c, l) ((int) l - OFFSET(r, c))
void printhex(uint8_t* buff, size_t len) {
    int col = 0;
    for (int row = 0; (REMAINING(row, 0, len) > 0); row++) {
        for (col = 0; (col < COLWIDTH) && (REMAINING(row, col, len) > 0);
                col++) {
            uint8_t b = buff[OFFSET(row, col)];
            char c = b >= 0x20 && b <= 0x7e ? b : '.';
            printf("%02x [%c] ", b, c);
        }
        printf("\n");
    }
}


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

    if (!modem->Start(ILoraModem::Configuration()))
    {
        std::cerr<< "Can't start modem";
        return -1;
    }

    m_pUDPConnection = std::make_unique<UDPConnection>(config);
    if (!m_pUDPConnection->Connect())
    {
        std::cout<<"Could not connect, failure";
        exit(-1);
    }


    const auto& ifr = m_pUDPConnection->GetIFreq();
    // ID based on MAC Adddress of interface
    printf( "Gateway ID: %.2x:%.2x:%.2x:ff:ff:%.2x:%.2x:%.2x\n",
                (uint8_t)ifr.ifr_hwaddr.sa_data[0],
                (uint8_t)ifr.ifr_hwaddr.sa_data[1],
                (uint8_t)ifr.ifr_hwaddr.sa_data[2],
                (uint8_t)ifr.ifr_hwaddr.sa_data[3],
                (uint8_t)ifr.ifr_hwaddr.sa_data[4],
                (uint8_t)ifr.ifr_hwaddr.sa_data[5]
    );

    while (1)
    {
        ILoraModem::Packet packet;
        if (modem->ReceiveNextPacket(packet))
        {
            std::cout << "Packet to be transmited : ";
            std::cout << packet.payload << std::endl
                      << "RSSI : " << packet.rssi << std::endl
                      << "SNR : " << packet.snr << std::endl;

            printhex((u_int8_t*)packet.payload, packet.payloadLen);

            /* 12-byte header */
            std::stringstream stream;

            stream
                    << (uint8_t)PROTOCOL_VERSION
                    << (uint8_t)rand()
                    << (uint8_t)rand()
                    << (uint8_t)PKT_PUSH_DATA
                    << (uint8_t)ifr.ifr_hwaddr.sa_data[0]
                    << (uint8_t)ifr.ifr_hwaddr.sa_data[1]
                    << (uint8_t)ifr.ifr_hwaddr.sa_data[2]
                    << (uint8_t)0xFF
                    << (uint8_t)0xFF
                    << (uint8_t)ifr.ifr_hwaddr.sa_data[3]
                    << (uint8_t)ifr.ifr_hwaddr.sa_data[4]
                    << (uint8_t)ifr.ifr_hwaddr.sa_data[5];


            for (auto& c : stream.str())
            {
                std::cout << +(uint8_t)c << " ";
            }
            std::cout << stream.str() << "Lenght : " << stream.tellp() << " bytes";
            //buff_index = 12;

            // TODO: tmst can jump is time is (re)set, not good.
            struct timeval now;
            gettimeofday(&now, nullptr);
            uint32_t tmst = (uint32_t)(now.tv_sec * 1000000 + now.tv_usec);

            // Encode payload.
            char b64[BASE64_MAX_LENGTH];
            bin_to_b64((uint8_t*)packet.payload, (int)packet.payloadLen, b64, BASE64_MAX_LENGTH);

            nlohmann::json message;
            message["rxpk"] = nlohmann::json().array();
            auto rxpk = nlohmann::json();
            rxpk["tmst"] = tmst;
            rxpk["freq"] = (double)868100000/1000000;

            //TODO
            rxpk["chan"] = 0;

            rxpk["rfch"] = 0;
            rxpk["stat"] = 1;
            rxpk["modu"] = "LORA";

            char datr[] = "SFxxBWxxx";


            // TODO : BW
            snprintf(datr, strlen(datr) + 1, "SF%hhuBW%hu", packet.sf, 125);
            rxpk["datr"] = datr;

            rxpk["codr"] = "4/5";
            rxpk["rssi"] = packet.rssi;
            rxpk["lsnr"] = packet.snr;
            rxpk["size"] = packet.payloadLen;
            rxpk["data"] = b64;

            message["rxpk"].push_back(rxpk);

            std::cout << "rxpk update: %s\n"
                      << message;

            // Build and send message.
            //memcpy(buff_up + 12, json.c_str(), json.size());
            //SendUdp(buff_up, buff_index + json.size());

            stream << message.dump();
            m_pUDPConnection->SendUdp(stream.str());
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


ifreq UDPConnection::ComputeIfreq(int socket)
{
    std::string mac;
    std::string socketInterface;
    struct sockaddr_in addr;
    struct ifaddrs* ifaddr;
    struct ifaddrs* ifa;
    socklen_t addr_len;
    struct ifreq ifr;

    addr_len = sizeof (addr);
    getsockname(m_Socket, (struct sockaddr*)&addr, &addr_len);
    getifaddrs(&ifaddr);

    // look which interface contains the wanted IP.
    // When found, ifa->ifa_name contains the name of the interface (eth0, eth1, ppp0...)
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr)
        {
            if (AF_INET == ifa->ifa_addr->sa_family)
            {
                struct sockaddr_in* inaddr = (struct sockaddr_in*)ifa->ifa_addr;

                if (inaddr->sin_addr.s_addr == addr.sin_addr.s_addr)
                {
                    if (ifa->ifa_name)
                    {
                        socketInterface = ifa->ifa_name;
                        // Found it
                    }
                }
            }
        }
    }
    freeifaddrs(ifaddr);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, socketInterface.c_str(), IFNAMSIZ-1);  // use configured network interface eth0 or wlan0
    ioctl(m_Socket, SIOCGIFHWADDR, &ifr);


    return ifr;
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

    m_ifr = ComputeIfreq(m_Socket);

    /* If we made it through to here, this server is live */
    printf("INFO: Successfully contacted server %s (port %i) \n", m_ServeurParams.address.c_str(), m_ServeurParams.port);

    return true;
}
