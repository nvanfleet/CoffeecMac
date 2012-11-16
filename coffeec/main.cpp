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

#include "coffeec.h"

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
