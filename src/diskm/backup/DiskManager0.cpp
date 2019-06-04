#include "DiskManager.h"



using namespace std;


void DiskManager:: stampaFilelist(){
	for(uint32_t i = 0; i< filelist.size(); ++i){
		std::cout<<filelist[i]->filename+"\n";
	}
}

DiskManager::DiskManager() {
	this->totalSpace = TOTAL_SPACE;
	this->usedSpace = 0;
	this->path = SAVE_PATH;
	this->path_tmp = SAVE_PATH_TMP;

	this->mutexFileList.lock();
	for (const auto & entry : std::experimental::filesystem::directory_iterator(this->path)){
		 string filename = entry.path().filename();
		 filelist.push_back(new File(filename, getFilesize(SAVE_PATH + filename) ) );
	}
	this->mutexFileList.unlock();
	deleteAllTmp();
}

DiskManager::DiskManager(string path, string path_tmp) {

	this->totalSpace = TOTAL_SPACE;
	this->usedSpace = 0;
	this->path = path;
	this->path_tmp = path_tmp;

	this->mutexFileList.lock();
	for (const auto & entry : std::experimental::filesystem::directory_iterator(this->path)){
		 string filename = entry.path().filename();
		 filelist.push_back(new File(filename, getFilesize(SAVE_PATH + filename) ) );
		 //cout<<filename<<" "<<getFilesize(SAVE_PATH + filename)<<endl;
	}
	this->mutexFileList.unlock();
}

string DiskManager::getSavePath() {
	return this->path;
}

DiskManager::~DiskManager() {
	this->mutexFileList.lock();
	for(File* f : filelist){
		delete f;
	}
	this->mutexFileList.unlock();
}

vector<string> DiskManager::getFilelist(){
	this->mutexFileList.lock();
	vector<string> vectorFileList;
	vectorFileList.reserve(filelist.size());
	for(uint32_t i = 0; i < filelist.size(); ++i){
		filelist[i]->mutexFile.lock();
		vectorFileList.push_back(filelist[i]->filename);
		filelist[i]->mutexFile.unlock();
	}
	this->mutexFileList.unlock();
	return vectorFileList;
}

ifstream DiskManager::openFile(string filename){

	this->mutexFileList.lock();
	File* f = find(filename);
	this->mutexFileList.unlock();
	if(f == NULL){
		cout<<"File not found"<<endl;
		throw "file_not_found";
	}

	f->mutexFile.lock();


	ifstream inputStream;

	string realPath = SAVE_PATH + filename;

	inputStream.open(realPath, ios::binary|ios::in);

	if(inputStream.fail()){
		cout<<"inputStream fallito"<<endl;
		exit(1);
	}

	return inputStream;

}

bool DiskManager::read(vector<byte>& dest, ifstream& inputStream, uint32_t howMuch) {
	inputStream.read((char*)dest.data(), howMuch);
	if(inputStream.fail()){
		return false;
	}
	return true;
}

void DiskManager::closeFile(ifstream& inputStream, string& filename){
	this->mutexFileList.lock();
	File* f = find(filename);
	this->mutexFileList.unlock();
	if(f == NULL){
		cout<<"File not found"<<endl;
		exit(1);
	}
	inputStream.close();
	f->mutexFile.unlock();

}

bool DiskManager::write(string filename, vector<byte>& data) {

	ofstream outputStream;

	string name_ = SAVE_PATH_TMP + filename + "_" + threadId();
	outputStream.open(name_, ios::binary|ios::app);
	//std::cout<<"sto scrivendo nel file: "<<name_<<endl;
	if(outputStream.fail()){
		std::cout<<"errore apertura file: "<<name_<<endl;
		return false;
	}
	outputStream.write((char*)data.data(), data.size());
	outputStream.close();
	if(outputStream.fail()){
		deleteFile(name_);
		return false;
	}

	return true;
}

bool DiskManager::deleteFile(string filename) {

  if(std::experimental::filesystem::remove(SAVE_PATH + filename) != 0)
    return false;
  else{
		this->mutexFileList.lock();

		this->mutexUsedSpace.lock();

		File* f = find(filename);
		if(f == NULL){
			this->mutexUsedSpace.unlock();
			this->mutexFileList.unlock();
			return false;
		}
		this->usedSpace -= f->size;
		filelist.erase(filelist.begin()+find(f));

		this->mutexUsedSpace.unlock();
		this->mutexFileList.unlock();

		return true;
	}
}

uint32_t DiskManager::find(File* f){
	for(uint32_t pos = 0; pos < filelist.size(); ++pos){
		if(filelist[pos]->filename == f->filename){
			return pos;
		}
	}
	return -1;
}

bool DiskManager::finalizeFile(string filename, uint64_t totalSize){


	string name_ = SAVE_PATH_TMP + filename + "_" + threadId();
	uint64_t file_size = getFilesize(name_);

	if(file_size != totalSize){
		cout<<"file_size != totalSize"<<endl;
		return false;
	}


	this->mutexFileList.lock();

	File* f = find(filename);

	if(f == NULL){
		if(std::experimental::filesystem::copy_file(name_, SAVE_PATH + filename, std::experimental::filesystem::copy_options::overwrite_existing)){
			std::experimental::filesystem::remove(name_);
			f = new File(filename, totalSize);
			this->filelist.push_back(f);
		}else{
			cerr << "Errore into copy_file()" << endl;
			return false;
		}
	}else{
		f->mutexFile.lock();
		if(std::experimental::filesystem::copy_file(name_, SAVE_PATH + filename, std::experimental::filesystem::copy_options::overwrite_existing)){
			std::experimental::filesystem::remove(name_);
			f->size = totalSize;
			f->mutexFile.unlock();
		}else{
			cerr << "Errore into copy_file()" << endl;
			f->mutexFile.unlock();
			return false;
		}
	}
	this->mutexFileList.unlock();

	return true;

}

File* DiskManager::find(string filename){

	for(uint32_t i = 0; i < filelist.size(); ++i){
		if(filelist[i]->filename == filename){
			return filelist[i];
		}
	}

	return NULL;

}

uint32_t DiskManager::getFilesize(string path){

	ifstream in_file(path, std::ios::binary | std::ios::ate);
	if(!in_file.is_open()){
		return false;
	}
	uint32_t file_size = in_file.tellg();
	return file_size;
}

string DiskManager::threadId() {
	thread::id thread_id = std::this_thread::get_id();
	stringstream tmp;
	tmp << thread_id;
	string t_id = tmp.str();
	return t_id;
}

bool DiskManager::reserveSpace(string filename, uint64_t size){
	this->mutexFileList.lock();
	this->mutexUsedSpace.lock();
	File* f = find(filename);
	if(f != NULL){
		/* file gia presente =  sovrascrittura */
		this->usedSpace -= f->size;
	}

	if(this->usedSpace + size >this->totalSpace){
		if(f != NULL)
			this->usedSpace += f->size;
		this->mutexUsedSpace.unlock();
		this->mutexFileList.unlock();
		return false;
	}

	this->usedSpace += size;
	this->mutexUsedSpace.unlock();
	this->mutexFileList.unlock();
	return true;
}

bool DiskManager::errorOn(string filename, uint64_t totalFilesize){
	this->mutexFileList.lock();
	File* f = find(filename);
	this->mutexFileList.unlock();

	this->mutexUsedSpace.lock();
	if(f != NULL){
		f->mutexFile.lock();
		this->usedSpace += f->size;
		f->mutexFile.unlock();
	}
	this->usedSpace -= totalFilesize;
	std::experimental::filesystem::remove(SAVE_PATH_TMP + filename + "_" + threadId());
	this->mutexUsedSpace.unlock();

	return true;
}

bool DiskManager::fileExists(string filename){
	this->mutexFileList.lock();
	File* f = find(filename);
	if(f != NULL){
		this->mutexFileList.unlock();
		return true;
	}
	this->mutexFileList.unlock();
	return false;
}

uint32_t DiskManager::getSize(string filename){
	this->mutexFileList.lock();
	File* f = find(filename);
	this->mutexFileList.unlock();
	if(f ==  NULL){
		return -1;
	}
	//f->mutexFile.lock();
	uint32_t size = f->size;
	//f->mutexFile.unlock();
	return size;

}

bool DiskManager::deleteAllTmp() {
	for (const auto & entry : std::experimental::filesystem::directory_iterator(SAVE_PATH_TMP)){
		 std::experimental::filesystem::remove(entry.path());
	}
	return true;
}