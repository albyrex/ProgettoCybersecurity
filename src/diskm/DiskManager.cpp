#include "DiskManager.h"
using namespace std;

DiskManager::DiskManager() {
	this->path = SAVE_PATH;
	this->path_tmp = SAVE_PATH_TMP;
	deleteAllTmp();
}

DiskManager::~DiskManager() {
	dmMutex.lock();
	FileLock* fl;
	while(locks.size() > 0) {
		fl = locks.back();
		delete fl;
		locks.pop_back();
	}
	dmMutex.unlock();
}

FileLock* DiskManager::lockFile(const string& path) {
	uint32_t locksSize = locks.size();
	FileLock* fl = NULL;
	for(uint32_t i = 0; i < locksSize; ++i) {
		if(locks[i]->path == path) {
			fl = locks[i];
			break;
		}
	}
	if(!fl) {
		fl = new FileLock;
		fl->path = path;
		locks.push_back(fl);
	}
	fl->mtx.lock();
	cout << "  >> Lock on " << fl->path << "\n";
	return fl;
}

void DiskManager::unlockFile(const string& path) {
	uint32_t locksSize = locks.size();
	FileLock* fl = NULL;
	for(uint32_t i = 0; i < locksSize; ++i) {
		if(locks[i]->path == path) {
			fl = locks[i];
			break;
		}
	}
	if(!fl) {
		cerr << "DiskManager usage error" << path << "\n";
		return;
	}
	fl->mtx.unlock();
	cout << "  >> Unlock on " << fl->path << "\n";
}

string DiskManager::getSavePath() {
	return this->path;
}

vector<FilelistElement> DiskManager::getFilelist(){
	dmMutex.lock();
	vector<FilelistElement> filelist;
	FilelistElement elem;
	for (const auto & entry : std::experimental::filesystem::directory_iterator(this->path)){
		elem.filename = entry.path().filename();
		elem.filesize = getFilesize(entry.path());
		filelist.push_back(elem);
	}
	dmMutex.unlock();
	return filelist;
}

FILE* DiskManager::openFile(string filename, Mode m ){
	dmMutex.lock();
	FILE* f = NULL;
	string filepath;
	if(m == READING) {
		filepath = SAVE_PATH + filename;
		f = fopen(filepath.c_str(), "rw");
		lockFile(filepath);
	}else if(m == SAVING) {
		filepath = SAVE_PATH_TMP + filename + "_" + threadId();
		f = fopen(filepath.c_str(), "w");
		lockFile(filepath);
	}
	dmMutex.unlock();
	return f;
}

int DiskManager::read(vector<byte>& dest, FILE* f, uint32_t howMuch) {
	return fread( dest.data(), sizeof(byte), howMuch, f );
}

bool DiskManager::closeFile(FILE* f, const string& filename, Mode m){
	string tmpFilePath;
	if(m == SAVING)
		tmpFilePath = SAVE_PATH_TMP + filename + "_" + threadId();
	else
		tmpFilePath = SAVE_PATH + filename;
	
	dmMutex.lock();
	if(f)
		fclose(f);
	unlockFile(tmpFilePath);
	dmMutex.unlock();
	return true;
}

bool DiskManager::write(FILE* f, vector<byte>& data) {
	if(!f)
		return false;
	
	if(fwrite ( data.data(), sizeof(byte), data.size(), f) != data.size()) {
		return false;
	}
	return true;
}

uint32_t DiskManager::getFilesize(const string& path){
    FILE* fp = fopen(path.c_str(), "rw");
    if (!fp) {
		cerr << "DiskManager::getFilesize(...) -> file not found\n";
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

bool DiskManager::finalizeFile(FILE* fp, string sourceTMP){
	if(!fp)
		return false;
	
	dmMutex.lock();
	fclose(fp);
	
	string tmpFilePath = SAVE_PATH_TMP + sourceTMP + "_" + threadId();
	string finalFilePath = SAVE_PATH + sourceTMP;
	
	FILE* source = fopen(tmpFilePath.c_str(), "rw");
	if(!source) {
		dmMutex.unlock();
		return false;
	}
	FILE* dest = fopen(finalFilePath.c_str(), "w");
	if(!dest) {
		fclose(source);
		dmMutex.unlock();
		return false;
	}
	lockFile(finalFilePath);
	
	/*if(std::experimental::filesystem::copy_file(SAVE_PATH_TMP + sourceTMP + "_" + threadId(), SAVE_PATH + sourceTMP, std::experimental::filesystem::copy_options::overwrite_existing)){
		std::experimental::filesystem::remove(SAVE_PATH_TMP + sourceTMP + "_" + threadId());*/
	std::experimental::filesystem::remove(finalFilePath);
	std::experimental::filesystem::rename(tmpFilePath,finalFilePath);
	fclose(source);
	fclose(dest);
	unlockFile(finalFilePath);
	unlockFile(tmpFilePath);
	dmMutex.unlock();
	return true;
	/*}else{
		fclose(source);
		fclose(dest);
		unlockFile(finalFilePath);
		unlockFile(tmpFilePath);
		dmMutex.unlock();
		return false;
	}*/
}



void DiskManager::errorOn(FILE* f, string filename, Mode m) {
	if(!f)
		return;
	
	string tmpFilePath;
	if(m == SAVING)
		tmpFilePath = SAVE_PATH_TMP + filename + "_" + threadId();
	else
		tmpFilePath = SAVE_PATH + filename;
	
	dmMutex.lock();
	fclose(f);
	std::experimental::filesystem::remove(tmpFilePath);
	unlockFile(tmpFilePath);
	dmMutex.unlock();
}


bool DiskManager::deleteAllTmp() {
	for(const auto & entry : std::experimental::filesystem::directory_iterator(SAVE_PATH_TMP)){
		 std::experimental::filesystem::remove(entry.path());
	}
	return true;
}
