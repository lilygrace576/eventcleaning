#include "EventCleaning.h"

// Event cleaning code see Latex for more information 
//https://www.overleaf.com/project/67bf40074acfe901c4446a39
//and the pdf in this folder


int main(int argc, char **argv){
    if(argc < 1){
        cout << "Too few arguments; please include the date (YYYYMMDD) data directory to summarize" << endl;
		return 1;
	}
    
    std::string mount = argv[2];
    std::string filename_argument = "";
    filename_argument = argv[3];
    
    if (mount == "y"){ // with usingin htcondor you need to have contianers and some use full paths and other use mounts this lets you specify
        std::cout << "using mounted directory path" << std::endl;
        mnt="/mnt/";
    } else if (mount != "n"){
        std::cout << "using specific directory path" << std::endl;
        mnt=mount.c_str();
    }
    
    dataDir = Form("%sDataAnalysis/MergedData/Output/",mnt.c_str());
    neighborDir = Form("%sDataAnalysis/event_cleaning/ClusterCleaning/neighbors/",mnt.c_str());
    CalibrationFactorDir = Form("%sDataAnalysis/flasher_calibration/Output/",mnt.c_str());
    outDir = Form("%sDataAnalysis/event_cleaning/Output/",mnt.c_str());
    outDirSim = Form("%sDataAnalysis/event_cleaning/OutputSim/",mnt.c_str());
    outDirBkg = Form("%sDataAnalysis/event_cleaning/OutputBkg/",mnt.c_str());
    outDirMuon = Form("%sDataAnalysis/event_cleaning/OutputMuon/",mnt.c_str());
    simDir = Form("%ssimscripts/convertSims/Output/",mnt.c_str());
    // simDir = Form("%sDataAnalysis/SimulationAnalysis/scripts/first_setdata/",mnt.c_str());
    

    // Get the Arguments 
    std::string folString = argv[1];

    // Load in all the files
    std::string FolderPath = Form("%s%s/",dataDir.c_str(),folString.c_str());
    cout << "FolderPath: " <<FolderPath << endl;
    // Createthe fileNames Vector based on the type of data found in the passes argument
    // Sim - Simulation data (Only Test Branch events)
    // muon - muon data (Forced Branch events)
    // bkg - Background data (Test Branch and HLED Branch events in seperate folder)
    // none - normal data
    std::vector<std::string> fileNamesVec;
    if (folString.find("bkg")==0) {
        whatData = "bkg";
        fileNamesVec=util->readFileToVectorString(Form("%s%s.txt",bkgDir.c_str(),folString.c_str()));
        
    } else if (folString.find("muon")==0) {
        whatData = "muon";

        fileNamesVec=util->readFileToVectorString(Form("%s%s.txt",muonDir.c_str(),folString.c_str()));

    } else if (folString.find("sim")==0) {
        whatData = "sim";
        folString = folString.substr(3, 19); // Extract date from filename
        std::string simfolder = folString.substr(8,20);
        cout << "Simulation Information: " << folString << endl;
        FolderPath = Form("%s%s/",simDir.c_str(),folString.c_str());
        cout << "FolderPath: " <<FolderPath << endl;
        fileNamesVec=util->GetFilesInDirectory(FolderPath,".root");
        // remove files in the vec that have plots_
        fileNamesVec.erase(
            std::remove_if(
                fileNamesVec.begin(),
                fileNamesVec.end(),
                [](const std::string& name) {
                    return name.find("plots_") != std::string::npos;
                }
            ),
            fileNamesVec.end()
        );
        cout << "Number of simulation files found: " << fileNamesVec.size() << endl;
    } else {
        if (filename_argument != "n"){ // if the file name not specified then do all files in the directory
        // std::cout << "using specific file name" << std::endl;
        // std::string specificfile = Form("%s%s/%s",dataDir.c_str(),folString.c_str(),filename_argument.c_str());
        // // cout << "Specific File: " << specificfile << endl;
        fileNamesVec.push_back(filename_argument);
    } else {
        // fileNamesVec = read_directory(dirName.c_str());
        fileNamesVec=util->GetFilesInDirectory(FolderPath,".root");
        fileNamesVec.erase(fileNamesVec.begin(), fileNamesVec.begin() + 2);
    }
    }
    
    CreateFileName(folString,whatData);
    
    // Creates a root file that stores all the data    
    file = new TFile(OutputFileRoot.c_str(), "RECREATE");  // "RECREATE" to overwrite if it exists
    file->Close();
    
    plothelp = new PlotHelp(); // initialize the plot help class in scope for whole program
    
    // Create a new canvas with 2x2 sub-panels
    TCanvas* c_cleaned = new TCanvas("CleanedDisplay", "Cleaned CameraPlot", 950, 1000);
    c_cleaned->Divide(2, 2);
    c_cleaned->Print(OutputFilePDFOpen.c_str());


    // new output file
    TFile *fileOutput = new TFile(OutputFileEventCleaningDataRoot.c_str(), "RECREATE");
    fileOutput->Close();  
    TTree *treeSims = new TTree("EventCleaning", "Events tree");
    eventInfo = new EventInfo(1);
    treeSims->Branch("Cleaned", eventInfo); // Store only serializable data members, not TH2F pointers
        
    
    // fileNamesVec.assign(fileNamesVec.begin() + 134, fileNamesVec.begin() + 135);
    
    // for(int f = 100; f<110; f++){
    for(int f = 0; f<static_cast<int>(fileNamesVec.size()); f++){
        if (whatData == "bkg" ||  whatData == "muon") {
            std::string date = fileNamesVec[f].substr(7, 10); // Extract date from filename
            // remove - from the date string
            date.erase(std::remove(date.begin(), date.end(), '-'), date.end());
            folString = date; // Update folString with the extracted date
            FolderPath = Form("%s%s/",dataDir.c_str(),folString.c_str());
        } 

        // checks to make sure the  data  file is  readable
        std::string FilePath = Form("%s%s",FolderPath.c_str(),fileNamesVec[f].c_str());
        // std::string FilePath = Form("%s",fileNamesVec[f].c_str());
        if (!util->isBranchPresentInFile(FilePath, "Test")) {
            continue; // Skip to the next branch if not present
        }

        int TotalEvents;
        int nEntries;
        TotalEvents = 0;
        nEntries = 0;
        // loads the events based on the type of data for each file
        if (whatData !="muon" && whatData !="sim"){
            LoadEvents(FilePath, "Test");
            LoadEventsHLED(FilePath, "HLED");
            ev = new IEvent();
            evHLED = new IEvent();
            sipmInfo = new ISiPM();
            SetBranches(ev);
            SetBranchesHLED(evHLED);
            tree->SetBranchAddress("SiPM", &sipmInfo);
            treeHLED->SetBranchAddress("SiPM", &sipmInfo);
            
            
            nEntries = tree->GetEntries();
            int nEntriesHLED = treeHLED->GetEntries();
            if (nEntries==0) {
                continue; // acts like continue in parrellization
            }    
            TotalEvents = nEntries + nEntriesHLED;
            std::cout << "Total Number of Events: " << nEntries << std::endl;
            std::cout << "Total Number of HLED Events: " << nEntriesHLED << std::endl;
            
        } else if (whatData== "muon"){
            
            LoadEvents(FilePath, "Forced");
            ev = new IEvent();
            sipmInfo = new ISiPM();
            SetBranches(ev);
            tree->SetBranchAddress("SiPM", &sipmInfo);
            
            nEntries = tree->GetEntries();
            if (nEntries==0) {
                continue;
            }    
            TotalEvents = nEntries;
            std::cout << "Total Number of Forced Events: " << nEntries << std::endl;
        } else if (whatData == "sim"){
            cout << "Loading Sim File: " << FilePath << endl;
            LoadEvents(FilePath, "Test");
            ev = new IEvent();
            // sipmInfo = new ISiPM();
            SetBranches(ev);
            // tree->SetBranchAddress("SiPM", &sipmInfo);
            
            nEntries = tree->GetEntries();
            if (nEntries==0) {
                continue;
            }    
            TotalEvents = nEntries;
            std::cout << "Total Number of Sim Test Events: " << nEntries << std::endl;
        }
        
        
        // loops through each event in a file
        for(int EventCounter = 0; EventCounter < TotalEvents; EventCounter++){
            cev = new CEvent(); // creates a new Clean Event object 
            eventInfo->Clear();
            cev->SetEventDate(stoi(folString));
            cev->SetFilename(fileNamesVec[f]);

            std::vector<Int_t> TrigMus;
            std::vector<float> AmplitudesTimeBin;
            std::vector<int> PeakTimeBin;
            std::vector<float> Pedestals;
            Pulse *pulse;
            
            if(whatData != "muon") {
                if (EventCounter < nEntries) {
                    tree->GetEntry(EventCounter);
                    TrigMus = ev->GetROIMusicID();
                } else {
                    treeHLED->GetEntry(EventCounter-nEntries);
                    TrigMus = evHLED->GetROIMusicID();
                    // EventCounter = EventCounter -nEntries;
                }
                // tree->GetEntry(EventCounter);
        
                // Set up all the information for the Cleaning Event class
                Pulse *pulse1;
                // std::vector<vector<int>> trace = event->GetSignalValue();
                
                
                for (int k = 0; k < MaxNofChannels; k++) {
                    if (EventCounter < nEntries) {
                        pulse = new Pulse(ev->GetSignalValue(k));
                        pulse1 = new Pulse(ev->GetSignalValue(k),TimeBinAll,TimeBinAll,512,true);
                        eventInfo->SetEventBranch(1);
                    } else {
                        pulse = new Pulse(evHLED->GetSignalValue(k));
                        pulse1 = new Pulse(evHLED->GetSignalValue(k),TimeBinAll,TimeBinAll,512,true);
                        eventInfo->SetEventBranch(9);
                    }
                    // pulse = new Pulse(ev->GetSignalValue(k));
                    Pedestals.push_back(pulse->GetPedestal());
                    PeakTimeBin.push_back(pulse->GetTimePeak());
                    AmplitudesTimeBin.push_back(pulse1->GetAmplitude());
                    delete pulse;
                    delete pulse1;
                }

            } else if  (whatData == "muon"){
                tree->GetEntry(EventCounter);
                TrigMus = ev->GetROIMusicID();
                
                for (int k = 0; k < MaxNofChannels; k++) {
                    // pulse = new Pulse(ev->GetSignalValue(k));
                    pulse = new Pulse(ev->GetSignalValue(k),230,250,512,true);

                    Pedestals.push_back(pulse->GetPedestal());
                    PeakTimeBin.push_back(pulse->GetTimePeak());
                    AmplitudesTimeBin.push_back(pulse->GetAmplitude());
                    delete pulse;

                }
            }
            
            
            
            cev->SetTriggeredMUSICID(TrigMus[0]);
            if (whatData == "sim"){
                PixelSurviveCutOff=1;
                sizeCutoff = 0; 
                NumberOfCoresCutoff = 0; 
                // PixelSurviveCutOff=5;
                // sizeCutoff = 200; 
                // NumberOfCoresCutoff = 3; 
                
                // CorePixelCutOff = 100;
                std::vector<double> fakeGain(256, 1.0); 
                cev->SetAmplitudeValuesTimeBin(AmplitudesTimeBin,CalibrationFactorDir, std::to_string(20241011),fakeGain);
            } else {
                cev->SetAmplitudeValuesTimeBin(AmplitudesTimeBin,CalibrationFactorDir, std::to_string(cev->GetEventDate()),sipmInfo->GetGain());

            }
            cev->SetPeakTimeBin(PeakTimeBin);
            AmplitudesTimeBin = cev->GetAmplitudeValuesTimeBin(); // reset the  amplitudesTimeBin to be aboslute gain calibrated
            cev->SetAverageAmplitude(util->GetEventAverageAmplitude(cev->GetAmplitudeValuesTimeBin()));
            cev->SetRMS(cev->GetAmplitudeValuesTimeBin());
            plothelp->AddtoRMSratioAVGamp(cev->GetRMSoverAvgAmp());
            // cout <<"Avg amplitude Before: "  << util->GetEventAverageAmplitude(AmplitudesTimeBin) << endl;
            // cout <<"Avg amplitude: "  << cev->GetAverageAmplitude() << endl;
            
            // the amplitude is different when its prior to the october 2024 site visit
            // if (cev->GetEventDate() < 20241001 && whatData != "muon") {
            //     FlasherEventsCutOff = 350; // 350 ADC/8 PE
            // }
            cev->SetEventNumber(EventCounter);
            if (EventCounter > nEntries && whatData != "muon") {
                cev->SetEventNumber(EventCounter-nEntries);
            } 
            
            // update limit for flasher events cutoff deperciated for now
            // if (cev->GetAverageAmplitude() > (FlasherEventsCutOff/util->GetADCtoPEratio()) && whatData != "muon" && whatData != "sim") {
            //     plothelp->AddHLEDEvent(1);
            //     continue;
            // }
            

            eventInfo->SetDate(cev->GetEventDate());
            eventInfo->SetFile(cev->GetFilename());
            eventInfo->SetEvent(cev->GetEventNumber());
            eventInfo->SetTriggeredMUSICID(cev->GetTriggeredMUSICID());
            eventInfo->SetTriggeredPixelID(cev->GetMaxAmplitudePixelID());
            eventInfo->SetRMS_AvgAmp(cev->GetRMSoverAvgAmp());
            eventInfo->SetRMS(cev->GetRMS());
            eventInfo->SetAvgAmp(cev->GetAverageAmplitude());
            
            
            if (cev->GetRMSoverAvgAmp() < 0.15 && whatData != "muon" && whatData != "sim") {
                plothelp->AddEventFlags(9);
                eventInfo->SetEventFlag(9); // HLED  event
                treeSims->Fill();
                delete cev;
                continue;
            }
            
            
            
            // Remove events where the door is closed. This can be done by checking the HV currents of the SiPMs and 
            // if the sky seeing Channels are below 3.8 mA you know that the door is closed.
            // if (ev->Gethvc()[0] < 3.8 && ev->Gethvc()[2] < 3.8){
                //     plothelp->AddCleanedEvent(1);
                //     continue;
                // }
                
                
            std::vector<int> SaturatedPixels = util->GetSaturatedPixels(ev->GetSignalValue());
            cev->SetSaturatedPixels(std::accumulate(SaturatedPixels.begin(), SaturatedPixels.end(), 0));
            if (cev->GetSaturatedPixels() > SaturatedPixelCutoff && whatData != "muon" ) {
                plothelp->AddEventFlags(1);
                eventInfo->SetEventFlag(1);
                treeSims->Fill();
                delete cev;
                continue;
            }
            cout << "Event Number: " << cev->GetEventNumber() << " triggered  Music: " <<cev->GetTriggeredMUSICID() <<endl;
            // cout << "triggered Music pair: " << util->GetMUSICMate(cev->GetTriggeredMUSICID(), true) << endl;
            
            // //check which music has the highest amplitude 
            // cout << "triggered Music amp: " << util->GetMaximum(AmplitudesTimeBin,cev->GetTriggeredMUSICID()) << endl;
            // cout << "triggered Music pair amp: " << util->GetMaximum(AmplitudesTimeBin,util->GetMUSICMate(cev->GetTriggeredMUSICID(), true)) << endl;
            if(util->GetMaximum(cev->GetAmplitudeValuesTimeBin(),cev->GetTriggeredMUSICID()) < util->GetMaximum(cev->GetAmplitudeValuesTimeBin(),util->GetMUSICMate(cev->GetTriggeredMUSICID(), true))){
                cev->SetTriggeredMUSICID(util->GetMUSICMate(cev->GetTriggeredMUSICID(), true));
                eventInfo->SetTriggeredMUSICID(cev->GetTriggeredMUSICID());
                eventInfo->SetTriggeredPixelID(cev->GetMaxAmplitudePixelID());
                // cout << "Replaced. " << endl;
                // cout << "triggered Music pair: " << util->GetMUSICMate(cev->GetTriggeredMUSICID(), true) << endl;
            }
            
            cout << "triggered  Music: " <<cev->GetTriggeredMUSICID() << endl;
            cev->SetMaxAmplitude(util->GetMaximum(cev->GetAmplitudeValuesTimeBin(),cev->GetTriggeredMUSICID()));
            if (cev->GetMaxAmplitude() < TriggeredChannelAmpCutOff/util->GetADCtoPEratio()) {
                plothelp->AddEventFlags(1);
                eventInfo->SetEventFlag(1);
                treeSims->Fill();
                delete cev;
                continue;
            }
            

            cev->SetPedestalValues(Pedestals);
            cev->SetMaxAmplitudePixelID(util->GetMaximumPixelID(cev->GetAmplitudeValuesTimeBin(),cev->GetTriggeredMUSICID()));
            cout << "Triggered Pixel ID: " << cev->GetMaxAmplitudePixelID() << endl;

            // removed events that have pixel 76 
            // if (cev->GetMaxAmplitudePixelID() == 78 || cev->GetMaxAmplitudePixelID() == 79 || cev->GetMaxAmplitudePixelID() == 77|| cev->GetMaxAmplitudePixelID() == 76 || cev->GetMaxAmplitudePixelID() == 75) {
            //     cout << "Triggered Pixel ID 78 or 79 skipping event." << endl;
            //     plothelp->AddPreCleaned(1);
            //     continue;
            // }

            // removed the hotspot on the worst night 
            if ((cev->GetTriggeredMUSICID() == 18 || cev->GetTriggeredMUSICID() == 19) && (folString == "20250305" || folString == "20250331" || folString == "20250328")){
                cout << "Triggered Music 18,19 skipping event." << endl;
                eventInfo->SetEventFlag(1);
                plothelp->AddEventFlags(1);
                treeSims->Fill();
                delete cev;
                continue;
            }
            
       
            cev->SetSumAmplitude(std::accumulate(AmplitudesTimeBin.begin(), AmplitudesTimeBin.end(), 0.0));
            
            
            /** 
            --------Create Panel 1 
            **/
            c_cleaned->cd(1);
            gPad->SetLeftMargin(0.15);  // Increase left margin
            gPad->SetBottomMargin(0.25); // Increase bottom margin
            gPad->SetRightMargin(0.15);
            

            std::string filenameTitle = cev->GetFilename();
            int pos = filenameTitle.find("T");
            filenameTitle = filenameTitle.substr(pos+1, 5);

            if (whatData == "sim") {
                std::string temp = (cev->GetFilename()).substr(52,3);
                filenameTitle = folString.substr(9,6);
                cout << filenameTitle << endl;
                filenameTitle = filenameTitle.append("_");
                filenameTitle = filenameTitle.append(temp);
                filenameTitle.erase(std::remove(filenameTitle.begin(), filenameTitle.end(), '.'), filenameTitle.end());
                filenameTitle.erase(std::remove(filenameTitle.begin(), filenameTitle.end(), 'r'), filenameTitle.end());
                
            }
            // create TH2D for Camera
            
            hcam_panel1 = new TH2F("hcam_panel1", Form("Calibrated Img --N# %i --F# %s-- E# %i ", cev->GetEventDate(), filenameTitle.c_str() ,cev->GetEventNumber()), 16, -0.5, 15.5, 16, -0.5, 15.5);
            hcam_panel1->SetDirectory(0);
            cev->SetPanel1(hcam_panel1);
            // eventInfo->SetHPanel1(hcam_panel1);
            // cev->SetAverageAmplitudeRMS(cev->GetAmplitudeValuesTimeBin());
            // Add a subtitle using TLatex
            TLatex *subtitle = new TLatex();
            subtitle->SetNDC(); // Set to Normalized Device Coordinates (NDC)
            subtitle->SetTextSize(0.03);
            
            subtitle->DrawLatex(0.1, 0.92, Form("Avg Amp Whole Camera : %.2f - Avg RMS: %.2f", cev->GetAverageAmplitude(), cev->GetRMS()));
            subtitle->DrawLatex(0.25, 0.12, Form("Triggered MUSIC: %i  Triggered Pixel: %i",cev->GetTriggeredMUSICID(),util->GetMaximumPixelID(cev->GetAmplitudeValuesTimeBin(),TrigMus[0])));
            delete subtitle;
            
            eventInfo->SetHPanel1(hcam_panel1);
            
            /** 
            --------Create Panel 2
            **/
            // Plot hcam using the data from the CSV file
            c_cleaned->cd(2);
            gPad->SetLeftMargin(0.15);  // Increase left margin
            gPad->SetBottomMargin(0.25); // Increase bottom margin
            gPad->SetRightMargin(0.15);
            
            
            
            hcam_panel2 = (TH2F*)hcam_panel1->Clone("hcam_panel2");
            hcam_panel2->SetTitle(Form("Amplitude cut Img --N# %i --F# %s-- E# %i ", cev->GetEventDate(), filenameTitle.c_str() ,cev->GetEventNumber()));
            hcam_panel2->SetDirectory(0);
            cev->SetPanel2(hcam_panel2,CorePixelAmpCutOff/util->GetADCtoPEratio());
            std::vector<int> survivingPanel2Pixels = cev->GetSurvivingPixelPanel2();
            if (survivingPanel2Pixels.size() < static_cast<std::vector<int>::size_type>(PixelSurviveCutOff)) {
                eventInfo->SetEventFlag(2);
                plothelp->AddEventFlags(2);
                treeSims->Fill();
                
                delete hcam_panel1;
                delete hcam_panel2;
                delete cev;
                
                
                cout << "Removed Panel 2" << endl;
                continue;
            }
            
            
            /** 
            --------Create Panel 3
            **/
            c_cleaned->cd(3);
            gPad->SetLeftMargin(0.15);  // Increase left margin
            gPad->SetBottomMargin(0.25); // Increase bottom margin
            gPad->SetRightMargin(0.15);
            
            hcam_panel3 = (TH2F*)hcam_panel2->Clone("hcam_panel3");
            hcam_panel3->SetTitle(Form("Connected Pixel Img --N# %i --F# %s-- E# %i ", cev->GetEventDate(), filenameTitle.c_str() ,cev->GetEventNumber()));
            hcam_panel3->SetDirectory(0);
            
            
            cev->SetPanel3(hcam_panel3, neighborDir,NumberOfCoresCutoff,CorePixelCutOff, CrosstalkAllowanceCutoff);
            
            
            //find configure events with amp high and above 13
            // cout << "Configure Event?: " << cev->isConfigureEvent() << endl;
            if (cev->isConfigureEvent() == 1){
                cout << "Removed Panel 3 Configure" << endl;
                eventInfo->SetEventFlag(3);
                plothelp->AddEventFlags(3);
                treeSims->Fill();
                delete hcam_panel1;
                delete hcam_panel2;
                delete hcam_panel3;
                delete cev;
                continue;
            }
            cout << "YOU "<<cev->GetSurvivingPixelPanel3().size() << endl;
            if (cev->GetSurvivingPixelPanel3().size() < static_cast<std::vector<int>::size_type>(PixelSurviveCutOff)) {
                eventInfo->SetEventFlag(3);
                plothelp->AddEventFlags(3);
                treeSims->Fill();
                delete hcam_panel1;
                delete hcam_panel2;
                delete hcam_panel3;
                delete cev;
                
                
                cout<< "Removed Panel 3  NUmber of Pixels" << endl;
                continue;
            }
            
            if (cev->GetSurvivingPixelTotalAmpPanel3() <= sizeCutoff ) {
                eventInfo->SetEventFlag(3);
                plothelp->AddEventFlags(3);
                treeSims->Fill();
                delete hcam_panel1;
                delete hcam_panel2;
                delete hcam_panel3;
                delete cev;
                
                
                cout<< "Removed Panel 3 Size" << endl;
                continue;
            }
            cout << cev->GetMaxAmplitude()  << " , " << cev->GetSecondCoreAmp() << endl;
            if (cev->GetMaxAmplitude() == 0.0 || cev->GetSecondCoreAmp() == 0.0) {
                eventInfo->SetEventFlag(3);
                plothelp->AddEventFlags(3);
                treeSims->Fill();
                delete hcam_panel1;
                delete hcam_panel2;
                delete hcam_panel3;
                delete cev;
                
                
                cout<< "Removed Panel 3 Pixel amount" << endl;
                continue;
            }
            
            /** 
            --------Create Panel 4
            **/
            c_cleaned->cd(4);
            gPad->SetLeftMargin(0.15);  // Increase left margin
            gPad->SetBottomMargin(0.25); // Increase bottom margin
            gPad->SetRightMargin(0.15);
            
            // At some point get panel four into the CEvent class
            // std::vector<double> CenterOfGravity(2, 0.0);
            //Create the PCA
            PCA pca(2);
            
            // do the Principle componet analysis on the cleaned data
            LoadDataPCA(pca,hcam_panel3,cev->GetSurvivingPixelTotalAmpPanel3());
            // //pca.ComputeCovarianceMatrix();
            
            TVectorD eigenVals; // remove the use of the pointer because this is already in the PCA class and calulated in LoadDataPCA
            TMatrixD eigenVecs; 
            std::vector<double> EllipicRatio=CreateWLRatio(pca,eigenVals,eigenVecs); // to do make EllipicRatio a member of cev
            //ex. cev->SetEllipticRatio(EllipicRatio);
            if (EllipicRatio[2] > (WLRatioCutOff/100.0)) {
                eventInfo->SetEventFlag(4);
                plothelp->AddEventFlags(4);
                treeSims->Fill();
                delete hcam_panel1;
                delete hcam_panel2;
                delete hcam_panel3;
                delete cev;
                continue;
            }
            
            
            eventInfo->SetEventFlag(5);
            plothelp->AddEventFlags(5);
            
            plothelp->AddtoEventDate(cev->GetEventDate());

            hcam_panel4 = (TH2F*)hcam_panel3->Clone("hcam_panel4");
            hcam_panel4->SetDirectory(0);
            hcam_panel4->SetTitle(Form("Cleaned Img --N# %i --F# %s-- E# %i ", cev->GetEventDate(), filenameTitle.c_str() ,cev->GetEventNumber()));
            CompletePanel4(pca,hcam_panel4,cev,EllipicRatio,eigenVals,eigenVecs,eventInfo);            
            treeSims->Fill();
            
            // create TH2D for Gain 
            file = new TFile(OutputFileRoot.c_str(), "UPDATE");
            c_cleaned->Write(Form("N%i-F%s-E%i", cev->GetEventDate(),filenameTitle.c_str() ,cev->GetEventNumber()));
            hcam_panel1->Write(Form("PreN%i-F%s-E%iTH2F", cev->GetEventDate(),filenameTitle.c_str() ,cev->GetEventNumber()));
            hcam_panel4->Write(Form("PostN%i-F%s-E%iTH2F", cev->GetEventDate(),filenameTitle.c_str() ,cev->GetEventNumber()));
            file->Close();
            c_cleaned->Print(OutputFilePDF.c_str());
            

            delete hcam_panel1;
            delete hcam_panel2;
            delete hcam_panel3;
            delete hcam_panel4;
            delete cev;
            
        }
     
        if (whatData != "muon"){
            delete treeHLED;
            delete evHLED;
        }
        delete tree;
        delete ev;
    }
    cout << "we here" << endl;
    fileOutput = new TFile(OutputFileEventCleaningDataRoot.c_str(), "UPDATE");
    treeSims->Write();
    // fileOutput->Write();
    fileOutput->Close();
    file = new TFile(OutputFileRoot.c_str(), "UPDATE");
    plothelp->PlotEventFlags(c_cleaned, OutputFilePDF);
    plothelp->PlothWL(c_cleaned,OutputFilePDF,outDir,to_string(cev->GetEventDate()));
    plothelp->PlothdistLandW(c_cleaned,OutputFilePDF);
    plothelp->PlothSize(c_cleaned,OutputFilePDF);    
    plothelp->PlothSizeConc(c_cleaned,OutputFilePDF);    
    plothelp->PlothSizeSPC(c_cleaned,OutputFilePDF);    
    plothelp->PlothCoreRatio(c_cleaned,OutputFilePDF);    
    plothelp->PlothNumberofCores(c_cleaned,OutputFilePDF);    
    plothelp->PlothCoreRatioandNumberofCores(c_cleaned,OutputFilePDF);    
    plothelp->PlothWLConc(c_cleaned,OutputFilePDF);    
    plothelp->PlothWLSPC(c_cleaned,OutputFilePDF); 
    plothelp->PlothSIZEWL(c_cleaned,OutputFilePDF);   
    plothelp->PlothLvW(c_cleaned,OutputFilePDF);
    plothelp->PlothSPCConc(c_cleaned,OutputFilePDF);    
    plothelp->PlothCOG(c_cleaned,OutputFilePDF);    
    plothelp->PlothTPID(c_cleaned,OutputFilePDF);
    plothelp->PlothRMSratioAvg(c_cleaned, OutputFilePDF);
    plothelp->PlothEventDateTriggeredPixel(c_cleaned, OutputFilePDF);
    plothelp->PlothncoreSIZE(c_cleaned, OutputFilePDF);
    plothelp->PlothnCoreSPC(c_cleaned, OutputFilePDF);
    plothelp->PlothCRSPC(c_cleaned, OutputFilePDF);
    plothelp->PlothSIZEWandL(c_cleaned, OutputFilePDF);

    file->Close();
    c_cleaned->Print(OutputFilePDFClose.c_str());
    util->setFilePermissions(OutputFileRoot.c_str());
    util->setFilePermissions(OutputFileEventCleaningDataRoot.c_str());
    cout << "Completed!" << endl;   
    // cout << "Total Events: " << plothelp->GetTotalEvent() << endl;
    delete plothelp;


    
}






