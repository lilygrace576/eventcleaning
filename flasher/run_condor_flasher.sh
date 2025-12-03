#!/bin/bash
# run_condor.sh

source /opt/root/bin/thisroot.sh

# Setup ExACT
export EXACT_DIR=/exact
export LD_LIBRARY_PATH=/exact/dict:/usr/lib/oracle/21/client64/lib:/usr/local/lib:$LD_LIBRARY_PATH


DATE=$1
FILENAME=$2
##
theUser=$3
##
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
mv DataFiles ${theUser}_DataFiles
cd ${theUser}_DataFiles
pwd
for f in *.root; do
    [ -e "$f" ] || continue  # skip if no .root files
    time=$(echo "$FILENAME" | grep -oP '\d{4}-\d{2}-\d{2}T\K\d{2}:\d{2}')
    echo "Time extracted: $time"
    newname="${f%.root}_$time.root"
    echo "Renaming: $f → $newname"
    mv "$f" "$newname"
done
##
pwd
ls -lh

cd
pwd
ls -lh