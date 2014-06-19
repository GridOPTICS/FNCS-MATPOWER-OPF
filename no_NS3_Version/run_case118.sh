#!/bin/bash

clear

tab="--tab"
cmd[0]="bash -c 'cd /home/laurentiu/work/fenix/ && ./fncsbroker 4';bash"
cmd[1]="bash -c 'cd /home/laurentiu/work/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S1_600.glm';bash"
cmd[2]="bash -c 'cd /home/laurentiu/work/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S2_480.glm';bash"
cmd[3]="bash -c 'cd /home/laurentiu/work/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S3_720.glm';bash"
cmd[4]="bash -c 'cd /home/laurentiu/work/simbins/bin/ && ./start_MATPOWER case118.m real_power_demand_case118.txt';bash"
toRun=""

for i in 0 1 2 3 4; do
	toRun+=($tab -e "${cmd[i]}")
done

echo ${foo[@]}
# echo ${path[@]}

gnome-terminal "${toRun[@]}"

exit 0
