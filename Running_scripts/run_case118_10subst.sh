#!/bin/bash

clear

tab="--tab"
cmd[0]="bash -c 'cd /home/mari009/fncs_matpower/fenix/ && ./fncsbroker 11';bash"
cmd[1]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S1_B67_H150.glm';bash"
cmd[2]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S2_B84_H240.glm';bash"
cmd[3]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S3_B33_H210.glm';bash"
cmd[4]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S4_B57_H300.glm';bash"
cmd[5]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S5_B115_H450.glm';bash"
cmd[6]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S6_B43_H420.glm';bash"
cmd[7]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S7_B98_H180.glm';bash"
cmd[8]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S8_B79_H750.glm';bash"
cmd[9]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S9_B47_H810.glm';bash"
cmd[10]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE118_noNS3_S10_B118_H900.glm';bash"
cmd[11]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./start_MATPOWER case118.m real_power_demand_case118_T.txt';bash"
toRun=""

for i in 0 1 2 3 4 5 6 7 8 9 10 11; do
	toRun+=($tab -e "${cmd[i]}" "&")
done

echo ${toRun[@]}
# echo ${path[@]}

gnome-terminal "${toRun[@]}"

exit 0
