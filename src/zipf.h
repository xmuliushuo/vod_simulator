#ifndef ZIPF_H_
#define ZIPF_H_

#include <vector>

using namespace std;

class Zipf {
public:
	Zipf(int clientnum, int resourcenum, double sita) : 
		m_clientnum(clientnum), 
		m_resourcenum(resourcenum), 
		m_sita(sita) { }
	~Zipf() {}
	bool CreateZipfDistribution(vector<double> &output);
	void CreateZipfList(vector<int> &filelist);
private:
	int m_clientnum;
	int m_resourcenum;
	double m_sita;
};

#endif