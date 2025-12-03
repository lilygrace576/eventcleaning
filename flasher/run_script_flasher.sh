#!/bin/bash
# run_script.sh

DATE=$1
FILE=$2
# add folder arg
FOLDER=$3
echo "Running for $DATE"
# add folder arg input
apptainer exec --bind /storage/osg-otte1/shared/TrinityDemonstrator:/mnt /storage/osg-otte1/shared/TrinityDemonstrator/DataAnalysis/containers/rootandexact.sif /mnt/DataAnalysis/flasher_calibration/FlasherCalibration $DATE y $FILE $FOLDER

# sleep 10
