#include <iostream>
#include<map>
#include<string>
#include<math.h>
#include<set>
#include<vector>
#include<algorithm>
#include "CycleBuff.h"
#include<thread>
#include<random>
#include<sys/time.h>

using namespace std;
default_random_engine e;
uniform_int_distribution<unsigned> u(10, 50);
volatile bool push_end = false;

void pop_data(CycleBuff *cyc_buff) {
	char *data = new char[50];
	size_t fence;
	const char *numchar = "0123456789";
	int num;

	while (true) {
		num = u(e);
		if (!cyc_buff->readMsg(&data, num, fence)) {
			if (!push_end) {
				continue;
			}
			else {
				cout << "thread end" << endl;
				delete []data;
				break;
			}
		}
		for (int i = 0; i < num; i++) {
			if (data[i] != numchar[(fence + i) % 10]) {
				cout << "pop error" << fence << data[0] << endl;
			}
		}
	}

}



int main() {
	char *testchar = new char[500 * 1024 * 1024];
	const char *numchar = "0123456789";
	for (int i = 0; i < 500 * 1024 * 1024; i++) {
		testchar[i] = numchar[i % 10];
	}

	CycleBuff *cyc_buff = new CycleBuff();
	timeval t_start, t_end;
	gettimeofday( &t_start, NULL);
	thread pop1(pop_data, cyc_buff);
	thread pop2(pop_data, cyc_buff);
	thread pop3(pop_data, cyc_buff);
	thread pop4(pop_data, cyc_buff);
	for (int i = 0; i < 10240 * 1024; i++) {
		cyc_buff->writeMsg(testchar + i * 50, 50);
	}
	delete []testchar;
	push_end = true;
	pop1.join();
	pop2.join();
	pop3.join();
	pop4.join();
	gettimeofday( &t_end, NULL);
    double delta_t = (t_end.tv_sec-t_start.tv_sec) +
                    (t_end.tv_usec-t_start.tv_usec)/1000000.0;
    cout << "all time : " << delta_t  << "s" << endl;
}
