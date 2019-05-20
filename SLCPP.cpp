// SLCPP.cpp : Defines the entry point for the console application.
//

#include"stdafx.h"

class BlankObj : public SimObj {
public:
	BlankObj(SimKernel& a_knl) : SimObj(a_knl) {}

	void m_run() {
		start_run

			end_run
	}//m_run
};//BlankObj


class Server : public SimObj {
public:
	Server(SimKernel& a_knl) : SimObj(a_knl), m_busy(a_knl) {}

	list<SimObj*> q;
	bool mb_busy;
	SensorVar<bool> m_busy;

	SimObj* p_client;

	void m_run() {
		start_run;

		m_busy = false;
		while (true) {
			if (q.size() == 0) {
				wait_until([this]() {return this->q.size() > 0;});
			}//if

			p_client = q.front();
			q.pop_front();
			m_busy = true;
			p_client->m_wakeup();
			sensor_wait([this]() {return !this->m_busy.get();}, m_busy);
		}//forever

		end_run;
	}//m_run
};//Server


class Client : public SimObj {
public:
	Server& m_srv;
	double md_arv_t;
	double md_prc_t;

	int mi_q_len;
	double md_start_srv;
	double md_finish;

	Client(SimKernel& a_knl, Server& a_srv, double ad_arv_t, double ad_prc_t) :
		SimObj(a_knl), m_srv(a_srv), md_arv_t(ad_arv_t), md_prc_t(ad_prc_t) {

	}//Client

	void m_run() {
		start_run;

		delay(md_arv_t);
		m_srv.q.push_back(this);
		mi_q_len = m_srv.q.size();
		sleep;

		md_start_srv = m_knl.md_time;
		delay(md_prc_t);
		m_srv.m_busy = false;

		md_finish = m_knl.md_time;

		cout << mi_get_id() << ": " << md_arv_t << " (q=" << mi_q_len << ") -> " << md_start_srv << " (" << md_prc_t << ") -> " << md_finish << endl;

		end_run;
	}//m_run
};//Client

void g_test_mm1() {
	SimKernel knl;
	Server srv(knl);
	srv.m_run();

	vector<Client*> s_cl;
	Client *p_cl;
	for (int i = 0; i < 100; ++i) {
		p_cl = new Client(knl, srv, 100.0 * rand() / RAND_MAX, 1.5 * rand() / RAND_MAX);
		p_cl->m_run();
		s_cl.push_back(p_cl);
	}//i

	knl.m_simulate(1000);

	p_cl = p_cl;
}//g_test_mm1

int main()
{
	int i;
	g_test_mm1();
	cin >> i;

	return 0;
}


