#include <iostream>
#include "protocol.h"
#include "server.h"
#include <openssl/evp.h>


#define WELCOME_SCREEN \
"  mmmm                 mmmm mmmmmmm  mmmm  mmmmm  mmmmmm\n"\
" #\"   \"  mmm    mmm   #\"   \"   #    m\"  \"m #   \"# #\n"\
" \"#mmm  #\"  #  #\"  \"  \"#mmm    #    #    # #mmmm\" #mmmmm\n"\
"     \"# #\"\"\"\"  #          \"#   #    #    # #   \"m #\n"\
" \"mmm#\" \"#mm\"  \"#mm\"  \"mmm#\"   #     #mm#  #    m #mmmmm\n"


using namespace std;

int main() {

	cout << WELCOME_SCREEN << '\n';
	
	
	Server* server = Server::getTheServer();
	server->switchOn(); //Bloccante fino al CTRL+C
	delete server;



	return 0;
}
