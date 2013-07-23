/*
 * myheap.cpp
 *
 *  Created on: 2013-2-19
 *      Author: zhaojun
 */

#include "myheap.h"

template <class T,class BinaryOperation>
Myheap<T,BinaryOperation>::Myheap(){
	m_vect.clear();
}

template <class T,class BinaryOperation>
void Myheap<T,BinaryOperation>::PushHeap(T t){
	m_vect.push_back(t);
//	cout << "~~~~~~~~~~~" << endl;
	_AdjustHeap(BinaryOperation());
}

template <class T,class BinaryOperation>
T &Myheap<T,BinaryOperation>::PopHeap(int tag){
	T t = m_vect[tag];
	m_vect[tag] = m_vect[m_vect.size() - 1];
	m_vect[m_vect.size() - 1] = t;
	m_vect.pop_back();
	//_AdjustHeap(BinaryOperation<T>());
	return t;
}

template <class T,class BinaryOperation>
void Myheap<T,BinaryOperation>::AdjustHeap(){
	_AdjustHeap(BinaryOperation());
}

template <class T,class BinaryOperation>
void Myheap<T,BinaryOperation>::_AdjustHeap(BinaryOperation binary_op){
	for(int i = (m_vect.size() - 1);i > 0;i--){
		int fatherTag = (i - 1) / 2;
//		cout << "........." << i << " " << m_vect[i].leftTime << " " << fatherTag
//				<< " " << m_vect[fatherTag].leftTime << endl;
		if(binary_op(m_vect[i],m_vect[fatherTag])){
			T t = m_vect[i];
			m_vect[i] = m_vect[fatherTag];
			m_vect[fatherTag] = t;
//			cout << i << " " << m_vect[i].leftTime << " " << fatherTag
//					<< " " << m_vect[fatherTag].leftTime << endl;
		}
	}
}

template <class T,class BinaryOperation>
T &Myheap<T,BinaryOperation>::GetElement(int tag){
	return m_vect[tag];
}
