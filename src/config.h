#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

using namespace std;

typedef struct configType {
    unsigned int resourceNumber;
    unsigned int subServerNum;
    unsigned long minPlayLen;
    unsigned long maxPlayLen;
    unsigned int maxCapacity;
    unsigned int minCapacity;
    unsigned int maxLoad;
    unsigned int minLoad;
    unsigned int fileLength;
    unsigned int maxDiskBand;
	unsigned int reservedTime;
    string serverIpAddress;
    string serverPort;
    string spreadAlgorithm;
    unsigned int period;
    double DRBeta;
    double lambda;	
    double loadThresh;
    double loadThreshHigh;
    string logFile;
    unsigned int poissonLambda;
    unsigned int clientNumber;
    unsigned int zipfParameter;
    string requestListFile;
    string resultFile;
    bool haveCost ;
    int maxCopyFlow;
    int maxInFlow;
    int runTime;
} ConfigType;

int parse_config_line(const string &line, string &key, string &value);
int read_vodspread_config(const string filename, ConfigType &configInfo);
int read_client_config(const string filename, ConfigType &);
int read_clientcircle_config(const string filename, double &zeta, double &sigma);
int read_config_file(const string fileName, map<string, string>& conf_map);

#endif
