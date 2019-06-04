#ifndef __DISKMANAGER_H__
#define __DISKMANAGER_H__


#include <string>
#include <mutex>
#include <vector>
#include <fstream>
#include <experimental/filesystem>
#include <iostream>
#include <thread>
#include <unistd.h>
#include "common.h"

using namespace std;
enum Mode{ SAVING , READING };

struct FileLock {
	string path;
	mutex mtx;
};

struct FilelistElement {
	string filename;
	uint64_t filesize;
};

class DiskManager{

private:
	string path;
	string path_tmp;
	mutex dmMutex;
	vector<FileLock*> locks;

private:
	uint32_t getFilesize(const string& path);
	string threadId();
	bool deleteAllTmp();
	FileLock* lockFile(const string& path);
	void unlockFile(const string& path);

public:
	DiskManager();
	~DiskManager();

	vector<FilelistElement> getFilelist();
	string getSavePath();
	FILE* openFile(string filename, Mode m);
	int read(vector<byte>& dest, FILE* f, uint32_t howMuch);
	bool write(FILE* f, vector<byte>& data);
	bool finalizeFile(FILE* fp, string sourceTMP);
	void errorOn(FILE* f, string filename, Mode m);
	bool closeFile(FILE* f, const string& filename, Mode m);

};


#endif
