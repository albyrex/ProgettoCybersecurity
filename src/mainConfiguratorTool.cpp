#include <iostream>
#include "common.h"
#include "Cmanager.h"


using namespace std;

int main(int argc, char* argv[]) {
	
	if(argc < 2)
		return 1;
	
	string filename(argv[1]);
	
	X509* cert = Cmanager::loadCERT(filename);
	string subjectName = Cmanager::getSubjectName(cert);
	
	cout << subjectName;
	
	X509_free(cert);

	return 0;
}
