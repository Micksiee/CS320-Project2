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
  for(int i = 0; i < 4; i++){
    int cacheLines = (cacheSize[i] * 1024) / 32;
    int cache[cacheLines] = {0};
    int indexBits = log2(cacheLines);
    unsigned int bitmask = (1 << indexBits) - 1;
    while(infile >> instructionType >> std::hex >> addr){
      int index = addr & bitmask;
      int tag = addr >> (log2(32) + log2(cacheLines));      //Differentiating the tag from the address doesn't matter for the direct mapped cache, since there is the index used, but it will matter later for fully-associative so for consistency I use it here
      if(cache[index] == tag){
        cacheHits++;
      }else{
        cache[index] = tag;
      }
      accesses++;
    }
    cout << cacheHits << "," << accesses << "; ";
    outfile << cacheHits << "," << accesses << "; "
    infile.clear();
    infile.seekg(0);
    cacheHits = 0;
    accesses = 0 ;
  }
  cout << endl;
  outfile << endl;

  //Set associative cache
  int associativity[4] = {2, 4, 8, 16};
  for(int i = 0; i < 4; i++){
    int ways = associativity[i];
    int cacheLines = (16 * 1024) / 32;
    int cacheLineSets = cacheLines / ways;
    int cache[cacheLineSets][ways] = {0};
    int LRU[cacheLineSets][ways] = {0};
    int indexBits = log2(cacheLineSets);
    unsigned int bitmask = (1 << indexBits) - 1;
    while(infile >> instructionType >> std:hex >> addr){
      int index = addr & bitmask;
      int tag = addr >> (log2(32) + log2(cacheLineSets))
      bool hit = false;
      for(int j = 0; j < ways; j++){  //Checking all of the sets in the line for the tag
        if(cache[index][j] == tag){

          cacheHits++;
          hit = true;

          for(int k = 0; k < ways; k++){  //Since there was a cache hit, we need to decrement the LRU value of all other sets
            if(k != j){
              LRU[index][k]--;
            }
          }
          LRU[index][j] = ways;           //Set the access's set to the MRU

          break;
        }
      }
      if(!hit){
        int LRUindex = 0;
        for(int j = 0; j < ways; j++){                //Find the set with the smallest LRU value
          if(LRU[index][j] < LRU[index][LRUindex]){
            LRUindex = j;
          }
        }

        cache[index][LRUindex] = tag;                //Evict LRU and replace with tag

        for(int j = 0; j < ways; j++){                //Make newly placed block MRU
          if(j != LRUindex){
            LRU[index][j]--;
          }
        }
        LRU[index][LRUindex] = ways;

      }

      accesses++;
    }
    cout << cacheHits << "," << accesses << "; ";
    outfile << cacheHits << "," << accesses << "; "
    infile.clear();
    infile.seekg(0);
    cacheHits = 0;
    accesses = 0 ;
  }
  cout << endl;
  outfile << endl;

  //Fully associative cache
  for(int i = 0; i < 2; i++){ //0 will represent LRU used, 1 will represent hot-cold approx used
    int cacheLines = (16 * 1024) / 32;
    int cache[cacheLines] = {0};
    int LRU[cacheLines] = {0};
    int HCLRUtree[cacheLines - 1] = {0};
    while(infile >> instructionType >> std:hex >> addr){
      int tag = addr >> (log2(32) + log2(cacheLines));
      bool hit = false;
      for(int j = 0; j < cacheLines; j++){
        if(cache[j] == tag){
          cacheHits++;
          hit = true;
          if(i){  //Hot cold approx used
            int tree_node = j + cacheLines - 1;         //Starting at the accessed set
            while(tree_node > 0){                      //Traversing up the tree and updating until we get to the root. once the tree_node is 0 it is the root and has no parent to update
              int parent_node = (tree_node - 1) / 2;  //We find the index of the parent node and it's left child. if the node isn't the left child, it's the right child
              int l_child = 2 * parent_node + 1;

              if(tree_node == l_child){        //Here is the check for which child it is, and then we update the parent accordingly. we only need the l
                HCLRUtree[parent_node] = 1;
              }else{
                HCLRUtree[parent_node] = 0;
              }

              tree_node = parent_node;

            }
          }else{  //LRU used
            for(int k = 0; k < cacheLines; k++){
              if(k != j){
                LRU[k]--;
              }
            }
            LRU[j] = cacheLines;
          }
        }
      }
      if(!hit){
        if(i){  //Hot cold approx
          int LRUindex;
          int tree_node = 0; //here we traverse DOWN the tree, so we start at the root

          while(tree_node < cacheLines - 1){
            if(HCLRUtree[tree_node]){       //if the current node is 1, move to the right child, if not, move left
              tree_node = 2 * tree_node + 2;
            }else{
              tree_node = 2 * tree_node + 1;
            }
          }
          LRUindex = tree_node - cacheLines + 1;  //Subtract the amount of non-leaf nodes to find the index in the cache

          cache[LRUindex] = tag;

          tree_node = LRUindex;   //Same as before, we traverse up the tree from the index of the node we just changed
          while(tree_node > 0){                      
              int parent_node = (tree_node - 1) / 2;  
              int l_child = 2 * parent_node + 1;

              if(tree_node == l_child){        
                HCLRUtree[parent_node] = 1;
              }else{
                HCLRUtree[parent_node] = 0;
              }

              tree_node = parent_node;

            }

        }else{  //LRU
          int LRUindex = 0;
          for(int j = 0; j < cacheLines; j++){  //Finding LRU
            if(LRU[j] < LRU[LRUindex]){
              LRUindex = j;
            }
          }

          for(int j = 0; j < cacheLines; j++){  //Decrementing all except LRU, which will be evicted and replaced
            if(j != LRUindex){
              LRU[LRUindex]--;
            }
          }

          LRU[LRUindex] = cacheLines;     //Setting MRU
          cache[LRUindex] = tag;          //block eviction
        }
      }
    }
    cout << cacheHits << "," << accesses << "; ";
    outfile << cacheHits << "," << accesses << "; "
    infile.clear();
    infile.seekg(0);
    cacheHits = 0;
    accesses = 0 ;
  }
  cout << endl;
  outfile << endl;

  //2-Level write through
  int L1cacheHits = 0;  //new variable names for clarity
  int L1accesses = 0;
  int L2cacheHits = 0;
  int L2accesses = 0;

  int L1cacheLines = (4 * 1024) / 32;
  int L1cacheLineSets = L1cacheLines / 4;
  int L1cache[L1cacheLinesSets][4] = {0};
  int L1LRU[L1cacheLinesSets][4] = {0};
  int L1indexBits = log2(L1cacheLineSets);
  unsigned int L1bitmask = (1 << L1indexBits) - 1;

  int L2cacheLines = (64 * 1024) / 64;
  int L2cacheLineSets = L2cacheLines / 8;
  int L2cache[L2cacheLineSets][8] = {0};
  int L2LRU[L2cacheLineSets][8] = {0};
  int L2indexBits = log2(L2cacheLineSets);
  unsigned int L2bitmask = (1 << L2indexBits) - 1;

  while(infile >> instructionType >> std:hex >> addr){
    L1accesses++;
    int L1index = addr & L1bitmask;
    int L1tag = addr >> (log2(32) + log2(L1cacheLineSets));

    int L2index = addr & L2bitmask;
    int L2tag = addr >> (log2(64) + log2(L2cacheLineSets));
    
    bool hit = false;

    for(int i = 0; i < 4; i++){
      if(L1cache[L1index][i] == L1tag){
        L1cacheHits++;
        hit = true;
        for(int j = 0; j < 4; j++){
          if(i != j){
            L1LRU[L1index][j]--;
          }
        }

        L1LRU[L1index][i] = 4;
        if(behavior == "S"){
          L2accesses++;
          for(int j = 0; j < 8; j++){
            if(L2cache[L2index][i] == L2tag){
              L2cacheHits++;
              hit = true;
              for(int j = 0; j < 8; j++){
                if(i != j){
                  L2LRU[L2index][j]--;
                }
              }
            L2LRU[L2index][i] = 8;
            break;
            }
          }
        }

        break;
      }
    }

    if(!hit){         //first we check L2 cache for a hit before we replace the block in L1
      L2accesses++;
      
      for(int i = 0; i < 8; i ++){
        if(L2cache[L2index][i] == L2tag){
          L2cacheHits++;
          hit = true;
          for(int j = 0; j < 8; j++){
            if(i != j){
              L2LRU[L2index][j]--;
            }
          }

          L2LRU[L2index][i] = 8;
          break;
        }
      }

      if(!hit){   //if no L2 cache hit, we first bring the block to L2
        int L2LRUindex = 0;
        for(int i = 0; i < 8; i++){       //finding L2 LRU
          if(L2LRU[L2index][i] < L2LRU[L2index][L2LRUindex]){
            L2LRUindex = i;
          }
        }

        L2cache[L2index][L2LRUindex] = L2tag; //evict and replace L2 LRU
        L2LRU[L2index][L2LRUindex] = 8;

        for(int i = 0; i < 8; i++){     //decrement non MRU
          if(i != L2LRUindex){
            L2LRU[L2index][i]--;
          }
        }
      }

      int L1LRUindex = 0; //now we bring the block to L1
      for(int i = 0; i < 4; i++){
        if(L1LRU[L1index][i] < L1LRU[L1index][L1LRUindex]){
          L1LRUindex = i;
        }
      }

      L1cache[L1index][L1LRUindex] = L1tag;
      L1LRU[L1index][L1LRUindex] = 4;

      for(int i = 0; i < 4; i++){
        if(i != L1LRUindex){
          L1LRU[L1index][i]--;
        }
      }
    }
  }
  cout << L1cacheHits << "," << L1accesses << ";" << L2cacheHits << "," << L2accesses << ";" << endl;
  outfile << L1cacheHits << "," << L1accesses << ";" << L2cacheHits << "," << L2accesses << ";" << endl;
  infile.clear();
  infile.seekg(0);


  //write back policy


  return 0;
}
