//R__LOAD_LIBRARY(libExACT.so)
//#include "libExACT.so"
#include "PCA.h"
#include "PlotHelp.h"
#include <set>
#include <TH1.h>
#include <TTree.h>
#include <TCanvas.h>
#include <typeinfo>
#include "TLine.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <TPaletteAxis.h>
#include <algorithm>
#include <TEllipse.h>
#include <IUtilities.h>
#include <TObjString.h>
#include <TH2F.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TString.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>
#include <dirent.h>
#include <TArrow.h>
#include <sys/types.h>
#include <vector>
#include <TLatex.h>
#include <cmath>
#include <limits>
#include <algorithm>
#include <cstdlib>
#include <Event.h>
#include <BiFocal.h>
#include <Pulse.h>
#include <TFile.h>
#include <TBox.h>
#include <TMatrixD.h>
#include <TVectorD.h>
#include <Getline.h>
#include <TRandom.h>
#include <TTimer.h>
#include <TApplication.h>
#include <TGraph.h>
#include <TLine.h>
#include <TLegend.h>
#include <TGaxis.h>
#include <TPaveStats.h>
#include <TList.h>
#include <TFile.h>
#include <TText.h>
#include <TStyle.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TMath.h>
#include <TPrincipal.h>
#include "Math/Integrator.h"
#include <TPaveText.h>
#include <random>
#include <utility>
#include <CEvent.h>
#include <IEvent.h>
#include <IPlotTools.h>
#include <ISiPM.h>
#include <execution>
#include "EventInfo.h"


// Define variables (global)
std::vector<double> MaxAmplitudePixel;
std::vector<int> MaxPixelID;
std::vector<int> MaxPeakTimePixel;
std::vector<int> MaxMUSICID;
std::vector<double> MaxAmplitudePixelTimeBin;
std::vector<int> MaxPixelIDTimeBin;
std::vector<double> AvgAmplitudePerEvent;
std::vector<double> CameraAmplitudeAtTimeBin;

IUtilities *util;
IPlotTools *plottools;
CEvent *cev;
PlotHelp *plothelp;
EventInfo *eventInfo=0;

TTree *tree = 0;
TTree *treeHLED = 0;
IEvent *ev;
IEvent *evHLED;
ISiPM *sipmInfo;


TH2F* hcam_panel1=0;
TH2F* hcam_panel2=0;
TH2F* hcam_panel3=0;
TH2F* hcam_panel4=0;
TH2F* COGgraph=0;
TH2F* COGgraphweighted=0;
TH2F* pixeldist=0;

TFile *fO;
TFile *file;

TCanvas *c_cleaned =0;
TCanvas *c_PCA=0;
int MaxNofChannels = 256;
std::string OutputFileRoot = "";
std::string OutputFilePDF = "";
std::string OutputFilePDFOpen = "";
std::string OutputFilePDFClose = "";
std::string OutputFileEventCleaningDataRoot = "";
std::string whatData = "Data";

std::string mnt = "/storage/osg-otte1/shared/TrinityDemonstrator/";
std::string dataDir = "/storage/osg-otte1/shared/TrinityDemonstrator/DataAnalysis/MergedData/Output/";
std::string outDir = "/storage/osg-otte1/shared/TrinityDemonstrator/DataAnalysis/event_cleaning/Output/";


std::string neighborDir = "/storage/osg-otte1/shared/TrinityDemonstrator/DataAnalysis/event_cleaning/ClusterCleaning/neighbors/";
std::string CalibrationFactorDir = "/storage/osg-otte1/shared/TrinityDemonstrator/DataAnalysis/flasher_calibration/Output/";

// FIX THESE 2025-09-23
std::string muonDir = "/storage/osg-otte1/shared/TrinityDemonstrator/DataAnalysis/DataTxtFiles/Muon/";
std::string bkgDir = "/storage/osg-otte1/shared/TrinityDemonstrator/DataAnalysis/DataTxtFiles/BackgroundSamples/RandomSampling/";
std::string simDir = "/storage/osg-otte1/shared/TrinityDemonstrator/DataAnalysis/SimulationData/data/";

std::string outDirBkg = "/storage/osg-otte1/shared/TrinityDemonstrator/event_cleaning/OutputBkg/";
std::string outDirMuon = "/storage/osg-otte1/shared/TrinityDemonstrator/event_cleaning/OutputSim/";
std::string outDirSim = "/storage/osg-otte1/shared/TrinityDemonstrator/event_cleaning/OutputSim/";


const int TriggeredChannelAmpCutOff = 481; // (200 ADC/8 PE) Cut off for the triggered music channel
const int TimeBinAll = 239; // Difference between triggered pixel time bin and the pixels around it time bin difference more that 1 risk saving cross talk events
const int CorePixelAmpCutOff = 481; // (200ish ADC/8PE) // Cut off for the pixels surrounding the triggered music channel
const int SaturatedPixelCutoff = 256; // 0 removed saturated pixels max channels from being cleaned. 1 allows them to be cleaned (changed from 0 on 2/26/2025)

// change from 800 (44V) to 200 (41.5V)
int FlasherEventsCutOff = 800; // (350 ish ADC/15 PE ) Average amplitude across the camera ~1200 is  Flasher event
// int FlasherEventsCutOff = 200;

int PixelSurviveCutOff = 3; //How many pixels need to survive cleaning to plot
const int WLRatioCutOff = 100; // length width How elliptical you require the events to be after the Principle Compomnent Analysis (helps removed crosstalk events.)
const int rmTopRow = 1; // removed the top row of the camera (sky) 0 = remove, 1 = dont remove
int sizeCutoff = 0; // in PE so 100, or 200 pe removed the top row of the camera (sky) 0 = remove, 1 = dont remove
int NumberOfCoresCutoff = 2; // 1, 2,3 cores removed the top row of the camera (sky) 0 = remove, 1 = dont remove
float CrosstalkAllowanceCutoff = 0.3; // typically 0.3 removed the top row of the camera (sky) 0 = remove, 1 = dont remove
float CorePixelCutOff = 1.5;


void SetBranches(IEvent *evD);
void SetBranchesHLED(IEvent *evD);
void LoadEvents(std::string filename, std::string treeString);
void LoadEventsHLED(string NameofFile, std::string treeString);
void saveEventInfo(EventInfo* evI, TTree* treeSims);

void LoadDataPCA(PCA& pca, TH2F* hist, int totalAmp);
std::vector<double> CreateWLRatio(PCA& pca, TVectorD& eigenVals, TMatrixD& eigenVecs);
std::vector<int> CheckCrossPoints(TArrow* arrow, TH2F* hist, EventInfo* eventInfo);
void CompletePanel4(PCA& pca, TH2F* hcam_panel4, CEvent* cev, std::vector<double> EllipicRatio, TVectorD& eigenVals, TMatrixD& eigenVecs, EventInfo* eventInfo);
std::vector<double> getM3Long(double xcog,double ycog, std::vector<int> sur_pix, std::vector<float> amps);
std::vector<double> generateRandomNumbers();
void CreateFileName(std::string filename, bool bkg);

void CreateFileName(std::string filename, std::string dataType) {
    if (dataType == "bkg") {
        outDir = outDirBkg;
    }
    if (dataType == "muon") {
        outDir = outDirMuon;
    }
    if (dataType == "sim") {
        outDir = outDirSim;
    }
    OutputFileRoot=Form("%sEventCleanedCluster%s_TC_%i_TB_%i_NP_%i_s_%i_FA_%i_mp_%i_er_%i_tr_%i.root",
        outDir.c_str(),
        filename.c_str(),
        TriggeredChannelAmpCutOff,
        TimeBinAll,
        CorePixelAmpCutOff, 
        SaturatedPixelCutoff,
        FlasherEventsCutOff,
        PixelSurviveCutOff,
        WLRatioCutOff,
        rmTopRow
    );

    OutputFileEventCleaningDataRoot = Form("%sDataFiles/Data_EventCleaning%s_TC_%i_TB_%i_NP_%i_s_%i_FA_%i_mp_%i_er_%i_tr_%i.root",
        outDir.c_str(),
        filename.c_str(),
        TriggeredChannelAmpCutOff,
        TimeBinAll,
        CorePixelAmpCutOff, 
        SaturatedPixelCutoff,
        FlasherEventsCutOff,
        PixelSurviveCutOff,
        WLRatioCutOff,
        rmTopRow
    );
    int check = mkdir(Form("%sDataFiles",outDir.c_str()),0777);

    // check if directory is created or not
    if (!check)
        printf("Directory created\n");
    else {
        printf("Unable to create directory\n");
    }


    // Create the file name with the cariables in the name for sorting of them. 
    // _TC_#_ = TriggeredChannelAmpCutOff
    // TB_#_ = TimeBinAll
    // NP_#_ = CorePixelAmpCutOff
    // s_#_ = SaturatedPixelCutoff
    // FA_#_ = FlasherEventsCutOff
    // mp_# = PixelSurviveCutOff
    OutputFilePDF=Form("%sEventCleanedCluster%s_TC_%i_TB_%i_NP_%i_s_%i_FA_%i_mp_%i_er_%i_tr_%i.pdf",
        outDir.c_str(),
        filename.c_str(),
        TriggeredChannelAmpCutOff,
        TimeBinAll,
        CorePixelAmpCutOff, 
        SaturatedPixelCutoff,
        FlasherEventsCutOff,
        PixelSurviveCutOff,
        WLRatioCutOff,
        rmTopRow
    );

    OutputFilePDFOpen=Form("%sEventCleanedCluster%s_TC_%i_TB_%i_NP_%i_s_%i_FA_%i_mp_%i_er_%i_tr_%i.pdf[",
        outDir.c_str(),
        filename.c_str(),
        TriggeredChannelAmpCutOff,
        TimeBinAll,
        CorePixelAmpCutOff, 
        SaturatedPixelCutoff,
        FlasherEventsCutOff,
        PixelSurviveCutOff,
        WLRatioCutOff,
        rmTopRow
    );

    OutputFilePDFClose=Form("%sEventCleanedCluster%s_TC_%i_TB_%i_NP_%i_s_%i_FA_%i_mp_%i_er_%i_tr_%i.pdf]",
        outDir.c_str(),
        filename.c_str(),
        TriggeredChannelAmpCutOff,
        TimeBinAll,
        CorePixelAmpCutOff, 
        SaturatedPixelCutoff,
        FlasherEventsCutOff,
        PixelSurviveCutOff,
        WLRatioCutOff,
        rmTopRow
    );
}


void LoadEvents(string NameofFile, std::string treeString)
{
    NameofFile = NameofFile;
    cout << "Loading file: " << NameofFile << endl;
    TFile *fO = new TFile(NameofFile.c_str(), "READ");
    tree = (TTree*)fO->Get(treeString.c_str());
}

void LoadEventsHLED(string NameofFile, std::string treeString)
{
    NameofFile = NameofFile;
    //cout << "Loading file for HLED: " << NameofFile << endl;
    TFile *fO = new TFile(NameofFile.c_str(), "READ");
    treeHLED = (TTree*)fO->Get(treeString.c_str());
}


void SetBranches(IEvent *evD)
{
    tree->SetBranchAddress("Events", &ev);
    // tree->SetBranchAddress("Event", &ev);
    // tree->Scan("Events.Event");
}

void SetBranchesHLED(IEvent *evD)
{
    treeHLED->SetBranchAddress("Events", &evHLED);
}

void saveEventInfo(EventInfo* evI, TTree* treeSims) {
    // TFile *fileOutput = new TFile("Test.root", "UPDATE");
    // treeSims->Branch("Cleaned","EventInfo",evI);
    treeSims->Fill();
    // fileOutput->Write();
    // fileOutput->Close();
}



// /***
// Start of Functions

// ***/



void LoadDataPCA(PCA& pca, TH2F* hist, int totalAmp){
    


    // (*COG)[0] = 0.0;
    // (*COG)[1] = 0.0;
   

    // Loop over the bins of the histogram and extract the bin content (data points)
    for (int bin = 0; bin <= MaxNofChannels; bin++) {
       
        int nx, ny;
        plottools->FindBin(bin, &nx, &ny);
        // Get the bin center for X and Y
        double weight = hist->GetBinContent(nx+1, ny+1);
         // Check if the value is infinity
        if (std::isinf(weight)) {
            weight = 0.0; // Set to 0
        }
        
        // Add the data point to the PCA object weighted by the bin content
        if (weight > 0) {  // Only add rows for non-zero bins

            std::vector<double> randomNumbers;
            for (double w = 0; w < weight; w++){
                randomNumbers = generateRandomNumbers();
                // cout << "nx: " << nx <<  " ny: " << ny << endl;
                // cout << "rand 1: " << nx + randomNumbers[0] << " rand 2: " << ny + randomNumbers[1] << endl;
                std::vector<double> data = {nx + randomNumbers[0], ny+randomNumbers[1]};
                // cout << "COG " << weight << "total" << totalAmp << endl;
                
                pca.AddRow(data);  
            }
            if (nx == 0){
                nx = 0.1;
            }
            if (ny == 0){
                ny = 0.1;
            }

            // (*COG)[0] += (nx * (weight/totalAmp));
            // (*COG)[1] += (ny * (weight/totalAmp));
        }   
    }
}

std::vector<double> generateRandomNumbers() {
    // Define the random number generator and distribution
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-0.49, 0.49);

    // Generate two random numbers and return them as a pair
    return {dist(gen), dist(gen)};
}

std::vector<double> CreateWLRatio(PCA& pca, TVectorD& eigenVals, TMatrixD& eigenVecs) {
    eigenVals.ResizeTo(2);
    eigenVecs.ResizeTo(2,2);
    std::vector<double> sigmas;

    pca.ComputeEigenDecomposition();
    eigenVals = pca.GetEigenValues();
    eigenVecs = pca.GetEigenVectors();
    sigmas = pca.GetSigmas();                            
    
    double r1 = sqrt(eigenVals[0]);
    double r2 = sqrt(eigenVals[1]);
    std::vector<double> EllipicRatio = {r1,r2,r2/r1};
    cout << "WL " << EllipicRatio[0] << "," << EllipicRatio[1] << ", ratio:" <<  EllipicRatio[2] << "is: " << WLRatioCutOff/100.0 << endl;
    
    return EllipicRatio;
}

std::vector<int> CheckCrossPoints(TArrow* arrow, TH2F* hist,EventInfo* eventInfo) {
    // get the arrow slope
    cout << "Checking cross points" << endl;
    double x1 = arrow->GetX1();
    double y1 = arrow->GetY1();
    double x2 = arrow->GetX2();
    double y2 = arrow->GetY2();

    // Line vector components
    double dx = x2 - x1;
    double dy = y2 - y1;
    double len = std::sqrt(dx*dx + dy*dy);

    // Degenerate line guard
    if (len == 0.0) return {0, 0};

    int cLineIntersectPixels    = 0;
    int cLineNonIntersectPixels = 0;

    int nBinsX = hist->GetNbinsX();
    int nBinsY = hist->GetNbinsY();

    double RMSMajorAxis = 0.0;
    double WeightedRMSMajorAxis = 0.0;
    double totalWeight = 0.0;

    for (int ix = 1; ix <= nBinsX; ++ix) {
        for (int iy = 1; iy <= nBinsY; ++iy) {

            if (hist->GetBinContent(ix, iy) <= 0) continue;

            // Bin center in axis coordinates
            double cx = hist->GetXaxis()->GetBinCenter(ix);
            double cy = hist->GetYaxis()->GetBinCenter(iy);

            // Perpendicular distance from bin center to the infinite line
            // d = |(P - P1) x d_hat|  (2D cross product gives scalar)
            double dist = std::abs((cy - y1)*dx - (cx - x1)*dy) / len;
            plothelp->AddtoDistance2MajorAxis(dist);
            eventInfo->SetDistance2MajorAxis(dist);
            RMSMajorAxis += dist*dist;
            WeightedRMSMajorAxis += dist*dist*hist->GetBinContent(ix, iy);
            totalWeight += hist->GetBinContent(ix, iy);
            // Half-diagonal of the bin as intersection threshold
            double hw = 0.5 * hist->GetXaxis()->GetBinWidth(ix);
            double hh = 0.5 * hist->GetYaxis()->GetBinWidth(iy);
            double threshold = 0.905*std::sqrt(hw*hw + hh*hh);
            cout << threshold << endl;
            if (dist <= threshold) {
                ++cLineIntersectPixels;
                cout << "Intersecting pixel at (" << cx << ", " << cy << ") with content " << hist->GetBinContent(ix, iy) << " distance " << dist << endl;

            }
            else {
                ++cLineNonIntersectPixels;
                cout << "Non-intersecting pixel at (" << cx << ", " << cy << ") with content " << hist->GetBinContent(ix, iy) << " distance " << dist << endl;
            }
        }
    }
    RMSMajorAxis = std::sqrt(RMSMajorAxis/(cLineIntersectPixels+cLineNonIntersectPixels));
    WeightedRMSMajorAxis = std::sqrt(WeightedRMSMajorAxis/(totalWeight));
    cout << "RMS Major Axis: " << RMSMajorAxis << endl;
    cout << "Weighted RMS Major Axis: " << WeightedRMSMajorAxis << endl;

    plothelp->AddtoPixelsonMajorAxis(cLineIntersectPixels);
    plothelp->AddtoPixelsoffMajorAxis(cLineNonIntersectPixels);
    plothelp->AddtoRatioPixelsMajorAxis((double)cLineIntersectPixels/(cLineIntersectPixels+cLineNonIntersectPixels));
    plothelp->AddtoRMSMajorAxis(RMSMajorAxis);
    plothelp->AddtoWeightedRMSMajorAxis(WeightedRMSMajorAxis);

    eventInfo->SetRMSMajorAxis(RMSMajorAxis);
    eventInfo->SetWeightedRMSMajorAxis(WeightedRMSMajorAxis);
    eventInfo->SetPixelsonMajorAxis(cLineIntersectPixels);
    eventInfo->SetPixelsoffMajorAxis(cLineNonIntersectPixels);
    eventInfo->SetRatioPixelsMajorAxis((double)cLineIntersectPixels/(cLineIntersectPixels+cLineNonIntersectPixels));


    return {cLineIntersectPixels, cLineNonIntersectPixels};

}

void CompletePanel4(PCA& pca, TH2F* hcam_panel4, CEvent* cev, std::vector<double> EllipicRatio, TVectorD& eigenVals, TMatrixD& eigenVecs,  EventInfo* eventInfo) {
    
    
    
    hcam_panel4->Draw("colz");
    plottools->DrawMUSICBoundaries();
    
    double meanx = hcam_panel4->GetMean(1);
    double meany = hcam_panel4->GetMean(2);
    // cout << "MEAN " << meanx << "," << meany << endl;
    
    double angledeg = atan(eigenVecs[1][0]/eigenVecs[0][0])*(180.0/3.141592653589793238463);
    double anglerad = atan(eigenVecs[1][0]/eigenVecs[0][0]);
    
    
    double r1 = sqrt(eigenVals[0]);
    double r2 = sqrt(eigenVals[1]);
    TArrow* arrow = new TArrow(meanx - 2*(r1*cos(anglerad)), meany - 2*(r1*sin(anglerad)), meanx + 2*(r1*cos(anglerad)), meany + 2*(r1*sin(anglerad)), 0.01, "|"); // "|>" option gives an arrowhead
    TArrow* arrow1 = new TArrow(meanx - 2*(r2*cos(anglerad+1.5708)), meany - 2*(r2*sin(anglerad+1.5708)), meanx + 2*(r2*cos(anglerad+1.5708)), meany + 2*(r2*sin(anglerad+1.5708)), 0.01, "|"); // "|>" option gives an arrowhead
    
    std::vector<int> CrossPoints = CheckCrossPoints(arrow, hcam_panel4,eventInfo);
    cout << "# Intersecting pixels with major axis: " << CrossPoints[0] << endl;
    cout << "# Non-intersecting pixels with major axis: " << CrossPoints[1] << endl;
    arrow->SetLineColor(kRed); // Optional: Set the color of the arrow
    arrow->Draw("SAME");             // Draw the arrow on the same canvas
    arrow1->SetLineColor(kBlue); // Optional: Set the color of the arrow
    arrow1->Draw("SAME");             // Draw the arrow on the same canvas
    
    TEllipse* ell = new TEllipse(meanx, meany,r1,r2,0,360,angledeg);
    ell->SetFillColorAlpha(kGreen,0.00);
    ell->Draw("SAME");
    TLatex* title = new TLatex();
    title->SetNDC(); // Set to Normalized Device Coordinates (NDC)
    title->SetTextSize(0.03);
    
    float areaEllipse = (3.8/16) * (3.8/16) * (cev->GetSurvivingPixelPanel3().size()); // area of an ellipse
    double conc = (cev->GetTotalCoreAmp())/cev->GetSurvivingPixelTotalAmpPanel3();
    
    
    std::vector<double> M3LongVar = getM3Long(meanx, meany, cev->GetSurvivingPixelPanel3(),cev->GetAmplitudeValuesTimeBin());
    std::string M3Longx = "+";
    if (M3LongVar[0] < 0){
        M3Longx = "-";
    } else if (M3LongVar[0] == 0){
        M3Longx = "0";
    }
    std::string M3Longy = "+";
    if (M3LongVar[1] < 0){
        M3Longy = "-";
    } else if (M3LongVar[1] == 0){
        M3Longy = "0";
    }
    // cev->GetSurvivingPixelTotalAmpPanel3()/util->GetADCtoPEratio(),
    title->DrawLatex(0.1, 0.92, Form("WL:%.2f Pixels:%i Area:%.2f Size:%.2f Conc:%.2f M3Long:(%s,  %s)",
        EllipicRatio[2],
        static_cast<int>((cev->GetSurvivingPixelPanel3()).size()),
        areaEllipse,
        cev->GetSurvivingPixelTotalAmpPanel3(),
        conc,
        M3Longx.data(),
        M3Longy.data()));
    delete title;

    // add values  for creating the plots
    plothelp->AddtoWL(EllipicRatio[2]);
    plothelp->AddtoL(EllipicRatio[0]);
    plothelp->AddtoW(EllipicRatio[1]);
    plothelp->AddtoSize(cev->GetSurvivingPixelTotalAmpPanel3());
    plothelp->AddtoSurvivingPixelCount(cev->GetSurvivingPixelPanel3().size());
    plothelp->AddtoConcentation(conc);
    plothelp->AddtoCOGx(meanx);
    plothelp->AddtoCOGy(meany);
    plothelp->AddtoTriggeredPixelsID(cev->GetMaxAmplitudePixelID());
    plothelp->AddtoCoreRatio(cev->GetCoreRatio());
    plothelp->AddtoNumberOfCores(cev->GetNumberofCorePixels());
    
    // add info for all the events
    eventInfo->SetHPanel4(hcam_panel4);
    eventInfo->SetL(EllipicRatio[0]);
    eventInfo->SetW(EllipicRatio[1]);
    eventInfo->SetWLRatio(EllipicRatio[2]);
    eventInfo->SetAngle(anglerad);
    eventInfo->Setr1(r1);
    eventInfo->Setr2(r2);
    eventInfo->SetSize(cev->GetSurvivingPixelTotalAmpPanel3());
    eventInfo->SetSurvivngPixels(cev->GetSurvivingPixelPanel3().size());
    eventInfo->SetConc(conc);
    eventInfo->SetCOGx(meanx);
    eventInfo->SetCOGy(meany);
    eventInfo->SetTriggeredPixelID(cev->GetMaxAmplitudePixelID());
    eventInfo->SetCoreRatio(cev->GetCoreRatio());
    eventInfo->SetNumberOfCores(cev->GetNumberofCorePixels());
    eventInfo->SetArea(areaEllipse);
    eventInfo->SetM3Longx(M3Longx.data());
    eventInfo->SetM3Longy(M3Longy.data());



            
}

std::vector<double> getM3Long(double xcog,double ycog, std::vector<int> sur_pix, std::vector<float> amps){
    // N = Max number of pixels survived
    // i = pixel index
    // x = pixel x location
    // x cog = x comp. Center of Gravity
    // q = charge of that pixel (start with amplitude for us)
    std::vector<double> M3Long = {0.0,0.0};
    int psize = sur_pix.size();

    cout << "xcog: " << xcog << " ycog: "<< ycog << endl;
    
    std::vector<double> randomNumbers;
    for(int p = 0; p < psize; p++) {
        // randomNumbers = generateRandomNumbers();
        int nx, ny;
        plottools->FindBin(sur_pix[p], &nx, &ny);
        double p1x = std::pow((nx*1.0 - xcog*1.0),3.0);
        double p1y = std::pow((ny*1.0 - ycog*1.0),3.0);
        cout << "nx: " << nx << " ny: " << ny << endl;
        // cout << (ny*1.0 - ycog*1.0) << endl;
        double p2x = std::pow(amps[sur_pix[p]],3.0)*p1x;
        double p2y = std::pow(amps[sur_pix[p]],3.0)*p1y;
        // cout << amps[sur_pix[p]] << endl;
        cout << "p2x: " << p2x << " p2y: " << p2y << endl;
        M3Long[0] = M3Long[0] + (p2x);
        M3Long[1] = M3Long[1] + (p2y);
        cout << "M3 summingx: " << M3Long[0] << endl;
        cout << "M3 summingy: " << M3Long[1] << endl;
    }
    // cout << "M3 summing2x: " << M3Long[0] << endl;
    // cout << "M3 summing2y: " << M3Long[1] << endl;
    // cout << psize << endl;
    // double p3x = M3Long[0] / psize;
    // double p3y = M3Long[1] / psize;
    M3Long[0] = std::pow((M3Long[0]/psize),0.333333333);
    M3Long[1] = std::pow((M3Long[0]/psize),0.333333333);
    
    // cout << "M3Long Complete: " << M3Long[0] << endl;
    // cout << "M3Long Complete: " << M3Long[1] << endl;
    return M3Long;
}