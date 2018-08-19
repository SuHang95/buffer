#ifndef __io_buffer_h__
#define __io_buffer_h__
#include"buffer.h"
class iobuffer:public buffer{
public:
	void pop_back_to_other_front_n(iobuffer& other,size_t n);
	void pop_front_to_other_back_n(iobuffer& other,size_t n);
protected:
	void self_pop_back_to_front_n(size_t n);
	void other_pop_back_to_front_n(iobuffer& other,size_t n);
	void self_pop_front_to_back_n(size_t n);
	void other_pop_front_to_back_n(iobuffer& other,size_t n);
};






#endif
