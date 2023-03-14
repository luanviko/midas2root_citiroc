#include "TTreeMaker.h"



TTree* MakeTree(){

    TTree *tree = new TTree("waveform_tree","Waveform Tree");

 
    return tree;
    
}
