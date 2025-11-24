#!/bin/bash
# run_condor.sh

source /opt/root/bin/thisroot.sh

# Setup ExACT
export EXACT_DIR=/exact
export LD_LIBRARY_PATH=/exact/dict:/usr/lib/oracle/21/client64/lib:/usr/local/lib:$LD_LIBRARY_PATH


DATE=$1
FILENAME=$2
echo "Running job for $DATE with file $FILENAME "
echo "Directories"
ls -lh

# Create necessary directories if they don't exist
mkdir -p DataAnalysis/MergedData/Output/$DATE/
mkdir -p DataAnalysis/flasher_calibration/Output/

mv Merged_$FILENAME DataAnalysis/MergedData/Output/$DATE/
echo "Running flasher calibration for $DATE and File $FILENAME"
# ldd ./FileMerge
./FlasherCalibration $DATE y $FILENAME

cd 
cd DataAnalysis/flasher_calibration/Output/
chmod 774 *.root
pwd
ls -lh

cd
pwd
ls -lh