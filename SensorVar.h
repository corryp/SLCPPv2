#pragma once

#include "SLCPPlib.h"

template <typename T>
class SensorVar {
public:
	SensorVar(SimKernel& a_knl);
	SensorVar(SimKernel& a_knl, const T a_value);

	const T& get();
	void m_watch(SimObj* ap_obj);

	SensorVar<T>& operator= (const T& a_rhs);

	list<SimObj*> ms_watchers;
private:
	T value;
	SimKernel& m_knl;
};

//SensorVar

template<typename T> SensorVar<T>::SensorVar(SimKernel& a_knl) : m_knl(a_knl) {}
template<typename T> SensorVar<T>::SensorVar(SimKernel& a_knl, const T a_value) : value(a_value) {}

template<typename T>
SensorVar<T>& SensorVar<T>::operator=(const T & a_rhs)
{
	if (value != a_rhs) {
		value = a_rhs;
		list<SimObj*>::iterator i;
		for (i = ms_watchers.begin(); i != ms_watchers.end(); ++i)
			m_knl.ms_triggered.insert(*i);
	}//if
	return *this;
}//operator=


template<typename T> const T& SensorVar<T>::get() { return value; }

template<typename T>
void SensorVar<T>::m_watch(SimObj *ap_obj)
{
	ms_watchers.push_back(ap_obj);
	ap_obj->ms_watcher_lists.push_back(&ms_watchers);
}
