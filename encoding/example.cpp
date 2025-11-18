
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "event.h"

#include <vector>
#include <algorithm>

#include "convert_base64.h"

using namespace std;

 template <class T>
 class generate_val
 {
   private:
      T val_;
      T inc_;
   public:
      generate_val(const T& val, const T& inc) : val_(val), inc_(inc) {}
      T& operator()() { val_ += inc_; return val_; }
};

/*
template <int n> class Fib {
public:
	//enum{value = Fib<n-1>::value + Fib<n-2>::value}
	int value = getValue();
	
	constexpr int getValue() {
		 return Fib<n-1>.value + Fib<n-2>.value;
	}
};

template <1> class Fib {
public:
	int value = 1;
};

template <0> class Fib {
public:
	int value = 1;
};
*/
//std::vector<int> 
uint64_t fibStore[200] = {0};


uint64_t fib(int n) {
	if(n < 2) {
		return 1;
	}
	
	if(fibStore[n-2] > 0) {
		return fibStore[n-2];
	}
	
	
	unsigned long result = fib(n-1) + fib(n-2);
	
	fibStore[n-2] = result;
	
	return result;
}

const char* codepoints = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int main(int numArgs, char** args) {

    Event<int> myEvent(123);
    //int res = myEvent();

    //cout<< "Hoppla:" << Event<int>(122)() << endl;

    vector< Event<int> > ints;

    const size_t n = 20;

    generate_val<int> gen(1, 2);

    generate_n(back_inserter(ints), n, gen);


    //cout << "Hallo. Wie geht es denn so."<< myEvent.getData()<<"\n";
    vector< Event<int> >::iterator endIt = ints.end();//end(ints);
    vector< Event<int> >::iterator it = ints.begin();//begin(ints);

    for( ; it != endIt; ++it) {
        //cout<< "Item: "<<it->getData()<<"\n";
    }
    
    //Fib<3> f;
    
    //cout << "Fib: "<< fib(250) << endl;
    
    cout << "ORDER: " << CONVERT_BASE64_BYTR_ORDER 
    << "|" << CONVERT_BASE64_BYTR_ORDER_STR << endl
    << "----" << endl;
    
    int code = (int)'A';
    
    //cout << "A" << "|" << code << endl;
    
    int idx=0;
    
    string pairs = "";
    ostringstream s;
    
    for(;idx < 64; idx++) {
		int bcode = (int)codepoints[idx];
		char val = do_convert(idx);
		
		s << '(' << idx << ',' << (int)val << ") ";
		
		char raw_val = do_convert_raw(idx);
		
		unsigned int asByte = do_convert_to_byte(raw_val);
		
		int intVal = raw_val;
		char ok = (val != raw_val) ? '-' : ' ';
		
		int diffVal = intVal - val;
		
		cout << ok << " | " << val << " | " << raw_val << " |b> " << asByte << "|"
			<< " <- " << idx << "(" << (idx - asByte) << ")"
			<< " | " << intVal 
			<< " | " << diffVal << endl;
		
		if(false && idx != bcode - code) {
			cout << "Mismatch: " << codepoints[idx] 
			<< " | BCODE: "<< bcode << " | CP: "<< idx
			<< " | DIFF: " << (bcode - code)
			<< " | DIFF: " << (bcode - code - 26) <<endl;
		}
	}
	
	cout << endl << "-------" << endl;
	//cout << s.str() << endl;
    
    convert_base64_set_cache_size( 8 * 1024 );
    
    convert_base64_remove_cache();

    return 0;
}

