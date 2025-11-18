
#include "yamalloc.h"

void* mallocNumBytes(int size, int numBytesAlignment) {

	char *ptr2, *aligned_ptr;
	int align_mask = numBytesAlignment - 1;

	char* ptr = (char *)malloc(size + numBytesAlignment + sizeof(int));
	if(ptr==NULL) return(NULL);

	ptr2 = ptr + sizeof(int);
	aligned_ptr = ptr2 + (numBytesAlignment - ((size_t)ptr2 & align_mask));


	ptr2 = aligned_ptr - sizeof(int);
	*((int *)ptr2)=(int)(aligned_ptr - ptr);

	return(aligned_ptr);
}
void* malloc4bytes(int size) {
	return mallocNumBytes(size, 4);
}
void* malloc8bytes(int size) {
	return mallocNumBytes(size, 8);
}
void* malloc16bytes(int size) {
	return mallocNumBytes(size, 16);
}
void* malloc32bytes(int size) {
	return mallocNumBytes(size, 32);
}

void aligned_free(void *ptr) {

  int *ptr2=(int *)ptr - 1;
  ptr -= *ptr2;
  free(ptr);
}
