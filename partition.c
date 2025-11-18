
#include <stdlib.h>
#include <stdio.h>


int num = 20;

int* getArray(char* filename) {
	int *p = (int*)malloc(4*num);
	return p;
}

int filterZeros(int* pArr, int n) {

	int n0 = 0;

	while( (n0<n) && (pArr[n0] == 0) ) {
		n0++;
	}
	return n0;
}

int* allocPart(int n, int defValue) {
	int *p = (int*)malloc(4*n);
	int i;
	if(p != 0) {
		for(i=0; i < n; i++) {
			p[i] = defValue;
		}
	}
	return p;
}

int arrSum(int *pArr, int n) {
	int sum = 0, i=0;
	if(pArr != 0) {
		for(i=0; i < n; i++) {
			sum += pArr[i];
		}
		
	}
	return sum;
}

int* arrSort(int *pArr, int n) {
	int *p = (int*)malloc(4*n);
	int i;
	if(p != 0) {
		for(i=0; i < n; i++) {
			p[i] = pArr[i];
		}
	}
	return p;
}

int* solve(int* pArr, int n, int sum, int* pPart, int sum1, int sum2);

int main(char** args, int argc) {

	int n = num;
	int *pArr = getArray(args[1]);
	int sum = arrSum(pArr, n);
	if(sum%2 != 0) {
		printf("Array sum NOT EVEN!");
		return 1;
	}
	
	int* pArrSorted = arrSort(pArr, n);
	
	int *p = allocPart(n, 255);

	int nZeros = filterZeros(pArr,n);

	int *part = solve(pArrSorted+nZeros, n-nZeros, sum, p+nZeros, 0, 0);

	if(part == 0) {
		printf("Not Solvable");
	} else  {
		printf("Solved");
	}
	return 0;
}

void setPartValues(int* pPart, int start, int end, int value) {

	if(pPart != 0) {
		for(; start < end; start++) {
			pPart[start] = value;
		}
	}
}

int* solve(int* pArr, int n, int sum, int* pPart, int sum1, int sum2) {

	int last = pArr[n-1];
	int first = pArr[0];
	int halfsum = sum/2;
	int i;
	if(last > halfsum) {
		printf("Greatest element value %i is greater than C = %i", last, sum);
		return 0;
	} else {
		if(last == halfsum) {
			pPart[n-1] = 1;
			setPartValues(pPart, 0, n-1, 0);
			return pPart;
		}
		if(last + first > halfsum) {
			printf("The sum between largest and smallest elements: %i + %i = %i ", first, last, first+last);
			printf(" IS LARGER than then C = %i", halfsum);
			return 0;
		} else {
			if(last + first == halfsum) {
				pPart[n-1] = 1;
				pPart[0] = 1;
				setPartValues(pPart, 1, n-1, 0);
				return pPart;
			}
		}
	}
	if( n < 2 ) {
		return 0;
	}
	if(pArr[n-1] + pArr[n-2] > halfsum ) {
		if(pArr[n-1] == pArr[n-2]) {
			
		}
		sum1 += pArr[n-2];
		sum2 += pArr[n-1];
		pPart[n-1] = 1;
		pPart[n-2] = 0;
		return solve(pArr, n-2, sum, pPart, sum1, sum2);
	} else {
		if(pArr[n-1] + pArr[n-2] == halfsum ) {
			pPart[n-1] = 1;
			pPart[n-2] = 1;
			setPartValues(pPart, 0, n-2, 0);
			return pPart;
		}
	}
	return 0;
}
