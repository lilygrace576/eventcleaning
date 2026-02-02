#!/bin/bash
# run_condor.sh

source /opt/root/bin/thisroot.sh

# Setup ExACT
export EXACT_DIR=/exact
export LD_LIBRARY_PATH=/exact/dict:/usr/lib/oracle/21/client64/lib:/usr/local/lib:$LD_LIBRARY_PATH

# changed file paths to /home/lilyg/...

DATE=$1
FILENAME=$2

echo "Running job for $DATE with file "
# $FILENAME"

echo "Directories"
pwd
ls -lh


# Create necessary directories if they don't exist

mkdir -p DataAnalysis/MergedData/Output/$DATE/
mkdir -p DataAnalysis/event_cleaning/ClusterCleaning/
mkdir -p DataAnalysis/flasher_calibration/Output/
mkdir -p DataAnalysis/event_cleaning/Output/

# period at end or no?
mv $DATE DataAnalysis/MergedData/Output/
echo "Moved $DATE to DataAnalysis/MergedData/Output/"
cd DataAnalysis/MergedData/Output/$DATE/
pwd
ls -lh
cd 

mv neighbors DataAnalysis/event_cleaning/ClusterCleaning/
echo "Moved neighbors to DataAnalysis/event_cleaning/ClusterCleaning/"
cd DataAnalysis/event_cleaning/ClusterCleaning/
pwd
ls -lh
cd

# added . to end of path ?
mv ${DATE}_FlasherCalibration_Factor.root DataAnalysis/flasher_calibration/Output/
echo "moving flasher calibration root files to DataAnalysis/flasher_calibration/Output/"
cd DataAnalysis/flasher_calibration/Output/
pwd
ls -lh
cd

##
mv EventCleaning DataAnalysis/event_cleaning/ClusterCleaning/EventCleaning
mv EventInfo.h DataAnalysis/event_cleaning/ClusterCleaning/EventInfo.h
mv EventInfoDict_rdict.pcm DataAnalysis/event_cleaning/ClusterCleaning/EventInfoDict_rdict.pcm
mv LinkDef.h DataAnalysis/event_cleaning/ClusterCleaning/LinkDef.h

echo "Running eventcleaning for $DATE and File "
# $FILENAME"
##

##
# mv EventCleaning DataAnalysis/event_cleaning/.

##
cd DataAnalysis/event_cleaning/ClusterCleaning/
pwd
ls -lh
chmod +x EventCleaning
./EventCleaning $DATE ~/ $FILENAME


cd 
cd DataAnalysis/event_cleaning/Output/
pwd
ls -lh

mv DataFiles ${USER}_DataFiles/
cd ${USER}_DataFiles
pwd
touch ${DATE}_test.txt
ls -lh

pwd
for f in *.root; do
    [ -e "$f" ] || continue  # skip if no .root files
    time=$(echo "$FILENAME" | grep -oP '\d{4}-\d{2}-\d{2}T\K\d{2}:\d{2}')
    echo "Time extracted: $time"
    newname="${f%.root}_$time.root"
    echo "Renaming: $f → $newname"
    mv "$f" "$newname"
done

ls -lh

# cp -r DataAnalysis/event_cleaning/Output .

pwd
ls -lh
