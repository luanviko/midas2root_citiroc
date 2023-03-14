#include "TDT743RawData.hxx"

#include <iostream>



TDT743RawData::TDT743RawData(int bklen, int bktype, const char* name, void *pdata):
    TGenericData(bklen, bktype, name, pdata)
{
  
  fGlobalHeader.push_back(GetData32()[2]);
  fGlobalHeader.push_back(GetData32()[3]);
  fGlobalHeader.push_back(GetData32()[4]);
  fGlobalHeader.push_back(GetData32()[5]);
  fGlobalHeader.push_back(GetData32()[0]);
  fGlobalHeader.push_back(GetData32()[1]);
  
  // Do some sanity checking.  
  // Make sure first word has right identifier
  if( (GetData32()[2] & 0xf0000000) != 0xa0000000) 
    std::cerr << "First word has wrong identifier; first word = 0x" 
	      << std::hex << GetData32()[0] << std::dec << std::endl;
  
  int counter = 6;
  
  int number_available_groups = 0;
  for(int ch = 0; ch < 16; ch++){		
    if((1<<ch) & GetChMask()){
      number_available_groups++;
    }
  }
  
  int nwords_per_group = (GetEventSize() - 4)/number_available_groups;
  if(0)std::cout << "Words per group : " << nwords_per_group << " " 
	    << number_available_groups << std::endl;

  // Loop over groups
  for(int gr = 0; gr < number_available_groups; gr++){
    
    if((GetData32()[counter] & 0xff000000) != 0xaa000000){
      std::cout << "Bad first word for group: 0x" << std::hex << GetData32()[counter] << std::dec << std::endl;
    }

    if((1<<gr) & GetChMask()){
      
      std::vector<uint32_t> Samples0;
      std::vector<uint32_t> Samples1;
      int time0=0, time1=0, hit0=0, hit1=0, freq=0;
      
      for(int i = 0; i < nwords_per_group; i++){
	// Get extra info
	if(i == 1) hit0  += (GetData32()[counter] & 0xff000000) >> 24;
	if(i == 2) hit0  += (GetData32()[counter] & 0xff000000) >> 16;
	if(i == 3) time0 += (GetData32()[counter] & 0xff000000) >> 24;
	if(i == 4) time0 += (GetData32()[counter] & 0xff000000) >> 16;
	if(i == 5) hit1  += (GetData32()[counter] & 0xff000000) >> 24;
	if(i == 6) hit1  += (GetData32()[counter] & 0xff000000) >> 16;
	if(i == 7) time1 += (GetData32()[counter] & 0xff000000) >> 24;
	if(i == 8) time1 += (GetData32()[counter] & 0xff000000) >> 16;
	if(i == 9) freq  = (GetData32()[counter] & 0xff000000) >> 24;
      
	// Get Samples (skip 17th sample)
	if(i%17 != 0){
	  uint32_t sample = (GetData32()[counter] & 0xfff);
	  Samples0.push_back(sample);
	  sample = (GetData32()[counter] & 0xfff000) >> 12;
	  Samples1.push_back(sample);				
	}

	// increment pointer
	counter++;
      }
      RawChannelMeasurement meas0 = RawChannelMeasurement(gr*2);
      meas0.AddSamples(Samples0);
      meas0.SetFrequency(freq);
      meas0.SetHitCounter(hit0);
      meas0.SetTimeCounter(time0);
      fMeasurements.push_back(meas0);

      RawChannelMeasurement meas1 = RawChannelMeasurement(gr*2+1);
      meas1.AddSamples(Samples1);
      fMeasurements.push_back(meas1);
      
      if(0)std::cout << freq << " " << hit0 << " " << hit1 
		     << " " << time0 << " " << time1 << std::endl;
      //for(int i = 0; i < Samples0.size(); i++) std::cout << std::hex << Samples0[i] <<std::dec << std::endl; 
    }
  }

}

void TDT743RawData::Print(){

  std::cout << "DT743 decoder for bank " << GetName().c_str() << std::endl;


}
