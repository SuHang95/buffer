#include"../iobuffer.h"
#include<atomic>
#include<thread>
#include"../../../SuLog/logger.h"
#include<exception>
#include<time.h>
#include<iostream>
#include<unistd.h>
#define testbuffsize 69128
std::atomic<int> threadnum(500);
std::mutex mutex_for_queue;
logger log("IoBufferTestLog");

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



inline char* __test_getnumptr(size_t n){
	size_t* ptr=new size_t[n];
	size_t j=0x7521563421;
	for(size_t i=0;i<n;i++){
		ptr[i]=j++;
	}
	return (char*)ptr;
}

inline void __test_print(iobuffer& testbuff){
	if (testbuff.size() != 0) {
		for (size_t j = 0; j < testbuff.size() / sizeof(size_t); j++) {
			size_t i = 0;
			for (size_t m = 0; m < sizeof(size_t); m++) {
				((char*)&i)[m] = testbuff.get(j*sizeof(size_t) +m);
			}
			std::cout << i<<" ";
		}
	}
}
void datatest(iobuffer testbuff){
	srand(time(0));
	mutex_for_queue.lock();

	log.print("before pop_back_to_other_front_n,now begin=%zd,last=%zd,size=%zd",
		__test_getbegin(testbuff),__test_getlast(testbuff),testbuff.size());

	
	iobuffer other=testbuff;
	size_t pop_size = ((size_t)rand()) % testbuffsize;
	testbuff.pop_back_to_other_front_n(other,pop_size*sizeof(size_t));

	log.print("after pop_back_to_other_front_n %zd bytes,now begin=%zd,last=%zd,size=%zd",
		pop_size*sizeof(size_t),__test_getbegin(testbuff),
		__test_getlast(testbuff),testbuff.size());


	mutex_for_queue.unlock();

}
void test(iobuffer testbuff);
void test1(iobuffer testbuff) {
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


void test(iobuffer testbuff) {
	if (threadnum.load() == 0) {
		return;
	}
	try{
		std::thread newthread(test1, testbuff);
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



int main(){
	iobuffer testbuff;
	char* temp0=new char[0x23*sizeof(size_t)];
	testbuff.push_front_n(temp0,0x23*sizeof(size_t));

	char* temp=__test_getnumptr(0x10000000);	
	testbuff.push_back_n(temp,0x10000000*sizeof(size_t));

	char *temp1=new char[0x2375*sizeof(size_t)];
	testbuff.push_front_n(temp1,0x2375*sizeof(size_t));

	testbuff.pop_front_n((0x2375+0x23)*sizeof(size_t));


	std::thread newthread(test, testbuff);
	newthread.detach();
	while (threadnum.load() != 0) {
#ifdef _WIN32
		_sleep(100);
#else
		usleep(100);
#endif
	}
	for(size_t i=0;i<1000000;i++){
		datatest(testbuff);

	}



	mutex_for_queue.lock();
	__test_print(testbuff);
	mutex_for_queue.unlock();

	return 0;
}
