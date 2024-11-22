#include<iostream>
#include<fstream>
#include<string>
#include<cmath>

using namespace std;

int main(int argc, char *argv[]) {

  // Temporary variables
  unsigned long long addr;
  string instructionType;

  // Open file for reading
  ifstream infile(argv[1]);

  //Open file for writing
  ofstream outfile(argv[2]);

  //Direct mapped cache
  int cacheHits = 0;
  int accesses = 0;
  int cacheSize[4] = {1, 4, 16, 32};
  int i = 0;
  while(i < 4){
    int cacheLines = (cacheSize[i] * 1024) / 32;
    int cache[cacheLines] = {0};
    int indexBits = log2(cacheLines);
    unsigned int bitmask = (1 << indexBits) - 1;
    while(infile >> instructionType >> std::hex >> addr){
      int index = addr & bitmask;
    }
    i++;
  }
  
  return 0;
}
