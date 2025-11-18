
#include <stdlib.h>

int* merge(int in[][4], int numArrays) {
	if(in != 0) {
		int* result = (int*) malloc(numArrays*sizeof(int)*4);
		if(result != 0) {
			//int idx = 0;
			//result[
			for(int i=0; i < numArrays; i++) {
				for(int j=0; j < 4; j++) {
					
				}
			}
			return result;
		}
	}
	return 0;
}

int main() {

	int input[][4] = {{1,2,4,5}, {1,4,4,5}, {1,6,7,8}, {2,2,5,5}};
	merge(input,4);
	return 0;
}
