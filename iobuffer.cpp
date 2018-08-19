#include"iobuffer.h"


void iobuffer::pop_back_to_other_front_n(iobuffer& other,size_t n){
	//if this two instance point to same buffer in fact
	if((size_t)(begin)==(size_t)(other.begin)){
		this->mutex_for_data->lock();
		self_pop_back_to_front_n(n);
		this->mutex_for_data->unlock();
	}

	//to compare the pointer,we get the order of the mutex lock,avoid the deadlock  
	else if((size_t)(begin)<(size_t)(other.begin)){
		this->mutex_for_data->lock();
		other.mutex_for_data->lock();

		other_pop_back_to_front_n(other,n);

		other.mutex_for_data->unlock();
		this->mutex_for_data->unlock();
	}

	else{
		other.mutex_for_data->lock();
		this->mutex_for_data->lock();
			
		other_pop_back_to_front_n(other,n);

		this->mutex_for_data->unlock();
		other.mutex_for_data->unlock();
	}
}
void iobuffer::pop_front_to_other_back_n(iobuffer& other,size_t n){

	//if this two instance point to same buffer in fact
	if((size_t)(begin)==(size_t)(other.begin)){
		this->mutex_for_data->lock();
		self_pop_front_to_back_n(n);
		this->mutex_for_data->unlock();
	}

	//to compare the pointer,we get the order of the mutex lock,avoid the deadlock  
	else if((size_t)(begin)<(size_t)(other.begin)){
		this->mutex_for_data->lock();
		other.mutex_for_data->lock();

		other_pop_front_to_back_n(other,n);

		other.mutex_for_data->unlock();
		this->mutex_for_data->unlock();
	}

	else{
		other.mutex_for_data->lock();
		this->mutex_for_data->lock();
			
		other_pop_front_to_back_n(other,n);

		this->mutex_for_data->unlock();
		other.mutex_for_data->unlock();
	}
}



void iobuffer::self_pop_back_to_front_n(size_t n){
	check();
	if( ((*begin)==(size_t)-1) || (n==0) || (n>=((*end)-(*begin))) ){
		return;
	}
		
	if(  ( ((*end)-(*begin)) % defaultsize ==0 ) && (n>2*defaultsize) ){
		int last=((*end)-1) % defaultsize;

		n-=((last+1)<defaultsize?(last+1):0);
		char *first_block_ptr=(*_buffer)[(*begin)/defaultsize];
		char *last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];				
		while(last>=0 && last<defaultsize-1){
			(*begin)--;
			first_block_ptr[(*begin)%defaultsize] = 
				last_block_ptr[((*end)-1)%defaultsize];
			last--;
			(*end)--;
		}

		while(n>defaultsize){
			if((*begin)!=0){
				char* temp=(*_buffer)[((*begin)-1)/defaultsize];
				(*_buffer)[((*begin)-1)/defaultsize]=(*_buffer)[((*end)-1)/defaultsize];
				(*_buffer)[((*end)-1)/defaultsize]=temp;
				(*end)-=defaultsize;
				(*begin)-=defaultsize;
			}else if( (((*end)-1)/defaultsize) < (_buffer->size()-1) ){
				int last_valid_block_index=((*end)-1)/defaultsize;
				_buffer->push_front( (*_buffer)[((*end)-1)/defaultsize] );
				/*the pointer to last vaild block push front to the buffer queue
				,now this index increment,and the index after increment storage 
				the same pointer with the first,we have to inplace it with the pointer
				in queue's back which was unused now*/
				last_valid_block_index++;
				(*_buffer)[last_valid_block_index]=_buffer->back();
				_buffer->pop_back();

			}else{
				_buffer->push_front( (*_buffer)[((*end)-1)/defaultsize] );
				_buffer->pop_back();
			}
			n-=defaultsize;
		}

	}

	char *first_block_ptr=(*_buffer)[(*begin)/defaultsize];
	char *last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];

	while(n>0){
			if((*begin)==0){
				if((((*end)-1)/defaultsize) < (_buffer->size()-1)){
					char* temp=_buffer->back();
					_buffer->push_front(temp);
					_buffer->pop_back();
					(*begin)+=defaultsize;
					(*end)+=defaultsize;
				}
				else{
					char* temp=new char[defaultsize];
					_buffer->push_front(temp);
					(*begin)+=defaultsize;
					(*end)+=defaultsize;
				}
				first_block_ptr=(*_buffer)[((*begin)-1)/defaultsize];
				last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];
			}
			if((*begin)%defaultsize==0)
				first_block_ptr=(*_buffer)[((*begin)-1)/defaultsize];
			if((*end)%defaultsize==0)
				last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];
			(*begin)--;
			n--;
			first_block_ptr[(*begin)%defaultsize] = 
				last_block_ptr[((*end)-1)%defaultsize];

			(*end)--;
	}
	check();
	__shrink_size();
}



void iobuffer::other_pop_back_to_front_n(iobuffer& other,size_t n){
	check();
	other.check();
	if( (*begin)==(size_t)-1 ){
		return;
	}
	
	if( n>=(*end)-(*begin) )
		n=(*end)-(*begin);
	
	if( (*other.begin)==(size_t)-1 ){
		if(other._buffer->empty()){
			if( (((*end)-1)/defaultsize) < (_buffer->size()-1)){
				other._buffer->push_back(_buffer->back());
				_buffer->pop_back();			
			}else if( ((*begin)/defaultsize)>0 ){
				other._buffer->push_back(_buffer->front());
				_buffer->pop_front();
				(*begin)-=defaultsize;
				(*end)-=defaultsize;
			}else{
				char *temp=new char[defaultsize];
				other._buffer->push_back(temp);
			}
		}
		(*other.begin)=(*end)%defaultsize;
		(*other.end)=(*other.begin);
	}

	if(  ( ((*end)-(*other.begin)) % defaultsize ==0 ) && (n>2*defaultsize) ){
		int last=((*end)-1) % defaultsize;

		n-=((last+1)<defaultsize?(last+1):0);
		char *this_last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];
		char *other_first_block_ptr=(*other._buffer)[(*other.begin)/defaultsize];		
		while(last>=0 && last<defaultsize-1){
			(*other.begin)--;
			other_first_block_ptr[(*other.begin)%defaultsize] = 
				this_last_block_ptr[((*end)-1)%defaultsize];
			last--;
			(*end)--;
		}
		while(n>defaultsize){
			if((*other.begin)!=0){
				char* temp=(*other._buffer)[((*other.begin)-1)/defaultsize];
				(*other._buffer)[((*other.begin)-1)/defaultsize]=(*_buffer)[((*end)-1)/defaultsize];
				(*_buffer)[((*end)-1)/defaultsize]=temp;
				(*end)-=defaultsize;
				(*other.begin)-=defaultsize;
			}else if( (((*end)-1)/defaultsize) < (_buffer->size()-1) ){
				other._buffer->push_front( (*_buffer)[((*end)-1)/defaultsize] );
				(*_buffer)[((*end)-1)/defaultsize]=_buffer->back();
				_buffer->pop_back();

				(*end)-=defaultsize;
				(*other.end)+=defaultsize;

			}else{
				other._buffer->push_front( (*_buffer)[((*end)-1)/defaultsize] );
				_buffer->pop_back();

				(*end)-=defaultsize;
				(*other.end)+=defaultsize;
			}
			n-=defaultsize;
		}
	}


	char *this_last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];
	char *other_first_block_ptr=(*other._buffer)[(*other.begin)/defaultsize];	
	while(n>0){
		if((*other.begin)==0){
			if( (((*end)-1)/defaultsize) < (_buffer->size()-1) ){
				char* temp=_buffer->back();
				other._buffer->push_front(temp);
				_buffer->pop_back();
				(*other.begin)+=defaultsize;
				(*other.end)+=defaultsize;
			}
			else if((other._buffer->size()>0) && 
				(((*other.end)-1)/defaultsize) < (other._buffer->size()-1) ){
				char* temp=other._buffer->back();
				other._buffer->push_front(temp);
				other._buffer->pop_back();

				(*other.begin)+=defaultsize;
				(*other.end)+=defaultsize;
			}

			else{
				char* temp=new char[defaultsize];
				other._buffer->push_front(temp);
				(*other.begin)+=defaultsize;
				(*other.end)+=defaultsize;
			}
			this_last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];
			other_first_block_ptr=(*other._buffer)[(*other.begin)/defaultsize];
		}
			if( ((*other.begin)%defaultsize)==0 )
				other_first_block_ptr=(*other._buffer)[((*other.begin)-1)/defaultsize];
			if( ((*end)%defaultsize)==0 )
				this_last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];
			(*other.begin)--;
			n--;
			other_first_block_ptr[(*other.begin)%defaultsize] = 
				this_last_block_ptr[((*end)-1)%defaultsize];

			(*end)--;
	}	
	check();
	other.check();
	__shrink_size();
	other.__shrink_size();
}


void iobuffer::self_pop_front_to_back_n(size_t n){
	check();
	if( ((*begin)==(size_t)-1) || (n==0) || (n>=((*end)-(*begin))) ){
		return;
	}	
	if(  ( ((*end)-(*begin)) % defaultsize ==0 ) && (n>2*defaultsize) ){
		int first=(*begin) % defaultsize;

		char *first_block_ptr=(*_buffer)[(*begin)/defaultsize];
		char *last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];

		n-=(first%defaultsize==0?0:defaultsize-first);				
		while( (first%defaultsize)!=0 ){
			
			last_block_ptr[(*end)%defaultsize] = 
				first_block_ptr[(*begin)%defaultsize];
			first++;
			(*end)++;
			(*begin)++;
		}
		while(n>defaultsize){
			if( (*end)!=(_buffer->size()*defaultsize) ){
				char* temp=(*_buffer)[(*end)/defaultsize];
				(*_buffer)[(*end)/defaultsize]=(*_buffer)[(*begin)/defaultsize];
				(*_buffer)[(*begin)/defaultsize]=temp;
				(*end)+=defaultsize;
				(*begin)+=defaultsize;
			}else if( *begin!=0 ){
				_buffer->push_back( (*_buffer)[(*begin)/defaultsize] );
				(*_buffer)[(*begin)/defaultsize]=_buffer->front();
				_buffer->pop_front();

			}else{
				_buffer->push_back( (*_buffer)[(*begin)/defaultsize] );
				_buffer->pop_front();
			}
			n-=defaultsize;
		}

	}

	char *first_block_ptr=(*_buffer)[(*begin)/defaultsize];
	char *last_block_ptr=(*_buffer)[((*end)-1)/defaultsize];
	while(n>0){
		if((*end)==(_buffer->size()*defaultsize)){
			if(*begin>=defaultsize){
				char* temp=_buffer->front();
				_buffer->push_back(temp);
				_buffer->pop_front();
				(*begin)-=defaultsize;
				(*end)-=defaultsize;
			}
			else{
				char* temp=new char[defaultsize];
				_buffer->push_back(temp);
			
			}
		}
		if((*end)%defaultsize==0)
			last_block_ptr=(*_buffer)[(*end)/defaultsize];
		if((*begin)%defaultsize==0)
			first_block_ptr=(*_buffer)[(*begin)/defaultsize];
		n--;
		last_block_ptr[(*end)%defaultsize] = 
			first_block_ptr[(*begin)%defaultsize];
		(*end)++;
		(*begin)++;
	}
	check();
	__shrink_size();
}


void iobuffer::other_pop_front_to_back_n(iobuffer& other,size_t n){
	check();
	other.check();
	if( ((*begin)==(size_t)-1) || (n==0)){
		return;
	}
	if( n>=(*end)-(*begin) )
		n=(*end)-(*begin);
	
	if( (*other.begin)==(size_t)-1 ){
		if(other._buffer->empty()){
			if( (((*end)-1)/defaultsize) < (_buffer->size()-1)){
				other._buffer->push_back(_buffer->back());
				_buffer->pop_back();			
			}else if( ((*begin)/defaultsize)>0 ){
				other._buffer->push_back(_buffer->front());
				_buffer->pop_front();
				(*begin)-=defaultsize;
				(*end)-=defaultsize;
			}else{
				char *temp=new char[defaultsize];
				other._buffer->push_back(temp);
			}
		}
		(*other.begin)=(*begin)%defaultsize;
		(*other.end)=(*other.begin);
	}

	if(  ( ((*begin)-(*other.end)) % defaultsize ==0 ) && (n>2*defaultsize)){
		int first=(*begin) % defaultsize;

		n-=(first%defaultsize==0?0:defaultsize-first);
		
		if((first%defaultsize)!=0){
			char *this_first_block_ptr=(*_buffer)[(*begin)/defaultsize];
			char *other_last_block_ptr=(*other._buffer)[(*other.end)/defaultsize];		
			while((first%defaultsize)!=0){
				other_last_block_ptr[(*other.end)%defaultsize] = 
					this_first_block_ptr[(*begin)%defaultsize];
				first++;
				(*other.end)++;
				(*begin)++;
			}
		}
		while(n>defaultsize){
			if((*other.end)!=(other._buffer->size()*defaultsize)){
				char* temp=(*other._buffer)[(*other.end)/defaultsize];
				(*other._buffer)[(*other.end)/defaultsize]=(*_buffer)[(*begin)/defaultsize];
				(*_buffer)[(*begin)/defaultsize]=temp;
				(*begin)+=defaultsize;
				(*other.end)+=defaultsize;
			}else if( *begin!=0 ){
				other._buffer->push_back( (*_buffer)[(*begin)/defaultsize] );
				(*_buffer)[(*begin)/defaultsize]=_buffer->front();
				_buffer->pop_front();

				(*end)-=defaultsize;
				(*other.end)+=defaultsize;

			}else{
				other._buffer->push_back( (*_buffer)[(*begin)/defaultsize] );
				_buffer->pop_front();

				(*end)-=defaultsize;
				(*other.end)+=defaultsize;
			}
			n-=defaultsize;
		}
	}


	char *this_first_block_ptr=(*_buffer)[(*begin)/defaultsize];
	char *other_last_block_ptr=((*other.end)%defaultsize==0)?nullptr:(*other._buffer)[((*other.end)-1)/defaultsize];

	while(n>0){
		if((*other.end)==(other._buffer->size()*defaultsize)){
			if( (*begin)>=defaultsize ){
				char* temp=_buffer->front();
				other._buffer->push_back(temp);
				_buffer->pop_front();

				(*begin)-=defaultsize;
				(*end)-=defaultsize;
			}
			else if( (other._buffer->size()>0) && (*other.begin>=defaultsize) ){
				char* temp=other._buffer->front();
				other._buffer->push_back(temp);
				other._buffer->pop_front();

				(*other.begin)-=defaultsize;
				(*other.end)-=defaultsize;

			}

			else{
				char* temp=new char[defaultsize];
				other._buffer->push_back(temp);
			}
		}

		if((*other.end)%defaultsize==0){
			other_last_block_ptr=(*other._buffer)[(*other.end)/defaultsize];
		}
		if((*begin)%defaultsize==0){
			this_first_block_ptr=(*_buffer)[(*begin)/defaultsize];
		}
		n--;
		other_last_block_ptr[(*other.end)%defaultsize] = 
			this_first_block_ptr[(*begin)%defaultsize];
		(*other.end)++;
		(*begin)++;
	}	
	check();
	other.check();
	__shrink_size();
	other.__shrink_size();
}

