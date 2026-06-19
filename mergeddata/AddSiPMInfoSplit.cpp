//R__LOAD_LIBRARY(libExACT.so)
#include "AddSiPMInfoToEvents.h"



int main(int argc, char **argv) {
    if(argc < 1){
        cout << "Too few arguments; please include the date data directory to summarize" << endl;
		return 1;
	}
    
    std::string mount = "";
    mount = argv[2];
    std::string filename_argument = "";
    filename_argument = argv[3];

    if (mount != ""){ // with usingin htcondor you need to have contianers and some use full paths and other use mounts this lets you specify
        std::cout << "using mounted directory path" << std::endl;
        mnt=mount.c_str();
        dataDir = Form("%sDataAnalysis/MergedData/Output/",mnt.c_str());

    }

    // Get the Arguments
    std::string folString = argv[1];
    // Load in all the files
    std::string FolderPath = Form("%s%s/",dataDir.c_str(),folString.c_str());
    std::vector<std::string>fileNamesVec;
    if (filename_argument != "Merged_n"){ // if the file name not specified then do all files in the directory
        std::cout << "using specific file name" << std::endl;
        // std::string specificfile = Form("%s%s",FolderPath.c_str(),filename_argument.c_str());
        fileNamesVec.push_back(filename_argument);
    } else {
        fileNamesVec=util->GetFilesInDirectory(FolderPath,".root");
    }
    cout << "Number of files: " << fileNamesVec.size() << endl;

    for(int f = 0; f<static_cast<int>(fileNamesVec.size()); f++){
        std::string FilePath = Form("%s%s",FolderPath.c_str(),fileNamesVec[f].c_str());
        cout << "Processing file: " << FilePath << endl;
        
        TFile* file = new TFile(FilePath.c_str(), "UPDATE");
        if (!util->isBranchPresentInFile(FilePath, "Test")) {
            continue; // Skip to the next branch if not present
        }
    
        // gain ref // at 25�C
        // 44V
        const double GAIN_REF44 = 22.1;
        // 41.5V
        const double GAIN_REF415 = 22.1; // change number

        std::vector<std::string> treeNames = {"HLED", "BiFocal", "Forced", "Test"};

        for (const auto& treeName : treeNames) {
            TTree* tree = (TTree*)file->Get(treeName.c_str());
            if (!tree) {
                std::cout << "Tree " << treeName << " not found. Skipping.\n";
                continue;
            }

            std::cout << "Processing tree: " << treeName << std::endl;

            // Set up input branch
            IEvent* ev = new IEvent();
            tree->SetBranchAddress("Events", &ev);

            // Ensure the file is the current directory before creating new branches
            file->cd();

            // Create new branch
            ISiPM* sipmInfo = new ISiPM(256);
            TBranch* sipmBranch = tree->Branch("SiPM", "ISiPM", &sipmInfo, 64000, 99);

            std::vector<float> hv;
            std::vector<float> sipmTemp;

            // 44V
            std::vector<double> absoluteGain44(256);
            std::vector<double> tempCorrection44(256);
            std::vector<double> relOverVoltage44(256);
            std::vector<double> amplitudeToPE44(256);
            // 41.5V
            std::vector<double> absoluteGain415(256);
            std::vector<double> tempCorrection415(256);
            std::vector<double> relOverVoltage415(256);
            std::vector<double> amplitudeToPE415(256);

            // 44V
            double hvSettingPx44, relOverV44, overV44;
            // 41.5 V
            double hvSettingPx415, relOverV415, overV415;

            Long64_t nEntries = tree->GetEntries();
            for (Long64_t i = 0; i < nEntries; ++i) {
                tree->GetEntry(i);

                hv = ev->Gethv();
                float evSumHV = accumulate(hv.begin(), hv.end(), 0.0);
                float evHVAvg = evSumHV / hv.size();
                float evRoundHVAvg = round(10 * evHVAvg) / 10;

                sipmTemp = ev->GetSiPMTemp();

                for (int j = 0; j < 256; ++j) {
                    if (evRoundHVAvg == 44.0){
                        hvSettingPx44 = hv[IUtilities::GetHVChannel(j) - 1];
                        relOverV44 = ICalibration::GetRelativeOverVoltage(j, hvSettingPx44, 44);
                        overV44 = ICalibration::GetOverVoltage(j, hvSettingPx44);
                        tempCorrection44[j] = ICalibration::GetRelativeGain(j, sipmTemp[j / 16], hvSettingPx44);

                        Pulse* pulse = new Pulse(ev->GetSignalValue(j));
                        double amplitude = pulse->GetAmplitude();
                        delete pulse;

                        absoluteGain44[j] = GAIN_REF44 * (1.0 / tempCorrection44[j]);
                        amplitudeToPE44[j] = amplitude / absoluteGain44[j];
                        relOverVoltage44[j] = relOverV44;
                    }
                    if (evRoundHVAvg == 41.5){
                        hvSettingPx415 = hv[IUtilities::GetHVChannel(j) - 1];
                        relOverV415 = ICalibration::GetRelativeOverVoltage(j, hvSettingPx, 41.5);
                        overV415 = ICalibration::GetOverVoltage(j, hvSettingPx415);
                        tempCorrection415[j] = ICalibration::GetRelativeGain(j, sipmTemp[j / 16], hvSettingPx415);

                        Pulse* pulse = new Pulse(ev->GetSignalValue(j));
                        double amplitude = pulse->GetAmplitude();
                        delete pulse;

                        absoluteGain415[j] = GAIN_REF415 * (1.0 / tempCorrection415[j]);
                        amplitudeToPE415[j] = amplitude / absoluteGain415[j];
                        relOverVoltage415[j] = relOverV415;
                    }
                    
                }

                if (evRoundHVAvg == 44.0){
                    sipmInfo->SetGain(absoluteGain44);
                    sipmInfo->SetTCorrection(tempCorrection44);
                    sipmInfo->SetAmplToPE(amplitudeToPE44);
                    sipmInfo->SetRelOverV(relOverVoltage44);

                    sipmBranch->Fill();
                }
                if (evRoundHVAvg == 41.5){
                    sipmInfo->SetGain(absoluteGain415);
                    sipmInfo->SetTCorrection(tempCorrection415);
                    sipmInfo->SetAmplToPE(amplitudeToPE415);
                    sipmInfo->SetRelOverV(relOverVoltage415);

                    sipmBranch->Fill();
                }



                // sipmInfo->SetGain(absoluteGain);
                // sipmInfo->SetTCorrection(tempCorrection);
                // sipmInfo->SetAmplToPE(amplitudeToPE);
                // sipmInfo->SetRelOverV(relOverVoltage);

                // sipmBranch->Fill();
            }
            tree->Write("", TObject::kOverwrite);

            delete ev;
            delete sipmInfo;
        }
        file->Close();
        std::cout << "Added SiPM branch to trees in: " << fileNamesVec[f] << std::endl;
    }

    return 0;
}
