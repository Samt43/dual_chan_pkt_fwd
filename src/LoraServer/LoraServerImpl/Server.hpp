#pragma once

class Server
{
public : 
    bool Start();
    void Stop();

    typedef struct Server
    {
        std::string address;
        uint16_t port;
        bool enabled;
    } IPServer;
        
protected:
    std::vector<IPServer> m_Servers;
    
    
};
