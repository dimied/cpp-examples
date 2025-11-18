//This is the first implementation of BBgS
//BBGS: Bit-Block guided Search


int counters[256];
int left[256];
int mins[256];
int maxs[256];
int counters_copy[256];

#define CLEAR_COUNTERS		for(int i=0; i < 256; i++) {counters[i] = 0;}
#define COPY_COUNTERS		for(int i=0; i < 256; i++) {left[i] = counters[i];}

void sortUnsignedChars(void* array, int n, int offset) {

	CLEAR_COUNTERS
	unsigned char* ucp = (unsigned char*)array;
	for(int i=0; i< n; i++) {
		counters[*ucp] += 1;
		ucp += offset;	
	}

	ucp = (unsigned char*)array;
	for(int j=0; j< 256; j++) {
		int l = counters[j];
		while(l != 0) {
			*ucp = j;
			ucp += offset;
			--l;
		}
	}
}


void bbgs(unsigned int *array, int n) {

	CLEAR_COUNTERS

	unsigned char* ucp = (unsigned char*) array;
	unsigned char* uc_ptr = ucp;
	unsigned int* uint_ptr = array;

	for(int i=0; i<n; i++) {
		counters[*uc_ptr] += 1;
		uc_ptr += 4;		//offset 4 for UINTs
	}
	int offset = 0;
	int val;
	
	//go through counters and adjust offsets
	for(int i=0; i<256; i++) {
		val = counters[i];
		if(val!=0) {
			counters[i] = offset;
			offset += val;
		}
	}
	int m = n;
	uc_ptr = ucp;
	int idx = 0;
	unsigned int val1, val2;
	val1 = uint_ptr[idx];		//read DWORD
	int cur_min_idx = 0;
	int old_idx;
	while(m != 0) {
		offset = (val1>>24);		//get upper 8 bits
		idx = counters[offset];		//get real position in the array
		counters[offset] += 1;		//for next iter, next bucket for this class
		if(old_idx == idx) {		//accidentially this value is in the right bucket
			while(left[cur_min_idx] == 0){
				++cur_min_idx;
			}
			idx = counters[cur_min_idx];
			val1 = uint_ptr[idx];
		} else {
			val2 = uint_ptr[idx];		//get value currently in the wrong position
			uint_ptr[idx] = val1;		//assign the right value
			val1 = val2;
			old_idx = idx;
		}
		--m;
	}
	//here we repacked the array, by moving n DWORDs

}
