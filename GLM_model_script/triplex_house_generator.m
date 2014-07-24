function [count1, count2, count3, count4, count5, count6, count7, cool_slider] = triplex_house_generator_NOns3(file,config,z,i,p,...
    house_VA,hvac_pf,system_pf,heat_fraction,scale_floor,scale_ZIP,wd_scale,no_of_cooling_sch,no_of_heating_sch,no_of_water_sch,...
    schedule_name,appliance_scalar,market_info,Case,jind,sigma_tstat,use_deadband,use_override, use_congestion_incentive, count1,...
    count2, count3, count4, count5, count6, count7, desired_house_index, house_index,decrease_heating_setpoint, use_fncs, use_ns3, tag)   
% file = file to write to
% expects the config file to have:
% - {1} house tag, {2} name, {3} phases, {4} parent, {5} power
% z = constant impedance fraction
% i = constant current fraction
% p = constant power fraction
% house_VA = average VA per house
% hvac_pf = power factor of the hvac system
% system_pf = power factor of the rest of the system
% heat_fraction = average fraction of power that's converted to heat
% scale_floor = scales the randomized floor area
% scale_ZIP = scales the magnitude of the ZIP load
% wd_scale = scales the water demand
% no_of_cooling_schedule = the number of cooling schedules to choose from
% no_of_heating_schedule = the number of heating schedules to choose from
% no_of_water_schedule = the number of schedules for H20
% schedule_name = list of the loadshapes to choose from
% array of the appliance weights - Lights/Plugs; Washer; Refrig; Dryer; Freezer; Dishwasher; Range; Microwave
% market info - {1}mode, {2}name, {3}period, {4}mean stat, {5}std stat, {6}percent penetration

%% Convert numbers to classifications, if needed
if config{1} == 1
    config{1} = 'Residential_1';
elseif config{1} == 2
    config{1} = 'Residential_2';
elseif config{1} == 3
    config{1} = 'Residential_3';
elseif config{1} == 4
    config{1} = 'Residential_4';
elseif config{1} == 5
    config{1} = 'Residential_5';
elseif config{1} == 6
    config{1} = 'Residential_6';
end

if config{6} == 1
    snowbird = 1;
else
    snowbird = 0;
end

%% Unchanging? parameters
% most of these values were obtained from survey data
heat_gas_perc_pre = 0.85;
heat_pump_perc_pre = 0.15;

heat_gas_perc_post = 0.70;
heat_pump_perc_post = 0.30;

cool_none_pre = 0.0;
cool_pump_pre = 0.0;
cool_electric_pre = 1 - cool_none_pre - cool_pump_pre;

cool_none_post = 0.0;
cool_pump_post = 0.0;
cool_electric_post = 1 - cool_none_post - cool_pump_post;

wh_perc_elec_pre = 0;
wh_perc_elec_post = 0;

%house parameters extracting them here
smallhome_floorarea_1 = 1100;
smallhome_floorarea_2 = 500;

largehome_floorarea_1 = 3000;
largehome_floorarea_2 = 900;

mobilehome_floorarea_1 = 750;


%%
house_tag = config{1};
name = config{2};
phases = config{3};
parent = config{4};
power = config{5};

house_load = abs(str2num(power));

% Parameter distributions (same for all) (average +/- range)
wtrdem_1=1.0;
wtrdem_2=0.2;
cool_1=1;
cool_2=0.05;
tankset_1=125;
tankset_2=5;
thermdb_1=2.0;
thermdb_2=0.2;
cooloffset_1=4;
cooloffset_2=2;
heatoffset_1=0;
heatoffset_2=3;
basepwr_1=1;
basepwr_2=0.5;
tankUA_1=2.0;
tankUA_2=0.2;

dryer_flag_perc  = rand();
dishwasher_flag_perc = rand();
freezer_flag_perc = rand();

% set the variables for each type
if (strfind(house_tag,'Residential_1') ~= 0)
    
    % OLD/SMALL
    % Distribution of parameters (average +/- range)    
%     cool_none_pre = 0.20;
%     cool_pump_pre = 0.0;
%     cool_electric_pre = 1 - cool_none_pre - cool_pump_pre;
    Rroof_1=19;
    Rroof_2=4;
    Rwall_1=11;
    Rwall_2=3;             
    Rfloor_1=11;
    Rfloor_2=1;
    Rdoors=3;
    Rwindows_1=1.25;
    Rwindows_2=0.5;
    airchange_1=1;
    airchange_2=0.2;      
    floorarea_1=smallhome_floorarea_1;
    floorarea_2=smallhome_floorarea_2;     
    tankvol_1=45;
    tankvol_2=5;
    heatcap_1=4500;
    heatcap_2=500;       
    wh_type1 = 'old';
    wh_type2 = 'small';
    hp_perc=heat_pump_perc_pre;
    g_perc=heat_gas_perc_pre;
    c_perc=cool_electric_pre;
    wh_elec = wh_perc_elec_pre;
    load_scalar = 1;
    
elseif (strfind(house_tag,'Residential_2') ~= 0)
    
    % NEW/SMALL
    % Distribution of parameters (average +/- range)   
    Rroof_1=30;
    Rroof_2=5;                
    Rwall_1=19;
    Rwall_2=3;             
    Rfloor_1=15;
    Rfloor_2=3;              
    Rdoors=5;
    Rwindows_1=1.75;
    Rwindows_2=0.5;      
    airchange_1=1;
    airchange_2=0.2;      
    floorarea_1=smallhome_floorarea_1;
    floorarea_2=smallhome_floorarea_2;      
    tankvol_1=45;
    tankvol_2=5;            
    heatcap_1=4500;
    heatcap_2=500;
    wh_type1 = 'new';
    wh_type2 = 'small';
    hp_perc=heat_pump_perc_post; 
    g_perc=heat_gas_perc_post; 
    c_perc=cool_electric_post;
    wh_elec = wh_perc_elec_post;
    load_scalar = 0.95;
    
elseif (strfind(house_tag,'Residential_3') ~= 0)
    
    % OLD/LARGE 
    % Distribution of parameters (average +/- range)
    Rroof_1=19;
    Rroof_2=4;                
    Rwall_1=11;
    Rwall_2=3;             
    Rfloor_1=11;
    Rfloor_2=1;              
    Rdoors=3;
    Rwindows_1=1.25;
    Rwindows_2=0.5;      
    airchange_1=1;
    airchange_2=0.2;      
    floorarea_1=largehome_floorarea_1;
    floorarea_2=largehome_floorarea_2;  
    tankvol_1=55;
    tankvol_2=5;            
    heatcap_1=4500;
    heatcap_2=500;     
    wh_type1 = 'old';
    wh_type2 = 'large';
    hp_perc=heat_pump_perc_pre; 
    g_perc=heat_gas_perc_pre; 
    c_perc=cool_electric_pre;
    wh_elec = wh_perc_elec_pre;
    load_scalar = 0.85;
    
elseif (strfind(house_tag,'Residential_4') ~= 0)
    
    % NEW/LARGE
    % Distribution of parameters (average +/- range)
    Rroof_1=30;
    Rroof_2=5;                
    Rwall_1=19;
    Rwall_2=3;             
    Rfloor_1=15;
    Rfloor_2=3;              
    Rdoors=5;
    Rwindows_1=1.75;
    Rwindows_2=0.5;      
    airchange_1=1;
    airchange_2=0.2;      
    floorarea_1=largehome_floorarea_1-200;
    floorarea_2=largehome_floorarea_2;      
    tankvol_1=55;
    tankvol_2=5;            
    heatcap_1=4500;
    heatcap_2=500;
    wh_type1 = 'new';
    wh_type2 = 'large';
    % we'll assume there are no 3000 sq ft new homes w/ resistive heat
    hp_perc=heat_pump_perc_post; 
    g_perc=1-hp_perc; 
    c_perc=cool_electric_post;
    wh_elec = wh_perc_elec_post;
    load_scalar = 0.8;
    
elseif (strfind(house_tag,'Residential_5') ~= 0)
    
    % Mobile homes
    % Distribution of parameters (average +/- range)        
    Rroof_1=14;
    Rroof_2=4;                
    Rwall_1=6;Rwall_2=2;             
    Rfloor_1=5;
    Rfloor_2=1;               
    Rdoors=3;
    Rwindows_1=1.25;
    Rwindows_2=0.5;      
    airchange_1=1.4;
    airchange_2=0.2;      
    floorarea_1=mobilehome_floorarea_1;
    floorarea_2=150;     
    tankvol_1=35;
    tankvol_2=5;            
    heatcap_1=3500;
    heatcap_2=500;   
    wh_type1 = 'old';
    wh_type2 = 'small';
%     hp_perc=0.0; g_perc=0.90; c_perc=cool_electric_pre;
%     wh_elec = wh_perc_elec_pre;
    hp_perc=heat_pump_perc_post; 
    g_perc=1-hp_perc; 
    c_perc=cool_electric_post;
    wh_elec = wh_perc_elec_post;
    load_scalar = 0.7;    
        
elseif (strfind(house_tag,'Residential_6') ~= 0)
    
    % Distribution of parameters (average +/- range)
    Rroof_1=14;
    Rroof_2=4;                
    Rwall_1=6;
    Rwall_2=2;             
    Rfloor_1=5;
    Rfloor_2=1;               
    Rdoors=3;
    Rwindows_1=1.25;
    Rwindows_2=0.5;      
    airchange_1=1.4;
    airchange_2=0.2;      
    floorarea_1=500;
    floorarea_2=200;    
    tankvol_1=35;
    tankvol_2=5;            
    heatcap_1=3500;
    heatcap_2=500;
    wh_type1 = 'old';
    wh_type2 = 'small';
    hp_perc=heat_pump_perc_pre; 
    g_perc=heat_gas_perc_pre; 
    c_perc=cool_electric_pre;
    wh_elec = wh_perc_elec_pre;
    load_scalar = 0.5;
    
elseif (strfind(house_tag,'Commercial') ~= 0)
    
    % Distribution of parameters
    Rroof_1=25;
    Rroof_2=5;                
    Rwall_1=17;
    Rwall_2=3;             
    Rfloor_1=12;
    Rfloor_2=3;              
    Rdoors=5;
    Rwindows_1=1.75;
    Rwindows_2=0.5;      
    airchange_1=1.3;
    airchange_2=0.2;      
    floorarea_1=2500;
    floorarea_2=300;            
    % we'll assume there are no commercial w/ resistive heat
    hp_perc=heat_pump_perc_post; 
    g_perc=1-hp_perc; 
    c_perc=cool_electric_post;
    load_scalar = 1;
    
else
    error('This broke somewhere in pulling info into the triplex_house_generator');
end

% Determine how many houses to create and how "big" the ZIP load should be
n_houses = floor(house_load / (load_scalar * house_VA));

if (n_houses < 1)
    n_houses = 1;
    ZIPloadVA = 1000;
elseif (strfind(house_tag,'Residential_6') ~= 0)
    ZIPloadVA=0.18 * (house_load - n_houses * load_scalar * house_VA) / n_houses;
else
    ZIPloadVA=(house_load - n_houses * load_scalar * house_VA) / n_houses;
end

for houses = 1:n_houses
 
    if(snowbird == 0)
        
        % if it's the 2nd+ times running through apts, then don't
        % update since these should all be about the same
        if (houses == 1)
            randomize = 1; %true
        elseif (strfind(house_tag,'Residential_6') ~= 0)
            randomize = 0; %false
        else
            randomize = 1; %true
        end

        % Choose the type of HVAC
        if (randomize == 1)
            h_style = 'GAS';
            f_style = 'ONE_SPEED';
            c_style = 'ELECTRIC';
        end
                
        % Print the house while randomizing variables
        fprintf(file,'object house {\n');
        fprintf(file,'     name %s_H_%d;\n',tag,jind); % name H_%d%s;\n',jind,tag);
        fprintf(file,'     //%s\n',house_tag);
        fprintf(file,'     parent %s;\n',parent);
        
        % Randomize the schedule time offset, but keep it the same
        % throughout the house
        sch_skew = 1200 * randn(1);
        if sch_skew > 3600
            sch_skew = 3600;
        elseif sch_skew < -3600
            sch_skew = -3600;
        end
        fprintf(file,'     schedule_skew %.0f;\n',sch_skew);
        
        if (randomize == 1)
                Rroof = (Rroof_1-Rroof_2)+2*Rroof_2*rand(1);
        end 
        fprintf(file,'     Rroof %2.2f;\n',Rroof);
        
        if (randomize == 1)
                Rwall = (Rwall_1-Rwall_2)+2*Rwall_2*rand(1);
        end
        fprintf(file,'     Rwall %2.2f;\n',Rwall);

        if (randomize == 1)
                Rfloor = (Rfloor_1-Rfloor_2)+2*Rfloor_2*rand(1);
        end
        fprintf(file,'     Rfloor %2.2f;\n',Rfloor);
        fprintf(file,'     Rdoors %d;\n',Rdoors);
        
        if (randomize == 1)
                Rwindows = (Rwindows_1-Rwindows_2)+2*Rwindows_2*rand(1);
        end
        fprintf(file,'     Rwindows %2.2f;\n',Rwindows);
        
        airchange = (airchange_1-airchange_2)+2*airchange_2*rand(1);
        fprintf(file,'     airchange_per_hour %1.2f;\n',airchange);
        
        fprintf(file,'     hvac_power_factor %0.2f;\n',hvac_pf);
        fprintf(file,'     cooling_system_type %s;\n',c_style);
        fprintf(file,'     heating_system_type %s;\n',h_style);
        fprintf(file,'     fan_type %s;\n',f_style);
        fprintf(file,'     hvac_breaker_rating 200;\n');
        
        % % %
        fprintf(file,'     total_thermal_mass_per_floor_area %.2f;\n',rand(1)*2 + 3);
        % % %
        
        if(strcmp(house_tag,'Residential_5')==1) 
            a = 2.0;
            b = 3.5; 
        elseif (strcmp(house_tag,'Residential_1')==1 || strcmp(house_tag,'Residential_3')==1)
            a = 2.6;
            b = 3.8;
        else
            a = 3.4;
            b = 4.2;        
        end

        temp_rand_num = a + (b-a).*rand(1);

        if (strcmp(c_style,'ELECTRIC') ~= 0)
            fprintf(file,'     motor_efficiency AVERAGE;\n');
            fprintf(file,'     motor_model BASIC;\n');
            fprintf(file,'     cooling_COP %.2f;\n', temp_rand_num);         
        end
        
        if (randomize == 1)
                floor_area = scale_floor * (floorarea_1-floorarea_2)+2*floorarea_2*rand(1);
        end
        fprintf(file,'     floor_area %.0f;\n',floor_area);
        fprintf(file,'     number_of_doors %d;\n',ceil(floor_area/1000));
        
        init_temp = 68 + 4*rand(1);
        fprintf(file,'     air_temperature %.1f;\n', init_temp);
        fprintf(file,'     mass_temperature %.1f;\n', init_temp);
        
        
%         if(house_index == desired_house_index)
%             fprintf(file,'     object recorder {\n');
%             fprintf(file,'          property cooling_setpoint,air_temperature;\n');
%             fprintf(file,'          limit 100000000;\n');
%             fprintf(file,'          interval 300;\n');
%             fprintf(file,'          file house%d_details.csv;\n', desired_house_index);
%             fprintf(file,'     };\n');
%         end

        % Choose our cooling schedules
        schedule_cool = randi(no_of_cooling_sch);
        cooloffset = (cooloffset_1-cooloffset_2)+2*cooloffset_2*rand(1);
        cool_temp = ((cool_1-cool_2)+2*cool_2*rand(1));

        % Choose our heating schedules
        schedule_heat = randi(no_of_heating_sch);
        heatoffset=heatoffset_1+heatoffset_2*rand(1);
        heat_temp = ((cool_1-cool_2)+2*cool_2.*rand(1));

        % Choose our bid_delay range from 30-90 seconds to within the
        % nearest second
        % =============================================================================
        % bid_delay = 30 + round((90-30)*rand(1));
        % =============================================================================

%             cool_slider = market_info{7};
%             heat_slider = market_info{7};

% %             a = 45;
% %             b = 30;
% %             market_test = a + b.*randn();
           
        a = 0;
        b = 1;
        market_test = a + (b-a).*rand();

        while(market_test > 100)
            count7 = count7 + 1; 
            market_test = a + (b-a).*rand();
            %market_test = a + b.*randn();
        end

        if(market_test <= 0.17)
            cool_slider = 0.0;                        
        elseif(market_test <= 0.27)
            cool_slider = 0.2; 
        elseif(market_test <= 0.62)
            cool_slider = 0.4;                         
        elseif(market_test <= 0.87)
            cool_slider = 0.6;
        elseif(market_test <= 0.97)
            cool_slider = 0.8; 
        elseif(market_test <= 1.0)
            cool_slider = 1.0;           
        end  

        heat_slider = cool_slider;

        market_test = market_test/100;

        heating_factor=0.8;
            
          
%             if (market_info{8} < 0)
%                 cool_slider = cool_slider*(1 - market_test);
%                 heat_slider = cool_slider;
%             end

            % TO DO: make sure the single ramp controller works
        if (strcmp(market_info{1},'MARKET') ~= 0 && market_test <= market_info{6} && strcmp(Case,'Fixed') == 0)
            if (market_info{7} == 0)    
                fprintf(file,'     cooling_setpoint cooling%d*%1.3f+%2.2f;\n',schedule_cool,cool_temp,cooloffset);        
                
                if(decrease_heating_setpoint==1)
                    fprintf(file,'     heating_setpoint heating%d*%1.3f;\n',schedule_heat,heating_factor);  
                else
                    fprintf(file,'     heating_setpoint heating%d*%1.3f+%2.2f;\n',schedule_heat,heat_temp,heatoffsett);                
                end
                
            elseif (strcmp(h_style,'GAS') == 0)
                fprintf(file,'\n     object controller {\n');
                fprintf(file,'          name testController_%d;\n',jind);
                if (use_override == 1)
                    fprintf(file,'          use_override ON;\n');
                    fprintf(file,'          override override;\n');
                end
%                fprintf(file,'          use_congestion_incentive %s;\n', use_congestion_incentive);
%                 if(strcmp(use_congestion_incentive,'TRUE'));                    
%                     fprintf(file,'          meter tpm_rt_%s%d;\n',config{3},jind);
%                     
%                     if(jind == desired_house_index)
%                         fprintf(file,'	 object recorder {\n');  
%                         fprintf(file,'        property controller_congestion_incentive,last_controller_congestion_incentive;\n');
%                         fprintf(file,'        limit 100000000;\n');
%                         fprintf(file,'        interval 300;\n');
%                         fprintf(file,'        file house%d_controller_rebate.csv;\n', desired_house_index);
%                         fprintf(file,'      };\n');
%                     end
%                     
%                 end
                fprintf(file,'          market %s;\n',market_info{2});
                fprintf(file,'          control_mode DOUBLE_RAMP;\n');
                %fprintf(file,'          resolve_mode DOMINANT;\n');               
                
                fprintf(file,'          schedule_skew %.0f;\n',sch_skew);
                fprintf(file,'          bid_mode ON;\n');
%                 fprintf(file,'          object controller_network_interface {\n');
%                 fprintf(file,'               name double_ramp_NIF_%d;\n',jind);
%                 fprintf(file,'               destination MarkNIF;\n');
%                 fprintf(file,'               to MpiNetwork;\n');
%                 fprintf(file,'          };\n\n');
                
                if(decrease_heating_setpoint==1)
                    fprintf(file,'          heating_base_setpoint heating%d*%1.3f;\n',schedule_heat,heating_factor);
                else 
                    fprintf(file,'          heating_base_setpoint heating%d*%1.3f+%2.2f;\n',schedule_heat,heat_temp,heatoffset);
                end
                
                fprintf(file,'          cooling_base_setpoint cooling%d*%1.3f+%2.2f;\n',schedule_cool,cool_temp,cooloffset);

                fprintf(file,'          target air_temperature;\n');      
                fprintf(file,'          deadband thermostat_deadband;\n');  
                if (use_deadband == 1)
                    fprintf(file,'          bid_deadband_ends true;\n');
                end

                if (strcmp(Case,'rt')==1)                 
                    fprintf(file,'          average_target %s;\n',market_info{4});
                    fprintf(file,'          standard_deviation_target %s;\n',market_info{5});    
                elseif(strcmp(Case,'da')==1 || strcmp(Case,'Merged')==1 ) 
                    fprintf(file,'          average_target day_ahead_average;\n');
                    fprintf(file,'          standard_deviation_target day_ahead_std;\n');
                elseif (strcmp(Case,'test')==1)                 
                    fprintf(file,'          average_target %s;\n',market_info{4});
                    fprintf(file,'          standard_deviation_target %s;\n',market_info{5}); 
                end

                fprintf(file,'          period %.0f;\n',market_info{3});  
                fprintf(file,'          cooling_setpoint cooling_setpoint;\n');
                fprintf(file,'          heating_setpoint heating_setpoint;\n');
                
                fprintf(file,'          heating_demand last_heating_load;\n');
                fprintf(file,'          cooling_demand last_cooling_load;\n');
                fprintf(file,'          bid_delay 1;\n');
                if (sigma_tstat ~= 0)
                    slider = cool_slider;
                    hrh = 5-5*(1-slider);
                    crh = 10-10*(1-slider);
                    hrl = -10+10*(1-slider);
                    crl = -5+5*(1-slider);
                    
                    if (sigma_tstat ~= 0)
                        hrh2 = -sigma_tstat - (1 - slider) * (3 - sigma_tstat);
                        crh2 = sigma_tstat + (1 - slider) * (3 - sigma_tstat);
                        hrl2 = -sigma_tstat - (1 - slider) * (3 - sigma_tstat);
                        crl2 = sigma_tstat + (1 - slider) * (3 - sigma_tstat);
                    else
                        hrh2 = 0;
                        crh2 = 0;
                        hrl2 = 0;
                        crl2 = 0;
                    end
                    
                    fprintf(file,'          heating_range_high %.3f;\n',hrh);
                    fprintf(file,'          cooling_range_high %.3f;\n',crh);
                    fprintf(file,'          heating_range_low %.3f;\n',hrl);
                    fprintf(file,'          cooling_range_low %.3f;\n',crl);
                    fprintf(file,'          heating_ramp_high %.3f;\n',hrh2);
                    fprintf(file,'          cooling_ramp_high %.3f;\n',crh2);
                    fprintf(file,'          heating_ramp_low %.3f;\n',hrl2);
                    fprintf(file,'          cooling_ramp_low %.3f;\n',crl2);

                else
                    fprintf(file,'          slider_setting_heat %.3f;\n',cool_slider);
                    fprintf(file,'          slider_setting_cool %.3f;\n',heat_slider);
                end
                fprintf(file,'          total total_load;\n');
                fprintf(file,'          load hvac_load;\n');
                fprintf(file,'          state power_state;\n'); 
                
                if(house_index == desired_house_index)
%                     fprintf(file,'           object recorder {\n');
%                     fprintf(file,'              property bid_price,bid_quantity,range_low,range_high,min,max,heat_min,heat_max,cool_min,cool_max;\n');
%                     fprintf(file,'              limit 100000000;\n');
%                     fprintf(file,'              interval 300;\n');
%                     fprintf(file,'              file house%d_testController_%d_details.csv;\n', desired_house_index, desired_house_index);
%                     fprintf(file,'           };\n');
                end
                
                                               
                fprintf(file,'       };\n\n');
            elseif (strcmp(c_style,'ELECTRIC') ~= 0) 
                if(decrease_heating_setpoint==1)
                    fprintf(file,'     heating_setpoint heating%d*%1.3f-50;\n',schedule_heat,heating_factor);  
                else
                    fprintf(file,'     heating_setpoint heating%d*%1.3f+%2.2f;\n',schedule_heat,heat_temp,heatoffset);
                end

                fprintf(file,'\n     object controller {\n');
                fprintf(file,'          name %s_C_%d;\n',tag,jind); % name C_%d;\n',jind);
                if (use_override == 1)
                    fprintf(file,'          use_override ON;\n');
                    fprintf(file,'          override override;\n');
                end
                %fprintf(file,'          use_congestion_incentive %s;\n', use_congestion_incentive);
%                 if(strcmp(use_congestion_incentive,'TRUE'));                    
%                     fprintf(file,'          meter tpm_rt_%s%d;\n',config{3},jind);
%                 end
                fprintf(file,'          market %s;\n',market_info{2});
                fprintf(file,'          schedule_skew %.0f;\n',sch_skew);
                if(use_fncs == 1),
                    % =========== 2014/07/23 Capability to select if we use
                    % NS3 or not ==========================================
                    if (use_ns3 == 1),
                        fprintf(file,'          bid_mode PROXY;\n');
                        % ====================== 2014/05/08 =======
                        fprintf(file,'          object controller_network_interface {\n');
                        fprintf(file,'               name %s_single_ramp_NIF_%d;\n',tag,jind);
                        fprintf(file,'               destination %s;\n', market_info{9}); % destination MarkNIF1;\n');
                        fprintf(file,'          };\n');
                    else
                        % ====================== 2014/05/08 =======
                        fprintf(file,'          bid_mode ON;\n');
                    end
                else
                    fprintf(file,'          bid_mode ON;\n');
                end
                % =============================================================================
                % fprintf(file,'          bid_delay %0.0f;\n',bid_delay);
                % =============================================================================
                fprintf(file,'          control_mode RAMP;\n');
                fprintf(file,'          base_setpoint cooling%d*%1.3f+%2.2f;\n',schedule_cool,cool_temp,cooloffset);
                
                fprintf(file,'          setpoint cooling_setpoint;\n');
                fprintf(file,'          target air_temperature;\n');      
                fprintf(file,'          deadband thermostat_deadband;\n');
                if (use_deadband == 1)
                    fprintf(file,'          use_predictive_bidding TRUE;\n');
                    %fprintf(file,'          bid_deadband_ends true;\n');
                end

                if (strcmp(Case,'rt')==1)                 
                    fprintf(file,'          average_target %s;\n',market_info{4});
                    fprintf(file,'          standard_deviation_target %s;\n',market_info{5});    
                elseif(strcmp(Case,'da')==1 || strcmp(Case,'Merged')==1 ) 
                    fprintf(file,'          average_target day_ahead_average;\n');
                    fprintf(file,'          standard_deviation_target day_ahead_std;\n');
                elseif (strcmp(Case,'test')==1)                 
                    fprintf(file,'          average_target %s;\n',market_info{4});
                    fprintf(file,'          standard_deviation_target %s;\n',market_info{5}); 
                end          

                fprintf(file,'          period %.0f;\n',market_info{3}); 
                fprintf(file,'          demand last_cooling_load;\n');
                if (sigma_tstat ~= 0)
                    slider = cool_slider;
                    crh = 10-10*(1-slider);
                    crl = -5+5*(1-slider);
                    
                    if (sigma_tstat ~= 0)
                        crh2 = sigma_tstat + (1 - slider) * (3 - sigma_tstat);
                        crl2 = sigma_tstat + (1 - slider) * (3 - sigma_tstat);                       
                    else
                        crh2 = 0;
                        crl2 = 0;
                    end
                    
                    fprintf(file,'           range_high %.3f;\n',crh);
                    fprintf(file,'           range_low %.3f;\n',crl);
                    fprintf(file,'           ramp_high %.3f;\n',crh2);
                    fprintf(file,'           ramp_low %.3f;\n',crl2);
                else
                    fprintf(file,'          slider_setting %.3f;\n',cool_slider);
                    fprintf(file,'          range_high 5;\n');
                    fprintf(file,'          range_low -3;\n');
                end
                fprintf(file,'          total total_load;\n');
                fprintf(file,'          load hvac_load;\n');
                fprintf(file,'          state power_state;\n');    
                
                
                if(house_index == desired_house_index)
%                     fprintf(file,'          object recorder {\n');
%                     fprintf(file,'               property bid_price,bid_quantity,range_low,range_high,min,max,heat_min,heat_max,cool_min,cool_max;\n');
%                     fprintf(file,'               limit 100000000;\n');
%                     fprintf(file,'               interval 300;\n');
%                     fprintf(file,'               file house%d_testController_%d_details.csv;\n', desired_house_index, desired_house_index);
%                     fprintf(file,'          };\n');
                end
                               
                fprintf(file,'     };\n\n');
            else         
                fprintf(file,'     cooling_setpoint cooling%d*%1.3f+%2.2f;\n',schedule_cool,cool_temp,cooloffset);
                
                if(decrease_heating_setpoint==1)
                    fprintf(file,'     heating_setpoint heating%d*%1.3f;\n',schedule_heat,heating_factor);  
                else
                    fprintf(file,'     heating_setpoint heating%d*%1.3f+%2.2f;\n',schedule_heat,heat_temp,heatoffset);    
                end
            end  
        else            
            fprintf(file,'     cooling_setpoint cooling%d*%1.3f+%2.2f;\n',schedule_cool,cool_temp,cooloffset);
            
            if(decrease_heating_setpoint==1)
                fprintf(file,'     heating_setpoint heating%d*%1.3f;\n',schedule_heat,heating_factor);  
            else
                fprintf(file,'     heating_setpoint heating%d*%1.3f+%2.2f;\n',schedule_heat,heat_temp,heatoffset);   
            end
        end


        if (strfind(house_tag,'Residential_6') ~= 0)
            % Assumes apartments are all in a single line
            if (houses == 1 || houses ==n_houses)
                fprintf(file,'     exterior_wall_fraction 0.75;\n');
            else
                fprintf(file,'     exterior_wall_fraction 0.5;\n');
            end
        end

        % Add ZIP load -> approximates all of the non-state driven loads
        if (sum(appliance_scalar) == 0)
            fprintf(file,'     object ZIPload {\n');
            
            base_power = scale_ZIP*((basepwr_1-basepwr_2)+2*basepwr_2.*rand(1))*ZIPloadVA/1000;
            schedule_no = randi(size(schedule_name));
            fprintf(file,'          base_power %s.value*%.4f;\n',char(schedule_name(schedule_no)),base_power);
            
            fprintf(file,'          schedule_skew %.0f;\n',sch_skew);
            fprintf(file,'          power_fraction %f;\n',p);
            fprintf(file,'          impedance_fraction %f;\n',z);
            fprintf(file,'          current_fraction %f;\n',i);
            
            power_pf = 2*(1-system_pf)*rand(1)+2*system_pf-1;
            fprintf(file,'          power_pf %1.3f;\n',power_pf);
            
            current_pf = 2*(1-system_pf)*rand(1)+2*system_pf-1;
            fprintf(file,'          current_pf %1.3f;\n',current_pf);
            
            impedance_pf = 2*(1-system_pf)*rand(1)+2*system_pf-1;
            fprintf(file,'          impedance_pf %1.3f;\n',impedance_pf);
            
            heat_frac = heat_fraction - (0.2)*heat_fraction + 2*(0.2)*heat_fraction*rand(1);
            fprintf(file,'          heat_fraction %0.2f;\n',heat_frac);
            
            fprintf(file,'     };\n');
        else % Use the appliance weight vector to populate ziploads

            dryer_present = 1;
            freezer_present = 1;                                
            dishwasher_present = 1;
                        
            if(dryer_flag_perc>0.65 && (strcmp(house_tag,'Residential_5')==1 || strcmp(house_tag,'Residential_1')==1|| strcmp(house_tag,'Residential_3')==1))
                dryer_present = 0;
            end
            
            if(dishwasher_flag_perc>0.65 && (strcmp(house_tag,'Residential_5')==1 || strcmp(house_tag,'Residential_1')==1 || strcmp(house_tag,'Residential_2')==1))
                dryer_present = 0;
            end
            
            if((strcmp(house_tag,'Residential_1')==1 || strcmp(house_tag,'Residential_2')==1) || strcmp(house_tag,'Residential_5')==1)
                freezer_present = 0;
            end

            if((freezer_flag_perc>0.5 && (strcmp(house_tag,'Residential_3')==1 || strcmp(house_tag,'Residential_4')==1)))
                freezer_present = 0;
            end     
            
            if((strcmp(house_tag,'Residential_1')==1))
                dishwasher_present = 0;
            end            
            
           
            appliances = {'LIGHTS'; 'CLOTHESWASHER'; 'REFRIGERATOR'; 'DRYER'; 'FREEZER'; 'DISHWASHER'; 'RANGE'; 'MICROWAVE'};
            for iindex = 1:length(appliance_scalar)           
                if (appliance_scalar(iindex) > 0)
                    if (iindex == 1) % lights
                        if rand(1) < 0.5 % CFL
                            p_frac = 0.6; z_frac = 0.4; i_frac = 0.0; p_pf = -0.78; z_pf = -0.88; i_pf = 0.42;
                        else % INCANDESCENT
                            p_frac = 0.0; z_frac = 1.0; i_frac = 0.0; p_pf = 0.0; z_pf = 1.0; i_pf = 0.0;
                        end
                    elseif (iindex == 4) % DRYER
                        p_frac = 0.1; z_frac = 0.8; i_frac = 0.1; p_pf = 0.9; z_pf = 1.0; i_pf = 0.9;
                    elseif (iindex == 7) % RANGE
                        p_frac = 0.0; z_frac = 1.0; i_frac = 0.0; p_pf = 0.0; z_pf = 1.0; i_pf = 0.0;
                    else
                        p_frac = p; z_frac = z; i_frac = i; p_pf = system_pf; z_pf = system_pf; i_pf = system_pf;
                    end    

                    if(((strcmp(house_tag,'Residential_1')==1) || (strcmp(house_tag,'Residential_1')==2)) && (iindex == 7))
                        appliance_scalar(iindex) = 0.5;
                    end

                    if((iindex~=1 && iindex~=5 && iindex~=6 && dryer_present==1) || (iindex==1) || (iindex==5 && freezer_present==1) || (iindex~=6 && dishwasher_present==1))
                        fprintf(file,'     object ZIPload {\n');
                        fprintf(file,'          schedule_skew %.0f;\n',sch_skew);
                        
                        base_power = (324.9/8907 * floor_area^0.442) * appliance_scalar(iindex)*((basepwr_1-basepwr_2)+2*basepwr_2.*rand(1));
                        schedule = appliances(iindex);
                        fprintf(file,'          base_power %s*%.4f;\n',char(schedule),base_power);
                        
                        fprintf(file,'          power_fraction %f;\n',p_frac);
                        fprintf(file,'          impedance_fraction %f;\n',z_frac);
                        fprintf(file,'          current_fraction %f;\n',i_frac);
                        
                        power_pf = p_pf;
                        fprintf(file,'          power_pf %1.3f;\n',power_pf);
                        
                        current_pf = i_pf;
                        fprintf(file,'          current_pf %1.3f;\n',current_pf);
                        
                        impedance_pf = z_pf;
                        fprintf(file,'          impedance_pf %1.3f;\n',impedance_pf);
                        
                        heat_frac = heat_fraction - (0.2)*heat_fraction + 2*(0.2)*heat_fraction*rand(1);
                        fprintf(file,'          heat_fraction %0.2f;\n',heat_frac);
                        
                        fprintf(file,'     };\n');
                    end   

                end
            end
        end

        % Add the water heater if needed
        if (wd_scale ~= 0)
            if (randomize == 1)
                wh_test = rand(1);
            end
            
            if (wh_test <= wh_elec)
                % comfort level estimated from Olypen report - 89% at 0 or 0.5
                comf_level = 3*market_info{7} ;

                fprintf(file,'     object waterheater {\n');
                fprintf(file,'     name WH_%d%s;\n',jind,tag);
                fprintf(file,'          location INSIDE;\n');
                fprintf(file,'          schedule_skew %.0f;\n',sch_skew);
                
                wh_schedule = randi(no_of_water_sch);
                wh_rand = wd_scale*((wtrdem_1-wtrdem_2)+2*wtrdem_2*rand(1));
                fprintf(file,'          demand %s_%s_%d*%.3f;\n',wh_type1,wh_type2,wh_schedule,wh_rand);

                if (randomize == 1)
                    tank_volume = (tankvol_1-tankvol_2)+2*tankvol_2*rand(1);
                end
                
                fprintf(file,'          tank_volume %.1f;\n',tank_volume);
                
                if (randomize == 1)
                    tank_UA = (tankUA_1-tankUA_2)+2*tankUA_2*rand(1);
                end
                fprintf(file,'          tank_UA %.3f;\n',tank_UA);

                if (randomize == 1)
                    element = (heatcap_1-heatcap_2)+2*heatcap_2*rand(1);
                end
                fprintf(file,'          heating_element_capacity %.0f W;\n',element);

                tank_setpoint=(tankset_1-tankset_2)+2*tankset_2*rand(1);
                fprintf(file,'          tank_setpoint %.2f;\n',tank_setpoint);
                fprintf(file,'          temperature %.2f;\n',tank_setpoint+rand(1));

                thermostat_deadband = (thermdb_1-thermdb_2)+2*thermdb_2*rand(1);
                fprintf(file,'          thermostat_deadband %.3f;\n',thermostat_deadband);

                if (strcmp(market_info{1},'MARKET') ~= 0 && market_info{6} >= market_test)
                    fprintf(file,'          object passive_controller {\n');
                    fprintf(file,'	             period %.0f;\n',market_info{3});
                    fprintf(file,'	             control_mode PROBABILITY_OFF;\n');
                    fprintf(file,'	             distribution_type NORMAL;\n');
                    fprintf(file,'	             observation_object %s;\n',market_info{2});
                    fprintf(file,'	             observation_property current_market.clearing_price;\n');
                    fprintf(file,'	             stdev_observation_property %s;\n',market_info{5});
                    fprintf(file,'	             expectation_object %s;\n',market_info{2});
                    fprintf(file,'	             expectation_property %s;\n',market_info{4});
                    fprintf(file,'	             comfort_level %.2f;\n',comf_level);
                    fprintf(file,'	             state_property override;\n');
                    fprintf(file,'          };\n');                
                end
                fprintf(file,'     };\n');
            end
        end    
        fprintf(file,'}\n\n');
    else
        cool_slider = 0; 
        a = 0.6; 
        b = 1.4; 
        temp_rand_num = a + (b-a).*rand(1);
        
        fprintf(file,'object house {\n');
        fprintf(file,'     name house%d_%s;\n',houses,name);
        fprintf(file,'     //%s\n',house_tag);
        fprintf(file,'     parent %s;\n',parent);
        fprintf(file,'     system_mode OFF;\n');
        fprintf(file,'     last_system_mode OFF;\n');
        fprintf(file,'     heating_system_type NONE;\n'); 
        fprintf(file,'     cooling_system_type NONE;\n');
        fprintf(file,'     auxiliary_system_type NONE;\n');     
                        
        % Randomize the schedule time offset, but keep it the same
        % throughout the house
        sch_skew = 1200 * randn(1);
        if sch_skew > 3600
            sch_skew = 3600;
        elseif sch_skew < -3600
            sch_skew = -3600;
        end     
        fprintf(file,'     schedule_skew %.0f;\n',sch_skew);
        
        fprintf(file,'     object ZIPload {\n');
        fprintf(file,'          base_power SNOWBIRD.value*%0.4f;\n', temp_rand_num);
        fprintf(file,'          power_fraction %f;\n',0);
        fprintf(file,'          impedance_fraction %f;\n',1);
        fprintf(file,'          current_fraction %f;\n',0);               
        fprintf(file,'          power_pf %1.3f;\n',0);                
        fprintf(file,'          current_pf %1.3f;\n',0);                
        fprintf(file,'          impedance_pf %1.3f;\n',1);              
        fprintf(file,'     };\n');
        fprintf(file,'}\n\n');
    end             
   
end
