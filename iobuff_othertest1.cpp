#include"iobuffer.h"
#include<atomic>
#include<thread>
#include"sulog/logger.h"
#include<exception>
#include<time.h>
#include<iostream>
#include<unistd.h>
#define testbuffsize 69128
#define arraysize 50
std::atomic<int> threadnum(5000);
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
	size_t j=0x7521;
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


void datatest(iobuffer (&buffarray)[arraysize]){
	srand(time(0));
	mutex_for_queue.lock();
	size_t popsize[arraysize];	

	for(int i=arraysize;i!=0;i--){
		log.print("Before test:buffarray[%d] begin=%zd,last=%zd,size=%zd",
			i,__test_getbegin(buffarray[i%arraysize]),
			__test_getlast(buffarray[i%arraysize]),buffarray[i%arraysize].size());
		log.print("Before test:buffarray[%d] begin=%zd,last=%zd,size=%zd",
			(i-1)%arraysize,__test_getbegin(buffarray[(i-1)%arraysize]),
			__test_getlast(buffarray[(i-1)%arraysize]),buffarray[(i-1)%arraysize].size());
		
		popsize[i%arraysize] = ((size_t)rand()) % testbuffsize;
		buffarray[i%arraysize].pop_front_to_other_back_n(buffarray[(i-1)%arraysize],popsize[i%arraysize]*sizeof(size_t));
	
		log.print("pop_front_to_other_back_n %zd bytes!,%zd num",popsize[i]*sizeof(size_t),popsize[i]);
		log.print("After test:buffarray[%d] begin=%zd,last=%zd,size=%zd",
			i%arraysize,__test_getbegin(buffarray[i%arraysize]),
			__test_getlast(buffarray[i%arraysize]),buffarray[i%arraysize].size());
		log.print("After test:buffarray[%d] begin=%zd,last=%zd,size=%zd\n",
			(i-1)%arraysize,__test_getbegin(buffarray[(i-1)%arraysize]),
			__test_getlast(buffarray[(i-1)%arraysize]),buffarray[(i-1)%arraysize].size());

	}

	mutex_for_queue.unlock();

}
/*void test1(iobuffer buffarray[arraysize]);
void test(iobuffer buffarray[arraysize]) {
	if (threadnum.load() == 0) {
		return;
	}
	try{
		std::thread newthread(test1, buffarray);
		newthread.detach();
	}
	catch(std::exception& ex){
		std::cout<<ex.what()<<std::endl;
		threadnum.store(0);
	}
	datatest(buffarray);
	if(threadnum.fetch_add(-1)==0)
		threadnum.store(0);
}

void test1(iobuffer buffarray[arraysize]) {
	if (threadnum.load() == 0) {
		return;
	}
	try{
		std::thread newthread(test, buffarray);
		newthread.detach();
	}
	catch(std::exception& ex){
		std::cout<<ex.what()<<std::endl;
		threadnum.store(0);
	}
	datatest(buffarray);
	if(threadnum.fetch_add(-1)==0)
		threadnum.store(0);
}*/




int main(){
	iobuffer buffarray[arraysize];

	log.print("Start fill data!");
	char* temp0=new char[0x23*sizeof(size_t)];
	buffarray[0].push_front_n(temp0,0x23*sizeof(size_t));

	char* temp=__test_getnumptr(0x10000000);	
	buffarray[0].push_back_n(temp,0x10000000*sizeof(size_t));

	char *temp1=new char[0x2375*sizeof(size_t)];
	buffarray[0].push_front_n(temp1,0x2375*sizeof(size_t));

	buffarray[0].pop_front_n((0x2375+0x23)*sizeof(size_t));
	
	delete temp0;
	delete temp1;
	delete temp;
	log.print("Start test!");
	for(size_t i=0;i<10000;i++){
		datatest(buffarray);

	}
	log.print("Test End!");
	mutex_for_queue.lock();
	for(int i=0;i<arraysize;i++)
		__test_print(buffarray[i]);
	mutex_for_queue.unlock();

	return 0;
}
