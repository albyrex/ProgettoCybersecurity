#include <iostream>
#include "Cmanager.h"

using namespace std;

int main() {
	Cmanager cm;
	
	X509* myCert = cm.loadCERT("Utente1_cert.pem");
	
	cout << "NomeCA: " << cm.getSubjectName(cm.loadCERT("./cert/CA.pem")) << "\n";
	cout << "Nome: " << cm.getCA(myCert) << "\n";
	cout << "Nome: " << cm.getSubjectName(myCert) << "\n";
	
	bool valido = cm.verify(myCert);
	
	cout << "valido: " << valido << "\n";
	
	vector<byte> serializzato = Cmanager::serialize(myCert);
	cout << "Serializzato\n";
	
	X509* deser = Cmanager::deserialize(serializzato);
	cout << "Deserializzato\n";
	
	cout << "Nomes: " << Cmanager::getSubjectName(deser) << endl;
	
	char c;
	cin >> c;
	
	return 0;
}