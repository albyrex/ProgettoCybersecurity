#!/bin/bash
echo -------------------------
echo - SecSTORE configurator -
echo -------------------------
chomd +x ./configuratorTool
while true; do
	echo ""
	echo "Steps:"
	echo "1. Copy the certification authority certificate and the CRL"
	echo "2. Configure the client"
	echo "3. Configure the server"
	echo "4. Allow another preconfigured client"
	echo "CTRL+C. Close this menu"
	read RISP
	#Stampo a video il risultato
	if [ ${RISP} = "1" ]
	then
		echo ""
		echo "This step must be done by the user."
		echo "Once you have the certificate of the certification authority copy and past it in the following folders:"
		echo "   SecSTOREroot/bin/server/cert/"
		echo "   SecSTOREroot/bin/client/cert/"
		echo "Remember: in SimpleAuthority to export this certificate you can do Tools->Export->CA Certificate..."
		echo ""
		echo "Once you have the certifictate of the CRL copy and past it in the following folders:"
		echo "   SecSTOREroot/bin/server/crl/"
		echo "   SecSTOREroot/bin/client/crl/"
		echo "Remember: in SimpleAuthority to export this certificate you can do Tools->Export->Certification Revocation List..."
		echo ""
		read -n1 -r -p "Press any key to continue..." key
	elif [ ${RISP} = "2" ]
	then
		echo ""
		rm -f ./client/priv.pem
		openssl ecparam -name prime256v1 -genkey -noout -out ./client/priv.pem
		openssl req -out clientSignRequest.csr -key ./client/priv.pem -new
		echo "A new private key and a new certification sign request for the client are generated"
		echo "You need to give the CSR (./clientSignRequest.csr) to the certification authority in order to obtain a new pem certificate"
		echo "Once you have the new pem certificate copy it with the following path:"
		echo "   ./client/ClientCERT.pem"
		read -n1 -r -p "Then, press any key to continue..." key
		echo "Certificate info:"
		openssl x509 -noout -subject -in ./client/ClientCERT.pem
		rm -f clientSignRequest.csr
		echo ""
		read -n1 -r -p "Press any key to continue..." key
	elif [ ${RISP} = "3" ]
	then
		echo ""
		rm -f ./server/priv.pem
		openssl ecparam -name prime256v1 -genkey -noout -out ./server/priv.pem
		openssl req -out serverSignRequest.csr -key ./server/priv.pem -new
		echo "A new private key and a new certification sign request for the client are generated"
		echo "You need to give the CSR (./serverSignRequest.csr) to the certification authority in order to obtain a new pem certificate"
		echo "Once you have the new pem certificate copy it with the following path:"
		echo "   ./server/ServerCERT.pem"
		read -n1 -r -p "Then, press any key to continue..." key
		echo "Certificate info:"
		openssl x509 -noout -subject -in ./server/ServerCERT.pem
		rm -f serverSignRequest.csr
		echo ""
		./configuratorTool ./client/ClientCERT.pem >> ./server/allowedClients.txt
		echo "The previous configured client is now added to the server whitelist."
		echo ""
		read -n1 -r -p "Press any key to continue..." key
	elif [ ${RISP} = "4" ]
	then
		echo ""
		echo "Type the client folder name"
		read RISP
		path="./$RISP/ClientCERT.pem"
		if [ ! -f "$path" ]
		then
			echo "Error: file not found"
			exit 1
		fi
		echo "" >> ./server/allowedClients.txt
		./configuratorTool ${path} >> ./server/allowedClients.txt
	else
		echo "Unknown command"
	fi
done
