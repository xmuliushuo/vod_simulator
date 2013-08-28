#ifndef CLIENT_H_
#define CLIENT_H_

#include <vector>

using namespace std;

class Poisson {
public:
	Poisson(double lambda) : m_lambda(lambda) { }
	~Poisson() {}
	bool GetPoissonList(int clientnum, vector<int> &output);
private:
	double m_lambda;
};

#endif