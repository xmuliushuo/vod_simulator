#ifndef FAKETRAN_H_
#define FAKETRAN_H_

class FakeTran{
public:
	FakeTran();
	~FakeTran();
	int GetWriteFd()
	{
		return m_faketranfd[0];
	}
	int GetReadFd()
	{
		return m_faketranfd[1];
	}
private:
	int m_faketranfd[2];
};


#endif /* FAKETRAN_H_ */