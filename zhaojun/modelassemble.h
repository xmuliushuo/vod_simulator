/*
 * behaviormodel.h
 *
 *  Created on: 2013-2-21
 *      Author: zhaojun
 */

#ifndef BEHAVIORMODEL_H_
#define BEHAVIORMODEL_H_

#include "mymessage.h"
#include "globalfunction.h"

#include <vector>

using namespace std;

struct MMBlock{
	int status;
	int prob;
};

class ModelAssemble{
public:
	ModelAssemble(int fileNum,double thelta,double lambda,
			double zeta,double sigma,int maxClientNum,
			int playToPlay,int playToPause,int playToForward,int playToBackward,
			int playToStop,bool isStartTogether);
	~ModelAssemble();
	int GetNextStatus(int curStatus);
	int GetStartFileId();
	int GetStartSegId(){return 1;}
	void GslInit();

	void CreateZipfDistribution();
	void CreateStartTime();

	double GetStartTime(int clientNum){
		if(mIsStartTogether)
			return 0;
		return mStartTime[clientNum - 1];
	}
	double GslRandLogNormal();
private:
	int mFileNum;
	vector<double> mProbability;
	vector<double> mStartTime;

	double mThelta;
	double mLambda;
	double mZeta;
	double mSigma;

	int mMaxClientNum;

	vector<MMBlock> mMMVect;

	bool mIsStartTogether;
};


#endif /* BEHAVIORMODEL_H_ */
