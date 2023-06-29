#pragma once

#include<iostream>
#include<set>
#include<list>
#include<functional>
#include<cassert>
using namespace std;

#define start_run \
m_status = RUNNING; \
switch(mi_phase) { \
case 0:

#define sleep \
m_status = SLEEPY; \
mi_phase = __LINE__; \
return; \
case __LINE__:

#define wait_until(a_lambda) \
m_status = WAITING; \
mb_wait_condition = a_lambda; \
m_knl.m_wait_until(this); \
mi_phase = __LINE__; \
return; \
case __LINE__:

#define sensor_wait(a_lambda, a_sensor) \
m_status = WAITING; \
mb_wait_condition = a_lambda; \
a_sensor.m_watch(this); \
mi_phase = __LINE__; \
return; \
case __LINE__:

#define delay(ad_restart) \
m_status = SCHEDULED; \
md_restart_time = m_knl.md_time + ad_restart; \
m_knl.m_schedule(this); \
mi_phase = __LINE__; \
return; \
case __LINE__:

#define end_run \
mi_phase = 0; \
m_status = DORMANT; \
}

typedef enum { DORMANT, RUNNING, SLEEPY, WAITING, SCHEDULED } SimState;

class SimKernel;
class SimObj;

bool gb_sched_compare(const SimObj* ap_lhs, const SimObj* ap_rhs);

class SimKernel {
public:
	double md_time;
	double md_tmax;
	list<SimObj*> ms_wait_q;
	set<SimObj*, bool(*)(const SimObj* ap_lhs, const SimObj* ap_rhs)> ms_triggered{ &gb_sched_compare };

	set<SimObj*, bool(*)(const SimObj* ap_lhs, const SimObj* ap_rhs)> ms_sched_q{ &gb_sched_compare };

	SimKernel();

	virtual void m_simulate(double ad_tmax);

	virtual void m_wait_until(SimObj* ap_obj);
	virtual void m_schedule(SimObj* ap_obj);
	virtual void m_yield_to(SimObj* ap_obj);	//once current process stops running, ap_obj will run next (only if it's sleepy)
private:
	static int mi_sched_ctr;	//counter of scheduled objects 

	list<SimObj*> ms_yield_to;		//sleepy objects to have highest priority to run next

	void m_run_yield_to_list();
};//SimKernel

class SimObj {
public:
	SimKernel& m_knl;

	SimObj(SimKernel& a_knl);

	virtual void m_run();			//called to start the process
	virtual void m_wakeup();		//called to wakeup after being but into SLEEPY status

	int mi_get_id();
	SimState mi_get_status();

	//these are public but only to be used by simulation kernel
	//
	function<bool()> mb_wait_condition;		//condition to satisfy for current wait-until
	double md_restart_time;					//time at which object to restart its run
	int mi_sched_idx;						//used as a tie-breaker for objects restarting at the same time
	void m_clear_watches();					//clears this object from all lists in SensorVar objects
	list<list<SimObj*>*> ms_watcher_lists;	//lists within SensorVar objects this is watching
protected:
	int mi_phase;	//tracks where in m_run the object is waiting
	int mi_id;
	SimState m_status;
private:
	static int mi_obj_ctr;
};//SimObj