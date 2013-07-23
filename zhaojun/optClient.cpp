#include<iostream>
#include <sys/time.h>

#include <string.h>

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>

using namespace std;

class FileSeg{
	public:
		FileSeg(int clientId,int fileId,int segId){
			mFileId = fileId;
			mSegId = segId;
			mClientId = clientId;
		}
	int mClientId;
	int mFileId;
	int mSegId;
};

vector<FileSeg> sequenceList;
void Trim(std::string &s){
	s.erase(0,s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
}
void ParseConfigFile(char *configFileName,std::map<std::string,std::string> &keyMap){
	std::ifstream infile;
	infile.open(configFileName);

	std::string line;
	while(std::getline(infile,line)){
		if(line.empty())
			continue;
		Trim(line);
		if(line.at(0) == '#')
			continue;
		int equalPos = line.find_first_of('=');
		std::string key = line.substr(0,equalPos);
		std::string value = line.substr(equalPos + 1);
		Trim(key);
		Trim(value);
		keyMap.insert(std::make_pair(key,value));

//		cout << key << "=" << value << endl;

//		if(!infile.good())
//			break;
	}


	infile.close();
}



void clientOptStrategy(int clientNum,int blockNum){
	unsigned int hitTimes =0,totalRequest=0;
	map<int ,vector<FileSeg> > buf;
	int curFileId,curSegId,curClient;
	for(unsigned int i=0;i<sequenceList.size();i++){
		curFileId = sequenceList[i].mFileId;
		curSegId = sequenceList[i].mSegId;
		curClient = sequenceList[i].mClientId;
		totalRequest++;
		//in the buf
		bool isHit = false;
		for(int i = 1;i<= clientNum;i++){
			for(size_t j=0;j<buf[i].size();j++){
				if(buf[i][j].mSegId == curSegId && buf[i][j].mFileId == curFileId){
					isHit = true;
					break;
				}
			}
			if(isHit == true)
				break;
		}
		if(isHit == true){
			hitTimes ++;
			continue;
		}
		//not in the buf
		if(buf[curClient].size()< blockNum){//just push in
			buf[curClient].push_back(sequenceList[i]);
		}else{//eliminate 
			vector<int> dis(blockNum);
			for(int j=0;j<dis.size();j++){
				dis[j]=sequenceList.size();
			}
			int max= 0;
			int targetIndex;
			for(size_t j=0;j<buf[curClient].size();j++){
				for(size_t k = i+1;k<sequenceList.size();k++){
					if( sequenceList[k].mFileId == buf[curClient][j].mFileId && sequenceList[k].mSegId == buf[curClient][j].mSegId){
						dis[j]=k;
						break;
					}
				}

			}
			//chose the max k
			for(size_t j=0;j<dis.size();j++){
				if(dis[j] > max){
					max = dis[j];
					targetIndex = j;
				}
			}
			buf[curClient].erase(buf[curClient].begin()+targetIndex);
			buf[curClient].push_back(sequenceList[i]);

		}
	}
	cout<<hitTimes<<" "<<totalRequest<<" "<<hitTimes*1.0/totalRequest<<endl;
}
void clientOptStrategy1(int clientNum,int blockNum){
	unsigned int hitTimes =0,totalRequest=0;
	map<int ,vector<FileSeg> > buf;
	int curFileId,curSegId,curClient;
	for(unsigned int i=0;i<sequenceList.size();i++){
		curFileId = sequenceList[i].mFileId;
		curSegId = sequenceList[i].mSegId;
		curClient = sequenceList[i].mClientId;
		totalRequest++;
		//in the buf
		bool isHit = false;
		for(size_t j=0;j<buf[curClient].size();j++){
			if(buf[curClient][j].mClientId == curClient && buf[curClient][j].mSegId == curSegId && buf[curClient][j].mFileId == curFileId){
				isHit = true;
				break;
			}
		}
		if(isHit == true){
			hitTimes ++;
			continue;
		}
		//not in the buf
		if(buf[curClient].size()< blockNum){//just push in
			buf[curClient].push_back(sequenceList[i]);
		}else{//eliminate 
			vector<int> dis(blockNum);
			for(int j=0;j<dis.size();j++){
				dis[j]=sequenceList.size();
			}
			int max= 0;
			int targetIndex;
			for(size_t j=0;j<buf[curClient].size();j++){
				for(size_t k = i+1;k<sequenceList.size();k++){
					if(sequenceList[k].mClientId == buf[curClient][j].mClientId && sequenceList[k].mFileId == buf[curClient][j].mFileId && sequenceList[k].mSegId == buf[curClient][j].mSegId){
						dis[j]=k;
						break;
					}
				}

			}
			//chose the max k
			for(size_t j=0;j<dis.size();j++){
				if(dis[j] > max){
					max = dis[j];
					targetIndex = j;
				}
			}
			buf[curClient].erase(buf[curClient].begin()+targetIndex);
			buf[curClient].push_back(sequenceList[i]);
		}
	}
	cout<<hitTimes<<" "<<totalRequest<<" "<<hitTimes*1.0/totalRequest<<endl;
}

void
serverOptStrategy(int blockNum){
	int curFileId,curSegId,hitTimes,totalRequest;
	vector<FileSeg> buf;
	hitTimes=0;
	totalRequest = 0;
	for(unsigned int i=0;i<sequenceList.size();i++){
		curFileId = sequenceList[i].mFileId;
		curSegId = sequenceList[i].mSegId;
		totalRequest++;
		//in the buf
		bool isHit = false;
		for(size_t j=0;j<buf.size();j++){
			if(buf[j].mSegId == curSegId && buf[j].mFileId == curFileId){
				isHit = true;
				break;
			}
		}
		if(isHit == true){
			hitTimes ++;
			continue;
		}
		//not in the buf
		if(buf.size()< blockNum){//just push in
			buf.push_back(sequenceList[i]);
		}else{//eliminate 
			vector<int> dis(blockNum);
			for(int j=0;j<dis.size();j++){
				dis[j]=sequenceList.size();
			}
			int max= 0;
			int targetIndex;
			for(size_t j=0;j<buf.size();j++){
				for(size_t k = i+1;k<sequenceList.size();k++){
					if( sequenceList[k].mFileId == buf[j].mFileId && sequenceList[k].mSegId == buf[j].mSegId){
						dis[j]=k;
						break;
					}
				}

			}
			//chose the max k
			for(size_t j=0;j<dis.size();j++){
				if(dis[j] > max){
					max = dis[j];
					targetIndex = j;
				}
			}
			buf.erase(buf.begin()+targetIndex);
			buf.push_back(sequenceList[i]);
		}
	}
	cout<<hitTimes<<" "<<totalRequest<<" "<<hitTimes*1.0/totalRequest<<endl;
}

int main(int argc,char *argv[]){
	if(argc !=2){
		cout<<"input error"<<endl;
		exit(1);
	}
	std::map<std::string,std::string> keyMap;

	char *configFileName = "./config/simulator.cfg";
	ParseConfigFile(configFileName,keyMap);
	int blockNums,serverBlockNums;
	int clientNums;
	bool isP2POpen;
	blockNums = atoi(keyMap["BlockNums"].c_str());
	cout<<"blockNum = "<<blockNums<<endl;
	isP2POpen = !strcmp(keyMap["isP2POpen"].c_str(),"true") ? true : false;
	serverBlockNums = atoi(keyMap["ServerBlockNums"].c_str());
	clientNums = atoi(keyMap["ClientNums"].c_str());
	cout<<"clientNum = "<<clientNums<<endl;
	//init the sequenceList
	sequenceList.clear();
	ifstream ifs(argv[1]);

	int a,b,c,d,e,f,g;
	while(!ifs.eof()){
		ifs >>a>>b>>c;
		if(ifs.fail())
			break;
		FileSeg newSeg(a,b,c);
		sequenceList.push_back(newSeg);
	}
	ifs.close();
	cout<<"p2p is ";
	if(isP2POpen == true){
		cout<<"open"<<endl;
		clientOptStrategy(clientNums,blockNums);
	}else{
		cout<<"not open"<<endl;
		clientOptStrategy1(clientNums,blockNums);
		//serverOptStrategy(serverBlockNums);
	}
	return 0;
}
