#include "DiskManager.h"
using namespace std;

DiskManager::DiskManager() {
	this->path = SAVE_PATH;
	this->path_tmp = SAVE_PATH_TMP;
	deleteAllTmp();
}


string DiskManager::getSavePath() {
	return this->path;
}

DiskManager::~DiskManager() {}

vector<string> DiskManager::getFilelist(){
	vector<string> filelist;
	string filename;
	for (const auto & entry : std::experimental::filesystem::directory_iterator(this->path)){
		 filename = entry.path().filename();
		 filelist.push_back(filename);
	}
	return filelist;
}

FILE* DiskManager::openFile(string filename, Mode m ){
	FILE* f;
	if(m == READING) {
		f = fopen( (SAVE_PATH + filename).c_str(), "rw");
	}else if(m == SAVING) {
		f = fopen( (SAVE_PATH_TMP + filename + "_" + threadId() ).c_str(), "w");
	}else {
		return NULL;
	}
	return f;
}

int DiskManager::read(vector<byte>& dest, FILE* f, uint32_t howMuch) {
	return fread( dest.data(), sizeof(byte), howMuch, f );
}

bool DiskManager::closeFile(FILE* f){
	if(fclose(f) != 0) {
		return false;
	}
	return true;
}

bool DiskManager::write(FILE* f, vector<byte>& data) {
	if(fwrite ( data.data(), sizeof(byte), data.size(), f) != data.size()) {
		return false;
	}
	return true;
}

uint32_t DiskManager::getFilesize(string path){
    FILE* fp = fopen(path.c_str(), "rw");
    if (!fp) {
        printf("File Not Found!\n");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    long int res = ftell(fp);
    fclose(fp);
    return res;
}

string DiskManager::threadId() {
	thread::id thread_id = std::this_thread::get_id();
	stringstream tmp;
	tmp << thread_id;
	string t_id = tmp.str();
	return t_id;
}

/* ================================================================= */

bool DiskManager::finalizeFile(FILE* fp, string sourceTMP){
	fclose(fp);
	FILE* source = fopen( (SAVE_PATH_TMP + sourceTMP + "_" + threadId()).c_str(), "rw");
	if(!source) {
		return false;
	}
	FILE* dest = fopen( (SAVE_PATH + sourceTMP).c_str(), "w");
	if(!dest) {
		fclose(source);
		return false;
	}
	if(std::experimental::filesystem::copy_file(SAVE_PATH_TMP + sourceTMP + "_" + threadId(), SAVE_PATH + sourceTMP, std::experimental::filesystem::copy_options::overwrite_existing)){
		std::experimental::filesystem::remove(SAVE_PATH_TMP + sourceTMP + "_" + threadId());
		fclose(source);
		fclose(dest);
		return true;
	}else{
		fclose(source);
		fclose(dest);
		return false;
	}
		fclose(source);
		fclose(dest);
	return false;
}



void DiskManager::errorOn(FILE* f, string filename) {
	string realFilename = SAVE_PATH_TMP + filename + "_" + threadId();
	fclose(f);
	std::experimental::filesystem::remove(realFilename);
}


bool DiskManager::deleteAllTmp() {
	for (const auto & entry : std::experimental::filesystem::directory_iterator(SAVE_PATH_TMP)){
		 std::experimental::filesystem::remove(entry.path());
	}
	return true;
}