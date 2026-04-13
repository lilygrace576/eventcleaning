#!/bin/bash
# run_condor.sh

source /opt/root/bin/thisroot.sh

# Setup ExACT
export EXACT_DIR=/exact
export LD_LIBRARY_PATH=/exact/dict:/usr/lib/oracle/21/client64/lib:/usr/local/lib:$LD_LIBRARY_PATH


DATE=$1

echo "Running job for $DATE with file  "
echo "Directories"
ls -lh
# Create necessary directories if they don't exist
mkdir -p DataAnalysis/MergedData/Output/
mkdir -p DataAnalysis/flasher_calibration/Output/

mv $DATE DataAnalysis/MergedData/Output/.
echo "Moved $DATE to DataAnalysis/MergedData/Output/"
cd DataAnalysis/MergedData/Output/
ls -lh 
cd $DATE
pwd
ls -lh
cd $_CONDOR_SCRATCH_DIR
mv FlasherCalibration DataAnalysis/flasher_calibration/.
echo "Running flasher calibration for $DATE and File "
cd DataAnalysis/flasher_calibration/
ls -lh
./FlasherCalibration $DATE $_CONDOR_SCRATCH_DIR/
ls -lh

cd $_CONDOR_SCRATCH_DIR
cd DataAnalysis/flasher_calibration/Output/
cd $_CONDOR_SCRATCH_DIR
ls -lh
# Copy results back to scratch
cp -r DataAnalysis/flasher_calibration/Output .

pwd
ls -lh