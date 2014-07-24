clear;
clc;

%% Parameters

rng(11);
matpower_case = '118'; % MATPOWER CASE
% feeder_buses = [67, 84, 33, 57, 115, 43, 98, 79, 47, 118];
feeder_buses = [67, 84, 33];
% no_of_houses_mat = [150, 240, 210, 300, 450, 420, 180, 750, 810, 900]; % Divided across 3 phases, so a multiple of 3 is good, but not necessary
                             % nomber of houses for each feeder
no_of_houses_mat = [150, 240, 210];
no_of_feeders = length(feeder_buses); % number of feeders

%% NS3? YAY or NAY?
use_ns3 = 1; % toggle NS3 ON/OFF
link_file_name = ['LinkModel_', num2str(no_of_feeders), '_feeders.txt']; % name of the link file needed when running with NS3
if (use_ns3 == 1),
    link_file = fopen(link_file_name, 'w');
end
no_of_other_houses_mat = zeros(1, no_of_feeders); % homes that are not attached to the same system (both power and market)
pen_perc = ones(1, no_of_feeders); % penetration percentage
Case = 'rt'; % Fixed or rt or da or Merged or test; FIXED = base case, no markets; RT = includes markets
dir = '.\'; % Destination folder
% ========= 2014/05/12 =============================
power_conv_value = 1000; % convergence value in W; I have set this up here to make it easier to modify for Vis Demo testing

for ind = 1:no_of_feeders,
    no_of_houses = no_of_houses_mat(ind); % number of houses for current feeder
    no_of_other_houses = no_of_other_houses_mat(ind);
    %% MARKET 
        % T/F - 'MARKET'=TRUE; 
        % market name, 
        % period, 
        % mean, 
        % stdev,
        % percent penetration,
        % slider setting (range: 0 - 1), 
        % here 
        % destination for controllers
    market_info = { 'MARKET';
                    ['Market', num2str(ind)];
                    300;
                    'current_price_mean_24h'; % 'current_price_mean_24h';
                    'current_price_stdev_24h'; % 'current_price_stdev_24h';
                    pen_perc(ind);
                    1;
                    0;
                    ['MarkNIF', num2str(ind)]};

    %% SUBSTATIONCOM
        % name
        % bus number to which it is connected
    substationcom_info = { ['SUBSTATIONCOM', num2str(ind)];
                           feeder_buses(ind)};

    %% TAG
        % feeder name
    tag_info = { ['F', num2str(ind)] };
    %%%%%%%%%%%%%%%%% Shouldn't have to change after here %%%%%%%%%%%%%%%%%%%%%
    %% Create the files to write to 
    if (use_ns3 == 1),
        my_name = ['CASE', matpower_case, '_NS3_S', num2str(ind),...
            '_B', num2str(feeder_buses(ind)), '_H', num2str(no_of_houses)];
        % write in the NS3 link file
        link_file = fopen(link_file_name, 'a');
        fprintf(link_file, [num2str(no_of_houses_mat(ind)), ' ', market_info{9}, ' ', tag_info{1},...
            '_single_ramp_NIF_ ', num2str(feeder_buses(ind)), '\n']);
    else,
        my_name = ['CASE', matpower_case, '_noNS3_S', num2str(ind),...
            '_B', num2str(feeder_buses(ind)), '_H', num2str(no_of_houses)];
    end
    %my_name = [Case,'_ss',num2str(market_info{7}*100),'_s',num2str(sigma_tstat),'_c',num2str(capacity_limit),'_p',num2str(market_info{6}*100),'_db',num2str(use_deadband),'_o',num2str(use_override)];
    filename = [dir,'Run_',my_name,'.glm'];
    write_file = fopen(filename,'w');

    %to be removed
    count1 = 0;
    count2 = 0;
    count3 = 0;
    count4 = 0;
    count5 = 0;
    count6 = 0;      
    count7 = 0;         
    %to be removed  

    slider_setting_array = zeros(no_of_houses, 2);

    use_billing = 0;
    use_fncs = 1; % toggle FNCS ON/OFF
    use_congestion = 1;
    use_deadband = 1;
    use_override = 1;



    add_unresp = 1;

    decrease_heating_setpoint=0;

    desired_house_index_array = [13;28;47;58;77;100;226;246;253;278];
    desired_house_index = 0;

    if (use_congestion ~= 0)
        capacity_limit = 1100;%%%3.6*1425; % kW   
        use_congestion_rebate = 'FALSE';
        show_congestion_rebates_incentives = 'FALSE';
        use_congestion_incentive = 'FALSE';
    else
        capacity_limit = 0;
        use_congestion_rebate = 'FALSE';
        show_congestion_rebates_incentives = 'FALSE';
        use_congestion_incentive = 'FALSE';
    end
    sigma_tstat = 2; 

    if (use_billing == 1)
        monthly_flat_fee = 21.4375999; % 6.350645416; % $
        monthly_rt_fee = 21.4375999; % 6.350645416; % 
        Fixed_First_Tier_Energy = 800;
        Fixed_Second_Tier_Energy = 2000;
        Fixed_Third_Tier_Energy = 15000;
        rt_First_Tier_Energy = 800;
        rt_Second_Tier_Energy = 2000;
        rt_Third_Tier_Energy = 15000;   
    end

    if (strcmp(Case,'Fixed')~=0)
        market_info{1} = '';
    end


    %% Make sure it's only psuedo-randomized, but repeatable
    s2 = RandStream.create('mrg32k3a','NumStreams',3,'StreamIndices',2);
    if ( verLessThan('matlab','8.1') )
        RandStream.setDefaultStream(s2);
    else
        RandStream.setGlobalStream(s2);
    end

    %% All of the header stuff
    fprintf(write_file,'clock {\n');
    fprintf(write_file,'     timezone EST+5EDT;\n');
    fprintf(write_file,'     starttime ''2009-07-21 00:00:00'';\n');
    fprintf(write_file,'     stoptime ''2009-07-23 00:01:00'';\n');
    % fprintf(write_file,'     stoptime ''2009-06-17 21:59:59'';\n');
    % fprintf(write_file,'     timestamp ''2013-07-14 00:00:00'';\n');
    % fprintf(write_file,'     stoptime ''2013-07-21 00:01:00'';\n');
    % fprintf(write_file,'     timestamp ''2013-12-08 00:00:00'';\n');
    % fprintf(write_file,'     stoptime ''2013-12-15 00:01:00'';\n');
    % fprintf(write_file,'     timestamp ''2013-04-07 00:00:00'';\n');
    % fprintf(write_file,'     stoptime ''2013-04-14 00:01:00'';\n');
    % fprintf(write_file,'     timestamp ''2013-09-29 00:00:00'';\n');
    % fprintf(write_file,'     stoptime ''2013-10-06 00:01:00'';\n');
    fprintf(write_file,'}\n\n');

    fprintf(write_file,'#set profiler=1;\n');
    fprintf(write_file,'#set double_format=%%+.12lg\n');
    fprintf(write_file,'#set randomseed=10;\n');
    fprintf(write_file,'#set relax_naming_rules=1;\n');
    % ============================== 2014/05/12 ===========================
    % fprintf(write_file,'#set minimum_timestep=1;\n'); % '#set minimum_timestep=60;\n');
    fprintf(write_file,'#set minimum_timestep=5;\n');
    %
    % ========================================================
    fprintf(write_file,'#include "water_and_setpoint_schedule_v3.glm";\n\n');
    fprintf(write_file,'#include "appliance_schedules.glm";\n\n');


    if (use_congestion == 1)
        fprintf(write_file,'#include "cap_ref_obj.glm";\n\n');
    end

    if (use_billing == 0)
    %     fprintf(write_file,'#include "fixed_rate_schedule.glm";\n\n');
    end


    fprintf(write_file,'module market;\n');
    fprintf(write_file,'module tape;\n');
    if(use_fncs == 1)
        fprintf(write_file,'module comm;\n');
    end
    fprintf(write_file,'module climate;\n');
    fprintf(write_file,'module residential {\n');
    fprintf(write_file,'     implicit_enduses NONE;\n');
    fprintf(write_file,'};\n');
    fprintf(write_file,'module powerflow {\n');
    fprintf(write_file,'     solver_method FBS;\n');
    fprintf(write_file,'     NR_iteration_limit 100;\n');
    fprintf(write_file,'};\n\n');

    fprintf(write_file,'class player {\n');
    fprintf(write_file,'     double value;\n'); 
    fprintf(write_file,'}\n');

    % fprintf(write_file,'object player {\n');
    % fprintf(write_file,'    name SNOWBIRD;\n');
    % fprintf(write_file,'    file AEP_SnowBird_2009.player;\n'); 
    % fprintf(write_file,'    loop 1;\n');
    % fprintf(write_file,'}\n');


    fprintf(write_file,'object player {\n');
    fprintf(write_file,'     name phase_A_load;\n');
    fprintf(write_file,'     file phase_A.player;\n'); 
    fprintf(write_file,'     loop 1;\n');
    fprintf(write_file,'}\n');

    fprintf(write_file,'object player {\n');
    fprintf(write_file,'     name phase_B_load;\n');
    fprintf(write_file,'     file phase_B.player;\n'); 
    fprintf(write_file,'     loop 1;\n');
    fprintf(write_file,'}\n');

    fprintf(write_file,'object player {\n');
    fprintf(write_file,'     name phase_C_load;\n');
    fprintf(write_file,'     file phase_C.player;\n'); 
    fprintf(write_file,'     loop 1;\n');
    fprintf(write_file,'}\n');


    fprintf(write_file,'object csv_reader {\n');
    fprintf(write_file,'     name CSVREADER;\n');
    fprintf(write_file,'     filename ColumbusWeather2009_2a.csv;\n');
    fprintf(write_file,'}\n\n');

    fprintf(write_file,'object climate {\n');
    fprintf(write_file,'     name "Columbus OH";\n'); % "Minneapolis MN";\n');
    fprintf(write_file,'     tmyfile ColumbusWeather2009_2a.csv;\n');
    fprintf(write_file,'     reader CSVREADER;\n');
    fprintf(write_file,'};\n\n');

    if (strcmp(Case,'Fixed')==0)  
        fprintf(write_file,'class auction {\n');
        fprintf(write_file,'     double day_ahead_average;\n');
        fprintf(write_file,' 	 double day_ahead_std;\n');
        fprintf(write_file,' 	 double %s;\n',market_info{4});
        fprintf(write_file,'     double %s;\n',market_info{5});
        fprintf(write_file,'}\n');

        fprintf(write_file,'object auction {\n');
        fprintf(write_file,'     name %s;\n',market_info{2});
        fprintf(write_file,'     unit kW;\n'); % unit kW;\n');
        fprintf(write_file,'     period %.0f;\n',market_info{3});
        fprintf(write_file,'     price_cap 3.78;\n'); % price_cap 3.78;\n');
        fprintf(write_file,'     transaction_log_file log_file_%s.csv;\n',my_name);
        fprintf(write_file,'     curve_log_file bid_curve_%s.csv;\n',my_name);
        fprintf(write_file,'     curve_log_info EXTRA;\n');
        if(use_fncs == 1)
            fprintf(write_file,'     object market_network_interface {\n');
            if (use_ns3 == 1),
                fprintf(write_file, '          send_broadcast true;\n'); % default should be false, setup from GridLAB-D
            end
            fprintf(write_file,'          market_id_prop market_id;\n');
            fprintf(write_file,'          name %s;\n', market_info{9});
            fprintf(write_file,'          average_price_prop %s;\n',market_info{4});
            fprintf(write_file,'          stdev_price_prop %s;\n',market_info{5});
            % ==============================================================================
            % ===================== 2014/04/29 ============================================
            fprintf(write_file,'          adjust_price_prop capacity_reference_bid_price;\n');
            % fprintf(write_file,'          adjust_price_prop adjust_price;\n');
            fprintf(write_file,'     };\n');
        end
        fprintf(write_file,'     object recorder {\n');
        fprintf(write_file,'          property current_market.clearing_quantity;\n');
        fprintf(write_file,'          limit 100000000;\n');
        fprintf(write_file,'          interval 300;\n');
        fprintf(write_file,'          file clearedquantity_%s.csv;\n',my_name);
        fprintf(write_file,'     };\n');
        fprintf(write_file,'     object recorder {\n');
        fprintf(write_file,'          property base_price;\n');
        fprintf(write_file,'          limit 100000000;\n');
        fprintf(write_file,'          interval 300;\n');
        fprintf(write_file,'          file LMPbaseprice_%s.csv;\n',my_name);
        fprintf(write_file,'     };\n');

        if (use_congestion == 0)
            fprintf(write_file,'     special_mode BUYERS_ONLY;\n');
        end

        if (strcmp(Case,'rt')==1 && use_congestion == 0)
            %{
            fprintf(write_file,'     object player {\n');
            fprintf(write_file,'         property fixed_price;\n');
            fprintf(write_file,'         file AEP_RT_LMP.player;\n'); 
            fprintf(write_file,'         loop 1;\n');
            fprintf(write_file,'     };\n');
            %}
            % ==============================================================================
            % ===================== 2014/04/29 ============================================
            % fprintf(write_file,'     fixed_price 0;\n');
            %{
            fprintf(write_file,'     object recorder {\n');
            fprintf(write_file,'          property fixed_price;\n');
            fprintf(write_file,'          limit 100000000;\n');
            fprintf(write_file,'          interval 300;\n');
            fprintf(write_file,'          file baseprice_%s.csv;\n',my_name);
            fprintf(write_file,'     };\n');
            fprintf(write_file,'     object recorder {\n');
            fprintf(write_file,'          property current_market.clearing_price;\n');
            fprintf(write_file,'          limit 100000000;\n');
            fprintf(write_file,'          interval 300;\n');
            fprintf(write_file,'          file clearedprice_%s.csv;\n',my_name);
            fprintf(write_file,'     };\n');
            fprintf(write_file,'     object recorder {\n');
            fprintf(write_file,'          property %s;\n', market_info{4});
            fprintf(write_file,'          limit 100000000;\n');
            fprintf(write_file,'          interval 300;\n');
            fprintf(write_file,'          file avgprice_%s.csv;\n',my_name);
            fprintf(write_file,'     };\n');  
            fprintf(write_file,'     object recorder {\n');
            fprintf(write_file,'          property %s;\n', market_info{5});
            fprintf(write_file,'          limit 100000000;\n');
            fprintf(write_file,'          interval 300;\n');
            fprintf(write_file,'          file stdprice_%s.csv;\n',my_name);
            fprintf(write_file,'     };\n');
            %}
            fprintf(write_file,'     object recorder {\n');
            fprintf(write_file,'          property %s,%s,capacity_reference_bid_price,current_market.clearing_price,current_market.clearing_quantity,adjusted_price;\n', market_info{4}, market_info{5});
            fprintf(write_file,'          limit 100000000;\n');
            fprintf(write_file,'          interval 60;\n');
            fprintf(write_file,'          file baseprice_clearedprice_clearedquantity_%s.csv;\n',my_name);
            fprintf(write_file,'     };\n');
        elseif (strcmp(Case,'rt')==1 && use_congestion == 1)
		    % ===============================================================================
			% ========================== 2014/05/02 ========================================
            % fprintf(write_file,'     capacity_reference_object %s_Transformer1;\n', tag_info{1});
			fprintf(write_file,'     capacity_reference_object %s_Transformer1;\n', tag_info{1});
			% =======================================================================================
            fprintf(write_file,'     capacity_reference_property power_out_real;\n');
            fprintf(write_file,'     max_capacity_reference_bid_quantity 999999999;\n');
            % ==============================================================================
            % ===================== 2014/04/29 ============================================
            fprintf(write_file,'     capacity_reference_bid_price 0;\n');
            %{
            fprintf(write_file,'     object player {\n');
            fprintf(write_file,'          property capacity_reference_bid_price;\n');
            fprintf(write_file,'          file AEP_RT_LMP.player;\n'); 
            fprintf(write_file,'          loop 1;\n');
            fprintf(write_file,'     };\n');
            %}
            % ==============================================================================
            fprintf(write_file,'     object recorder {\n');
            fprintf(write_file,'          property %s,%s,capacity_reference_bid_price,current_market.clearing_price,current_market.clearing_quantity,adjusted_price;\n', market_info{4}, market_info{5});
            fprintf(write_file,'          limit 100000000;\n');
            fprintf(write_file,'          interval 60;\n');
            fprintf(write_file,'          file baseprice_clearedprice_clearedquantity_%s.csv;\n',my_name);
            fprintf(write_file,'     };\n');
    %         fprintf(write_file,'    object recorder {\n');
    %         fprintf(write_file,'        property current_market.clearing_price;\n');
    %         fprintf(write_file,'        limit 100000000;\n');
    %         fprintf(write_file,'        interval 300;\n');
    %         fprintf(write_file,'        file clearedprice_%s.csv;\n',my_name);
    %         fprintf(write_file,'    };\n');
    %         fprintf(write_file,'    object recorder {\n');
    %         fprintf(write_file,'        property current_price_mean_24h;\n');
    %         fprintf(write_file,'        limit 100000000;\n');
    %         fprintf(write_file,'        interval 300;\n');
    %         fprintf(write_file,'        file avgprice_%s.csv;\n',my_name);
    %         fprintf(write_file,'    };\n');  
    %         fprintf(write_file,'    object recorder {\n');
    %         fprintf(write_file,'        property current_price_stdev_24h;\n');
    %         fprintf(write_file,'        limit 100000000;\n');
    %         fprintf(write_file,'        interval 300;\n');
    %         fprintf(write_file,'        file stdprice_%s.csv;\n',my_name);
    %         fprintf(write_file,'    };\n');  

        end

        fprintf(write_file,'     init_price 0.042676;\n'); % init_price 0.042676;\n');
        fprintf(write_file,'     init_stdev 0.02;\n'); % init_stdev 0.02;\n');
        fprintf(write_file,'     use_future_mean_price FALSE;\n');
        fprintf(write_file,'     warmup 0;\n');
        fprintf(write_file,'}\n');
    end

    % Creates mini-distribution system
    fprintf(write_file,'object transformer_configuration {\n');                 
    fprintf(write_file,'     name substation_config;\n');                  
    fprintf(write_file,'     connect_type WYE_WYE;\n');
    fprintf(write_file,'     install_type PADMOUNT;\n');
    fprintf(write_file,'     primary_voltage 345000 V;\n'); % primary_voltage 7200 V;\n');
    fprintf(write_file,'     secondary_voltage 7200 V;\n');
    fprintf(write_file,'     power_rating %.0f;\n',3 * max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     powerA_rating %.0f;\n',max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     powerB_rating %.0f;\n',max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     powerC_rating %.0f;\n',max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     impedance 0.0015+0.00675j;\n');                  
    fprintf(write_file,'}\n\n');  

    fprintf(write_file,'object transformer_configuration {\n');
    fprintf(write_file,'     name default_transformer_A;\n');
    fprintf(write_file,'     connect_type SINGLE_PHASE_CENTER_TAPPED;\n');
    fprintf(write_file,'     install_type PADMOUNT;\n');
    fprintf(write_file,'     primary_voltage 7200 V;\n');
    fprintf(write_file,'     secondary_voltage 124 V;\n');
    fprintf(write_file,'     power_rating %.0f;\n',max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     powerA_rating %.0f;\n',max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     impedance 0.015+0.0675j;\n');
    fprintf(write_file,'}\n\n');

    fprintf(write_file,'object transformer_configuration {\n');
    fprintf(write_file,'     name default_transformer_B;\n');
    fprintf(write_file,'     connect_type SINGLE_PHASE_CENTER_TAPPED;\n');
    fprintf(write_file,'     install_type PADMOUNT;\n');
    fprintf(write_file,'     primary_voltage 7200 V;\n');
    fprintf(write_file,'     secondary_voltage 124 V;\n');
    fprintf(write_file,'     power_rating %.0f;\n',max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     powerB_rating %.0f;\n',max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     impedance 0.015+0.0675j;\n');
    fprintf(write_file,'}\n\n');

    fprintf(write_file,'object transformer_configuration {\n');
    fprintf(write_file,'     name default_transformer_C;\n');
    fprintf(write_file,'     connect_type SINGLE_PHASE_CENTER_TAPPED;\n');
    fprintf(write_file,'     install_type PADMOUNT;\n');
    fprintf(write_file,'     primary_voltage 7200 V;\n');
    fprintf(write_file,'     secondary_voltage 124 V;\n');
    fprintf(write_file,'     power_rating %.0f;\n',max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     powerC_rating %.0f;\n',max([no_of_houses,no_of_other_houses]) * 5);
    fprintf(write_file,'     impedance 0.015+0.0675j;\n');
    fprintf(write_file,'}\n\n');

    fprintf(write_file,'object triplex_line_conductor {\n');               
    fprintf(write_file,'     name Name_1_0_AA_triplex;\n');              
    fprintf(write_file,'     resistance 0.57;\n');                
    fprintf(write_file,'     geometric_mean_radius 0.0111;\n');
    fprintf(write_file,'}\n\n');

    fprintf(write_file,'object triplex_line_configuration {\n');               
    fprintf(write_file,'     name TLCFG;\n');                
    fprintf(write_file,'     conductor_1 Name_1_0_AA_triplex;\n');
    fprintf(write_file,'     conductor_2 Name_1_0_AA_triplex;\n');
    fprintf(write_file,'     conductor_N Name_1_0_AA_triplex;\n');              
    fprintf(write_file,'     insulation_thickness 0.08;\n');                
    fprintf(write_file,'     diameter 0.368;\n');                
    fprintf(write_file,'}\n\n');

    if(use_fncs == 0)
        fprintf(write_file,'object meter {\n');
        fprintf(write_file,'     bustype SWING;\n');
        fprintf(write_file,'     name ROOT;\n');
        fprintf(write_file,'     phases ABCN;\n');
        fprintf(write_file,'     nominal_voltage 7200;\n');
        fprintf(write_file,'}\n\n');
    elseif(use_fncs == 1)
        fprintf(write_file,'object substation {\n');
        fprintf(write_file,'     bustype SWING;\n');
        fprintf(write_file,'     name substation_root;\n'); % name ROOT;\n');
        fprintf(write_file,'     nominal_voltage 345000;\n');
        fprintf(write_file,'     power_convergence_value %d;\n', power_conv_value);
        fprintf(write_file,'     reference_phase PHASE_A;\n');
        fprintf(write_file,'     phases ABCN;\n');
        fprintf(write_file,'     object transmissioncom {\n');
        fprintf(write_file,'          name %s;\n', substationcom_info{1});
        fprintf(write_file,'          connected_bus %d;\n', substationcom_info{2});
        fprintf(write_file,'          parent_voltage_property positive_sequence_voltage;\n');
        fprintf(write_file,'          parent_power_property distribution_load;\n');
        fprintf(write_file,'     };\n');
        fprintf(write_file,'    object recorder {\n');
        fprintf(write_file,'          property measured_real_power;\n');
        fprintf(write_file,'          property distribution_load.real;\n');
        fprintf(write_file,'          limit 100000000;\n');
        fprintf(write_file,'          interval 60;\n');
        fprintf(write_file,'          file substLoad_%s.csv;\n',my_name);
        fprintf(write_file,'     };\n');
        fprintf(write_file,'}\n\n');
    end

    for kind = 1:1 % loop for connected and not-connected homes

        if (kind == 1)
            tag = tag_info{1};
        else
            tag = 'other';
        end

        fprintf(write_file,'object transformer {\n');                 
        fprintf(write_file,'     phases ABCN;\n');
        fprintf(write_file,'     groupid %s_Network_Trans;\n',tag);
        fprintf(write_file,'     name %s_Transformer1;\n',tag);
        fprintf(write_file,'     from substation_root;\n');                  % from ROOT;\n');
        fprintf(write_file,'     to %s_transformer_meter;\n',tag);                 
        fprintf(write_file,'     configuration substation_config;\n');
        fprintf(write_file,'     object recorder {\n');
        fprintf(write_file,'          property power_in.real;\n'); % property power_out_real;\n');
        fprintf(write_file,'          limit 100000000;\n');
        fprintf(write_file,'          interval 300;\n'); % interval 60;\n');
        fprintf(write_file,'          file transLoad_%s.csv;\n',my_name); % file totalload_%s%s.csv;\n',my_name,tag);
        fprintf(write_file,'     };\n');
        fprintf(write_file,'}\n\n');

        fprintf(write_file,'object meter {\n');
        fprintf(write_file,'     name %s_transformer_meter;\n',tag);
        fprintf(write_file,'     phases ABCN;\n');
        fprintf(write_file,'     nominal_voltage 7200;\n');
        % fprintf(write_file,'     object recorder {\n');
        % fprintf(write_file,'          property measured_real_power,measured_reactive_power,measured_real_energy,measured_reactive_energy;\n');
        % fprintf(write_file,'          limit 100000000;\n');
        % fprintf(write_file,'          interval 300;\n');
        % fprintf(write_file,'          file totalload_%s.csv;\n',my_name);
        % fprintf(write_file,'     };\n');
        fprintf(write_file,'}\n\n');

        if (add_unresp == 1)
            fprintf(write_file,'object load {\n');
            fprintf(write_file,'     name %s_unresp_load;\n', tag); % name unresp_load;\n');
            fprintf(write_file,'     nominal_voltage 7200;\n');
            fprintf(write_file,'     phases ABCN;\n');
            fprintf(write_file,'     parent %s_transformer_meter;\n',tag);
            % =======================================================================================
            % ====== 2014/05/05 ==============================
            %
            %{
            fprintf(write_file,'     constant_power_A_real phase_A_load.value*0.0167;\n'); % constant_power_A_real phase_A_load.value*0.1;\n');
            fprintf(write_file,'     constant_power_B_real phase_B_load.value*0.0167;\n');
            fprintf(write_file,'     constant_power_C_real phase_C_load.value*0.0167;\n');
            %}
            fprintf(write_file,'     constant_power_A_real phase_A_load.value*%.4f;\n', no_of_houses*0.0167/50);
            fprintf(write_file,'     constant_power_B_real phase_B_load.value*%.4f;\n', no_of_houses*0.0167/50);
            fprintf(write_file,'     constant_power_C_real phase_C_load.value*%.4f;\n', no_of_houses*0.0167/50);
            %
            % =============================================================================================
            fprintf(write_file,'};\n\n');
        end

        fprintf(write_file,'object transformer {\n');
        fprintf(write_file,'     name %s_center_tap_transformer_A;\n',tag);
        fprintf(write_file,'     phases AS;\n');
        fprintf(write_file,'     from %s_transformer_meter;\n',tag);
        fprintf(write_file,'     to %s_triplex_node_A;\n',tag);
        fprintf(write_file,'     configuration default_transformer_A;\n');
        fprintf(write_file,'}\n\n');

        fprintf(write_file,'object transformer {\n');
        fprintf(write_file,'     name %s_center_tap_transformer_B;\n',tag);
        fprintf(write_file,'     phases BS;\n');
        fprintf(write_file,'     from %s_transformer_meter;\n',tag);
        fprintf(write_file,'     to %s_triplex_node_B;\n',tag);
        fprintf(write_file,'     configuration default_transformer_B;\n');
        fprintf(write_file,'}\n\n');

        fprintf(write_file,'object transformer {\n');
        fprintf(write_file,'     name %s_center_tap_transformer_C;\n',tag);
        fprintf(write_file,'     phases CS;\n');
        fprintf(write_file,'     from %s_transformer_meter;\n',tag);
        fprintf(write_file,'     to %s_triplex_node_C;\n',tag);
        fprintf(write_file,'     configuration default_transformer_C;\n');
        fprintf(write_file,'}\n\n');

        fprintf(write_file,'object triplex_meter {\n');
        fprintf(write_file,'     name %s_triplex_node_A;\n',tag);
        fprintf(write_file,'     phases AS;\n');
        fprintf(write_file,'     nominal_voltage 124.00;\n');
        fprintf(write_file,'}\n\n');

        fprintf(write_file,'object triplex_meter {\n');
        fprintf(write_file,'     name %s_triplex_node_B;\n',tag);
        fprintf(write_file,'     phases BS;\n');
        fprintf(write_file,'     nominal_voltage 124.00;\n');
        fprintf(write_file,'}\n\n');

        fprintf(write_file,'object triplex_meter {\n');
        fprintf(write_file,'     name %s_triplex_node_C;\n',tag);
        fprintf(write_file,'     phases CS;\n');
        fprintf(write_file,'     nominal_voltage 124.00;\n');
        fprintf(write_file,'}\n\n');

        if (kind == 1)
            total_houses = no_of_houses;
        else
            total_houses = no_of_other_houses;
            market_info{6} = 0;
        end	

        for jind = 1:total_houses
            if jind <= 1/3 * total_houses
                phase = 'A';
            elseif jind <= 2/3 * total_houses
                phase = 'B';
            else
                phase = 'C';
            end

            I = find(desired_house_index_array == jind);

            if(size(I)>=1)
                desired_house_index = desired_house_index_array(I);
            end

            fprintf(write_file,'object triplex_line {\n');
            fprintf(write_file,'     from %s_triplex_node_%s;\n',tag,phase);
            fprintf(write_file,'     to %s_tpm_flatrate_%s%d;\n',tag,phase,jind);
            fprintf(write_file,'     groupid %s_Triplex_Line;\n',tag);
            fprintf(write_file,'     phases %sS;\n',phase);
            fprintf(write_file,'     length 10;\n');
            fprintf(write_file,'     configuration TLCFG;\n');
            fprintf(write_file,'}\n\n');

            fprintf(write_file,'object triplex_meter {\n');   
            fprintf(write_file,'     nominal_voltage 124;\n');
            fprintf(write_file,'     phases %sS;\n',phase);
            fprintf(write_file,'     name %s_tpm_flatrate_%s%d;\n',tag,phase,jind);
            fprintf(write_file,'     groupid %s_flatrate_meter;\n',tag);
            if (use_billing == 1)
                fprintf(write_file,'     bill_mode TIERED;\n');
                fprintf(write_file,'     price FIXED_PRICE;\n');
                fprintf(write_file,'     first_tier_price FIXED_TIER1;\n');
                fprintf(write_file,'     second_tier_price FIXED_TIER2;\n');
                fprintf(write_file,'     third_tier_price FIXED_TIER3;\n');
                fprintf(write_file,'     first_tier_energy %d;\n', Fixed_First_Tier_Energy);
                fprintf(write_file,'     second_tier_energy %d;\n', Fixed_Second_Tier_Energy);
                fprintf(write_file,'     third_tier_energy %d;\n', Fixed_Third_Tier_Energy);
                fprintf(write_file,'     bill_day 1;\n');
                fprintf(write_file,'     monthly_fee %d;\n',monthly_flat_fee);
            end
            fprintf(write_file,'}\n\n');

            house_config{4} = ['' tag '_tpm_flatrate_',phase,num2str(jind)];      

            fprintf(write_file,'object triplex_meter {\n');   
            fprintf(write_file,'     nominal_voltage 124;\n');
            fprintf(write_file,'     phases %sS;\n',phase);
            fprintf(write_file,'     name %s_tpm_rt_%s%d;\n',tag,phase,jind);
            fprintf(write_file,'     groupid %s_rt_meter;\n',tag);
            fprintf(write_file,'     parent %s_tpm_flatrate_%s%d;\n',tag,phase,jind);	
            fprintf(write_file,'}\n\n');

            house_config{4} = ['' tag '_tpm_rt_',phase,num2str(jind)];                  

            type_determine = rand();
            snowbird = 0;

            if(type_determine <= 0.30)
                house_config{1} = 1; 
                snowbird = 0;
            elseif(type_determine > 0.30 && type_determine <= 0.58)
                house_config{1} = 2;
                snowbird = 0;
            elseif(type_determine > 0.58 && type_determine <= 0.75)
                house_config{1} = 3;
                snowbird = 0;
            elseif(type_determine > 0.75 && type_determine <= 0.86)   
                house_config{1} = 4;
                snowbird = 0;
            elseif(type_determine > 0.86 && type_determine <= 0.96)    
                house_config{1} = 5;
                snowbird = 0;
            elseif(type_determine > 0.96) 
                house_config{1} = 5;
                snowbird = 0;
            end
        %     end

            house_config{2} = ['' tag '_house_',phase,num2str(jind)];
            house_config{3} = phase;
            house_config{5} = '1';
            house_config{6} = snowbird;
            appliance_scalar = [2;1;1;1;1;1;1;1];    


            [count1, count2, count3, count4, count5, count6, count7, cool_slider] = triplex_house_generator(write_file,house_config,0,...
                0,1,1,0.97,0.97,0.8,1,1,1,8,8,5,'',appliance_scalar,market_info,Case,jind,sigma_tstat,use_deadband,use_override,...
                use_congestion_incentive, count1, count2, count3, count4, count5, count6, count7, desired_house_index, jind,...
                decrease_heating_setpoint, use_fncs, use_ns3, tag);

            slider_setting_array(jind, 1) = jind;   
            slider_setting_array(jind, 2) = cool_slider;

        end
    end

    %{
    fprintf(write_file,'object recorder {\n');
    fprintf(write_file,'     parent ROOT;\n');
    fprintf(write_file,'     property measured_real_energy,measured_power.real;\n');
    fprintf(write_file,'     interval 3600;\n');
    fprintf(write_file,'     limit 10000000;\n');  
    fprintf(write_file,'     file %s_Feeder_Energy_Power.csv;\n', my_name);
    fprintf(write_file,'}\n\n');


    fprintf(write_file,'object group_recorder {\n');
    fprintf(write_file,'     group class=waterheater AND groupid=fwh;\n');
    fprintf(write_file,'     property demand;\n');
    fprintf(write_file,'     interval 3600;\n');
    fprintf(write_file,'     limit 10000000;\n');  
    fprintf(write_file,'     file %s_PDE_Demand.csv;\n', my_name);
    fprintf(write_file,'}\n\n');
    %}

    % fprintf(write_file,'object group_recorder {\n');
    % fprintf(write_file,'     group class=waterheater AND groupid=fwh;\n');
    % fprintf(write_file,'     property power;\n');
    % fprintf(write_file,'     interval 3600;\n');
    % fprintf(write_file,'     limit 10000000;\n');  
    % fprintf(write_file,'     file %s_PDE_Power.csv;\n', my_name);
    % fprintf(write_file,'}\n\n');
    %{
    fprintf(write_file,'object recorder {\n');
    fprintf(write_file,'     parent ROOT;\n');
    fprintf(write_file,'     property measured_real_energy;\n');
    fprintf(write_file,'     interval 60;\n');
    fprintf(write_file,'     limit 10000000;\n');  
    fprintf(write_file,'     file %s_trash.csv;\n', my_name);
    fprintf(write_file,'}\n\n');
    %}
    % fprintf(write_file,'object recorder {\n');
    % fprintf(write_file,'     parent regulation_signal;\n');
    % fprintf(write_file,'     property value;\n');
    % fprintf(write_file,'     interval 3600;\n');
    % fprintf(write_file,'     limit 10000000;\n');  
    % fprintf(write_file,'     file %s_Regulation_Signal.csv;\n', my_name);
    % fprintf(write_file,'}\n\n');
    % 
    %     fprintf(write_file,'object collector {\n');
    %     fprintf(write_file,'     group class=triplex_line AND groupid=Triplex_Line;\n');
    %     fprintf(write_file,'     property sum(power_losses_A.real),sum(power_losses_B.real),sum(power_losses_C.real),sum(power_losses_A.imag),sum(power_losses_B.imag),sum(power_losses_C.imag);\n');
    %     fprintf(write_file,'     interval 300;\n');
    %     fprintf(write_file,'     limit 10000000;\n');  
    %     fprintf(write_file,'     file Triplex_Line_Losses.csv;\n');
    %     fprintf(write_file,'}\n\n');
    % 
    %     fprintf(write_file,'object collector {\n');
    %     fprintf(write_file,'     group class=triplex_meter AND groupid=flatrate_meter;\n');
    %     fprintf(write_file,'     property sum(measured_real_power),sum(measured_reactive_power);\n');
    %     fprintf(write_file,'     interval 300;\n');
    %     fprintf(write_file,'     limit 10000000;\n');
    %     fprintf(write_file,'     file Fixed_Rate_Loads.csv;\n');
    %     fprintf(write_file,'}\n\n');
    %     
    %     if (strcmp(Case,'Fixed')==0)  
    %         fprintf(write_file,'object collector {\n');
    %         fprintf(write_file,'     group class=triplex_meter AND groupid=rt_meter;\n');
    %         fprintf(write_file,'     property sum(measured_real_power),sum(measured_reactive_power);\n');
    %         fprintf(write_file,'     interval 300;\n');
    %         fprintf(write_file,'     limit 10000000;\n');
    %         fprintf(write_file,'     file RT_Rate_Loads.csv;\n');
    %         fprintf(write_file,'}\n\n');
    %     end
    %     
    %     fprintf(write_file,'object collector {\n');   
    %     fprintf(write_file,'     group "class=triplex_meter";\n');
    %     fprintf(write_file,'     property sum(measured_real_energy);\n');
    %     fprintf(write_file,'     interval 300;\n');
    %     fprintf(write_file,'     limit 10000000;\n');    
    %     fprintf(write_file,'     file energy_%s.csv;\n',my_name);
    %     fprintf(write_file,'}\n\n');
    % 
    %     fprintf(write_file,'object collector {\n');   
    %     fprintf(write_file,'     group "class=house";\n');
    %     fprintf(write_file,'     property avg(cooling_setpoint),avg(heating_setpoint),avg(air_temperature);\n');
    %     fprintf(write_file,'     interval 300;\n');
    %     fprintf(write_file,'     limit 10000000;\n');    
    %     fprintf(write_file,'     file setpoint_%s.csv;\n',my_name);
    %     fprintf(write_file,'}\n\n');

    if (strcmp(Case,'Fixed')==0)  
    %         for house_number=1:30:300
    %             hn(1) = house_number;
    %             for i = 2:30
    %                 hn(i)= hn(i-1)+1;
    %             end
    %             fprintf(write_file,'object multi_recorder {\n');
    %             fprintf(write_file,'     property C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price,C_%d:proxy_clear_price;\n',hn(1),hn(2),hn(3),hn(4),hn(5),hn(6),hn(7),hn(8),hn(9),hn(10),hn(11),hn(12),hn(13),hn(14),hn(15),hn(16),hn(17),hn(18),hn(19),hn(20),hn(21),hn(22),hn(23),hn(24),hn(25),hn(26),hn(27),hn(28),hn(29),hn(30));
    %             fprintf(write_file,'     limit 10000000;\n');
    %             fprintf(write_file,'     interval 300;\n');
    %             fprintf(write_file,'     file controller_cleared_prices_%d_%d_%s.csv;\n',hn(1),hn(30),my_name);
    %             fprintf(write_file,'}\n\n');
    %             fprintf(write_file,'object multi_recorder {\n');
    %             fprintf(write_file,'     property C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason,C_%d:no_bid_reason;\n',hn(1),hn(2),hn(3),hn(4),hn(5),hn(6),hn(7),hn(8),hn(9),hn(10),hn(11),hn(12),hn(13),hn(14),hn(15),hn(16),hn(17),hn(18),hn(19),hn(20),hn(21),hn(22),hn(23),hn(24),hn(25),hn(26),hn(27),hn(28),hn(29),hn(30));
    %             fprintf(write_file,'     limit 10000000;\n');
    %             fprintf(write_file,'     interval 300;\n');
    %             fprintf(write_file,'     file controller_no_bid_reason_%d_%d_%s.csv;\n',hn(1),hn(30),my_name);
    %             fprintf(write_file,'}\n\n');
    %             fprintf(write_file,'object multi_recorder {\n');
    %             fprintf(write_file,'     property C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response,C_%d:bid_response;\n',hn(1),hn(2),hn(3),hn(4),hn(5),hn(6),hn(7),hn(8),hn(9),hn(10),hn(11),hn(12),hn(13),hn(14),hn(15),hn(16),hn(17),hn(18),hn(19),hn(20),hn(21),hn(22),hn(23),hn(24),hn(25),hn(26),hn(27),hn(28),hn(29),hn(30));
    %             fprintf(write_file,'     limit 10000000;\n');
    %             fprintf(write_file,'     interval 300;\n');
    %             fprintf(write_file,'     file controller_bid_response_%d_%d_%s.csv;\n',hn(1),hn(30),my_name);
    %             fprintf(write_file,'}\n\n');
    %         end

    %     fprintf(write_file,'object collector {\n');
    %     fprintf(write_file,'     property sum(is_COOL_on);\n');
    %     fprintf(write_file,'     file HVAC_states_%s.csv;\n',my_name);
    %     fprintf(write_file,'     group "class=house";\n');
    %     fprintf(write_file,'     limit 10000000;\n');
    %     fprintf(write_file,'     interval 300;\n');
    %     fprintf(write_file,'}\n\n');
    end

    if (use_billing == 1)
        for bill_dump_count=1:12
            if((bill_dump_count+1) < 10)
                temp_timestr = strcat('2009-0', num2str(bill_dump_count+1));
            else
                if((bill_dump_count+1) < 13)
                    temp_timestr = strcat('2009-', num2str(bill_dump_count+1));
                else
                    temp_timestr = strcat('2010-01');
                end
            end
            temp_timestr = strcat(temp_timestr, '-01 02:00:00');      

            fprintf(write_file,'object billdump {\n');           
            fprintf(write_file,'     filename BDF_%d.csv;\n', bill_dump_count);
            fprintf(write_file,'     runtime %s;\n', temp_timestr);
            fprintf(write_file,'     group flatrate_meter;\n');
            fprintf(write_file,'}\n\n');
        end

        if (strcmp(Case,'Fixed')==0)  
            for bill_dump_count=1:12
                if((bill_dump_count+1) < 10)
                    temp_timestr = strcat('2009-0', num2str(bill_dump_count+1));
                else
                    if((bill_dump_count+1) < 13)
                        temp_timestr = strcat('2009-', num2str(bill_dump_count+1));
                    else
                        temp_timestr = strcat('2010-01');
                    end
                end
                temp_timestr = strcat(temp_timestr, '-01 02:00:00');  

                fprintf(write_file,'object billdump {\n');
                fprintf(write_file,'     filename BD_rt_%d.csv;\n', bill_dump_count);
                fprintf(write_file,'     runtime %s;\n', temp_timestr);
                fprintf(write_file,'     show_congestion_rebates_incentives %s;\n', show_congestion_rebates_incentives);
                fprintf(write_file,'     group  rt_meter;\n');
                fprintf(write_file,'}\n\n'); 

            end
        end 
    end

    %%% xlswrite('F:\AEP Dave Analysis\Final Runs\New Runs\slider_setting',slider_setting_array); 

    fclose('all');

    count1
    count2
    count3
    count4
    count5
    count6
    count7       

    disp(['Done with ', filename]);
end