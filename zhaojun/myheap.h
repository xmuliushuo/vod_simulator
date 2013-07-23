/*
 * myheap.h
 *
 *  Created on: 2013-2-19
 *      Author: zhaojun
 */

#ifndef MYHEAP_H_
#define MYHEAP_H_

#include <vector>
#include <iostream>

using namespace std;

template <class T,class BinaryOperation>
class Myheap{
public:
	Myheap();
//	void MakeHeap(vector<T>::iterator firstIter,vector<T>::iterator secondIter){}
	void PushHeap(T t);
	T &PopHeap(int tag);
	void SortHeap();
	void Erase(T t);
	T &Front(){return m_vect[0];}
	T &GetElement(int tag);
	bool IsEmpty(){return m_vect.empty();}
	int Size(){return m_vect.size();}
	void AdjustHeap();
	void PrintHeap(){
		for(int i = 0;i < m_vect.size();i++)
			cout << m_vect[i].leftTime << " ";
		cout << endl;
	}
private:
	void _AdjustHeap(BinaryOperation binary_op);
private:
	vector<T> m_vect;
};


#endif /* MYHEAP_H_ */
