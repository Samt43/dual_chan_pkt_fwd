#pragma once
#include <string>
#include <memory>

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

    bool Connect();

private:
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

