#include"buffer.h"

mem_container buffer::mem(0x40000000);


buffer::buffer(){
	_buffer = new std::deque<char *>();
	counter = new std::atomic<int>(1);
	mutex_for_data = new std::mutex;
	begin = new size_t(-1);
	end = new size_t(0);
}

buffer::buffer(const buffer& other) {
	other.counter->fetch_add(1);

	this->_buffer = other._buffer;
	this->counter = other.counter;
	this->mutex_for_data = other.mutex_for_data;
	this->begin = other.begin;
	this->end = other.end;
}


buffer& buffer::operator=(const buffer& other) {
	this->counter->fetch_sub(1);
	other.counter->fetch_add(1);

	if (this->counter->load() == 0) {
		delete this->counter;
		mutex_for_data->lock();
		while (!_buffer->empty()) {
			mem.push((*_buffer)[0]);
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

buffer::~buffer() {
	this->counter->fetch_sub(1);
	if (this->counter->load() == 0) {
		delete this->counter;
		mutex_for_data->lock();
		while (!_buffer->empty()) {
			mem.push((*_buffer)[0]);
			_buffer->pop_front();
		}
		mutex_for_data->unlock();
		delete _buffer;
		delete mutex_for_data;
		delete begin;
		delete end;
	}
}


void buffer::push_back_n(const char *src, size_t size) {
	std::unique_lock<std::mutex> protect(*mutex_for_data);
	check();
	if ( size == 0 || src==NULL) {
		return;
	}
	
	if( (*begin)==(size_t)-1){
		/*如果buffer为空，因为size不为0，所以现在把begin置0，为确保end正常，
		也置0，但这事实上是一个不太正常的状态，需要注意*/
		(*begin) = 0;
		(*end) = 0;
	}

	while ((size+(*end))>(_buffer->size()*defaultsize) ) {
		char *new_memory = mem.get();
		_buffer->push_back(new_memory);
	}

	char *this_block_ptr=(*_buffer)[(*end) / defaultsize];
	for (size_t i = 0; i < size; i++) {
		if(((*end) + i)%defaultsize==0)
			this_block_ptr = (*_buffer)[((*end) + i) / defaultsize];
		this_block_ptr[(i + (*end)) % defaultsize] = src[i];
	}
	(*end) += size;
}

void buffer::pop_back_n(size_t size) {
	std::lock_guard<std::mutex> protect(*mutex_for_data);

	check();
	
	if((*begin)==(size_t)-1)
		return;
	//注意，end-begin是队列现在实际的size
	else if (size > (*end) - (*begin)) {
		(*begin) = (size_t)-1;
		(*end) = 0;
	}
	else {
		(*end) -= size;
	}

	check();
	__shrink_size();

}
void buffer::pop_front_n(size_t size) {
	std::lock_guard<std::mutex> protect(*mutex_for_data);

	check();
	
	if((*begin)==(size_t)-1){
		return;
	}
	else if (size > (*end - *begin)) {
		(*begin) = (size_t)-1;
		(*end) = 0;
	}
	else {
		(*begin) += size;
	}

	__shrink_size();
}
void buffer::push_front_n(const char *src, size_t size) {
	std::lock_guard<std::mutex> protect(*mutex_for_data);
	if (size == 0 || src==nullptr) {
		return;
	}
	check();
	/*如空间不足申请空间，否则直接复制*/
	if ((*begin) == (size_t)-1) {
		(*begin)=0;
		(*end) = 0;
		while(size>(_buffer->size()*defaultsize)){
			char *newbuff = mem.get();
			_buffer->push_front(newbuff);
		}
		(*end)+=size;
	}
	else {
		while (size > (*begin)) {
			char *newbuff = mem.get();
			_buffer->push_front(newbuff);
			(*begin) += defaultsize;
			(*end) += defaultsize;
		}
		(*begin) -= size;
	}
	char* block_to_write=(*_buffer)[(*begin) / defaultsize];
	for (size_t i = 0; i < size; i++) {
		if(((*begin)+i)%defaultsize==0){
			block_to_write=(*_buffer)[((*begin)+i) / defaultsize];
		}
		block_to_write[((*begin) + i) % defaultsize] = src[i];
		//(*_buffer)[((*begin)+i) / defaultsize][((*begin) + i) % defaultsize] = src[i];
	}
}


char buffer::get(size_t n) {
	std::lock_guard<std::mutex> protect(*mutex_for_data);
	if (n >= ((*end) - (*begin))|| *begin == (size_t)-1) {
		throw std::logic_error("index out of bounds!");
	}
	else {
		return (*_buffer)[((*begin) + n) / defaultsize][((*begin) + n) % defaultsize];
	}
}
//not checked
int buffer::get_n(size_t n,char *dest,size_t size){
	std::lock_guard<std::mutex> protect(*mutex_for_data);
	if( ((*begin)==(size_t)-1) && ((*begin)>=(*end))){
		(*begin)=-1;
		(*end)=0;
		return 0;
	}
	if ( n >= ((*end) - (*begin)) ) {
		return 0;
	}
	if ( ((*begin) + n + size - 1) >= (*end) ){
		size = (*end) - ((*begin) + n);
	}
	char *this_block_ptr=(*_buffer)[((*begin) + n) / defaultsize];
	for (size_t i = 0; i < size; i++) {
		if(((*begin) + n + i)%defaultsize==0)
			this_block_ptr = (*_buffer)[((*begin) + n + i) / defaultsize];
		dest[i] = this_block_ptr[((*begin) + n + i) % defaultsize];
		//dest[i] = (*_buffer)[((*begin) + n + i) / defaultsize][((*begin) + n + i) % defaultsize];
	}
	return size;
}
void buffer::set(size_t n,char a) {
	std::lock_guard<std::mutex> protect(*mutex_for_data);
	if (n > ((*end) - (*begin))) {
		throw std::logic_error("Buffer Overflow!");
	}
	else {
		(*_buffer)[((*begin) + n) / defaultsize][((*begin) + n) % defaultsize] = a;
	}
}
size_t buffer::size() const{
	std::lock_guard<std::mutex> protect(*mutex_for_data);
	return __size();
}

void buffer::shrink_size(){
	std::lock_guard<std::mutex> protect(*mutex_for_data);
	__shrink_size();
}




