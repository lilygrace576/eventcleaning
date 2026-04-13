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

    std::vector<double> TotalAmplitudeValues(MaxNofChannels, 0.0);  // //MaxNoOfChannels = number of elements in vector (256), all starting at 0.0
    std::vector<double> AvgAmplitudeValuesPixels(MaxNofChannels, 0.0);  // avg amps for hled
    int hled_event_counter = 0;

    // Avg Amp Distrib
    TH1F* h1 = new TH1F("h1", "Average Amplitude Distribution", 300, 0, 1500);  // Test 44
    h1->SetXTitle("Average Amplitude (ADC)");
    h1->SetYTitle("Counts");
    h1->GetXaxis()->SetTitleOffset(1.1); // Adjust X-axis title offset
    h1->GetYaxis()->SetTitleOffset(1.0); // Adjust Y-axis title offset
    TH1F* h4 = (TH1F*)h1->Clone("h4");  // HLED 44
    //
    TH1F* h5 = (TH1F*)h1->Clone("h5");  // Test 415
    TH1F* h6 = (TH1F*)h1->Clone("h6");  // HLED 415

    // RMS
    TH1F* h2 = new TH1F("h2", "Avg Amplitude RMS", 100, 0, 100);    //Test 44
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
                continue;
            }   

            for(int EventCounter = 0; EventCounter < (nEntries+nEntriesHLED); EventCounter++){
                TH2F* hEvent = new TH2F("hEvent", "Event",16, -0.5, 15.5, 16, -0.5, 15.5);
                //// std::vector<float> Amplitudes;
                std::vector<float> Amplitudes44;    // Amp vector for 44 V events
                std::vector<float> Amplitudes415;   // Amp vector for 41.5 V events

                std::vector<float> BiasVoltage; // BV Vector for event
                
                std::string WhichEvent = "Test";

                // this allow you to have all the info you need from the event in one for loop so you dont have to have one for HLED and one for Test since the events can sometimes cross over.
                Pulse *pulse; 
                if (EventCounter < nEntries) {  // Test Events

                    tree->GetEntry(EventCounter); 

                    BiasVoltage = ev->Gethv();  // get all bv values for this event (4)
                    float evSumV = accumulate(BiasVoltage.begin(), BiasVoltage.end(), 0.0); // sum bv values
		            float evBVAvg = evSumV / BiasVoltage.size();    // avg bv values
                    float evRoundBVAvg = round(10 * evBVAvg) / 10; // round event bv avg

                    for (int k = 0; k < MaxNofChannels; k++) {  //for pixel
                        pulse = new Pulse(ev->GetSignalValue(k));   // get pulse signal

                        // Amplitudes.push_back(pulse->GetAmplitude());
                        // check if 44 or 41.5 V event
                        if (evRoundBVAvg == 44.0){
                            Amplitudes44.push_back(pulse->GetAmplitude());  // add event amplitude to 44 V event amp vector
                        }
                        if (evRoundBVAvg == 41.5){
                            Amplitudes415.push_back(pulse->GetAmplitude()); // add event amplitude to 41.5 V event amp vector
                        }
                        delete pulse;
                    }
                    
                }   // end if test 
                else {  // HLED events
                    
                    treeHLED->GetEntry(EventCounter-nEntries);
                    
                    BiasVoltage = evHLED->Gethv();  // get all bv values for this event (4)
                    float evSumV = accumulate(BiasVoltage.begin(), BiasVoltage.end(), 0.0); // sum bv values
		            float evBVAvg = evSumV / BiasVoltage.size();    // avg bv values
                    float evRoundBVAvg = round(10 * evBVAvg) / 10;  // round event bv avg

                    for (int k = 0; k < MaxNofChannels; k++) { // for pixel
                        pulse = new Pulse(evHLED->GetSignalValue(k));   // get pulse signal
                        
                        // Amplitudes.push_back(pulse->GetAmplitude());
                        // check if 44 or 41.5 V event
                        if (evRoundBVAvg == 44.0){
                            Amplitudes44.push_back(pulse->GetAmplitude());  // add event amplitude to 44 V event amp vector
                        }
                        if (evRoundBVAvg == 41.5){
                            Amplitudes415.push_back(pulse->GetAmplitude()); // add event amplitude to 41.5 V event amp vector
                        }

                        
                        WhichEvent = "HLED";
                        delete pulse;
                    }
                }   // end if hled

                // float sumsq = 0;
                float sumsq44 = 0;
                float sumsq415 = 0;

                // float RMS = 0;
                float RMS44 = 0;
                float RMS415 = 0;
                
                for (int i = 0; i < MaxNofChannels; i++){   // for pixel
                    // sumsq += Amplitudes[i]*Amplitudes[i];    // add squared amp entry value to sumsQ
                    sumsq44 += Amplitudes44[i]*Amplitudes44[i];
                    sumsq415 += Amplitudes415[i]*Amplitudes415[i];

                    // RMS = sqrt(sumsq)/MaxNofChannels;
                    RMS44 = sqrt(sumsq44)/MaxNofChannels;
                    RMS415 = sqrt(sumsq415)/MaxNofChannels;
                }

                if (WhichEvent == "Test"){
                    // h1->Fill(util->GetEventAmplitudeSum(Amplitudes)/MaxNofChannels);
                    h1->Fill(util->GetEventAmplitudeSum(Amplitudes44)/MaxNofChannels);  // fill h1 with avg amp for 44 V Test events
                    h5->Fill(util->GetEventAmplitudeSum(Amplitudes415)/MaxNofChannels); // fill h5 with avg amp of 41.5 V Test events

                    // h2->Fill(RMS);
                    h2->Fill(RMS44);    // fill h2 with 44 V Test event RMS
                    h7->Fill(RMS415);   // fill h7 with 41.5 V Test event RMS
                    
                } else {
                    // h4->Fill(util->GetEventAmplitudeSum(Amplitudes)/MaxNofChannels);
                    h4->Fill(util->GetEventAmplitudeSum(Amplitudes44)/MaxNofChannels);  // fill h4 with avg amp for 44 V HLED events
                    h6->Fill(util->GetEventAmplitudeSum(Amplitudes415)/MaxNofChannels); // fill h6 with avg amp for 41.5 V HLED events

                    // h3->Fill(RMS);
                    h3->Fill(RMS44);    // fill h3 with 44 V HLED event RMS
                    h8->Fill(RMS415);   // fill h8 with 41.5 V HLED event RMS
                }

                delete hEvent;
                
                // // old
                // if (folString < "20241001"){
                //     FlasherEventsCutOff = 350;
                // }

    ////////////////////////////////////////////////////////////////////////////////////
                if (folString >= "20251017"){   // if after 2025/10/17
                    // FlasherEventsCutOff = 835;
                    FlasherEventsCutOff44 = 835;
                    FlasherEventsCutOff415 = 200;   // ??
                }
                if (folString >= "20251204"){   // if after 2025/12/04
                    // FlasherEventsCutOff = 700;
                    FlasherEventsCutOff44 = 700;
                    FlasherEventsCutOff415 = 200;   // ??
                }

                // if (util->GetEventAmplitudeSum(Amplitudes)/MaxNofChannels >= FlasherEventsCutOff){    
                //     hled_event_counter +=1;
                //     for(int j = 0; j<MaxNofChannels; j++){
                //         TotalAmplitudeValues[j] = TotalAmplitudeValues[j]+Amplitudes[j];
                //     }
                // }
                if (util->GetEventAmplitudeSum(Amplitudes44)/MaxNofChannels >= FlasherEventsCutOff44){    // if avg 44 V event amp > 44 cutoff -> HLED
                    hled_event_counter +=1;
                    for(int j = 0; j<MaxNofChannels; j++){  //for pixel
                        // TAV[pixel 0-256] = TAV[pixel 0-256] + Amp44[pixel 0-256]
                        // add pixel amp from each event to that pixel TAV ->sum of amps for that pixel from each HLED event
                        TotalAmplitudeValues[j] = TotalAmplitudeValues[j]+Amplitudes44[j];
                    }
                }
                if (util->GetEventAmplitudeSum(Amplitudes415)/MaxNofChannels >= FlasherEventsCutOff415){    // if avg 41.5 V event amp > 44 cutoff -> HLED
                    hled_event_counter +=1;
                    for(int j = 0; j<MaxNofChannels; j++){  //for pixel
                        // add pixel amp from each event to that pixel TAV -> sum of amps for that pixel from each HLED event
                        TotalAmplitudeValues[j] = TotalAmplitudeValues[j]+Amplitudes415[j];
                    }
                }
                // TAV[j] = amplitude sum for each pixel from each HLED event regardless of BV
    ////////////////////////////////////////////////////////////////////////////////////

            }   // end event loop

            delete evHLED;
            delete ev;
            delete fO;
            delete tree;
            delete treeHLED;
        }   // end try
        catch (const std::exception& e){
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }
    }   // end file loop


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
    if (folString >= "20251204"){
        dead_pixel_cutoff = 700;
    }

    for(int j = 0; j<MaxNofChannels; j++){
        // pixel AAVP = hled pixel amp sum / total num of HLED events = average amp for that pixel
        AvgAmplitudeValuesPixels[j] = TotalAmplitudeValues[j]/hled_event_counter;
        if (AvgAmplitudeValuesPixels[j] < dead_pixel_cutoff){   // if pixel AAVP < dead pixel cutoff -> pixel AAVP = -1
            AvgAmplitudeValuesPixels[j] = -1;
        } else {    // if pixel AAVP >= dead pixel cutoff -> add pixel TAV/total num of HLED events (AAVP) to AAVPM vector
            AvgAmplitudeValuesPixelsMedian.push_back(TotalAmplitudeValues[j]/hled_event_counter);
            // AAVPM - contains average amp values for each pixel
        }
    }


    Double_t median = util->Median(AvgAmplitudeValuesPixelsMedian); // find median of average amp values
    double minimum = 3; // for the canvas plot making the visualization better
    for(int j = 0; j<MaxNofChannels; j++){  // for pixel
        int nx, ny;
        plottools->FindBin(j, &nx, &ny);
        double norm_median = AvgAmplitudeValuesPixels[j]/median;    // normalized median = pixel avg amp / median
        // keep the value -1 for bad pixels
        if (AvgAmplitudeValuesPixels[j] == -1){
            norm_median = -1;
        }
        hcam_calib->SetBinContent(nx + 1, ny + 1, norm_median);
        cout << "Pixel: " << j << " Norm Median: " << norm_median << endl;
        if (norm_median < minimum && norm_median > 0.1){
            minimum = norm_median;
        }
    }

    hcam_calib->SetStats(0);
    hcam_calib->GetXaxis()->SetLabelSize(0.03);
    hcam_calib->GetYaxis()->SetLabelSize(0.03);
    hcam_calib->SetXTitle("Pixel Bin along x axis of Camera");
    hcam_calib->SetYTitle("Pixel Bin along y axis of Camera");
    hcam_calib->GetXaxis()->SetTitleOffset(1.1); // Adjust X-axis title offset
    hcam_calib->GetYaxis()->SetTitleOffset(1.0); // Adjust Y-axis title offset
    hcam_calib->Draw("colz");
    plottools->DrawMUSICBoundaries();
    
    file = new TFile(Form("%s%s_FlasherCalibration_Factor.root", outDir.c_str(), folString.c_str()), "RECREATE");  // "RECREATE" to overwrite if it exists
    hcam_calib->Write("CamFlasher");
    // h1->Write("AvgAmpDistributionTest");
    h1->Write("AvgAmpDistributionTest44");
    h5->Write("AvgAmpDistributionTest415");
    // h4->Write("AvgAmpDistributionHLED");
    h4->Write("AvgAmpDistributionHLED44");
    h6->Write("AvgAmpDistributionHLED415");
    // h2->Write("RMSTestDistribution");
    h2->Write("RMSTestDistribution44");
    h7->Write("RMSTestDistribution415");
    // h3->Write("RMSHLEDDistribution");
    h3->Write("RMSHLEDDistribution44");
    h8->Write("RMSHLEDDistribution415");
    hcam_calib->SetMinimum(minimum-0.01);
    c_cleaned->Write("CanvasDisplayFlasherCalibration");

    c_cleaned->cd(0);
   


    // Avg Amp Test Events Plot
    h1->SetLineColor(kBlue);    // h1 - Test 44 Avg Amp
    h5->SetLineColor(kViolet);  // h5 - Test 41.5 Avg Amp
    h5->Draw("HIST");
    h1->Draw("SAME");
    TLegend* leg = new TLegend(0.6,0.7,0.9,0.9);
    leg->AddEntry(h1, "Avg Amplitude Distribution Test 44V", "l");
    leg->AddEntry(h5, "Avg Amplitude Distribution Test 41.5V", "l");
    leg->Draw("SAME");
    c_cleaned->Write("TestAvgAmp");
    delete leg;

    c_cleaned->cd(0);

    // Avg Amp HLED Events Plot
    h4->SetLineColor(kRed); // h4 - HLED 44 Avg Amp
    h6->SetLineColor(kOrange);  // h6 - HLED 41.5 Avg Amp  
    h6->Draw("HIST");
    h4->Draw("SAME");
    leg = new TLegend(0.6,0.7,0.9,0.9);
    leg->AddEntry(h4, "Avg Amplitude Distribution hled 44V", "l");
    leg->AddEntry(h6, "Avg Amplitude Distribution hled 41.5V", "l");
    leg->Draw("SAME");
    c_cleaned->Write("HLEDAvgAmp");
    delete leg;

    c_cleaned->cd(0);
    
    // RMS Test Events Plot
    h2->SetLineColor(kBlue);    // h2 - Test 44 RMS
    h7->SetLineColor(kViolet);  // h7 - Test 41.5 RMS
    h7->Draw("HIST");
    h2->Draw("SAME");
    leg = new TLegend(0.6,0.7,0.9,0.9);
    leg->AddEntry(h2, "Avg Amplitude RMS Distribution Test 44", "l");
    leg->AddEntry(h7, "Avg Amplitude RMS Distribution Test 415", "l");
    leg->Draw("SAME");
    c_cleaned->Write("TestAvgAmplitudeRMS");
    delete leg;

    c_cleaned->cd(0);

    // RMS HLED Events Plot
    h3->SetLineColor(kRed); // h3 - HLED 44 RMS
    h8->SetLineColor(kOrange);  // h8 - HLED 41.5 RMS
    h8->Draw("HIST");
    h3->Draw("SAME");
    leg = new TLegend(0.6,0.7,0.9,0.9);
    leg->AddEntry(h3, "Avg Amplitude RMS Distribution hled 44", "l");
    leg->AddEntry(h8, "Avg Amplitude RMS Distribution hled 415", "l");
    leg->Draw("SAME");
    c_cleaned->Write("HLEDAvgAmplitudeRMS");
    delete leg;

    c_cleaned->cd(0);

    // Avg Amps ALL
    h1->SetLineColor(kBlue);    // h1 - Test 44 Avg Amp
    h5->SetLineColor(kViolet);  // h5 - Test 415 Avg Amp
    h4->SetLineColor(kRed);     // h4 - HLED 44 Avg Amp
    h6->SetLineColor(kOrange);  // h6 - HLED 415 Avg Amp
    h5->Draw("HIST");          // first histogram, sets the frame (idk which to put)
    h1->Draw("HIST SAME");
    h4->Draw("HIST SAME");
    h6->Draw("HIST SAME");
    leg = new TLegend(0.6, 0.65, 0.9, 0.9);
    leg->AddEntry(h1, "Test 44V", "l");
    leg->AddEntry(h5, "Test 41.5V", "l");
    leg->AddEntry(h4, "HLED 44V", "l");
    leg->AddEntry(h6, "HLED 41.5V", "l");
    leg->Draw("SAME");
    c_cleaned->Write("CombinedAvgAmp");
    delete leg;

    c_cleaned->cd(0);
    

    // RMS All
    h2->SetLineColor(kBlue);    // h2 - Test 44 RMS
    h7->SetLineColor(kViolet);  // h7 - Test 41.5 RMS
    h3->SetLineColor(kRed);     // h3 - HLED 44 RMS
    h8->SetLineColor(kOrange);  // h8 - HLED 41.5 RMS
    h7->Draw("HIST");          // first histogram, sets the frame (idk which to put)
    h2->Draw("HIST SAME");
    h3->Draw("HIST SAME");
    h8->Draw("HIST SAME");
    leg = new TLegend(0.6, 0.65, 0.9, 0.9);
    leg->AddEntry(h2, "Test 44V", "l");
    leg->AddEntry(h7, "Test 41.5V", "l");
    leg->AddEntry(h3, "HLED 44V", "l");
    leg->AddEntry(h8, "41.5V", "l");
    leg->Draw("SAME");
    c_cleaned->Write("CombinedRMS");
    delete leg;

    c_cleaned->cd(0);

    
    // h4->SetLineColor(kRed);
    // h4->Draw("HIST");
    // h1->Draw("SAME");
    // TLegend* leg = new TLegend(0.6,0.7,0.9,0.9);
    // leg->AddEntry(h1, "Avg Amplitude Distribution Test", "l");
    // leg->AddEntry(h4, "Avg Amplitude Distribution HLED", "l");
    // leg->Draw("SAME");
    // c_cleaned->Write("AvgAmp");
    // delete leg;
    
    // c_cleaned->cd(0);

    
    // h3->SetLineColor(kRed);
    // h3->Draw("HIST");
    // h2->Draw("SAME");
    // leg = new TLegend(0.6,0.7,0.9,0.9);
    // leg->AddEntry(h2, "Avg Amplitude RMS Distribution Test", "l");
    // leg->AddEntry(h3, "Avg Amplitude RMS Distribution HLED", "l");
    // leg->Draw("SAME");
    // c_cleaned->Write("AvgAmplitudeRMS");
    // delete leg;


    file->Close();
    util->setFilePermissions(Form("%s%s_FlasherCalibration_Factor.root", outDir.c_str(), folString.c_str()));

    return 0;
}




