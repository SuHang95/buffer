#ifndef __BUFFER_H__
#define __BUFFER_H__
#include<deque>
#include<atomic>
#include<mutex>
#include<iostream>
#define defaultsize 0x1000

class buffer {
public:
	buffer(); 
	buffer(size_t size);
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
protected:
	//the function below is the nonlock inline func,is to used in other member function or the friend class,function
	inline void __shrink_size();
	inline size_t __size() const;


	std::deque<char *>* _buffer;
	std::atomic<int>* counter;
	std::mutex* mutex_for_data;
	size_t* begin;
	size_t* end;
};

//the function below is the nonlock inline func,is to used in other member function or the friend class,function
inline void buffer::__shrink_size(){
	if((*begin)== (size_t)-1 || (*begin)>=(*end)){
		(*begin)=(size_t)-1;
		(*end)=0;
	}


	if ((*begin) == (size_t)-1) {
		while (_buffer->size()> 10) {
			delete (*_buffer)[0];
			_buffer->pop_front();
		}
	}
	else {
		while (((*begin) / defaultsize) > 10) {
			delete (*_buffer)[0];
			(*begin) -= defaultsize;
			(*end) -= defaultsize;
			_buffer->pop_front();
		}
	}
		
		
	if ((*end) == 0) {
		size_t now = _buffer->size();
		while (now > 10) {
			delete (*_buffer)[0];
			now--;
			_buffer->pop_front();
		}
	}
	else {
		size_t now = _buffer->size();
		while ((now - ((*end - 1) / defaultsize)) > 10) {
			delete (*_buffer)[now - 1];
			now--;
			_buffer->pop_back();
		}
	}
}


inline size_t buffer::__size() const{
	if ((*begin) == size_t(-1)) {
		return 0;
	}
	return ((*end) - (*begin));
}

#endif
