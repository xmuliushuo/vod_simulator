#include "utils.h"

#include <fstream>

using namespace std;

static void Trim(string &str){
	str.erase(0, s.find_first_not_of(" "));
	str.erase(s.find_last_not_of(" ") + 1);
}

void ParseConfigFile(string config_filename, map<string, string> &keymap) {
	ifstream infile;
	infile.open(config_filename);

	string line;
	while (getline(infile, line)) {
		if(line.empty())
			continue;
		Trim(line);
		if(line.at(0) == '#')
			continue;
		int equalPos = line.find_first_of('=');
		string key = line.substr(0, equalPos);
		string value = line.substr(equalPos + 1);
		Trim(key);
		Trim(value);
		keymap.insert(make_pair(key, value));
	}
	infile.close();
}