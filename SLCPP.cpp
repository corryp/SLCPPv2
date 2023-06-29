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
	Server(SimKernel& a_knl, const int ai_capacity) : SimObj(a_knl), m_busy_count(a_knl), mi_capacity(ai_capacity) {}

	int mi_capacity;

	list<SimObj*> q;
	bool mb_busy;
	SensorVar<int> m_busy_count;

	SimObj* p_client;

	void m_run() {
		start_run;

		m_busy_count = 0;
		while (true) {
			while (q.size() == 0) {
				//wait_until([this]() {return this->q.size() > 0;});
				sleep;
			}//if

			p_client = q.front();
			q.pop_front();
			m_busy_count = m_busy_count.get() + 1;
			p_client->m_wakeup();
			if (this->m_busy_count.get() >= mi_capacity) {
				sensor_wait([this]() {return this->m_busy_count.get() < mi_capacity; }, m_busy_count);
			}//if
		}//forever

		end_run;
	}//m_run

	void m_request(SimObj* a_client) {
		q.push_back(a_client);
		m_wakeup();
	}//m_request

	void m_release() {
		m_busy_count = m_busy_count.get() - 1;
	}//m_release
};//Server


class Client : public SimObj {
public:
	Server& m_srv;
	double md_arv_t;
	double md_prc_t;

	int mi_q_len, mi_in_srv;
	double md_start_srv;
	double md_finish;

	Client(SimKernel& a_knl, Server& a_srv, double ad_arv_t, double ad_prc_t) :
		SimObj(a_knl), m_srv(a_srv), md_arv_t(ad_arv_t), md_prc_t(ad_prc_t) {

	}//Client

	void m_run() {
		start_run;

		delay(md_arv_t);
		m_srv.m_request(this);

		mi_q_len = m_srv.q.size();
		mi_in_srv = m_srv.m_busy_count.get();
		sleep;

		md_start_srv = m_knl.md_time;
		delay(md_prc_t);
		m_srv.m_release();

		md_finish = m_knl.md_time;

		//cout << mi_get_id() << ": " << md_arv_t << " (q=" << mi_q_len << ",s=" << mi_in_srv << ") -> " << md_start_srv << " (" << md_prc_t << ") -> " << md_finish << endl;

		end_run;
	}//m_run
};//Client

void g_test_mms() {
	SimKernel knl;

	int m = 10000;
	Server srv(knl, m);
	srv.m_run();

	vector<Client*> s_cl;
	Client *p_cl;
	
	int n = 1200000;
	double d_T = 40*3600;
	for (int i = 0; i < n; ++i) {
		p_cl = new Client(knl, srv, d_T * rand() / RAND_MAX, (2 * m * d_T/n) * rand() / RAND_MAX);
		p_cl->m_run();
		s_cl.push_back(p_cl);
	}//i

	knl.m_simulate(2*d_T);

	p_cl = p_cl;
}//g_test_mm1

int main()
{
	int i;

	ofstream f("out.txt");

	streambuf* oldCoutBuffer = std::cout.rdbuf();
	cout.rdbuf(f.rdbuf());

	g_test_mms();

	cout.rdbuf(oldCoutBuffer);
	cout << "done\n";
	cin >> i;

	return 0;
}


