#ifndef __BUFFER_H__
#define __BUFFER_H__
#include<deque>
#include<atomic>
#include<mutex>
#include<iostream>
#define defaultsize 0x1000

class buffer {
public:
	buffer() {
		_buffer = new std::deque<char *>();
		counter = new std::atomic<int>(1);
		mutex_for_data = new std::mutex;
		begin = new size_t(-1);
		end = new size_t(0);
	}
	buffer(size_t size) {
		_buffer = new std::deque<char *>();
		counter = new std::atomic<int>(1);
		mutex_for_data = new std::mutex;
		begin = new size_t(-1);
		end = new size_t(0);

		if (size<defaultsize) {
			char *newbuff = new char[defaultsize];
			_buffer->push_back(newbuff);
		}
		else {
			for (size_t i = 0; i<((size-1) / defaultsize) + 1; i++) {
				char *newbuff = new char[defaultsize];
				_buffer->push_back(newbuff);
			}
		}
	}
	buffer(const buffer& other) {
		other.counter->fetch_add(1);

		this->_buffer = other._buffer;
		this->counter = other.counter;
		this->mutex_for_data = other.mutex_for_data;
		this->begin = other.begin;
		this->end = other.end;
	}
	buffer& operator=(const buffer& other) {
		this->counter->fetch_sub(1);
		other.counter->fetch_add(1);

		if (this->counter->load() == 0) {
			delete this->counter;
			mutex_for_data->lock();
			while (!_buffer->empty()) {
				char *temp = _buffer->front();
				delete temp;
				_buffer->pop_front();
			}
			mutex_for_data->unlock();
			delete _buffer;
			delete mutex_for_data;
			delete begin;
			delete end;
		}

		this->_buffer = other._buffer;
		this->counter = other.counter;
		this->mutex_for_data = other.mutex_for_data;
		this->begin = other.begin;
		this->end = other.end;
	}
	~buffer() {
		this->counter->fetch_sub(1);
		if (this->counter->load() == 0) {
			std::cout << "Start destroy!";
			delete this->counter;
			mutex_for_data->lock();
			while (!_buffer->empty()) {
				char *temp = _buffer->front();
				delete temp;
				_buffer->pop_front();
			}
			mutex_for_data->unlock();
			delete _buffer;
			delete mutex_for_data;
			delete begin;
			delete end;
		}
	}


	void push_back_n(char *src, size_t size) {
		std::lock_guard<std::mutex> protect(*mutex_for_data);
		if (size == 0 || src==NULL) {
			return;
		}
#ifdef _PRINT_DEBUG
		std::cout << std::endl;
		if ((*begin) == (size_t)-1) {
			std::cout << "begin is -1 and end is " << *end << std::endl;
		}
		else {
			std::cout << "begin is " << *begin << " and end is " << *end << std::endl;
			std::cout << "now size=" << ((*end) - (*begin)) << std::endl;
			std::cout << "now buffer size= " << _buffer->size() << std::endl;
		}
		std::cout << "we will push back " << size << " bytes!" << std::endl;
#endif
		if ((*begin) != (size_t)-1) {
			if ((*begin) >= (*end)) {
				throw std::logic_error("Buffer end lower than start!");
			}
		}
		else{
			/*如果buffer为空，因为size不为0，所以现在把begin置0，为确保end正常，
			也置0，但这事实上是一个不太正常的状态，需要注意*/
			(*begin) = 0;
			(*end) = 0;
		}

		while ((size+(*end))>(_buffer->size()*defaultsize) ) {
			char *newbuff = new char[defaultsize];
			_buffer->push_back(newbuff);
			
		}
		for (size_t i = 0; i < size; i++) {
#ifdef _PRINT_DEBUG
			if(((i+(*end))/defaultsize)>=_buffer->size()){
				std::cout<<"Now _buffer size= "<<_buffer->size()<<std::endl;
				throw std::logic_error("_buffer size is not enough!");
			}
			if((*_buffer)[(i + (*end)) / defaultsize]==NULL){
				throw std::logic_error("Memory Allocate or destroy error!");
			}

#endif
			(*_buffer)[(i + (*end)) / defaultsize][(i + (*end)) % defaultsize] = src[i];
		}
		(*end) += size;
#ifdef _PRINT_DEBUG
		if ((*begin) == (size_t)-1) {
			std::cout << "begin is -1 and end is " << *end << std::endl;
		}
		else {
			std::cout << "begin is " << *begin << " and end is " << *end << std::endl;
			std::cout << "now size=" << ((*end) - (*begin)) << std::endl;
			std::cout << "now buffer size= " << _buffer->size() << std::endl;
		}
		std::cout << "we have push back " << size << " bytes!" << std::endl;
		std::cout << std::endl;
#endif

	}
	void pop_back_n(size_t size) {
		std::lock_guard<std::mutex> protect(*mutex_for_data);
#ifdef _PRINT_DEBUG
		std::cout << std::endl;
		if ((*begin) == (size_t)-1) {
			std::cout << "begin is -1 and end is " << *end << std::endl;
		}
		else {
			std::cout << "begin is " << *begin << " and end is " << *end << std::endl;
			std::cout << "now size=" << ((*end) - (*begin)) << std::endl;
			std::cout << "now buffer size= " << _buffer->size() << std::endl;
		}
		std::cout << "we will pop back " << size << " bytes!" << std::endl;
#endif		
		if ((*begin) == (size_t)-1) {
			*end = 0;
		}
		//注意，end-begin是队列现在实际的size
		else if (size > (*end) - (*begin)) {
			(*begin) = (size_t)-1;
			(*end) = 0;
		}
		else {
			(*end) -= size;
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
#ifdef _PRINT_DEBUG
		if ((*begin) == (size_t)-1) {
			std::cout << "begin is -1 and end is " << *end << std::endl;
		}
		else {
			std::cout << "begin is " << *begin << " and end is " << *end << std::endl;
			std::cout << "now size=" << ((*end) - (*begin)) << std::endl;
			std::cout << "now buffer size= " << _buffer->size() << std::endl;
		}
		std::cout << "we have pop back " << size << " bytes!" << std::endl;
		std::cout << std::endl;
#endif
	}
	void pop_front_n(size_t size) {
		std::lock_guard<std::mutex> protect(*mutex_for_data);
#ifdef _PRINT_DEBUG
		std::cout << std::endl;
		if ((*begin) == (size_t)-1) {
			std::cout << "begin is -1 and end is " << *end << std::endl;
		}
		else {
			std::cout << "begin is "<<*begin<<" and end is " << *end << std::endl;
			std::cout << "now size=" << ((*end) - (*begin)) << std::endl;
			std::cout << "now buffer size= " << _buffer->size() << std::endl;
		}
		std::cout << "we will pop front " << size << " bytes!" << std::endl;
#endif
		if ((*begin) == (size_t)-1) {
			*end = 0;
		}
		else if (size > (*end - *begin)) {
			(*begin) = (size_t)-1;
			(*end) = 0;
		}
		else {
			(*begin) += size;
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
#ifdef _PRINT_DEBUG
		if ((*begin) == (size_t)-1) {
			std::cout << "begin is -1 and end is " << *end << std::endl;
		}
		else {
			std::cout << "begin is " << *begin << " and end is " << *end << std::endl;
			std::cout << "now size=" << ((*end) - (*begin)) << std::endl;
			std::cout << "now buffer size= " << _buffer->size() << std::endl;
		}
		std::cout << "we have pop front " << size << " bytes!" << std::endl;
		std::cout << std::endl;
#endif
	}
	void push_front_n(char *src, size_t size) {
		std::lock_guard<std::mutex> protect(*mutex_for_data);
		if (size == 0 || src==NULL) {
			return;
		}
#ifdef _PRINT_DEBUG
			std::cout << std::endl;
			if ((*begin) == (size_t)-1) {
				std::cout << "begin is -1 and end is " << *end << std::endl;
			}
			else {
				std::cout << "begin is " << *begin << " and end is " << *end << std::endl;
				std::cout << "now size=" << ((*end) - (*begin)) << std::endl;
				std::cout << "now buffer size= " << _buffer->size() << std::endl;
			}
			std::cout << "we will push front " << size << " bytes!" << std::endl;
#endif
		/*如空间不足申请空间，否则直接复制*/
		if ((*begin) == (size_t)-1) {
			(*begin)=0;
			(*end) = 0;
			while(size>(_buffer->size()*defaultsize)){
				char *newbuff = new char[defaultsize];
				_buffer->push_front(newbuff);
			}
			(*end)+=size;

		}
		else {
			if ((*begin) >= (*end)) {
				throw std::logic_error("Buffer end lower than start!");
			}
			while (size > (*begin)) {
				char *newbuff = new char[defaultsize];
				_buffer->push_front(newbuff);
				(*begin) += defaultsize;
				(*end) += defaultsize;
			}
			(*begin) -= size;
		}		
		for (size_t i = 0; i < size; i++) {
			(*_buffer)[((*begin)+i) / defaultsize][((*begin) + i) % defaultsize] = src[i];
		}
#ifdef _PRINT_DEBUG
		if ((*begin) == (size_t)-1) {
			
			std::cout << "begin is -1 and end is " << *end << std::endl;
		}
		else {
			std::cout << "begin is " << *begin << " and end is " << *end << std::endl;
			std::cout << "now size=" << ((*end) - (*begin)) << std::endl;
			std::cout << "now buffer size= " << _buffer->size() << std::endl;
		}
		std::cout << "we have push front " << size << " bytes!" << std::endl; 
		std::cout << std::endl;
#endif	
	}


	char get(size_t n) {
		std::lock_guard<std::mutex> protect(*mutex_for_data);
		if (n >= ((*end) - (*begin))|| *begin == (size_t)-1) {
			throw std::logic_error("Buffer Overflow!");
		}
		else {
			return (*_buffer)[((*begin) + n) / defaultsize][((*begin) + n) % defaultsize];
		}
	}
	void get_n(size_t n,char *dest,size_t size){
		std::lock_guard<std::mutex> protect(*mutex_for_data);
		if (((n+size-1) >= ((*end) - (*begin))) || (*begin == (size_t)-1)) {
			throw std::logic_error("Buffer Overflow!");
		}
		for (size_t i = 0; i < size; i++) {
			dest[i] = (*_buffer)[((*begin) + n + i) / defaultsize][((*begin) + n + i) % defaultsize];
		}
	}
	void set(size_t n,char a) {
		std::lock_guard<std::mutex> protect(*mutex_for_data);
		if (n > ((*end) - (*begin))) {
			throw std::logic_error("Buffer Overflow!");
		}
		else {
			(*_buffer)[((*begin) + n) / defaultsize][((*begin) + n) % defaultsize] = a;
		}
	}
	size_t size() {
		std::lock_guard<std::mutex> protect(*mutex_for_data);
		if ((*begin) == size_t(-1)) {
			return 0;
		}
		return ((*end) - (*begin));
	}


private:
	std::deque<char *>* _buffer;
	std::atomic<int>* counter;
	std::mutex* mutex_for_data;
	size_t* begin;
	size_t* end;
};

#endif
