echo "Please enter the client name according to the specification:"
read RISP
echo ${RISP} >> ./server/allowedClients.txt
echo ""
read -n1 -r -p "Press any key to continue..." key
