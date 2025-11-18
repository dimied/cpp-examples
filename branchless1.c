#include <stdlib.h>

// Type your code here, or load an example.
float addWithBranch1(float* pMem, float thresh, std::size_t n) {
    float p = 1.0;

    for(std::size_t i=0; i < n; i++) {
        float x = pMem[i];
        if(x > thresh) {
            p *= 2.0*x;
        } else {
            p *= 1.5*x;
        }
    }
    return p;
}

float addWithBranch2(float* pMem, float thresh, std::size_t n) {
    float p = 1.0;

    for(std::size_t i=0; i < n; i++) {
        float x = pMem[i];
        p *= (x > thresh)?2.0*x:1.5*x;
    }
    return p;
}

float addWithBranch3(float* pMem, float thresh, std::size_t n) {
    float p = 1.0;

    std::size_t hits = 0;
    for(std::size_t i=0; i < n; i++) {
        float x = pMem[i];
        hits += (x > thresh);
        p *= x;
    }
    p *= ((float)hits)*2.0*((float)(n-hits))*1.5;
    return p;
}