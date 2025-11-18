#include <stdio.h>
#include <iostream>

int main() {

  unsigned int y=0, z,z2, x, x_rest, y_rest;
  unsigned int c[2] = {0,0};
  long long max = (1L<<24);
  int printed = 0, printed2 = 0;
  std::cout<<"------------------------------\n";
  for(int j = -5; j < 5; j++) {
    c[0] = 0;
    c[1] = 0;
    if(j==0) {
      std::cout<<"Offset: "<<j<<"\t x == y\n";
    } else {
      if(j>0) {
	std::cout<<"Offset: "<<j<<"\t x > y\n";
      } else {
	std::cout<<"Offset: "<<j<<"\t x < y\n";
      }
    }
    printed = 0;
    printed2 = 0;
  for(long long i=0; i < max; i++) {
    if(j==0) {
      x = (unsigned int) i;
      y = (unsigned int) i;
    } else {
      if(j > 0) {
	x = (unsigned int) i;
	y = (i > j) ? (i - j) : 0;
      } else {
	y = (unsigned int) i;
	int a = -j;
	x = (i > a) ? (i - a) : 0;
      }
    }
    //equality check
    z = x ^ y;
    z = (z | (z>>16))&0xFFFF;
    z = (z | (z>>8))&0xFF;
    z = (z | (z>>4))&0xF;
    z = (z | (z>>2))&0x3;
    z = (z | (z>>1))&0x1;
    //z=0, if equal; z=1 otherwise
    //unequality check
    z2 = (((y>>16)&0xFFFF)+0xFFFF) - ((x>>16)&0xFFFF) | (((y&0xFFFF)+0xFFFF) - (x&0xFFFF));
    z2 = (z2>>16)&0x1;
    z = (z*z2);//>>1;
    
    if((z==0)&&(c[z]==0)&&(printed == 0)){
      std::cout<<"0I: "<<i<<"\n";
      printed = 1;
    }
    if((z==1)&&(c[z]==0)&&(printed2 == 0)){
      std::cout<<"1I: "<<i<<"\n";
      printed2 = 1;
    }
    if((z==1)&&(y <= x )) {
      std::cout<<"x = "<<x<<"\ty = "<<y<<"\n";
      break;
    }
    if((z==0)&&(y > x)) {
      std::cout<<"x = "<<x<<"\ty = "<<y<<"\n";
      break;
    }
    c[z]++;
  }
  std::cout<<"y"<<"<=""x:\tC[0]="<<c[0]<<"\t\tx>y:\tC[1]="<<c[1]<<"\n\n";
  }
  std::cout<<"------------------------------\n";
  return 0;
}
