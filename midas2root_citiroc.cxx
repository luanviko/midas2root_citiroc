/** Program based on Rootana to convert Midas into 
ROOT events. Inputs a .mid file and ouputs a .root file, 
containing a tree called "waveform_tree."
There are up to 8 trees to contain the waveforms 
of each the digitizer's channels. 

Originally created by Thomas Lindner for the Big Read Board's ADCs.
Adapted by Luan Koerich for the CAEN-DT5743 digitizer, February 2021.

*/
#include <stdio.h>
#include <iostream>
#include "TRootanaEventLoop.hxx"
#include "TH1F.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TTreeMaker.h"
#include <TTree.h>
#include <TFile.h>
#include "TDT743RawData.hxx"


#define TRUE 1
#define FALSE 0



#define nPoints_max 15000
#define num_phidg_max 10000
#define num_dt5743_max 1024 // IMPOTANT: IF THIS IS EVER CHANGED, ALSO CHANGE THE HARDCODED VALUES FOR WAVEFORM BRANCH WIDTHS AS WELL (see: "v1730 data")

//Digitizer variables
double Start_time0[nPoints_max], Window_width0[nPoints_max], Start_time1[nPoints_max], Window_width1[nPoints_max], start_0, start_1;

// Digitizer channels
double fifoLG[nPoints_max], fifoHG[nPoints_max];
double dt5743_wave0[nPoints_max][num_dt5743_max], dt5743_wave1[nPoints_max][num_dt5743_max], dt5743_wave2[nPoints_max][num_dt5743_max], dt5743_wave3[nPoints_max][num_dt5743_max];
double dt5743_wave4[nPoints_max][num_dt5743_max], dt5743_wave5[nPoints_max][num_dt5743_max], dt5743_wave6[nPoints_max][num_dt5743_max], dt5743_wave7[nPoints_max][num_dt5743_max];

//PMT readout
int HGsize;
int LGsize;
int start_val_stat;
int window_width;
int trigger;
int counter;
int num_points;
int num_points_dig0;
int timeStamp;

#define timeStart 130 // defines start of PMT Pulse timing window, currently at the 130th sample of 200, with a window size of 70 samples.

// Offset for the ADC channel number
#define Ch_Offset 1

// 
class ScanToTreeConverter: public TRootanaEventLoop {

    int nnn;
    TH1F *SampleWave0;

    TH1F *StartVal0;

    private:
        
        // Number of channels to be saved
        int fNChan;

        //TFile *outputfile; //made by TRootAnaEventLoop with name of inputfile +.root
        TTree *tree;

        // Counters for TDC bank
        int ngoodTDCbanks;
        int nbadTDCbanks;

    public:

        ScanToTreeConverter() {
            
            // Disable ROOTANA graphical mode.
            UseBatchMode();
            nnn = 0;
            fNChan = 8; // < Saving waveforms from 0 to 7
        };

    virtual ~ScanToTreeConverter() {};

    void BeginRun(int transition,int run,int time){
    
        // Start conversion
        std::cout << "Custom: begin run " << run << std::endl;

        // A tree to contain the waveform information
        tree = new TTree("fifo_tree","FIFO Tree");
        tree->Branch("timeStamp",&timeStamp,"timeStamp/I");
        tree->Branch("LGsize",&LGsize,"LGsize/I");
        tree->Branch("HGsize",&HGsize,"HGsize/I");
        tree->Branch("fifoLG",fifoLG,"fifoLG[LGsize]/D");
        tree->Branch("fifoHG",fifoHG,"fifoHG[HGsize]/D");

    }

    void EndRun(int transition,int run,int time){
        // Print out summary.
        std::cout << "End of conversion." << run <<std::endl;
        std::cout << "Good TDC banks : " << ngoodTDCbanks << std::endl;;
        std::cout << "Bad TDC banks  : " << nbadTDCbanks << std::endl;;
    }

    bool ProcessMidasEvent(TDataContainer& dataContainer){

        TGenericData *dataLG = dataContainer.GetEventData<TGenericData>("1ALG");
        TGenericData *dataHG = dataContainer.GetEventData<TGenericData>("1AHG");

        const char* banks = dataContainer.GetMidasEvent().GetBankList();
        std::cout << "\nBANKS: " << banks << std::endl;
        std::cout << "TIME STAMP: " << dataContainer.GetMidasEvent().GetTimeStamp() << std::endl;

        if ((dataLG) && (dataHG)) {
        // if ((dataLG) && (dataHG)) {
 
            timeStamp  = dataContainer.GetMidasEvent().GetTimeStamp(); 

            // HGsize = dataHG->GetSize();
            LGsize = dataLG->GetSize();
            HGsize = dataHG->GetSize();

            std::cout << "\nFound LG and HG banks." << std::endl;
            std::cout << "LG Bank size: " << LGsize << std::endl;
            std::cout << "HG Bank size: " << HGsize << std::endl;


            for (int j = 0; j < LGsize; j++){ 
                fifoLG[j] = (double)dataLG->GetData32()[j]; 
                // std::cout << "Data type: " << dataLG->GetType() << std::endl;
                std::cout << "Data LG["<<j<<"] 32: " << (double)dataLG->GetData32()[j] << std::endl;
                // std::cout << "Data LG["<<j<<"] 64: " << dataLG->GetData64()[j] << std::endl;
                // std::cout << "Data LG["<<j<<"] Double: " << dataLG->GetDouble()[j] << std::endl;
                // std::cout << "Data LG["<<j<<"] Char: " << dataLG->GetChar()[j] << std::endl;
                // printf("Data char %x:\n", dataLG->GetChar()[j]);
                // std::cout << "Data LG["<<j<<"]: " << dataLG->GetData64()[j] << std::endl;
            }
            
            for (int j = 0; j < HGsize; j++){
                fifoHG[j] = (double)dataHG->GetData32()[j];  
                std::cout << "Data HG["<<j<<"]: " << dataHG->GetData32()[j] << std::endl;
            }

            tree->Fill();
        }
        

        return true;
    }


    // Describe some other command line argument
    void Usage(void){
        std::cout << "\t-nchan option: specify how many channels of digitizer to save " << std::endl;
    }

    // Define some other command line argument
    bool CheckOption(std::string option){
    const char* arg = option.c_str();
    
    if (strncmp(arg,"-nchan",2)==0){
        fNChan = atoi(arg+6);
        std::cout << "Number of channels to save: " << fNChan << std::endl;
        return true;
    }

    return false;
    }
}; 

// Main function to initiliaze converter
int main(int argc, char *argv[]){

    // Test arguments
    if (argc != 2) {
        std::cerr<<"Usage: midas2root_citiroc.exe run.mid.lz4"<<std::endl;
        exit(0);
    }

    ScanToTreeConverter::CreateSingleton<ScanToTreeConverter>();
    return ScanToTreeConverter::Get().ExecuteLoop(argc, argv);
}

