#include "SLCPPlib.h"

bool gb_sched_compare(const SimObj * ap_lhs, const SimObj *ap_rhs)
{
	if (ap_lhs->md_restart_time == ap_rhs->md_restart_time)
		return ap_lhs->mi_sched_idx < ap_rhs->mi_sched_idx;
	return ap_lhs->md_restart_time < ap_rhs->md_restart_time;
}

//SimKernel

int SimKernel::mi_sched_ctr = 0;

SimKernel::SimKernel() : md_time(0) {}

void SimKernel::m_simulate(double ad_tmax)
{
	bool b_stop;
	double d_t;

	list<SimObj*>::iterator l_it;
	set<SimObj*>::iterator s_it;
	SimObj* p_obj;

	md_tmax = ad_tmax;
	md_time = ms_sched_q.size() > 0 ? (*ms_sched_q.begin())->md_restart_time : ad_tmax;
	while (md_time < ad_tmax) {
		//run objects that are scheduled at the current time
		//
		s_it = ms_sched_q.begin();
		while (s_it != ms_sched_q.end()) {
			p_obj = *s_it;
			if (p_obj->md_restart_time > md_time)
				break;
			s_it = ms_sched_q.erase(s_it);
			p_obj->m_run();
			m_run_yield_to_list();
		}//wend

		//evaluate all objects waiting on a condition
		//
		do {
			b_stop = true;

			s_it = ms_triggered.begin();
			while (s_it != ms_triggered.end()) {
				p_obj = *s_it;
				if (p_obj->mb_wait_condition()) {
					p_obj->m_clear_watches();
					p_obj->m_run();
					m_run_yield_to_list();
					b_stop = false;
				}//if
				s_it = ms_triggered.erase(s_it);
			}//s_it

			for (l_it = ms_wait_q.begin(); l_it != ms_wait_q.end(); ++l_it) {
				p_obj = *l_it;
				if (p_obj->mb_wait_condition()) {
					ms_wait_q.erase(l_it);
					p_obj->m_run();
					m_run_yield_to_list();
					b_stop = false;
					break;
				}//if
			}//l_it
		} while (!b_stop);

		d_t = ms_sched_q.size() > 0 ? (*ms_sched_q.begin())->md_restart_time : ad_tmax;
		assert(d_t >= md_time);
		md_time = d_t;
	}//wend
}//m_simulate

void SimKernel::m_run_yield_to_list()
{
	while (ms_yield_to.size() > 0) {
		SimObj* p_obj = ms_yield_to.front();
		ms_yield_to.pop_front();
		p_obj->m_run();
	}//if
}

void SimKernel::m_wait_until(SimObj* ap_obj)
{
	ms_wait_q.emplace_back(ap_obj);
}

void SimKernel::m_schedule(SimObj* ap_obj)
{
	assert(ap_obj->md_restart_time >= md_time);
	ap_obj->mi_sched_idx = mi_sched_ctr++;
	ms_sched_q.insert(ap_obj);
}

void SimKernel::m_yield_to(SimObj* ap_obj)
{
	if (ap_obj->mi_get_status() == SLEEPY)
		ms_yield_to.push_back(ap_obj);
}


//SimObj

int SimObj::mi_obj_ctr = 0;

SimObj::SimObj(SimKernel &a_knl) : mi_phase(0), m_knl(a_knl), mi_id(mi_obj_ctr++), m_status(DORMANT) {}
void SimObj::m_run() {}

void SimObj::m_wakeup()
{
	//assert(m_status == SLEEPY);
	if (m_status != SLEEPY)
		return;

	m_knl.m_yield_to(this);
}//m_wakeup

int SimObj::mi_get_id() { return mi_id; }
SimState SimObj::mi_get_status() { return m_status; }

void SimObj::m_clear_watches()
{
	list<list<SimObj*>*>::iterator i;
	list<SimObj*>::iterator j;

	for (i = ms_watcher_lists.begin(); i != ms_watcher_lists.end(); ++i) {
		for (j = (*i)->begin(); j != (*i)->end(); ++j) {
			if (*j == this) {
				(*i)->erase(j);
				break;
			}//if
		}//h
	}//i
	ms_watcher_lists.clear();
}//m_clear_watches

