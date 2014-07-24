#!/bin/bash

clear

tab="--tab"
cmd[0]="bash -c 'cd /home/mari009/fncs_matpower/fenix/ && ./fncsbroker 4';bash"
cmd[1]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S1_B67_H150.glm';bash"
cmd[2]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S2_B84_H240.glm';bash"
cmd[3]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S3_B33_H210.glm';bash"
cmd[4]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./start_MATPOWER case118_3subst.m real_power_demand_case118_T.txt';bash"
toRun=""

for i in 0 1 2 3 4; do
	toRun+=($tab -e "${cmd[i]}" "&")
done

echo ${toRun[@]}
# echo ${path[@]}

gnome-terminal "${toRun[@]}"

exit 0
