#include <iostream>
#include <string.h>
#include <stdio.h>
#include "client.h"
#include "protocol.h"
#include <openssl/evp.h>


#define HELP_SCREEN "Usage:\n  client serverIp\n"

using namespace std;

int main(int argc, char** argv) {
	string ipServer;
	if(argc < 2) {
		printf(HELP_SCREEN);
		exit(1);
	}
	ipServer = string(argv[1]);

	Client client(ipServer, SERVER_PORT);

	client.run(); //Bloccante
	
	return 0;
}
