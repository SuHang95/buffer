#ifndef __BUFFER_H__
#define __BUFFER_H__
#include<deque>
#include<atomic>
#include<mutex>
#include<iostream>
#define defaultsize 0x1000

class mem_container{
public:

	//this mem_container can manage at most 1GB,
	//at least 1MB by default,you can use set_max to resize
	mem_container(int size=0x10000000){
		max_size=size>0x100?size:0x100;
	}

	mem_container(const mem_container&)=delete;
	mem_container(mem_container&&)=delete;
	mem_container& operator=(const mem_container&)=delete;
	mem_container& operator=(mem_container&&)=delete;

	void inline set_max(int size){
		max_size=size>0x100?size:0x100;
	}

	inline char *get(){
		data.lock();
		if(container.size()!=0){
			char *ret=container.front();
			container.pop_front();
			data.unlock();
			return ret;
		}
		else{
			data.unlock();
			return new char[defaultsize];
		}
	};
	inline void push(char *ptr){
		data.lock();
		if(container.size()<=max_size){
			container.push_back(ptr);
		}else{
			delete []ptr;
		}
		data.unlock();
	}
	~mem_container(){
		std::unique_lock<std::mutex> lock(data);
		while(!container.empty()){
			delete container[0];
			container.pop_front();
		}
	}
private:
	std::deque<char*> container;
	std::mutex data;
	size_t max_size;
};




class buffer {
public:
	buffer();
	buffer(const buffer& other);
	buffer& operator=(const buffer& other);
	virtual ~buffer();

	void push_back_n(const char *src, size_t size);
	void pop_back_n(size_t size);
	void pop_front_n(size_t size);
	void push_front_n(const char *src, size_t size);
	char get(size_t n);
	//not checked
	int get_n(size_t n,char *dest,size_t size);
	void set(size_t n,char a);
	size_t size() const;
	void shrink_size();	
	inline std::string testinfo() const;

protected:
	//the function below is the nonlock inline func,is to used in other member function or the friend class,function
	inline void __shrink_size();
	inline size_t __size() const;
	inline void check() const;
	
	std::deque<char *>* _buffer;
	std::atomic<int>* counter;
	std::mutex* mutex_for_data;
	static mem_container mem;
	size_t* begin;
	size_t* end;
};


//the function below is the nonlock inline func,is to used in other member function or the friend class,function
inline void buffer::__shrink_size(){
	check();

	if ((*begin) == (size_t)-1) {
		while (!_buffer->empty()) {
			mem.push((*_buffer)[0]);
			_buffer->pop_front();
		}
	}
	else {
		while (((*begin) / defaultsize) > 0) {
			mem.push((*_buffer)[0]);
			(*begin) -= defaultsize;
			(*end) -= defaultsize;
			_buffer->pop_front();
		}

		while( (_buffer->size()) >
			((*end)/defaultsize)+1 ){
			mem.push(_buffer->back());
			_buffer->pop_back();
		}
	}
}


inline size_t buffer::__size() const{
	if ((*begin) == (size_t)-1 || (*end)==0) {
		return 0;
	}
	return ((*end) - (*begin));
}

inline void buffer::check() const{
	if((*begin)==size_t(-1) || (*end)==0 || (*begin)>=(*end)){
		(*begin)=size_t(-1);
		(*end)=0;
		return;
	}
	if( ((*end)-(*begin))>(_buffer->size())*defaultsize ){
		#ifdef _DEBUG
			std::cout<<"The begin="<<*begin<<",the end="<<*end<<",buffer size="<<_buffer->size()<<std::endl;
		#endif
		throw std::logic_error("buffer overflow!");
	}
}

inline std::string buffer::testinfo() const{
	char result[60];
	sprintf(result,"Begin=%lu,End=%lu,size of buffer=%lu",*begin,*end,_buffer->size());
	return std::string(result);
}

#endif
