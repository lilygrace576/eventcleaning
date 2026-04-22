#include <CalibrationFactor.h>


int main(int argc, char **argv){
    if(argc < 1){
		cout << "Too few arguments; please include the date YYYYMMDD data directory to summarize" << endl;
		return 1;
	}

    std::string mount = argv[2];
    
    if (mount == "y"){ // with usingin htcondor you need to have contianers and some use full paths and other use mounts this lets you specify
        std::cout << "using mounted directory path" << std::endl;
        mnt="/mnt/";
    } else if (mount != "n"){
        std::cout << "using specific directory path" << std::endl;
        mnt=mount.c_str();
    }


    dataDir = Form("%sDataAnalysis/MergedData/Output/",mnt.c_str());
    outDir = Form("%sDataAnalysis/flasher_calibration/Output/",mnt.c_str());

    std::string folString = argv[1];    //date

    // Load in all the files
    std::string FolderPath = Form("%s%s/",dataDir.c_str(),folString.c_str());   // MergedData/Output/DATE
    std::vector<std::string>fileNamesVec=util->GetFilesInDirectory(FolderPath,".root");
    cout << fileNamesVec[0] << endl;

    // Remove elements matching the pattern
    fileNamesVec.erase(
        std::remove_if(
            fileNamesVec.begin(),
            fileNamesVec.end(),
            [](const std::string& name) {
                return name.find("_0001.root") != std::string::npos;
            }
        ),
        fileNamesVec.end()
    );

    std::vector<double> TotalAmplitudeValues44(MaxNofChannels, 0.0);  // //MaxNoOfChannels = number of elements in vector (256), all starting at 0.0
    std::vector<double> AvgAmplitudeValuesPixels44(MaxNofChannels, 0.0);  // avg amps for hled
    int hled_event_counter44 = 0;
    std::vector<double> TotalAmplitudeValues415(MaxNofChannels, 0.0);  // //MaxNoOfChannels = number of elements in vector (256), all starting at 0.0
    std::vector<double> AvgAmplitudeValuesPixels415(MaxNofChannels, 0.0);  // avg amps for hled
    int hled_event_counter415 = 0;

    int test_event_counter44 = 0;
    int test_event_counter415 = 0;

    // Avg Amp Distrib
    // was 1500
    TH1F* h1 = new TH1F("h1", "Average Amplitude Distribution", 300, 0, 1000);  // Test 44
    h1->SetXTitle("Average Amplitude (ADC)");
    h1->SetYTitle("Counts");
    h1->GetXaxis()->SetTitleOffset(1.1); // Adjust X-axis title offset
    h1->GetYaxis()->SetTitleOffset(1.0); // Adjust Y-axis title offset
    TH1F* h4 = (TH1F*)h1->Clone("h4");  // HLED 44
    //
    TH1F* h5 = (TH1F*)h1->Clone("h5");  // Test 415
    TH1F* h6 = (TH1F*)h1->Clone("h6");  // HLED 415

    // RMS
    // was 100
    TH1F* h2 = new TH1F("h2", "Avg Amplitude RMS", 100, 0, 60);    //Test 44
    h2->SetXTitle("Pedestal RMS (ADC)");
    h2->SetYTitle("Counts");
    h2->GetXaxis()->SetTitleOffset(1.1); // Adjust X-axis title offset
    h2->GetYaxis()->SetTitleOffset(1.0); // Adjust Y-axis title offset
    TH1F* h3 = (TH1F*)h2->Clone("h3");  // HLED 44
    //
    TH1F* h7 = (TH1F*)h2->Clone("h7");  // Test 415
    TH1F* h8 = (TH1F*)h2->Clone("h8");  // HLED 415

    for(int f = 0; f<static_cast<int>(fileNamesVec.size()); f++){
        try {
            std::string FilePath = Form("%s%s",FolderPath.c_str(),fileNamesVec[f].c_str());
            if (!util->isBranchPresentInFile(FilePath, "Test")) {
                continue; // Skip to the next branch if not present
            }

            LoadEvents(FilePath, "Test");
            LoadEventsHLED(FilePath, "HLED");
            ev = new IEvent();
            evHLED = new IEvent();
            int nEntries = 0;
            int nEntriesHLED = 0;
            
            SetBranches(ev);
            SetBranchesHLED(evHLED);
            nEntries = tree->GetEntries();
            nEntriesHLED = treeHLED->GetEntries();


            if (nEntries==0) {
                cout << "continue" << endl;
                continue;
            }   

            for(int EventCounter = 0; EventCounter < (nEntries+nEntriesHLED); EventCounter++){

                TH2F* hEvent = new TH2F("hEvent", "Event",16, -0.5, 15.5, 16, -0.5, 15.5);
                std::vector<float> BiasVoltage; // BV Vector for event

                BiasVoltage = ev->Gethv();  // get all bv values for this event (4)
                float evSumV = accumulate(BiasVoltage.begin(), BiasVoltage.end(), 0.0); // sum bv values
                float evBVAvg = evSumV / BiasVoltage.size();    // avg bv values
                float evRoundBVAvg = round(10 * evBVAvg) / 10; // round event bv avg
                
                std::vector<float> Amplitudes44;    // Amp vector for 44 V events
                std::vector<float> Amplitudes415;   // Amp vector for 41.5 V events

                if(evRoundBVAvg == 44.0){
                    Pulse *pulse; 
                    if (EventCounter < nEntries) {  // Test Events
                        std::string WhichEvent = "Test";
                        tree->GetEntry(EventCounter);
                        for (int k = 0; k < MaxNofChannels; k++) {  //for pixel
                            pulse = new Pulse(ev->GetSignalValue(k));
                            Amplitudes44.push_back(pulse->GetAmplitude());
                            delete pulse;
                        }
                    }
                    else{
                        treeHLED->GetEntry(EventCounter-nEntries);
                        for (int k = 0; k < MaxNofChannels; k++) { // for pixel
                            pulse = new Pulse(evHLED->GetSignalValue(k)); 
                    }
                    }
                }
                if(evRoundBVAvg == 41.5){

                }
               