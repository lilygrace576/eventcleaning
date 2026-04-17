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
    
    int totalNightEntriesTest = 0;
    int totalNightEntriesHLED = 0;
    int totalNightEntriesTest44 = 0;
    int totalNightEntriesTest415 = 0;
    int totalNightEntriesHLED44 = 0;
    int totalNightEntriesHLED415 = 0;

    int sumAvgAmpPerFileTest44 = 0;
    int sumAvgAmpPerFileTest415 = 0;
    int sumAvgAmpPerFileHLED44 = 0;
    int sumAvgAmpPerFileHLED415 = 0;


    // for(int f = 47; f<80; f++){
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
                //// std::vector<float> Amplitudes;
                std::vector<float> Amplitudes44;    // Amp vector for 44 V events
                std::vector<float> Amplitudes415;   // Amp vector for 41.5 V events

                std::vector<float> BiasVoltage; // BV Vector for event

                BiasVoltage = ev->Gethv();  // get all bv values for this event (4)
                float evSumV = accumulate(BiasVoltage.begin(), BiasVoltage.end(), 0.0); // sum bv values
                float evBVAvg = evSumV / BiasVoltage.size();    // avg bv values
                float evRoundBVAvg = round(10 * evBVAvg) / 10; // round event bv avg
                
                std::string WhichEvent = "Test";

                // this allow you to have all the info you need from the event in one for loop so you dont have to have one for HLED and one for Test since the events can sometimes cross over.
                Pulse *pulse; 
                if (EventCounter < nEntries) {  // Test Events

                    tree->GetEntry(EventCounter); 

                    for (int k = 0; k < MaxNofChannels; k++) {  //for pixel
                        pulse = new Pulse(ev->GetSignalValue(k));   // get pulse signal
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
                    for (int k = 0; k < MaxNofChannels; k++) { // for pixel
                        pulse = new Pulse(evHLED->GetSignalValue(k));   // get pulse signal
                        
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

                float sumsq44 = 0;
                float sumsq415 = 0;

                float RMS44 = 0;
                float RMS415 = 0;
                
                for (int i = 0; i < MaxNofChannels; i++){   // for pixel
                    if (Amplitudes44.size() != 0){
                        sumsq44 += Amplitudes44[i]*Amplitudes44[i];
                        RMS44 = sqrt(sumsq44)/MaxNofChannels;
                    }
                    if (Amplitudes415.size() != 0){
                        sumsq415 += Amplitudes415[i]*Amplitudes415[i];
                        RMS415 = sqrt(sumsq415)/MaxNofChannels;
                    }       
                }

                if (WhichEvent == "Test"){
                    totalFileEntriesTest += 1;
                    if(evRoundBVAvg == 44.0){
                        totalFileEntriesTest44 += 1;
                        h1->Fill(util->GetEventAmplitudeSum(Amplitudes44)/MaxNofChannels);  // fill h1 with avg amp for 44 V Test events
                        int EvAvgAmpTest44 = util->GetEventAmplitudeSum(Amplitudes44)/MaxNofChannels;
                        sumEvAvgAmpTest44 += EvAvgAmpTest44;
                        h2->Fill(RMS44);    // fill h2 with 44 V Test event RMS
                    } 
                    if(evRoundBVAvg == 41.5){
                        totalFileEntriesTest415 += 1;
                        h5->Fill(util->GetEventAmplitudeSum(Amplitudes415)/MaxNofChannels); // fill h5 with avg amp of 41.5 V Test events
                        int EvAvgAmpTest415 = util->GetEventAmplitudeSum(Amplitudes415)/MaxNofChannels;
                        sumEvAvgAmpTest415 += EvAvgAmpTest415;
                        h7->Fill(RMS415);   // fill h7 with 41.5 V Test event RMS
                    }
                    // h1->Fill(util->GetEventAmplitudeSum(Amplitudes)/MaxNofChannels);
                    // h2->Fill(RMS);
                    
                } else {
                    totalFileEntriesHLED += 1;
                    if(evRoundBVAvg == 44.0){
                        totalFileEntriesHLED44 += 1;
                        h4->Fill(util->GetEventAmplitudeSum(Amplitudes44)/MaxNofChannels);  // fill h4 with avg amp for 44 V HLED events
                        int EvAvgAmpHLED44 = util->GetEventAmplitudeSum(Amplitudes44)/MaxNofChannels;
                        sumEvAvgAmpHLED44 += EvAvgAmpHLED44;
                        h3->Fill(RMS44);    // fill h3 with 44 V HLED event RMS
                    } 
                    if(evRoundBVAvg == 41.5){
                        totalFileEntriesHLED415 += 1;
                        h6->Fill(util->GetEventAmplitudeSum(Amplitudes415)/MaxNofChannels); // fill h6 with avg amp for 41.5 V HLED events
                        int EvAvgAmpHLED415 = util->GetEventAmplitudeSum(Amplitudes415)/MaxNofChannels;
                        sumEvAvgAmpHLED415 += EvAvgAmpHLED415;
                        h8->Fill(RMS415);   // fill h8 with 41.5 V HLED event RMS
                    }

                    // h4->Fill(util->GetEventAmplitudeSum(Amplitudes)/MaxNofChannels);
                    // h3->Fill(RMS);
                    
                }

                delete hEvent;
                
                // // old
                // if (folString < "20241001"){
                //     FlasherEventsCutOff = 350;
                // }

                if (folString >= "20251017"){   // if after 2025/10/17
                    FlasherEventsCutOff44 = 835;
                    FlasherEventsCutOff415 = 200;   // ??
                }
                if (folString >= "20251204"){   // if after 2025/12/04
                    FlasherEventsCutOff44 = 700;
                    FlasherEventsCutOff415 = 200;   // ??
                }

                // if (util->GetEventAmplitudeSum(Amplitudes)/MaxNofChannels >= FlasherEventsCutOff){    
                //     hled_event_counter +=1;
                //     for(int j = 0; j<MaxNofChannels; j++){
                //         TotalAmplitudeValues[j] = TotalAmplitudeValues[j]+Amplitudes[j];
                //     }
                // }
                if(evRoundBVAvg == 44.0){
                    if (util->GetEventAmplitudeSum(Amplitudes44)/MaxNofChannels >= FlasherEventsCutOff44){    // if avg 44 V event amp > 44 cutoff -> HLED
                        hled_event_counter44 +=1;
                        for(int j = 0; j<MaxNofChannels; j++){  //for pixel
                            TotalAmplitudeValues44[j] = TotalAmplitudeValues44[j]+Amplitudes44[j];
                        }
                    }
                    else{
                        test_event_counter44 += 1;
                    }
                }
                
                if(evRoundBVAvg == 41.5){
                    if (util->GetEventAmplitudeSum(Amplitudes415)/MaxNofChannels >= FlasherEventsCutOff415){    // if avg 41.5 V event amp > 44 cutoff -> HLED
                        hled_event_counter415 +=1;
                        for(int j = 0; j<MaxNofChannels; j++){  //for pixel
                            TotalAmplitudeValues415[j] = TotalAmplitudeValues415[j]+Amplitudes415[j];
                        }
                    }
                    else{
                        test_event_counter415 += 1;
                    }
                }
                
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
    // TH2F* hcam_calib = new TH2F("hcam_calib", "Calibration factor from Flasher",16, -0.5, 15.5, 16, -0.5, 15.5);
    TH2F* hcam_calib44 = new TH2F("hcam_calib", "Calibration factor from Flasher",16, -0.5, 15.5, 16, -0.5, 15.5);
    TH2F* hcam_calib415 = (TH2F*)hcam_calib44->Clone("hcam_calib415");
    
    std::vector<double> AvgAmplitudeValuesPixelsMedian44;
    std::vector<double> AvgAmplitudeValuesPixelsMedian415;
    double dead_pixel_cutoff = 750;
    if (folString < "20241001"){
        dead_pixel_cutoff = 250;
    }
    if (folString >= "20251017"){
        dead_pixel_cutoff = 700;
    }
    if (folString >= "20251204"){
        //changing from 700 
        dead_pixel_cutoff = 200;
    }
    
    for(int j = 0; j<MaxNofChannels; j++){
        AvgAmplitudeValuesPixels44[j] = TotalAmplitudeValues44[j]/hled_event_counter44;
        AvgAmplitudeValuesPixels415[j] = TotalAmplitudeValues415[j]/hled_event_counter415;
        if (AvgAmplitudeValuesPixels44[j] < dead_pixel_cutoff){   // if pixel AAVP < dead pixel cutoff -> pixel AAVP = -1
            AvgAmplitudeValuesPixels44[j] = -1;
        } else {    // if pixel AAVP >= dead pixel cutoff -> add pixel TAV/total num of HLED events (AAVP) to AAVPM vector
            AvgAmplitudeValuesPixelsMedian44.push_back(TotalAmplitudeValues44[j]/hled_event_counter44);
        }
        if (AvgAmplitudeValuesPixels415[j] < dead_pixel_cutoff){   // if pixel AAVP < dead pixel cutoff -> pixel AAVP = -1
            AvgAmplitudeValuesPixels415[j] = -1;
        } else {    // if pixel AAVP >= dead pixel cutoff -> add pixel TAV/total num of HLED events (AAVP) to AAVPM vector
            AvgAmplitudeValuesPixelsMedian415.push_back(TotalAmplitudeValues415[j]/hled_event_counter415);
        }
    }


    Double_t median44 = util->Median(AvgAmplitudeValuesPixelsMedian44); // find median of average amp values
    Double_t median415 = util->Median(AvgAmplitudeValuesPixelsMedian415); // find median of average amp values
    double minimum44 = 3; // for the canvas plot making the visualization better
    double minimum415 = 3; // for the canvas plot making the visualization better
    for(int j = 0; j<MaxNofChannels; j++){  // for pixel
        int nx, ny;
        plottools->FindBin(j, &nx, &ny);
        double norm_median44 = AvgAmplitudeValuesPixels44[j]/median44;    // normalized median = pixel avg amp / median
        double norm_median415 = AvgAmplitudeValuesPixels415[j]/median415;    // normalized median = pixel avg amp / median
        // keep the value -1 for bad pixels
        if (AvgAmplitudeValuesPixels44[j] == -1){
            norm_median44 = -1;
        }
        if (AvgAmplitudeValuesPixels415[j] == -1){
            norm_median415 = -1;
        }
        hcam_calib44->SetBinContent(nx + 1, ny + 1, norm_median44);
        hcam_calib415->SetBinContent(nx + 1, ny + 1, norm_median415);
        if (norm_median44 < minimum44 && norm_median44 > 0.1){
            minimum44 = norm_median44;
        }
        if (norm_median415 < minimum415 && norm_median415 > 0.1){
            minimum415 = norm_median415;
        }
    }

    
    file = new TFile(Form("%s%s_FlasherCalibration_Factor.root", outDir.c_str(), folString.c_str()), "RECREATE");  // "RECREATE" to overwrite if it exists
    hcam_calib44->SetStats(0);
    hcam_calib44->GetXaxis()->SetLabelSize(0.03);
    hcam_calib44->GetYaxis()->SetLabelSize(0.03);
    hcam_calib44->SetXTitle("Pixel Bin along x axis of Camera");
    hcam_calib44->SetYTitle("Pixel Bin along y axis of Camera");
    hcam_calib44->GetXaxis()->SetTitleOffset(1.1); // Adjust X-axis title offset
    hcam_calib44->GetYaxis()->SetTitleOffset(1.0); // Adjust Y-axis title offset
    hcam_calib44->Draw("colz");
    plottools->DrawMUSICBoundaries();
    hcam_calib44->Write("CamFlasher44");
    hcam_calib44->SetMinimum(minimum44-0.01);
    c_cleaned->Write("CanvasDisplayFlasherCalibration44");
    c_cleaned->cd(0);


    hcam_calib415->SetStats(0);
    hcam_calib415->GetXaxis()->SetLabelSize(0.03);
    hcam_calib415->GetYaxis()->SetLabelSize(0.03);
    hcam_calib415->SetXTitle("Pixel Bin along x axis of Camera");
    hcam_calib415->SetYTitle("Pixel Bin along y axis of Camera");
    hcam_calib415->GetXaxis()->SetTitleOffset(1.1); // Adjust X-axis title offset
    hcam_calib415->GetYaxis()->SetTitleOffset(1.0); // Adjust Y-axis title offset
    hcam_calib415->Draw("colz");
    plottools->DrawMUSICBoundaries();
    hcam_calib415->Write("CamFlasher415");
    hcam_calib415->SetMinimum(minimum415-0.01);
    c_cleaned->Write("CanvasDisplayFlasherCalibration415");
    c_cleaned->cd(0);

    // Avg Amp Test Events Plot
    h1->SetLineColor(kBlue);    // h1 - Test 44 Avg Amp
    h5->SetLineColor(kViolet);  // h5 - Test 41.5 Avg Amp
    h5->Draw("HIST");
    gPad->Update();
    TPaveStats *st1 = (TPaveStats*)h5->FindObject("stats");
    st1->SetX1NDC(0.5);
    st1->SetX2NDC(0.75);
    st1->SetLineColor(kViolet);
    h1->Draw("SAMES");
    gPad->Update();
    TPaveStats *st2 = (TPaveStats*)h1->FindObject("stats");
    st2->SetLineColor(kBlue);
    TLegend* leg = new TLegend(0.6,0.5,0.9,0.7);
    leg->AddEntry(h1, "Avg Amplitude Distribution Data 44V", "l");
    leg->AddEntry(h5, "Avg Amplitude Distribution Data 41.5V", "l");
    leg->Draw("SAME");
    c_cleaned->Write("DataAvgAmp");
    delete leg;

    c_cleaned->cd(0);

    // Avg Amp HLED Events Plot
    h4->SetLineColor(kRed); // h4 - HLED 44 Avg Amp
    h6->SetLineColor(kOrange);  // h6 - HLED 41.5 Avg Amp  
    h6->Draw("HIST");
    gPad->Update();
    TPaveStats *st3 = (TPaveStats*)h6->FindObject("stats");
    st3->SetX1NDC(0.5);
    st3->SetX2NDC(0.75);
    st3->SetLineColor(kOrange);
    h4->Draw("SAMES");
    gPad->Update();
    TPaveStats *st4 = (TPaveStats*)h4->FindObject("stats");
    st4->SetLineColor(kRed);
    leg = new TLegend(0.6,0.5,0.9,0.7);
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
    gPad->Update();
    TPaveStats *st5 = (TPaveStats*)h7->FindObject("stats");
    st5->SetX1NDC(0.5);
    st5->SetX2NDC(0.75);
    st5->SetLineColor(kViolet);
    h2->Draw("SAMES");
    gPad->Update();
    TPaveStats *st6 = (TPaveStats*)h2->FindObject("stats");
    st6->SetLineColor(kBlue);
    leg = new TLegend(0.6,0.5,0.9,0.7);
    leg->AddEntry(h2, "Avg Amplitude RMS Distribution Data 44", "l");
    leg->AddEntry(h7, "Avg Amplitude RMS Distribution Data 415", "l");
    leg->Draw("SAME");
    c_cleaned->Write("DataAvgAmplitudeRMS");
    delete leg;

    c_cleaned->cd(0);

    // RMS HLED Events Plot
    h3->SetLineColor(kRed); // h3 - HLED 44 RMS
    h8->SetLineColor(kOrange);  // h8 - HLED 41.5 RMS
    h8->Draw("HIST");
    gPad->Update();
    TPaveStats *st7 = (TPaveStats*)h8->FindObject("stats");
    st7->SetX1NDC(0.5);
    st7->SetX2NDC(0.75);
    st7->SetLineColor(kOrange);
    h3->Draw("SAMES");
    gPad->Update();
    TPaveStats *st8 = (TPaveStats*)h3->FindObject("stats");
    st8->SetLineColor(kRed);
    leg = new TLegend(0.6,0.5,0.9,0.7);
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
    h1->SetStats(0);
    h5->SetStats(0);
    h4->SetStats(0);
    h6->SetStats(0);
    double mean1 = h1->GetMean();
    double mean5 = h5->GetMean();
    double mean4 = h4->GetMean();
    double mean6 = h6->GetMean();
    h5->Draw("HIST");          // first histogram, sets the frame (idk which to put)
    h1->Draw("HIST SAME");
    h4->Draw("HIST SAME");
    h6->Draw("HIST SAME");
    leg = new TLegend(0.6, 0.65, 0.9, 0.9);
    leg->AddEntry(h1, Form("Data 44V, Mean: %.2f", mean1), "l");
    leg->AddEntry(h5, Form("Data 41.5V, Mean: %.2f", mean5), "l");
    leg->AddEntry(h4, Form("HLED 44V, Mean: %.2f", mean4), "l");
    leg->AddEntry(h6, Form("HLED 41.5V, Mean: %.2f", mean6), "l");
    leg->Draw("SAME");
    c_cleaned->Write("CombinedAvgAmp");
    delete leg;

    c_cleaned->cd(0);
    

    // RMS All
    h2->SetLineColor(kBlue);    // h2 - Test 44 RMS
    h7->SetLineColor(kViolet);  // h7 - Test 41.5 RMS
    h3->SetLineColor(kRed);     // h3 - HLED 44 RMS
    h8->SetLineColor(kOrange);  // h8 - HLED 41.5 RMS
    h2->SetStats(0);
    h7->SetStats(0);
    h3->SetStats(0);
    h8->SetStats(0);
    double mean2 = h2->GetMean();
    double mean7 = h7->GetMean();
    double mean3 = h3->GetMean();
    double mean8 = h8->GetMean();
    h7->Draw("HIST");          // first histogram, sets the frame (idk which to put)
    h2->Draw("HIST SAME");
    h3->Draw("HIST SAME");
    h8->Draw("HIST SAME");
    leg = new TLegend(0.6, 0.65, 0.9, 0.9);
    leg->AddEntry(h2, Form("Data 44V, Mean: %.2f", mean2), "l");
    leg->AddEntry(h7, Form("Data 41.5V, Mean: %.2f", mean7), "l");
    leg->AddEntry(h3, Form("HLED 44V, Mean: %.2f", mean3), "l");
    leg->AddEntry(h8, Form("HLED 41.5V, Mean: %.2f", mean8), "l");
    leg->Draw("SAME");
    c_cleaned->Write("CombinedRMS");
    delete leg;

    c_cleaned->cd(0);


    file->Close();
    util->setFilePermissions(Form("%s%s_FlasherCalibration_Factor.root", outDir.c_str(), folString.c_str()));

    return 0;
}




