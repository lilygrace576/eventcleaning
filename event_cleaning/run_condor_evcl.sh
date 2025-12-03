#!/bin/bash
# run_condor.sh

source /opt/root/bin/thisroot.sh

# Setup ExACT
export EXACT_DIR=/exact
export LD_LIBRARY_PATH=/exact/dict:/usr/lib/oracle/21/client64/lib:/usr/local/lib:$LD_LIBRARY_PATH


DATE=$1
FILENAME=$2
theUser=$3
# add folder arg
echo "Running job for $DATE with file $FILENAME "
echo "Directories"
ls -lh

# Create necessary directories if they don't exist
# add folder arg to path 
mkdir -p DataAnalysis/MergedData/Output/$FOLDER/$DATE/
mkdir -p DataAnalysis/event_cleaning/ClusterCleaning/
mkdir -p DataAnalysis/event_cleaning/Output/
# add folder arg to path 
mkdir -p DataAnalysis/flasher_calibration/$FOLDER/Output/
mkdir -p DataAnalysis/event_cleaning/Output/

# add folder arg to path 
mv $FILENAME DataAnalysis/MergedData/Output/$FOLDER/$DATE/
mv neighbors DataAnalysis/event_cleaning/ClusterCleaning/
# mv EventInfo.h DataAnalysis/event_cleaning/ClusterCleaning/
# mv EventInfoDict_rdict.pcm DataAnalysis/event_cleaning/ClusterCleaning/
# mv LinkDef.h DataAnalysis/event_cleaning/ClusterCleaning/
# mv EventCleaning DataAnalysis/event_cleaning/ClusterCleaning/

# add folder arg to path 
mv ${DATE}_FlasherCalibration_Factor.root DataAnalysis/flasher_calibration/Output/$FOLDER/

echo "Running eventcleaning for $DATE and File $FILENAME"
# ldd ./FileMerge

# add folder arg 
# ~/ ??
./EventCleaning $DATE ~/ $FILENAME ~/ $FOLDER

# # sleep 10
# echo "Running Calibration for $DATE and File $FILENAME"
# ./AddCalibData $DATE /srv/ Merged_$FILENAME
ls -lh
# cd DataAnalysis/event_cleaning/Output/
# rm *.pdf
# rm *.root
# for f in *.root; do
#     [ -e "$f" ] || continue  # skip if no .root files
#     time=$(echo "$FILENAME" | grep -oP '\d{4}-\d{2}-\d{2}T\K\d{2}:\d{2}')
#     echo "Time extracted: $time"
#     newname="${f%.root}_$time.root"
#     echo "Renaming: $f → $newname"
#     mv "$f" "$newname"
# done

cd DataAnalysis/event_cleaning/Output/
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

# time=$(echo "$FILENAME" | grep -oP '\d{4}-\d{2}-\d{2}T\K\d{2}:\d{2}')
# echo "Time extracted: $time"
# newname="${filename%.root}_$time.root"
# mv "$filename" "$newname"
ls -lh
pwd
cd
