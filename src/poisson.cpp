#include "poisson.h"

#include <cmath>

#include "utils.h"

bool Poisson::GetPoissonList(int clientnum, vector<int> &output)
{
	if (output.size() != 0) return false;
	if (clientnum <= 0) return false;
	double temp;
	output.push_back(0);
	for (int i = 0; i < clientnum; ++i) {
		temp = Randomf(0, 1);
		temp = 0 - log(temp) * m_lambda;
		output.push_back(static_cast<int>(temp * 1000000));
	}
	return true;
}