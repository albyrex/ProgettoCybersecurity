#Costanti
INC=-I./src/includes/ -I./src/socket/ -I./src/crypto/ -I./src/diskm/ -I./src/conn/ -I./src/hash/ -I./src/keyManager/ -I./src/random/ -I./src/Cmanager/ -I./src/signer/
OPT=-Wall -Wextra -lcrypto -lpthread -lstdc++fs -g
COMMONDEP=./src/includes/protocol.h ./src/includes/common.h


#Script per avere una divisione netta tra un make e l'altro
start:
	@echo "================================"
	@echo "= Compiling SecSTORE           ="
	@echo "= `date` ="
	@echo "================================\n"
	@make all

#Tutto
all: ./bin/server/server ./bin/client/client ./bin/configuratorTool

#Eseguibili server e client
./bin/server/server: ./src/mainServer.cpp ./src/server.cpp ./src/server.h ./build/TcpSocket.o ./build/TcpListeningSocket.o ./build/Decryptor.o ./build/Encryptor.o ./build/DiskManager.o  ./build/ConnectionServer.o ./build/HmacCalculator.o  ./build/protocol.o ./build/Random.o ./build/Connection.o ./build/Cmanager.o ./build/Signer.o ./build/Verifier.o ./build/KeyManager.o ./build/Utilities.o ./build/SimpleHash.o $(COMMONDEP)

	@echo "\n=== Server executable ==="
	@echo "  >> $@"
	@g++ ./src/mainServer.cpp ./src/server.cpp \
	./build/Decryptor.o ./build/Encryptor.o ./build/TcpSocket.o ./build/TcpListeningSocket.o ./build/DiskManager.o  ./build/ConnectionServer.o ./build/HmacCalculator.o \
	./build/protocol.o ./build/KeyManager.o ./build/Connection.o ./build/Cmanager.o ./build/Signer.o ./build/Verifier.o ./build/Utilities.o ./build/Random.o ./build/SimpleHash.o \
	$(OPT) $(INC)                                             -o $@
	@echo ""

./bin/client/client: ./src/mainClient.cpp ./src/client.cpp ./src/client.h ./build/TcpSocket.o ./build/TcpListeningSocket.o ./build/Decryptor.o ./build/Encryptor.o ./build/HmacCalculator.o ./build/ConnectionClient.o  ./build/protocol.o ./build/Random.o ./build/Connection.o ./build/Cmanager.o ./build/Signer.o ./build/Verifier.o ./build/KeyManager.o ./build/Utilities.o ./build/SimpleHash.o $(COMMONDEP)

	@echo "\n=== Client executable ==="
	@echo "  >> $@"
	@g++ ./src/mainClient.cpp ./src/client.cpp \
	./build/Decryptor.o ./build/Encryptor.o ./build/TcpSocket.o ./build/TcpListeningSocket.o ./build/HmacCalculator.o ./build/ConnectionClient.o ./build/protocol.o  ./build/Connection.o ./build/KeyManager.o ./build/Cmanager.o ./build/Signer.o ./build/Verifier.o ./build/Utilities.o ./build/Random.o ./build/SimpleHash.o \
	$(OPT) $(INC)                                             -o $@
	@echo ""

./bin/configuratorTool: ./src/mainConfiguratorTool.cpp ./build/Cmanager.o

	@echo "\n=== Configurator tool ==="
	@echo "  >> $@"
	@g++ ./src/mainConfiguratorTool.cpp ./build/Cmanager.o \
	$(OPT) $(INC)                                             -o $@
	@echo ""

#File oggetto
./build/TcpSocket.o: ./src/socket/TcpSocket.cpp ./src/socket/TcpSocket.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/socket/TcpSocket.cpp $(OPT) $(INC)          -o $@

./build/TcpListeningSocket.o: ./src/socket/TcpListeningSocket.cpp ./src/socket/TcpListeningSocket.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/socket/TcpListeningSocket.cpp $(OPT) $(INC) -o $@
	
./build/HmacCalculator.o: ./src/hash/HmacCalculator.cpp ./src/hash/HmacCalculator.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/hash/HmacCalculator.cpp $(OPT) $(INC)       -o $@
	
./build/Decryptor.o: ./src/crypto/Decryptor.cpp ./src/crypto/Decryptor.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/crypto/Decryptor.cpp $(OPT) $(INC)          -o $@

./build/Encryptor.o: ./src/crypto/Encryptor.cpp ./src/crypto/Encryptor.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/crypto/Encryptor.cpp $(OPT) $(INC)          -o $@

./build/DiskManager.o: ./src/diskm/DiskManager.cpp ./src/diskm/DiskManager.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/diskm/DiskManager.cpp $(OPT) $(INC)         -o $@

./build/Connection.o: ./src/conn/Connection.cpp ./src/conn/Connection.h ./build/KeyManager.o ./build/Signer.o ./build/Verifier.o $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/conn/Connection.cpp $(OPT) $(INC)           -o $@

./build/ConnectionServer.o: ./src/conn/ConnectionServer.cpp ./src/conn/ConnectionServer.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/conn/ConnectionServer.cpp $(OPT) $(INC)     -o $@

./build/ConnectionClient.o: ./src/conn/ConnectionClient.cpp ./src/conn/ConnectionClient.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/conn/ConnectionClient.cpp $(OPT) $(INC)     -o $@

./build/protocol.o: ./src/includes/protocol.cpp ./src/includes/protocol.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/includes/protocol.cpp $(OPT) $(INC)         -o $@

./build/Random.o: ./src/random/Random.cpp ./src/random/Random.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/random/Random.cpp $(OPT) $(INC)             -o $@
	
./build/KeyManager.o: ./src/keyManager/KeyManager.h ./src/keyManager/KeyManager.cpp $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/keyManager/KeyManager.cpp $(OPT) $(INC)     -o $@

./build/Signer.o: ./src/signer/Signer.h ./src/signer/Signer.cpp $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/signer/Signer.cpp $(OPT) $(INC)             -o $@

./build/Verifier.o: ./src/signer/Verifier.h ./src/signer/Verifier.cpp $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/signer/Verifier.cpp $(OPT) $(INC)           -o $@

./build/Cmanager.o: ./src/Cmanager/Cmanager.h ./src/Cmanager/Cmanager.cpp $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/Cmanager/Cmanager.cpp $(OPT) $(INC)         -o $@

./build/Utilities.o: ./src/includes/Utilities.cpp ./src/includes/Utilities.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/includes/Utilities.cpp $(OPT) $(INC)        -o $@

./build/SimpleHash.o: ./src/hash/SimpleHash.cpp ./src/hash/SimpleHash.h $(COMMONDEP)
	@echo "  >> $@"
	@g++ -c ./src/hash/SimpleHash.cpp $(OPT) $(INC)           -o $@

#Clen
clean:
	rm -f ./build/*.o
	rm -f ./bin/server/server
	rm -f ./bin/client/client
	rm -f ./bin/configuratorTool

#SecSTORE configuration
delete_conf:
	rm -f ./bin/client/priv.pem \
	      ./bin/client/ClientCERT.pem \
	      ./bin/client/cert/* \
	      ./bin/client/crl/* \
	      ./bin/server/priv.pem \
	      ./bin/server/ServerCERT.pem \
	      ./bin/server/cert/* \
	      ./bin/server/crl/* \
	      ./bin/server/allowedClients.txt

restore_default_conf:
	tar -pxvzf default_conf.tar.gz