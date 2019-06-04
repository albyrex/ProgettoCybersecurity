#ifndef __DISKMANAGER_H__
#define __DISKMANAGER_H__


#include <string>
#include <mutex>
#include <vector>
#include <fstream>
#include <experimental/filesystem>
#include <iostream>
#include <thread>
#include "common.h"

using namespace std;
enum Mode{ SAVING , READING };

class DiskManager{

	string path;
	string path_tmp;

private:
	uint32_t getFilesize(string path);
	string threadId();

public:
	DiskManager();
	~DiskManager();

	vector<string> getFilelist();
	string getSavePath();
	FILE* openFile(string filename, Mode m);
	int read(vector<byte>& dest, FILE* f, uint32_t howMuch);
	bool closeFile(FILE* f);
	bool write(FILE* f, vector<byte>& data);
	bool finalizeFile(FILE* fp, string sourceTMP);

	void errorOn(FILE* f, string filename);
	
	bool deleteAllTmp();

};


#endif
