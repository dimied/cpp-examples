//This is the first implementation of BBgS
//BBGS: Bit-Block guided Search


int counters[256];
int left[256];
//int mins[256];
//int maxs[256];

#define CLEAR_COUNTERS		for(int i=0; i < 256; i++) {counters[i] = 0;}
#define COPY_COUNTERS		for(int i=0; i < 256; i++) {left[i] = counters[i];}
#define FILL_COUNTERS_UCHAR(PTR, END_PTR, OFFSET)	\
							while(PTR != END_PTR) { \
								counters[*PTR] += 1; \
								PTR += OFFSET; \
							}

#define SHIFT_BY(VAL,SHIFT)	(VAL>>SHIFT)
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

#define GET_CUR_MIN_IDX		while(left[cur_min_idx] == 0) {	++cur_min_idx;	}

void bbgsUINT(unsigned int *array, int n) {

	unsigned char* ucp = (unsigned char*) array;
	unsigned char* uc_ptr = ucp;
	unsigned int* uint_ptr = array;
	int offset=0;
	int idx = 0, cur_min_idx = 0, old_idx = 0;
	unsigned int val1, val2;
	int m = n;
	unsigned char* ucp2 = ucp + n*4;
	
	CLEAR_COUNTERS
	FILL_COUNTERS_UCHAR(ucp, ucp2, 4)
	COPY_COUNTERS
	
	//go through counters and adjust offsets
	

	int max_idx = cur_min_idx;
	for(; max_idx < 256; max_idx++) {
		val1 = counters[max_idx];
		if(val1!=0) {
			counters[max_idx] = offset;
			offset += val1;
			if(offset==n) {	break;	}	//no adds more possibly
		}
	}

	ucp = ucp_ptr;
	val1 = *uint_ptr;		//read DWORD
	
	while(m != 0) {
		offset = SHIFT_BY(val1,24);	//get upper 8 bits
		idx = counters[offset];		//get target position in the array
		counters[offset] += 1;		//adjust in-bucket position for next iter
		if(old_idx == idx) {		//accidentially this value is in the right bucket
			GET_CUR_MIN_IDX			//go to next bucket
									// where is still something to do
			idx = counters[cur_min_idx];
			val1 = uint_ptr[idx];	//in the next iter the else-brach will be executed
		} else {
			val2 = uint_ptr[idx];		//get value currently in the wrong position
			uint_ptr[idx] = val1;		//assign the right value
			val1 = val2;
			old_idx = idx;
		}
		--m;
	}
	//here we repacked the array, by moving n DWORDs
	//Let's sort by 3rd byte of 4 bytes datastructures

}

unsigned int testA[] = {
0x10FFBBAA,0x10FFBB6A,0x10FFBB8A,0x10FFBB7A,
0x34FFBBAA,0x10FFBB2A,0x10FFBB4A,0x54FFBB3A };
void main() {


}
