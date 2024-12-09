#include<iostream>
#include<fstream>
#include<string>
#include<cmath>
#include<bitset>

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
  for(int i = 0; i < 4; i++){
    int cacheLines = (cacheSize[i] * 1024) / 32;
    int cache[cacheLines] = {0};
    int indexBits = log2(cacheLines);
    int blockOffset = log2(32);
    unsigned int bitmask = (1 << (indexBits)) - 1;
    while(infile >> instructionType >> std::hex >> addr){
      addr = addr >> blockOffset;
      int index = addr & bitmask;
      int tag = addr >> indexBits;
      if(cache[index] == tag){
        cacheHits++;
      }else{
	cache[index] = tag;
      }
      accesses++;
    }
    cout << cacheHits << "," << accesses << "; ";
    outfile << cacheHits << "," << accesses << "; ";
    infile.clear();
    infile.seekg(0);
    cacheHits = 0;
    accesses = 0 ;
  }
  cout << endl;
  outfile << endl;

  //Set associative cache
  int associativity[4] = {2, 4, 8, 16};
  for(int i = 0; i < 4; i++){ //CHANGE BACK TO 4
    int ways = associativity[i];
    int cacheLines = (16 * 1024) / 32; //change to 16
    int cacheLineSets = cacheLines / ways;
    int cache[cacheLineSets][ways] = {0, 0};
    int LRU[cacheLineSets][ways] = {0, 0};
    int indexBits = log2(cacheLineSets);
    int blockOffset = log2(32);
    unsigned int bitmask = (1 << indexBits) - 1;
    bool hit;
    while(infile >> instructionType >> std::hex >> addr){
      addr = addr >> blockOffset;
      int index = addr & bitmask;
      int tag = addr >> indexBits;
      hit = false;
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
    outfile << cacheHits << "," << accesses << "; ";
    infile.clear();
    infile.seekg(0);
    cacheHits = 0;
    accesses = 0 ;
  }
  cout << endl;
  outfile << endl;

  //Fully associative cache
  for(int i = 0; i < 2; i++){ //0 will represent LRU used, 1 will represent hot-cold approx used
    int cacheLines = (16 * 1024) / 32;  //CHANGE BACK TO 16
    int cache[cacheLines] = {0};
    int blockOffset = log2(32);
    int LRU[cacheLines] = {0};
    int HCLRUtree[cacheLines - 1] = {0};
    while(infile >> instructionType >> std::hex >> addr){
      accesses++;
      int tag = addr >> blockOffset;
      bool hit = false;
      //cout << "Tag is " << tag << endl;
      for(int j = 0; j < cacheLines; j++){
        if(cache[j] == tag){
          cacheHits++;
          hit = true;
          if(i){  //Hot cold approx used
            int treeNode = j + cacheLines - 1;         //Starting at the accessed cache line
            while(treeNode > 0){                      //Traversing up the tree and updating until we get to the root. once the treeNode is 0 it is the root and has no parent to update
              int parentNode = (treeNode - 1) / 2;  //We find the index of the parent node and it's left child. if the node isn't the left child, it's the right child
              int leftChild = (2 * parentNode) + 1;

              if(treeNode == leftChild){        //Here is the check for which child it is, and then we update the parent accordingly. we only need the l
                HCLRUtree[parentNode] = 1;
              }else{
                HCLRUtree[parentNode] = 0;
              }

              treeNode = parentNode;

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
          int treeNode = 0; //here we traverse DOWN the tree, so we start at the root

          while(treeNode < cacheLines - 1){
            if(HCLRUtree[treeNode]){       //if the current node is 1, move to the right child, if not, move left
              treeNode = 2 * treeNode + 2;
            }else{
              treeNode = 2 * treeNode + 1;
            }
          }
          LRUindex = treeNode - cacheLines + 1;  //Subtract the amount of non-leaf nodes to find the index in the cache

          cache[LRUindex] = tag;

          treeNode = LRUindex + cacheLines - 1;   //Same as before, we traverse up the tree from the index of the node we just changed
          while(treeNode > 0){                      
              int parentNode = (treeNode - 1) / 2;  
              int leftChild = 2 * parentNode + 1;
              if(treeNode == leftChild){
                HCLRUtree[parentNode] = 1;
              }else{
                HCLRUtree[parentNode] = 0;
              }

              treeNode  = parentNode;

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
              LRU[j]--;
            }
          }
          LRU[LRUindex] = cacheLines;     //Setting MRU
          cache[LRUindex] = tag;          //block eviction
        }
      }
      
    }
    cout << cacheHits << "," << accesses << "; ";
    outfile << cacheHits << "," << accesses << "; ";
    infile.clear();
    infile.seekg(0);
    cacheHits = 0;
    accesses = 0 ;
  }
  cout << endl;
  outfile << endl;

  //2-Level write through
  //=================================================================================
  //=================================================================================
  int L1cacheHits = 0;  //new variable names for clarity
  int L1accesses = 0;
  int L2cacheHits = 0;
  int L2accesses = 0;

  int L1cacheLines = (4 * 1024) / 32;
  int L1cacheLineSets = L1cacheLines / 4;
  int L1blockOffset = log2(32);
  int L1cache[L1cacheLineSets][4] = {0};
  int L1LRU[L1cacheLineSets][4] = {0};
  int L1indexBits = log2(L1cacheLineSets);
  unsigned int L1bitmask = (1 << L1indexBits) - 1;

  int L2cacheLines = (64 * 1024) / 64;
  int L2cacheLineSets = L2cacheLines / 8;
  int L2blockOffset = log2(64);
  int L2cache[L2cacheLineSets][8] = {0};
  int L2LRU[L2cacheLineSets][8] = {0};
  int L2indexBits = log2(L2cacheLineSets);
  unsigned int L2bitmask = (1 << L2indexBits) - 1;
  int loopbreaker = 0;
  while(infile >> instructionType >> std::hex >> addr){
    if(loopbreaker > 10){
      break;
    }
    //loopbreaker++;
    
    L1accesses++;
    unsigned long long L1addr = addr >> L1blockOffset;
    int L1index = L1addr & L1bitmask;
    int L1tag = L1addr >> L1indexBits;

    unsigned long long L2addr = addr >> L2blockOffset;
    int L2index = L2addr & L2bitmask;
    int L2tag = L2addr >> L2indexBits;
    
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
	
        if(instructionType == "S"){
	  bool L2hit = false;
          L2accesses++;
          for(int j = 0; j < 8; j++){
            if(L2cache[L2index][j] == L2tag){
	      L2hit = true;
              L2cacheHits++;
              for(int k = 0; k < 8; k++){
                if(j != k){
                  L2LRU[L2index][k]--;
                }
              }
            L2LRU[L2index][j] = 8;
            break;
            }
          }
	  if(!L2hit){ //since there was an L1 hit and L2 miss on store, we replace the block in L2
	    int L2LRUindex = 0;
	    for(int j = 0; j < 8; j++){//find lru and evict it
	      if(L2LRU[L2index][j] < L2LRU[L2index][L2LRUindex]){
		L2LRUindex = j;
	      }
	    }

	    L2cache[L2index][L2LRUindex] = L2tag;
	    L2LRU[L2index][L2LRUindex] = 8;

	    for(int j = 0; j < 8; j++){
	      if(j != L2LRUindex){
		L2LRU[L2index][j]--;
	      }
	    }
	  }
	  
        }
        break;
      }
    }

    if(!hit){         //first we check L2 cache for a hit before we replace the block in L1
      L2accesses++;
      
      for(int i = 0; i < 8; i++){
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


  //write back policy------------------------------------------------------------------
  //===================================================================================
  //===================================================================================



  int L2associativity[3] = {8, 1024, 1};//8 is for 8 way associative, 1024 is for fully associative and 1 is for direct mapped
  for(int ass = 0; ass < 3; ass++){//CHANGE TO 3
    //Re-initializing counters
    L1cacheHits = 0;
    L1accesses = 0;
    L2cacheHits = 0;
    L2accesses = 0;
    L2cacheLineSets = L2cacheLines / L2associativity[ass];
    L2indexBits = log2(L2cacheLineSets);
    L2bitmask = (1 << L2indexBits) - 1;
    //initializing the arrays
    int wbL1cache[L1cacheLineSets][4] = {0};
    int wbL1LRU[L1cacheLineSets][4] = {0};
    bool wbL1dirty[L1cacheLineSets][4] = {0};

    int wbL2cache[L2cacheLineSets][L2associativity[ass]] = {0};
    int wbL2LRU[L2cacheLineSets][L2associativity[ass]] = {0};
  
    while(infile >> instructionType >> std::hex >> addr){
    
      L1accesses++;
      unsigned long long L1addr = addr >> L1blockOffset;
      int L1index = L1addr & L1bitmask;
      int L1tag = L1addr >> L1indexBits;

      unsigned long long L2addr = addr >> L2blockOffset;
      int L2index = L2addr & L2bitmask;
      int L2tag = L2addr >> L2indexBits;

      bool L1hit = false;
      bool L2hit = false;

      for(int i = 0; i < 4; i++){
	if(wbL1cache[L1index][i] == L1tag){
	  L1cacheHits++;
	  L1hit = true;
	  for(int j = 0; j < 4; j++){
	    if(i != j){
	      wbL1LRU[L1index][j]--;
	    }
	  }
	  wbL1LRU[L1index][i] = 4;
	  if(instructionType == "S"){     //If the instruction is a store, we now mark the accessed block as dirty
	    wbL1dirty[L1index][i] = true;
	  }
	}
      }

      if(!L1hit){//L1 cache miss, so we access L2
      
	//Finding the L1LRU index first, because we'll need it to check if the block we're replacing is dirty
	int L1LRUindex = 0;
	int L2LRUindex = 0;
	for(int i = 0; i < 4; i++){
	  if(wbL1LRU[L1index][i] < wbL1LRU[L1index][L1LRUindex]){
	    L1LRUindex = i;
	  }
	}

      
	if(wbL1dirty[L1index][L1LRUindex]){ //if the block we are evicting is dirty, we need to write-back to L2 cache first
	  L2accesses++;
	  for(int i = 0; i < L2associativity[ass]; i++){
	    if(wbL2cache[L2index][i] == wbL1cache[L1index][L1LRUindex] >> 3){//if there is a cache hit for the block in L2, it is written back to L2 and the block in L2 is now dirty as well
	      L2hit = true;
	      L2cacheHits++;
	      L2LRUindex = i;
	    }
	  }

	  if(!L2hit){//If there is an L2 miss, the LRU is evicted and replaced with the dirty block from L1
	    L2LRUindex = 0;
	    for(int i = 0; i < L2associativity[ass]; i++){
	      if(wbL2LRU[L2index][i] < wbL2LRU[L2index][L2LRUindex]){
		L2LRUindex = i;
	      }
	    }	  
	    wbL2cache[L2index][L2LRUindex] = wbL1cache[L1index][L1LRUindex] >> 3; //shifted 3 bits, as the L1 tags are 22 bits long and the L2 tags are 19.
	    wbL2LRU[L2index][L2LRUindex] = L2associativity[ass];
	    for(int i = 0; i < L2associativity[ass]; i++){
	      if(i != L2LRUindex){
		wbL2LRU[L2index][i]--;
	      }
	    }	  
	  }else{//If it was in L2 hit, then we make the MRU the block that the dirty block was written back to
	    for(int i = 0; i < L2associativity[ass]; i++){
	      if(i != L2LRUindex){
		wbL2LRU[L2index][i]--;
	      }
	    }
	    wbL2LRU[L2index][L2LRUindex] = L2associativity[ass];
	  }
	
	  if(instructionType == "L"){
	    wbL1dirty[L1index][L1LRUindex] = false; //L1 dirty block is being evicted, if it was a load then the block is not dirty anymore
	  } 
	}

      
	//Now we've written it back, we can continue the L2 cache load
	L2hit = false;
	L2accesses++;
      
	for(int i = 0; i < L2associativity[ass]; i++){
	  if(wbL2cache[L2index][i] == L2tag){
	    L2cacheHits++;
	    L2hit = true;
	    for(int j = 0; j < L2associativity[ass]; j++){
	      if(i != j){
		wbL2LRU[L2index][j]--;
	      }
	    }
	    wbL2LRU[L2index][i] = L2associativity[ass];
	  }
	}
      
	if(!L2hit){
	  L2LRUindex = 0;
	  for(int i = 0; i < L2associativity[ass]; i++){
	    if(wbL2LRU[L2index][i] < wbL2LRU[L2index][L2LRUindex]){
	      L2LRUindex = i;
	    }
	  }
	  wbL2cache[L2index][L2LRUindex] = L2tag;

	  //now we update the LRU for L2
	  for(int i = 0; i < L2associativity[ass]; i++){
	    if(i != L2LRUindex){
	      wbL2LRU[L2index][i]--;
	    }
	  }
	  wbL2LRU[L2index][L2LRUindex] = L2associativity[ass];	
	}

	//Now we replace the block in the L1 cache and update the LRU
	wbL1cache[L1index][L1LRUindex] = L1tag;
	if(instructionType == "S"){
	  wbL1dirty[L1index][L1LRUindex] = true;
	}else{
	  wbL1dirty[L1index][L1LRUindex] = false;
	}
	for(int i = 0; i < 4; i++){
	  if(i != L1LRUindex){
	    wbL1LRU[L1index][i]--;
	  }
	}
	wbL1LRU[L1index][L1LRUindex] = 4;
      }   
    }

    float L2size = 0;
    float L2util = 0;
    for(int i = 0; i < L2cacheLineSets; i++){
      for(int j = 0; j < L2associativity[ass]; j++){
	if(wbL2cache[i][j] != 0){
	  L2util++;
	}
	L2size++;
      }
    }
    float L2fullutil = L2util / L2size;
    cout << L1cacheHits << "," << L1accesses << ";" << L2cacheHits << "," << L2accesses << "; " << L2fullutil << endl;
    outfile << L1cacheHits << "," << L1accesses << ";" << L2cacheHits << "," << L2accesses << "; " << L2fullutil << endl;
    infile.clear();
    infile.seekg(0);
  }
  return 0;
}
