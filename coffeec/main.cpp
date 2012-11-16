//
//  main.cpp
//  coffeec
//
//  Created by Nathan Van Fleet on 2012-11-15.
//  Copyright (c) 2012 Nathan Van Fleet. All rights reserved.
//

#include <iostream>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/time.h>

#include <sys/poll.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <errno.h>
#include <limits.h>

#include <fcntl.h>
#include <errno.h>

#pragma mark - Setup Code

static int connectWithTimeout (int sfd, struct sockaddr *addr, int addrlen, struct timeval *timeout)
{
    struct timeval sv;
    socklen_t svlen = sizeof sv;
    int ret;
    
    if (!timeout)
        return connect (sfd, addr, addrlen);
    if (getsockopt (sfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&sv, &svlen) < 0)
        return -1;
    if (setsockopt (sfd, SOL_SOCKET, SO_RCVTIMEO, (char *)timeout, sizeof *timeout) < 0)
        return -1;
    ret = connect (sfd, addr, addrlen);
    setsockopt (sfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&sv, sizeof sv);
    return ret;
}

int sendMessage(char *addr, int port, char *command, char *buffer, int bsize)
{
    ssize_t z;
    int com_socket;
    struct sockaddr_in server_address;
    struct timeval timeout;
	in_addr_t address = inet_addr(addr);
	
    // Server
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = address;
	
	// Bad IP or possibly a DNS address
    if (server_address.sin_addr.s_addr == INADDR_NONE)
	{
        fprintf(stderr, "Server IP address failed trying DNS...\n");
		// Check DNS
		char portstr[10];
		sprintf(portstr, "%d", port);
		
		struct addrinfo hints, *p, *servinfo;
		
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		
		// get ready to connect
		if ((z = getaddrinfo(addr, portstr, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(z));
			return 0;
		}
		
		for(p = servinfo;p != NULL; p = p->ai_next)
		{
			// get the pointer to the address itself,
			// different fields in IPv4 and IPv6:
			if (p->ai_family == AF_INET) { // IPv4
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
				server_address.sin_addr = ipv4->sin_addr;
				z = 1;
			} else { // IPv6
				printf("no ipv6 support");
				//            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
				//            server_address.sin6_addr = ipv6->sin6_addr;
				z = 0;
			}
		}
		
		// servinfo now points to a linked list of 1 or more struct addrinfos
		freeaddrinfo(servinfo); // free the linked-list
		
		if(z == 0)
		{
			fprintf(stderr, "No DNS found\n");
			return 0;
		}
	}
    
    if(strlen(command)<=0)
	{
		fprintf(stderr, "No command given\n");
		return 0;
	}
	
    // Create com_socket
    com_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (com_socket == -1)
	{
		fprintf(stderr, "Socket failed\n");
		return 0;
	}
	
    /*
	 // Client
	 struct sockaddr_in client_address;
	 memset(&client_address,0,sizeof client_address);
	 client_address.sin_family = AF_INET;
	 client_address.sin_port = 0;
	 client_address.sin_addr.s_addr = ntohl(INADDR_ANY);
	 
	 if (client_address.sin_addr.s_addr == INADDR_NONE)
	 fprintf(stderr, "Client address failed\n");
	 
	 // Bind
	 z= bind(com_socket, (struct sockaddr *)&client_address, sizeof (client_address));
	 if ( z == -1 )
	 fprintf(stderr,"Binding port\n");
	 */
    
    timeout.tv_sec = 2; /* 2 seconds */
    timeout.tv_usec = 0; /* + 0 usec */
    
    // Connect
    //z = connectWithTimeout(com_socket, (struct sockaddr *) &server_address, len_inet, &timeout);
    z = connect(com_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if(z == -1)
    {
        if(errno == EINPROGRESS)
        {
            fprintf(stderr, "EINPROGRESS non block start\n");
        }
        
        if(errno == EALREADY)
        {
            fprintf(stderr, "EALREADY non block subsequent request\n");
        }
        
        fprintf(stderr, "Connect failed\n");
		
		return 0;
    }
	
    // SEND
    z = send(com_socket, command, strlen(command)+1, 0);
    if (z < 0)
	{
        fprintf(stderr,"send failure\n");
		return 0;
	}
    
    // READ
    z = recv(com_socket, buffer, bsize, 0);
    if (z < 0)
	{
        fprintf(stderr,"receive failure\n");
		return 0;
	}
    
    close(com_socket);
	
	return 1;
}

void printhelp()
{
	fprintf(stderr, "-a IPADDRESS -p PORT -c COMMAND -v\n");
	printf(
		   "usage: GET or SET coffeed with remote commands\n"
		   "------------------\n"
		   "If a value is read+write and you just want to read do not set it\n"
		   "So if you want to know BPOINT just put \"BPOINT\" and it will return it\n"
		   "Chain commands with a comma \"coffeec -c SLEEP=1,PTERM,AUTOT=1\"\n\n"
		   "COMMANDS\n"
		   "------------------\n"
		   "SLEEP=<0/1>         Set to sleep or wake\n"
		   "ACTIVE=<0/1>        Set to sleep or wake (inverse of above)\n"
		   "AUTOT=<0/1>         Get/Set Initiate autotuning cycle for the PID\n"
		   "BPOINT=<float>      Get/Set Brew setpoint\n"
		   "SPOINT=<float>      Get/Set Steam setpoint\n"
		   "PGAIN=<float>       Get/Set PID p-gain\n"
		   "IGAIN=<float>       Get/Set PID i-gain\n"
		   "DGAIN=<float>       Get/Set PID d-gain\n"
		   "OFFSET=<float>      Get/Set boiler temp offset\n"
		   "BMODE=<0/1>         Get/Set to brewmode or steam mode\n"
		   "SMODE=<0/1>         Get/Set to brewmode or steam mode (inverse of above)\n"
		   "TPOINT              Get the current temperature\n"
		   "POW                 Get the current heater power\n"
		   "PTERM               Get the current P value\n"
		   "ITERM               Get the current I value\n"
		   "DTERM               Get the current D value\n"
		   "SETPOINT            Get the current target temperature\n"
		   "SHUTD               Shutdown the system (WARNING: stops remote daemon)\n"
		   "VERSION             Get the version of coffeed\n"
		   );
}

//int main(int argc, const char * argv[])
//{
//
//	// insert code here...
//	std::cout << "Hello, World!\n";
//    return 0;
//}

int main(int argc, char **argv)
{
	int z;
    int opt;
	int port;
	
	int buffersize = 256;
	char buffer[buffersize];
	
	int addrSize = 128;
	char addr[addrSize];
	memset (addr, '\0', addrSize);
	
	int cmdSize = 256;
	char command[cmdSize];
	memset (command, '\0', cmdSize);
	
    //Defaults
	sprintf(addr,"127.0.0.1");
    port = 4949;
    
	//INCLUDED ARGUMENTS FROM CLI
	while((opt = getopt(argc, argv, "a:p:c:v:")) > 0)
	{
		switch(opt)
		{
            case 'a':
				snprintf(addr, addrSize-1, "%s", (char*) optarg);
				break;
            case 'p':
				port = atoi(optarg);
                break;
            case 'c':
				snprintf(command, cmdSize-1, "%s", (char*) optarg);
				break;
			case 'v':
				printf("coffeec version 2.5\n");
				
            default:
				printhelp();
				exit(0);
        }
    }
	
    if(strlen(command)<=0)
	{
		fprintf(stderr, "No command given\n");
		return 0;
	}
	
	z = sendMessage(addr, port, command, buffer, buffersize);
	
	if(z)
		printf("Received response %s\n",buffer);
	else
		printf("ERROR : command failed");
    exit(1);
}
