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
        dataDir = "/mnt/DataAnalysis/MergedData/Output/";
        outDir = "/mnt/DataAnalysis/flasher_calibration/Output/";
    }

    std::string folString = argv[1];

    // Load in all the files
    std::string FolderPath = Form("%s%s/",dataDir.c_str(),folString.c_str());
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

    std::vector<double> TotalAmplitudeValues(MaxNofChannels, 0.0);
    std::vector<double> AvgAmplitudeValuesPixels(MaxNofChannels, 0.0);
    int hled_event_counter = 0;

    for(int f = 0; f<static_cast<int>(fileNamesVec.size()); f++){
    // for(int f = 45; f<60; f++){ // use for testing with a few files
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
                continue;
            }   

            for(int EventCounter = 0; EventCounter < (nEntries+nEntriesHLED); EventCounter++){
                TH2F* hEvent = new TH2F("hEvent", "Event",16, -0.5, 15.5, 16, -0.5, 15.5);
                std::vector<float> Amplitudes;
                
                std::string WhichEvent = "Test";
                // this allow you to have all the info you need from the event in one for loop so you dont have to have one for HLED and one for Test since the events can sometimes cross over.
                Pulse *pulse; 
                if (EventCounter < nEntries) {
                    tree->GetEntry(EventCounter);      
                    for (int k = 0; k < MaxNofChannels; k++) {
                        pulse = new Pulse(ev->GetSignalValue(k));
                        // AmplitudesTest.push_back(pulse->GetAmplitude());
                        Amplitudes.push_back(pulse->GetAmplitude());        
                        delete pulse;
                    }
                    
                } else {
                    treeHLED->GetEntry(EventCounter-nEntries);
                    for (int k = 0; k < MaxNofChannels; k++) {
                        pulse = new Pulse(evHLED->GetSignalValue(k));
                        Amplitudes.push_back(pulse->GetAmplitude());
                        // PedestalRMSHLED.push_back(pulse->GetPedestalRMS()); 
                        WhichEvent = "HLED";
                        delete pulse;
                    }
                }


                // cout << std::accumulate(PedestalRMS.begin(), PedestalRMS.end(), 0.0) / MaxNofChannels << endl;
                delete hEvent;
                if (folString < "20241001"){
                    FlasherEventsCutOff = 350;
                }
                if (folString >= "20251017"){
                    FlasherEventsCutOff = 835;
                }
                if (util->GetEventAmplitudeSum(Amplitudes)/MaxNofChannels >= FlasherEventsCutOff){    
                                
                    hled_event_counter +=1;
                    for(int j = 0; j<MaxNofChannels; j++){
                        TotalAmplitudeValues[j] = TotalAmplitudeValues[j]+Amplitudes[j];
                    }
                }
            }
            delete evHLED;
            delete ev;
            delete fO;
            delete tree;
            delete treeHLED;
        } catch (const std::exception& e){
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }
    }


    TCanvas* c_cleaned = new TCanvas("CleanedDisplay", "Cleaned CameraPlot", 950, 1000);
    TH2F* hcam_calib = new TH2F("hcam_calib", "Calibration factor from Flasher",16, -0.5, 15.5, 16, -0.5, 15.5);
    
    std::vector<double> AvgAmplitudeValuesPixelsMedian;
    double dead_pixel_cutoff = 750;
    if (folString < "20241001"){
        dead_pixel_cutoff = 250;
    }
    if (folString >= "20251017"){
        dead_pixel_cutoff = 700;
    }
    for(int j = 0; j<MaxNofChannels; j++){
        AvgAmplitudeValuesPixels[j] = TotalAmplitudeValues[j]/hled_event_counter;
        if (AvgAmplitudeValuesPixels[j] < dead_pixel_cutoff){
            AvgAmplitudeValuesPixels[j] = -1;
        } else {
            AvgAmplitudeValuesPixelsMedian.push_back(TotalAmplitudeValues[j]/hled_event_counter);
        }
    }


    // std::vector<double> CalibratedAmpCameraTimeBin(MaxNofChannels, 0.0);
    Double_t median = util->Median(AvgAmplitudeValuesPixelsMedian);
    double minimum = 3; // for the canvas plot making the visualization better
    for(int j = 0; j<MaxNofChannels; j++){
        int nx, ny;
        plottools->FindBin(j, &nx, &ny);
        double norm_median = AvgAmplitudeValuesPixels[j]/median;
        // keep the value -1 for bad pixels
        if (AvgAmplitudeValuesPixels[j] == -1){
            norm_median = -1;
        }
        // CalibratedAmpCameraTimeBin[j]=norm_median;
        hcam_calib->SetBinContent(nx + 1, ny + 1, norm_median);
        cout << "Pixel: " << j << " Norm Median: " << norm_median << endl;
        if (norm_median < minimum && norm_median > 0.1){
            // cout << "Avg Amplitude for pixel " << j << ": " << norm_median << endl;
            minimum = norm_median;
        }
    }
    // for if you need a csv but histograms are so much better 
    // util->writeVectorToFile(CalibratedAmpCameraTimeBin, Form("%s%s_FlasherCalibration_Factor.csv", outDir.c_str(), folString.c_str()));

    hcam_calib->SetStats(0);
    hcam_calib->GetXaxis()->SetLabelSize(0.03);
    hcam_calib->GetYaxis()->SetLabelSize(0.03);
    hcam_calib->SetXTitle("Pixel Bin along x axis of Camera");
    hcam_calib->SetYTitle("Pixel Bin along y axis of Camera");
    hcam_calib->GetXaxis()->SetTitleOffset(1.1); // Adjust X-axis title offset
    hcam_calib->GetYaxis()->SetTitleOffset(1.0); // Adjust Y-axis title offset
    hcam_calib->Draw("colz");
    plottools->DrawMUSICBoundaries();
    // cout << "Minimum: " << minimum << endl;
    
    file = new TFile(Form("%s%s_FlasherCalibration_Factor.root", outDir.c_str(), folString.c_str()), "RECREATE");  // "RECREATE" to overwrite if it exists
    hcam_calib->Write("CamFlasher");
    hcam_calib->SetMinimum(minimum-0.01);
    c_cleaned->Write("CanvasDisplayFlasherCalibration");

    file->Close();
    util->setFilePermissions(Form("%s%s_FlasherCalibration_Factor.root", outDir.c_str(), folString.c_str()));

    return 0;
}




