#include<iostream>
#include<cstring>
#include "spinlock.h"
#include<mutex>

using namespace std;

class CycleBuff {
public:
	CycleBuff();
	~CycleBuff();
	bool writeMsg(const char *msg, size_t num);
	bool readMsg(char **result, size_t num, size_t &fence);

private:
	void spin();
	void release();
	void cas(volatile int &target, int old, int val);
	int read_fence;				//读偏移，下一个读操作开始的偏移
	int write_fence;			//写偏移，下一个写操作开始的偏移
	volatile int overlap;
	volatile int read_barrier;			//读屏障，当前在读的内存偏移
	volatile int write_barrier;			//写屏障，当前在写的内存偏移
	spinlock *lock;
	char *buff;
	const size_t MAX_SIZE;
	mutex mt;
};

CycleBuff::CycleBuff() :
	MAX_SIZE(5 * 1024 * 1024),
	read_fence(0),
	write_fence(0),
	overlap(0),
	read_barrier(0),
	write_barrier(0)
{
	buff = new char[MAX_SIZE];
	lock = new spinlock();
	spinlock_init(lock);
}

CycleBuff::~CycleBuff() {
	delete[] buff;
	delete lock;	
}

void CycleBuff::spin() {
//	while (this->lock) {
//		continue;
//	}
//	this->lock = 1;
	mt.lock();
//	spinlock_lock(lock);
}

void CycleBuff::release() {
	mt.unlock();
//	this->lock = 0;
//	spinlock_unlock(lock);
}

void CycleBuff::cas(volatile int &target, int old, int val) {
	while (true) {
		if (target == old) {
			target = val;
			break;
		}
	}
}


bool CycleBuff::readMsg(char **result, size_t num, size_t &fence) {
	if (num > MAX_SIZE) {
		cout << "read too many" << endl;
		return false;
	}
	int cur_fence;
	int cur_barrier;
	spin();
	if (read_fence > write_barrier) {
		if (num > MAX_SIZE - read_fence + write_barrier) {
			release();
			return false;
		}
	}
	else {
		if (num > write_barrier - read_fence) {
			release();
			return false;
		}
	}
	fence = cur_fence = read_fence;
	read_fence += num;
	if (read_fence > MAX_SIZE) {
		read_fence -= MAX_SIZE;
	}
	cur_barrier = read_fence;
	release();
	int left_size = MAX_SIZE - cur_fence;
	if (num > left_size) {
		memcpy(*result, buff + cur_fence, left_size);
		memcpy((*result) + left_size, buff, num - left_size);
	}
	else {
		memcpy(*result, buff + cur_fence, num);
	}
	cas(this->read_barrier, cur_fence, cur_barrier);
}

bool CycleBuff::writeMsg(const char *msg, size_t num) {
	int cur_fence;
	int cur_barrier;
	spin();
	if (write_fence >= read_barrier) {
		if (num > MAX_SIZE - write_fence + read_barrier) {
			release();
			return false;
		}
	}
	else {
		if (num > read_barrier - write_fence) {
			release();
			return false;
		}
	}
	cur_fence = write_fence;
	write_fence += num;
	if (write_fence >= MAX_SIZE) {
		write_fence -= MAX_SIZE;
	}
	cur_barrier = write_fence;
	release();
	int left_size = MAX_SIZE - cur_fence;
	if (num > left_size) {
		memcpy(buff + cur_fence, msg, left_size);
		memcpy(buff, msg + left_size, num - left_size);
	}
	else {
		memcpy(buff + cur_fence, msg, num);
	}
	cas(this->write_barrier, cur_fence, cur_barrier);
}
