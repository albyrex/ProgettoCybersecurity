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
#include "File.h"

using namespace std;


class DiskManager{

	uint64_t totalSpace;
	uint64_t usedSpace;
	vector<File*> filelist;
	mutex mutexFileList;
	mutex mutexUsedSpace;
	string path;
	string path_tmp;

private:
	//prova
	void stampaFilelist();
	File* find(string filename);
	uint32_t find(File* f);
	uint32_t getFilesize(string path);
	string threadId();
	bool deleteAllTmp();
}

public:
	DiskManager();
	DiskManager(string path, string path_tmp);
	~DiskManager();

	vector<string> getFilelist();
	string getSavePath();
	bool deleteFile(string filename);
	ifstream openFile(string filename);
	bool read(vector<byte>& dest, ifstream& inputStream, uint32_t howMuch);
	bool reserveSpace(string filename, uint64_t size);
	bool finalizeFile(string filename, uint64_t totalSize);

	//controllare queste sotto

	void closeFile(ifstream& inputStream, string& filename);
	bool write(string filename, vector<byte>& data);
	bool errorOn(string filename, uint64_t totalFilesize);

	bool fileExists(string filename);
	uint32_t getSize(string filename);


};


#endif
