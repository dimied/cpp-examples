
#include <stdlib.h>
#include <vector>

using namespace std;

struct IntArray
{
  int length;
  int *field;
};

// 1.te Version

struct IntArray* createIntArray (int length)
{
  if(length < 0) {	//auch <= sinnvoll
	return 0;	//arrays mit negativen Groessen unzulaessig
  }
 
  struct IntArray* a = (struct IntArray*) malloc(sizeof(struct IntArray));
 
  if(a != 0) {
	  int* mem = (int*) malloc(length);
	  if(mem == 0) {
		  //Allokation fehlgeschlagen
		  free((void*)a);  //bereit allozierten Speicher freigeben.
		  return 0;
	  }
	  //Groesse & Pointer zuweisen
	  a->length = length;
	  a->field = mem;
	  return a;
  }
  //sonst Allokation fehlgeschlagen
  return 0;
}

// 2.te Version
struct IntArray* createIntArray2 (int length)
{
  if(length < 0) {	//auch <= sinnvoll
	return 0;	//arrays mit negativen Groessen unzulaessig
  }
  
  int* mem = (int*) malloc(length);
  if(mem == 0) {
	  //Allokation fehlgeschlagen
	  return 0;
  }
  struct IntArray* a = (struct IntArray*) malloc(sizeof(struct IntArray));
  
  if(a != 0) {
	  //Groesse & Pointer zuweisen
	  a->length = length;
	  a->field = mem;
	  return a;
  }
  
  free(mem);//bereit allozierten Speicher freigeben.
  //sonst Allokation fehlgeschlagen
  return 0;
}

typedef struct {
	int length;
	double* data;
} SimpleDoubleArray;

SimpleDoubleArray* callocSimpleArray(size_t size) {
	double* mem = (double*) calloc(sizeof(double), size);
	if(mem != 0) {
		SimpleDoubleArray* arr = (SimpleDoubleArray*) malloc(sizeof(SimpleDoubleArray));
		if(arr != 0) {
			arr->length = size;
			arr->data = mem;
			return arr;
		}
		free(mem);
	}
	return 0;
}

int my_hash(double a, double* alphabet) {
	return 0;
}

vector< vector<double> > data;
vector< vector<double> > emission;
vector< vector<double> > transition;
vector<double> states;
vector<double> start;
double* alphabet;

void func(int max_length) {

int numStates = (int)states.size();
int sizeData = (int)data.size();
int statesMulLength = numStates * max_length;
int matrixSize = statesMulLength * sizeData;

double* forward_matrix = new double[matrixSize];
double* backward_matrix = new double[matrixSize];
double* seq_prob = new double[sizeData];

for(int i=0; i < matrixSize; i++) {
	forward_matrix[i] = 0;
}

for(int i=0; i < matrixSize; i++) {
	backward_matrix[i] = 0;
}

for(int i=0; i < sizeData; i++) {
	seq_prob[i] = 0;
}

//int matIdx;
int matRowOffset;
int matRowOffsetLoopConst;
int data_nsize;

for(int n = 0; n < sizeData; n++) {
       // Berechnung aller Forward-Matrizen
       matRowOffsetLoopConst = statesMulLength*n;
       data_nsize = data[n].size();
		int hash_value = my_hash(data[n][0], alphabet);
       
       for(int i = 0; i < numStates; i++)
       { 
		   forward_matrix[matRowOffsetLoopConst + i] = start[i] * emission[i][hash_value]; 
		}

    
    matRowOffset2 = matRowOffsetLoopConst + numStates;
    for(int i = 1; i < data_nsize; i++) {
		hash_value = my_hash( data[n][i], alphabet);
        for(int j = 0; j < numStates; j++) {

            double tmp_prob = 0;
            for(int k = 0; k < numStates; k++)
            { 
				tmp_prob +=  forward_matrix[matRowOffset + k] * transition[k][j]; 
			}

            forward_matrix[matRowOffset2+ j] = tmp_prob * emission[j][hash_value];
        }
        matRowOffset += numStates;
        matRowOffset2 += numStates;
    }

    // Berechnung aller Backward-Matrizen
		matRowOffset = matRowOffsetLoopConst + (data_nsize-1)*numStates;
		
        for(int i = 0; i < numStates; i++) { 
			backward_matrix[matRowOffset + i] = 1; 
		}

	matRowOffset = matRowOffsetLoopConst;
	int matRowOffset2 = matRowOffsetLoopConst + numStates*(data_nsize-1);
	
     for(int i = data_nsize-2; i >= 0; i--) {
		 
		 hash_value = my_hash(data[n][i+1],alphabet);
		 
        for(int j = 0; j < numStates; j++) {

            double tmp_prob = 0;
            for(int k = 0; k < numStates; k++)
            { 
				tmp_prob += backward_matrix[matRowOffset2 + k] * transition[j][k] * emission[k][hash_value];
			}

            backward_matrix[matRowOffset + j] = tmp_prob;
        }
        matRowOffset += numStates;
        matRowOffset2 -= numStates;
    }
     
    // Berechnung der Sequenz-Wahrscheinlichkeiten
    matRowOffset = matRowOffsetLoopConst + 1*numStates;
    
    double sp= 0;
    for(int i = 0; i < numStates; i++) {
		sp += backward_matrix[matRowOffset + i] * forward_matrix[matRowOffset + i];
	}
	
	seq_prob[n] = sp;
} 	

}

int main (void)
{
   struct IntArray *b;
   // b ist jetzt ein Pointer auf ein nicht initialisiertes IntArray ohne allokierten Speicherbereich
   b = createIntArray (5);
   if(b != 0) {
	// mit b weiterarbeiten   ...
   }
   return 0;
}
