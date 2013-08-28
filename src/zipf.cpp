#include "zipf.h"

#include <cmath>
#include <iostream>

#include "utils.h"

bool Zipf::CreateZipfDistribution(vector<double> &output)
{
	if (output.size() != 0) return false;
	double denominator = 0.0;
	double u;
	for (int i = 0; i < m_resourcenum; ++i) {	// get the sum of pow(i+1,-skew);
		denominator += pow(i + 1, 0 - m_sita);
	}
	for (int i = 0; i < m_resourcenum; ++i) {	// get the u1,u2,u3,...
		u = pow(i + 1, 0 - m_sita);
		// cout << "u = " << u <<endl;
		u = u / denominator;
		output.push_back(u);
		cout<<"probability of file "<< i << " is " << u << endl;
	}
	return true;
}

void Zipf::CreateZipfList(vector<int> &filelist)
{
	vector<double> probability;
	CreateZipfDistribution(probability);
	filelist.clear();
	for(int i = 0; i < m_clientnum; ++i) {
		double temp = Randomf(0, 1);
		double sum = 0.0;
		for(int j = m_resourcenum - 1; j >= 0 ; --j) {
			sum += probability.at(j);
			if (sum > temp) {
				filelist.push_back(i);
				break;
			}
		}
	}
}