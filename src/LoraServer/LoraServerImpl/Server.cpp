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
    serveur.Start();
    
    // Todo : implement stop when SIG received
    
    return 0;
    
}

void Die(const char *s)
{
  perror(s);
  exit(1);
}

bool Server::Start()
{
    std::cout << "Server Starting" << std::endl;
    auto modem = LoraModemBuilder::CreateModem();
    modem->Start(ILoraModem::Configuration());

	/* network socket creation */
	struct addrinfo hints;
	struct addrinfo *result; /* store result of getaddrinfo */
	struct addrinfo *q; /* pointer to move into *result data */
	char host_name[64];
	char port_name[64];
	/* prepare hints to open network sockets */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; /* should handle IP v4 or v6 automatically */
	hints.ai_socktype = SOCK_DGRAM;

    for(const auto& server : m_Servers) {
        if (server.enabled) 
        {
            si_other.sin_port = htons(it->port);

            SolveHostname(it->address.c_str(), it->port, &si_other);

            i = getaddrinfo(it->address.c_str(), "1700", &hints, &result);
            if (i != 0) {
                printf("ERROR: [down] getaddrinfo on address %s (port %i) returned: %s\n", it->address.c_str(), it->port, gai_strerror(i));
            }

            /* try to open socket for downstream traffic */
            for (q=result; q!=NULL; q=q->ai_next) {
                sock_down[ic] = socket(q->ai_family, q->ai_socktype,q->ai_protocol);
                if (sock_down[ic] == -1) continue; /* try next field */
                else break; /* success, get out of loop */
            }
            if (q == NULL) {
                printf("ERROR: [down] failed to open socket to any of server %s addresses (port %i)\n", it->address.c_str(), it->port);
                i = 1;
                for (q=result; q!=NULL; q=q->ai_next) {
                    getnameinfo(q->ai_addr, q->ai_addrlen, host_name, sizeof host_name, port_name, sizeof port_name, NI_NUMERICHOST);
                    printf("INFO: [down] result %i host:%s service:%s\n", i, host_name, port_name);
                    ++i;
                }
            }
            /* connect so we can send/receive packet with the server only */
            i = connect(sock_down[ic], q->ai_addr, q->ai_addrlen);
            if (i != 0) {
            printf("ERROR: [down] connect address %s (port %i) returned: %s\n", it->address.c_str(), it->port, strerror(errno));
            }
            freeaddrinfo(result);

            /* If we made it through to here, this server is live */
            printf("INFO: Successfully contacted server %s (port %i) \n", it->address.c_str(), it->port);
    }

    
    while (1)
    {
        nlohmann::json json;
        if (modem->ReceiveNextPacket(json))
        {
            std::cout << json;
        }
        else
        {
            std::cout << "Error while receiving next Packet";
            
        }
    }
    
}

void Server::Stop()
{
    
    
}


void SolveHostname(const char* p_hostname, uint16_t port, struct sockaddr_in* p_sin)
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  char service[6] = { '\0' };
  snprintf(service, 6, "%hu", port);

  struct addrinfo* p_result = NULL;

  // Resolve the domain name into a list of addresses
  int error = getaddrinfo(p_hostname, service, &hints, &p_result);
  if (error != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
      exit(EXIT_FAILURE);
  }

  // Loop over all returned results
  for (struct addrinfo* p_rp = p_result; p_rp != NULL; p_rp = p_rp->ai_next) {
    struct sockaddr_in* p_saddr = (struct sockaddr_in*)p_rp->ai_addr;
    //printf("%s solved to %s\n", p_hostname, inet_ntoa(p_saddr->sin_addr));
    p_sin->sin_addr = p_saddr->sin_addr;
  }

  freeaddrinfo(p_result);
}


void SendUdp(char *msg, int length)
{
    for(const auto& server : m_Servers) {
        if (server.enabled) 
        {
            si_other.sin_port = htons(server.port);

            if (debug>1) {printf ("Sending message to server: %s\n", server.address.c_str());
            // for (int i=0;i<length;i++) {
            //   printf("%i:", msg[i]);
            // }
            //		printf("\n");
            }
            SolveHostname(server.address.c_str(), server.port, &si_other);
            if (sendto(s, (char *)msg, length, 0 , (struct sockaddr *) &si_other, slen)==-1)
            {
                Die("sendto()");
            }
        }
    }
}

