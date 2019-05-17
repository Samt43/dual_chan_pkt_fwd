#pragma once
#include <string>
#include <memory>
#include <net/if.h>

typedef struct IPServer
{
    std::string address;
    uint16_t port;
} IPServer;

class UDPConnection
{
public :
    UDPConnection(const IPServer& serveurParams) : m_ServeurParams(serveurParams)
    {}

    void SendUdp(const std::string& str);

    const ifreq& GetIFreq() const
    {
        return m_ifr;
    }

    bool Connect();

private:
    struct ifreq ComputeIfreq(int socket);
    struct ifreq m_ifr;
    IPServer m_ServeurParams;
    int m_Socket;

};

class Server
{
public : 
    bool Start(const IPServer& config);
    void Stop();

    Server() = default;

    void SendUdp(const std::string& str);

        
protected:
    std::unique_ptr<UDPConnection> m_pUDPConnection;
    
};

