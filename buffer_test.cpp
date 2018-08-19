#include"buffer.h"
#include<thread>
#include<cstdlib>  
#include<cstdio>  
#include<time.h>
#include<string>
#include<iostream>
#include"sulog/logger.h"
#include<exception>


logger log("buffertest");

std::atomic<int> threadnum;
std::mutex mutex_for_queue;
#define testbuffsize 9128


#ifdef __gnu_linux__
#include<unistd.h>
#endif


void test(buffer _test);
inline size_t __test_getbegin(buffer& testbuff){
	if(testbuff.size()==0)
		return 0;
	size_t i = 0;
	for (size_t j = 0; j < sizeof(size_t); j++) {
		((char*)&i)[j] = testbuff.get(j);
	}
	return i;
}


inline size_t __test_getlast(buffer& testbuff){
	if(testbuff.size()==0)
		return 0;
	size_t i = 0;
	for (size_t j = 0; j < sizeof(size_t); j++) {
		((char*)&i)[j] = testbuff.get(testbuff.size() - sizeof(size_t) + j);
	}
	return i;
}

int main() {
	buffer testbuff;
	threadnum.store(50); 
	char *_buff = new char[(0x7521)*sizeof(size_t)];
	for (size_t i=0,j= 5 * 0x10000; i< 0x7521;i++,j++){
		((size_t *)_buff)[i] = j;
	}
	testbuff.push_back_n(_buff, (0x7521) * sizeof(size_t));
	delete _buff;

	log.print("push_back_n %lu number,now begin=%lu,last=%lu\n,size=%lu,bufferinfo:%s",
		0x7521,__test_getbegin(testbuff),
		__test_getlast(testbuff),testbuff.size(),
		testbuff.testinfo().c_str());
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
		if (testbuff.size() != 0){
			log.print("before pop_front_n,now begin=%lu,last=%lu,size=%lu"
				"buffer_info:%s",__test_getbegin(testbuff),__test_getlast(testbuff),
				testbuff.size(),testbuff.testinfo().c_str());

			size_t pop_front_size=((size_t)rand()) % testbuffsize;
			testbuff.pop_front_n(pop_front_size * sizeof(size_t));

			log.print("pop_front_n %lu number,now begin=%lu,last=%lu,size=%lu"
				"buffer_info:%s",pop_front_size,__test_getbegin(testbuff),
				__test_getlast(testbuff),testbuff.size(),testbuff.testinfo().c_str());
		}
	}
	else if (rand() % 4 == 1) {
		if (testbuff.size() != 0) {
			log.print("before push_front_n,now begin=%lu,last=%lu,size=%lu,"
				"buffer_info:%s",__test_getbegin(testbuff),__test_getlast(testbuff),
				testbuff.size());

			size_t begin=__test_getbegin(testbuff);
			size_t push_front_size = ((size_t)rand()) % testbuffsize;
			begin -= push_front_size;
			size_t* _buff = new size_t[push_front_size];
			for (size_t j = 0; j< push_front_size; j++) {
				_buff[j] = begin;
				begin++;
			}
			testbuff.push_front_n((char *)_buff, push_front_size * sizeof(size_t));
			delete _buff;

			log.print("push_front_n %lu number,now begin=%lu,last=%lu,size=%lu,"
				"buffer_info:%s",push_front_size,__test_getbegin(testbuff),
				__test_getlast(testbuff),testbuff.size(),testbuff.testinfo().c_str());
		}
		else {
			log.print("Now the testbuff is null!");

			size_t push_front_size = ((size_t)rand()) % testbuffsize;
			size_t* _buff = new size_t[push_front_size];

			for (size_t j = 0; j< push_front_size; j++) {
				_buff[j] = j;
			}
			testbuff.push_front_n((char *)_buff, push_front_size * sizeof(size_t));
			delete _buff;

			log.print("push_front_n %lu number,now begin=%lu,last=%lu,size=%lu,"
				"buffer_info:%s",push_front_size,__test_getbegin(testbuff),
				__test_getlast(testbuff),testbuff.size(),testbuff.testinfo().c_str());
		}
	}
	else if (rand() % 4 == 2) {
		if (testbuff.size() != 0){
			log.print("before pop_back_n,now begin=%lu,last=%lu,size=%lu,"
				"buffer_info:%s",__test_getbegin(testbuff),__test_getlast(testbuff),
				testbuff.size(),testbuff.testinfo().c_str());

			size_t pop_back_size = ((size_t)rand()) % testbuffsize;
			testbuff.pop_back_n(pop_back_size*sizeof(size_t));

			log.print("pop_back_n %lu number,now begin=%lu,last=%lu\n,size=%lu,"
				"buffer_info:%s",pop_back_size,__test_getbegin(testbuff),
				__test_getlast(testbuff),testbuff.size(),testbuff.testinfo().c_str());
		}
	}
	else {
		if (testbuff.size() != 0) {
			log.print("before push_back_n,now begin=%lu,last=%lu,size=%lu,"
				"buffer_info:%s",__test_getbegin(testbuff),__test_getlast(testbuff),
				testbuff.size(),testbuff.testinfo().c_str());

			size_t last=__test_getlast(testbuff);
			size_t push_back_size = ((size_t)rand()) % testbuffsize;
			size_t* _buff = new size_t[push_back_size];
			for (size_t j = 0; j< push_back_size; j++) {
				_buff[j] = ++last;
			}
			testbuff.push_back_n((char *)_buff, push_back_size * sizeof(size_t));
			delete _buff;

			log.print("push_back_n %lu number,now begin=%lu,last=%lu\n,size=%lu,"	
				"buffer_info:%s",push_back_size,__test_getbegin(testbuff),
				__test_getlast(testbuff),testbuff.size(),testbuff.testinfo().c_str());
		}
		else {
			log.print("Now the testbuff is null!");

			size_t push_back_size = ((size_t)rand()) % testbuffsize;
			size_t *_buff = new size_t[push_back_size];

			for (size_t j = 0; j< push_back_size; j++) {
				_buff[j] = j;
			}
			testbuff.push_back_n((char *)_buff, push_back_size * sizeof(size_t));
			delete _buff;

			log.print("push_back_n %lu number,now begin=%lu,last=%lu,size=%lu,"
				"buffer_info:%s",push_back_size,__test_getbegin(testbuff),
				__test_getlast(testbuff),testbuff.size(),testbuff.testinfo().c_str());
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
	catch(std::exception& ex){
		std::cout<<ex.what()<<std::endl;
		threadnum.store(0);
	}
	datatest(testbuff);
	if(threadnum.fetch_add(-1)==0)
		threadnum.store(0);
}


void test(buffer testbuff) {
	if (threadnum.load() == 0) {
		return;
	}
	try{
		std::thread newthread(test, testbuff);
		newthread.detach();
	}
	catch(std::exception& ex){
		std::cout<<ex.what()<<std::endl;
		threadnum.store(0);
	}
	datatest(testbuff);
	if(threadnum.fetch_add(-1)==0)
		threadnum.store(0);
}

