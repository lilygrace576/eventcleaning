#!/bin/bash
# run_script.sh
# changed file paths to /home/lilyg/...

DATE=$1
FILENAME=$2
echo "Running for $DATE"
apptainer exec --bind /home/lilyg/TrinityDemonstrator:/mnt /home/lilyg/TrinityDemonstrator/DataAnalysis/containers/rootandexact.sif /mnt/DataAnalysis/event_cleaning/ClusterCleaning/EventCleaning $DATE y $FILENAME    
