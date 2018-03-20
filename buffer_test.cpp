#include"buffer.h"
#include<thread>
#include <cstdlib>  
#include <cstdio>  
#include <time.h>
#include<iostream>
std::atomic<int> threadnum;
std::mutex mutex_for_queue;
#define testbuffsize 69128


#ifdef __gnu_linux__
#include<unistd.h>
#endif


void test(buffer _test);


int main() {
	buffer testbuff;
	threadnum.store(50); 
	char *_buff = new char[(0x7521)*sizeof(size_t)];
	for (size_t i=0,j= 5 * 0x10000; i< 0x7521;i++,j++){
		((size_t *)_buff)[i] = j;
	}
	testbuff.push_back_n(_buff, (0x7521) * sizeof(size_t));
	delete _buff;

	std::thread newthread(test, testbuff);
	newthread.detach();
	while (threadnum.load() != 0) {
#ifdef _WIN32
		_sleep(100);
#else
		usleep(100);
#endif
	}
	mutex_for_queue.lock();
	if (testbuff.size() != 0) {
		for (size_t j = 0; j < testbuff.size() / sizeof(size_t); j++) {
			size_t i = 0;
			for (size_t m = 0; m < sizeof(size_t); m++) {
				((char*)&i)[m] = testbuff.get(j*sizeof(size_t) +m);
			}
			std::cout << i<<" ";
		}
	}
	mutex_for_queue.unlock();
#ifdef _WIN32
	system("pause");
#endif
	return 0;

}


inline void datatest(buffer testbuff){
	srand(time(0));
	mutex_for_queue.lock();
	if (rand() % 4 == 0) {
		if (testbuff.size() != 0)
			testbuff.pop_front_n((rand() * sizeof(size_t)) % testbuffsize);
	}
	else if (rand() % 4 == 1) {
		if (testbuff.size() != 0) {
			size_t i = 0;
			for (size_t j = 0; j < sizeof(size_t); j++) {
				((char*)&i)[j] = testbuff.get(j);
			}
			size_t push_front_size = ((size_t)rand()) % testbuffsize;
			i -= push_front_size;
			char *_buff = new char[push_front_size * sizeof(size_t)];
			for (size_t j = 0; j< push_front_size; j++) {
				((size_t *)_buff)[j] = i;
				i++;
			}
			testbuff.push_front_n(_buff, push_front_size * sizeof(size_t));
			delete _buff;
		}
		else {
			size_t push_front_size = ((size_t)rand()) % testbuffsize;
			char *_buff = new char[push_front_size * sizeof(size_t)];

			for (size_t j = 0; j< push_front_size; j++) {
				((size_t *)_buff)[j] = j;
			}
			testbuff.push_front_n(_buff, push_front_size * sizeof(size_t));
			delete _buff;
		}
	}
	else if (rand() % 4 == 2) {
		size_t size = testbuff.size();
		if (size != 0)
			testbuff.pop_back_n(rand() % testbuffsize);
	}
	else {
		if (testbuff.size() != 0) {
			size_t i = 0;
			for (size_t j = 0; j < sizeof(size_t); j++) {
				((char*)&i)[j] = testbuff.get(testbuff.size() - sizeof(size_t) + j);
			}
			size_t push_back_size = ((size_t)rand()) % testbuffsize;
			char *_buff = new char[push_back_size * sizeof(size_t)];
			for (size_t j = 0; j< push_back_size; j++) {
				((size_t *)_buff)[j] = ++i;
			}
			testbuff.push_back_n(_buff, push_back_size * sizeof(size_t));
			delete _buff;
		}
		else {
			size_t push_back_size = ((size_t)rand()) % testbuffsize;
			char *_buff = new char[push_back_size * sizeof(size_t)];

			for (size_t j = 0; j< push_back_size; j++) {
				((size_t *)_buff)[j] = j;
			}
			testbuff.push_back_n(_buff, push_back_size * sizeof(size_t));
			delete _buff;
		}
	}
	mutex_for_queue.unlock();
}


void test1(buffer testbuff) {
	if (threadnum.load() == 0) {
		return;
	}
	try{
		std::thread newthread(test, testbuff);
		newthread.detach();
	}
	catch(std::exception& e){
		std::cout<<e.what();
		return;
	}
	datatest(testbuff);
	threadnum.fetch_add(-1);
}


void test(buffer testbuff) {

	if (threadnum.load() == 0) {
		return;
	}
	try{
		std::thread newthread(test, testbuff);
		newthread.detach();
	}
	catch(std::exception& e){
		std::cout<<e.what();
		return;
	}
	datatest(testbuff);
	threadnum.fetch_add(-1);
}

