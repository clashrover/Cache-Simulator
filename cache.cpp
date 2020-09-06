#include <bits/stdc++.h>
using namespace std;

#define ll long int

class block{
private:
	int size, valid, dirty, T, address;
	ll data;
public:
	block(int size, int valid, int dirty, int address,ll data, int time)
	{
		this->size = size;
		this->valid = valid;
		this->data = data;
		this->address = address;
		this->dirty = dirty;
		this->T = time;
	}
	void setTime(int t1){
		T = t1;
	}
	void show(){
		cout << "v:"<< valid << " " ;
		cout << "d:"<< dirty << " " ;
		cout << "add:"<< address << " "; 
		cout << "( "<< data << " ) "; 
		cout << "time:"<< T <<endl; 
	}
	int getAddress(){
		return address;
	}
	void setData(ll d){
		data = d;
	}
	ll getData(){
		return data;
	}
	int getSize(){
		return size;
	}
	int getTime(){
		return T;
	}
	bool checkDirty(){
		return (dirty==0)?false : true;
	}
	bool checkValid(){
		return (valid==0)?false:true;
	}
	void setValid(int v){
		valid = v;
	}
	void setDirty(int d){
		dirty = d;
	}
	void setSize(int ss){
		size = ss;
	}
};

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


vector <block*> mem;

class write_buffer
{
private:
	int size;
	queue<block*> buffer;
public:
	write_buffer(int size);
	void push(block* b);
	block* search(int block_address);
	void clear_buffer();
};

write_buffer::write_buffer(int sz){
	size=sz; 
}

void write_buffer:: push(block* b){
	if(buffer.size()>=size){
		clear_buffer();
	}
	buffer.push(b);
}

block* write_buffer:: search(int block_address){
	// search and if found return the block and remove it from buffer
	int i=0;
	block* r=NULL;
	int n=buffer.size();
	while(i<n){
		block* b = buffer.front();
		buffer.pop();
		if(b->getAddress() ==block_address && b->checkValid()){
			r=b;
			r->setDirty(0);
		}else{
			buffer.push(b);
		}
		i++;
	}
	return r;
}

void write_memory(block* b){
	bool x=false;
	for(int i=0;i<mem.size();i++){
		if(b->getAddress() == mem[i]->getAddress()){
			mem[i]->setData(b->getData());
			x=true;
			break;
		}
	}
	if(!x){
		mem.push_back(b);
	}
}

void write_buffer:: clear_buffer(){
	while(!buffer.empty()){
		write_memory(buffer.front());
		buffer.pop();
	}
}

block* read_memory(int address){
	int i=0;
	while(i<mem.size()){
		if(mem[i]->getAddress()==address && mem[i]->checkValid()){
			block* b = new block(mem[i]->getSize(), 1, 0, address, mem[i]->getData(),0);
			return b;
		}
		i++;
	}
	block* b = new block(0, 1, 0, address, 0, 0);
	return b;
}




// //---------------------------------------------------------------------------------------
class cache_set
{
private:
	int size, divide,  low, T;
	int bsize;
	queue<block*> l;
	queue<block*> h;
	write_buffer* buf;
public:
	cache_set(int s, int bs, int tt, write_buffer* bf){
		T=tt;
		bsize=bs;
		buf = bf;
		size = s;
		divide = s/4;
		if(divide==0){
			low =  1;
		}else{
			low = divide;
		}
		divide = s;
		for(int i=0;i<s;i++){
			block* b = new block(bsize, 0, 0, 0,0, T);
			l.push(b);
		}
	}

	void pushLow(block* b){
		// inserting a block to low queue
		// LRU used for inserting
		// Size of low increases only when block from higher
		// come to lower group
		// It decreases when block from lower goes to higher
		// group.
		// If size of low goes down to certain limit then size is maintained
		// by getting LRU of higer group to lower when conversion take place toward right.
		// If size of low goes up to certain limit then MRU of low is 
		// pushed to higher group. whenever conversion take place towards left.
		block* n = l.front();
		l.pop();
		l.push(b);
		if(n->checkDirty() && n->checkValid()){
			buf->push(n);
		}
		maintainHigh();
	}

	void pushHigh(block* b){
		// inserting a block to high queue
		b->setTime(T+1);
		h.push(b);
		if(l.size()<low){  // if low group has reached less size than lower limit 
			block* n = h.front();
			h.pop();
			l.push(n); // low has lower size so directly push LRU of high to MRU of low
		}
		maintainHigh();
	}

	void maintainHigh(){
		// move all T non-access blocks to low group
		// maintain size;
		int i=0;
		int hsize = h.size();
		while(i<hsize){
			block* n = h.front();
			int tt=n->getTime()-1;
			n->setTime(tt);
			h.pop();
			if(tt==0){
				l.push(n);
			}else{
				h.push(n);
			}
			i++;
		}
	}

	void write(int address, int data){
		// search for address, if there then write
		int i=0;
		block* ans = NULL;
		int lsize = l.size();
		while(i<lsize){
			block* b = l.front();
			l.pop();
			if(b->getAddress()==address && b->checkValid()){
				ans = b;
				i++;
				continue;
			}
			l.push(b);
			i++;
		}
		if(ans!=NULL){
			ans->setData(data); 
			ans->setValid(1);
			ans->setDirty(1);
			// add ans to higher group
			pushHigh(ans);
			return;
		}
		i=0;
		int hsize = h.size();
		while(i<hsize){
			block* b = h.front();
			h.pop();
			if(b->getAddress()==address && b->checkValid()){
				ans = b;
				i++;
				continue;
			}
			h.push(b);
			i++;
		}
		if(ans!=NULL){
			ans->setData(data); ans->setTime(T+1);
			ans->setDirty(1);
			h.push(ans);
			ans->setValid(1);
			maintainHigh();
			return;
		}
		// if not there search in buffer
		// if there make new block and insert to low group
		// maintain LRU
		ans = buf->search(address);
		if(ans!=NULL){
			ans->setData(data); 
			ans->setValid(1);
			ans->setDirty(1);
			// add to low group
			pushLow(ans);
			return;
		}

		// if not in buffer get block from memory
		// evict the LRU block and send to write buffer if valid and dirty
		ans = read_memory(address);
		if(ans!=NULL){
			// push to low group maintain LRU
			ans->setData(data); 
			ans->setValid(1);
			ans->setDirty(1);
			ans->setSize(bsize);

			pushLow(ans);
		}else{
			cout << "Invalid Address couldn't be found in mem" << endl;
		}
		return;
	}

	block* read(int address){
		int i=0;
		block* ans = NULL;
		// check in low group
		int lsize = l.size();
		while(i< lsize){
			block* b = l.front();
			l.pop();
			if(address == b->getAddress() && b->checkValid()){
				ans = b;
				i++;
				continue;
			}
			l.push(b);
			i++; 
		}
		if(ans!=NULL){
			// set time of ans to T and promote to high
			pushHigh(ans);
			return ans;
		}
		// check in high group if not found in low
		i=0;
		int hsize = h.size();
		while(i<hsize){
			block* b = h.front();
			h.pop();
			if(address == b->getAddress() && b->checkValid()){
				ans = b;
				i++;
				continue;
			}
			h.push(b);
			i++; 
		}	
		// reduce timer of all other by 1
		if(ans!=NULL){
			ans->setTime(T+1);
			h.push(ans);
			maintainHigh();
			return ans;
		}

		// if not found in set check in write buffer
		// search in buffer.
		// If found then make new block from req returned.
		// push the block into low using LRU
		// policy, evicted block goes to buffer if valid and dirty
		ans = buf->search(address);
		if(ans!=NULL){
			pushLow(ans);
			// add to low group maintain LRU
			return ans;
		}

		// If not found in buffer return from memory
		// push to low group while maintaining LRU policy
		// the LRU block evicted if is dirty and valid then
		// push to write buffer else don't
		ans = read_memory(address);
		if(ans!=NULL){
			ans->setValid(1);
			ans->setDirty(0);
			ans->setSize(bsize);
			pushLow(ans);
			// push to low group
		}else{
			cout << "Invalid Address couldn't be found in mem" << endl;
		}
		return ans;
	}

	void show(){
		int i=0;
		cout << "Low group\n";
		int lsize = l.size();
		while(i<lsize){
			block* b = l.front();
			l.pop();
			l.push(b);
			b->show();
			i++;
		}
		cout << "High group\n";
		i=0;
		int hsize = h.size();
		while(i<hsize){
			block* b = h.front();
			h.pop();
			h.push(b);
			b->show();
			i++;
		}
		cout << endl;

	}
	
};


class cache{
private:
    int size,block_size,n_way,T;
    int no_sets;
    vector<cache_set*> arr;
    write_buffer* buf;
public:
    cache(int s, int b, int a, int t);
    void write(int address, ll data);
    block* read(int address);
    void show();
};

cache::cache(int s, int b, int a, int time)
{
    size = s;
    block_size = b;
    n_way = a;
    T = time;
    no_sets = s/a;
    buf = new write_buffer(10);  // static size of write buffer 10
	
    for(int i=0;i<size;i++){
    	if(i%a == 0){
    		cache_set* c = new cache_set(n_way, b, T,buf);
    		arr.push_back(c);
    	}
    }
}

void cache::write(int address, ll data){
	int set = address% no_sets;
	arr[set]->write(address,data);
	for(int i=0;i<no_sets;i++){
		if(i==set){
			continue;
		}
		arr[i]->maintainHigh();
	}
}

block* cache::read(int address){
	int set = address%no_sets;
	block* ans = arr[set]->read(address);
	for(int i=0;i<no_sets;i++){
		if(i==set){
			continue;
		}
		arr[i]->maintainHigh();
	}
	if(ans==NULL){
		cout << "Invalid Address given to Read\n";
	}
	return ans;
}

void cache :: show(){
	buf->clear_buffer();
	cout << "---------------Cache State---------------\n";
	for(int i=0;i<no_sets;i++){
		cout << "Set - " << i << endl;
		arr[i]->show();
	}
}



//--------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

int main(){
	ifstream infile("input.txt");
	string line;
	getline(infile,line);
	istringstream iss1(line);
    int size;
    iss1 >> size;

    getline(infile,line);
	istringstream iss2(line);
    int block_size;
    iss2 >> block_size;

    getline(infile,line);
	istringstream iss3(line);
    int a;
    iss3 >> a;

    getline(infile,line);
	istringstream iss4(line);
    int t;
    iss4 >> t;

    cache* c = new cache(size,block_size,a,t);
 	

 	
	while(getline(infile,line)){
		istringstream iss(line);
		int address;
		iss>>address;
		string op;
		iss>>op;
		if(op=="R"){
			cout << "Read" << endl;
			block* b = c->read(address);
	 		if(b==NULL){
	 			cout << "Wrong Address To Read" << endl;
	 		}else{
	 			b->show();
	 		}
		}else if(op=="W"){
			cout << "Write" << endl;
			ll data;
	 		iss >> data;
	 		c->write(address,data);
		}else{
			cout << "Invalid operation" << endl;
		}
	}
	c->show();
 	// NICE example 64 4 4 5
 //    for(int i=0;i<25;i++){
 //    	ll d = i*10;
 //    	c->write(i,d);
 //    }
 //    c->show();
 //    for(int k=0;k<15;k++){
 //    	for(int i=0;i<=48;i+=16){
	//     	block* b = c->read(i);
	//     	c->show();
	//     }
	//     // c->show();
	// }

	// NICE example 64 4 4 5
 //    for(int i=0;i<25;i++){
 //    	ll d = i*10;
 //    	c->write(i,d);
 //    }
 //    c->show();
 //    for(int k=0;k<15;k++){
 //    	for(int i=0;i<=48;i+=16){
	//     	block* b = c->read(i);
	//     	c->show();
	//     }
	//     // c->show();
	// }

    // cout << endl;
    // cout << "Reading commands" << endl;
    // for(int i=0;i<10;i++){
    // 	ll d = i*100;
    // 	block* bb = c->read(i);
    // 	// bb->show();
    // }
    // cout << "-----------------" << endl;
    // c->show();
}