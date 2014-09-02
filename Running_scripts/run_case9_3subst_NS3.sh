#!/bin/bash

clear

tab="--tab"
cmd[0]="bash -c 'cd /home/mari009/fncs_matpower/fenix/ && ./fncsbroker 5';bash"
cmd[1]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./first LinkModel_CASE9_3_feeders.txt';bash"
cmd[2]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE9_NS3_S1_B7_H250.glm';bash"
cmd[3]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE9_NS3_S2_B5_H150.glm';bash"
cmd[4]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./gridlabd Run_CASE9_NS3_S3_B9_H200.glm';bash"
cmd[5]="bash -c 'cd /home/mari009/fncs_matpower/simbins/bin/ && ./start_MATPOWER case9_3subst_NS3.m real_power_demand_case9_T.txt';bash"
toRun=""

for i in 0 1 2 3 4 5; do
	toRun+=($tab -e "${cmd[i]}" "&")
done

echo ${toRun[@]}
# echo ${path[@]}

gnome-terminal "${toRun[@]}"

exit 0
