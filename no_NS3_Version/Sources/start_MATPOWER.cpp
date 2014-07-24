/*
Launching the MATPOWER Optimal Power Flow solver
  - MATPOWER OPF resides as a shared object in libopf.so, after being compiled on a computer with MATLAB isntalled.
  - Running the MATPOWER OPF requires that at least Matlab Compiler Runtime (downloaded for free from MATHWORKS webpage) is installed.
  - The code below reads the data file that resides in a .m file (the MATPOWER case file).
  - It creates the data structure needed by the OPF solver, calls the solver, and returns whatever it is desired.
  - Files needed for deployment (for this case, at least, in order to be able to compile): start_MATPOWER.cpp, libopf.h, libopf.so, libmpoption.so, libmpoption.h, case9.m, matpowerintegrator.h, matpowerintegrator.c, and the newly added, librunpf.so, librunpf.h, librunopf.so, librunopf.h.
==========================================================================================
Copyright (C) 2013, Battelle Memorial Institute
Written by Laurentiu Dan Marinovici, Pacific Northwest National Laboratory
Updated: 03/05/2014
   Purpose: Implement possibility of running it with multiple instances of GridLAB-D.
Updated: 03/21/2014
   Purpose: Added the possibility to change the generation/trsnmission topology, by making on generator go off-line.
            Branches could also be set-up to go off-line. (not implemented yet though).
Updated: 04/08/2014
   Purpose: Ability to run both the regular power flow and the optimal power flow.
            The optimal power flow is going to be solved 5 seconds before the end of every minute,
            to be able to communicate the newly calculated price to GLD in time.
Updated: 05/02/2014
   Purpose: Add the ability to receive a load profile as the "static load" at the feeder buses, profile that would simulate a real-life one day load profile
   WARNING: Currently, the code is written to accomodate the model used, that is there are only 6 load buses (out of a total of 9 buses), and only 3 out of these 6
            have non-zero loads, where the profile is going to be modified such that it follows a 24h real-life-like shape.
Updated: 06/17/2014
   Purpose: Took out some of the functions to separate cpp files and created read_input_data.h header that includes all the functions required to read the simulation model.\
Last updated: 07/02/2014
   Purpose: Modified the read load profile function in read_load_profile.cpp, to be able to read as many profiles as neccessary, depending on how many substations I have.
            Basically, the load profile data comes into a file containing 288 values per row (every 5-minute data for 24 hours), and a number of rows greater than or equal to the number of substations.
            
==========================================================================================
*/
#include <stdio.h>
#include <math.h>
// #include <windows.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
using namespace std;
// #include <shellapi.h>
#include "libopf.h"
#include "librunpf.h"
#include "librunopf.h"
#include "libmpoption.h"
#include "/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/extern/include/mclmcrrt.h"
#include "/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/extern/include/mclcppclass.h"
#include "/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/extern/include/matrix.h"

# define PI 3.14159265

#include "matpowerintegrator.h"
#include "read_input_data.h"

// Transposing matrices; wee need this function becuase the way the file reading is done leads to the transpose of the necessary matrix
// Careful: it is 1-base indexing because we are working with MATLAB type array mwArray
mwArray mwArrayTranspose(int nrows, int ncolumns, mwArray matrix_in) {
   mwArray matrix_out(nrows, ncolumns, mxDOUBLE_CLASS);
   for (int ind_row = 1; ind_row <= nrows; ind_row++) {
   for (int ind_col = 1; ind_col <= ncolumns; ind_col++) {
      matrix_out(ind_row, ind_col) = matrix_in(ind_col, ind_row); } }
   return matrix_out; }


int run_main(int argc, char **argv) {
   if (!mclInitializeApplication(NULL, 0)) {
      cerr << "Could not initialize the application properly !!!" << endl;
      return -1;
      }
   if (!libopfInitialize() || !libmpoptionInitialize() || !librunpfInitialize() || !librunopfInitialize()) {
      cerr << "Could not initialize one or more libraries properly !!!" << endl;
      return -1;
      }
   else {
      try {
// ================================ VARIABLE DECLARATION AND INITIALIZATION =============================================
         cout << "Just entered the MAIN function of the driver application." << endl;
         // Initialize the input parameters giving the MATPOWER model file, and the load profile
         char *file_name; // {"case9.m"};
         file_name = argv[1];
         // char load_profile_file[] = {"real_power_demand.txt"};
         char *load_profile_file;
         load_profile_file = argv[2];
// ======================================================================================================================
         // Read the MATPOWER transmission model file in order to get the suze of the system, that is number of busses, generators, etc.
         // These dimensions are needed to be able to create the model matrices later without dynamic allocation of memory.
         // Declaration of dimension variables.
         int nbrows = 0, nbcolumns = 0, ngrows = 0, ngcolumns = 0;
         int nbrrows = 0, nbrcolumns = 0, narows = 0, nacolumns = 0;
         int ncrows = 0, nccolumns = 0, nFNCSelem = 0, noffgelem = 0, NS3_flag = 0;

         read_model_dim(file_name, &nbrows, &nbcolumns, &ngrows, &ngcolumns,
              &nbrrows, &nbrcolumns, &narows, &nacolumns,
              &ncrows, &nccolumns, &nFNCSelem, &noffgelem);
         /*
         cout << nbrows << '\t' << nbcolumns << '\t' << ngrows << '\t' << ngcolumns << '\t' << endl;
         cout << nbrrows << '\t' << nbrcolumns << '\t' << narows << '\t' << nacolumns << endl;
         cout << ncrows << '\t' << nccolumns << '\t' << nFNCSSub << '\t' << noffGen << endl;
         */
// ========================================================================================================================
         // Load profile for the "static" load at all the buses.
         // The number of profiles should be at least equal to the number of feeders nFNCSelem, which is given in the MATPOWER case file. Careful, though!!!
         // Each profile needs to start from the value that exists initially in the MATPOWER model at the specific bus.
         // Each profile consists of data for 24 hours every 5 minutes (288 values taken repeatedly every day)
         double real_power_demand[nFNCSelem][288], reactive_power_demand[nFNCSelem][288];
         for (int i = 0; i < sizeof(real_power_demand)/sizeof(real_power_demand[0]); i++) {
            for (int j = 0; j < sizeof(real_power_demand[0])/sizeof(real_power_demand[0][0]); j++) {
               real_power_demand[i][j] = 0;
               reactive_power_demand[i][j] = 0;
            }
         }
         // Get load profile data, to make the load evolution in time more realistic
         
         read_load_profile(load_profile_file, real_power_demand, nFNCSelem);
         // Printing out the values just for the sake of testing
         for (int i = 0; i < sizeof(real_power_demand)/sizeof(real_power_demand[0]); i++){
            for (int j = 0; j < sizeof(real_power_demand[0])/sizeof(real_power_demand[0][0]); j++){
               cout << real_power_demand[i][j] << " ";
            }
            cout << endl;
         }
// ========================================================================================================================
         // Rest of the variables declaration
         double baseMVA, nomfreq;
         double amp_fact; // amplification factor for the controlable load.
         // The powerflow solution is going to be calculated in the following variables
         mwArray mwMVAbase, mwBusOut, mwGenOut, mwBranchOut, f, success, info, et, g, jac, xr, pimul, mwGenCost;
         // Results from RUNPF or RUNOPF will be saved as in MATLAB in a mat file, and printed in a nice form in a file
         mwArray printed_results(mwArray("printed_results.txt"));
         mwArray saved_results(mwArray("saved_results.mat"));
//         double mwBusOut_copy[9];
         int repeat = 1;
         // matrix dimensions based on test case; they need to be revised if other case is used
         // for C code we need the total number of elements, while the matrices will be passed to MATLAB as mwArray with rows and columns
         // BUS DATA MATRIX DEFINITION
         // bus matrix dimensions, and total number of elements
         // int nbrows = 9, nbcolumns = 13, nbelem = nbrows * nbcolumns;
         int nbelem = nbrows * nbcolumns;
         double bus[nbelem];
         mwArray mwBusT(nbcolumns, nbrows, mxDOUBLE_CLASS); // given the way we read the file, we initially get the transpose of the matrix
         mwArray mwBus(nbrows, nbcolumns, mxDOUBLE_CLASS);
         // GENERATOR DATA MATRIX DEFINITION
         // generator matrix dimensions, and total number of elements
         // int ngrows = 3, ngcolumns = 21, ngelem = ngrows * ngcolumns;
         int ngelem = ngrows * ngcolumns;
         double gen[ngelem];
         mwArray mwGenT(ngcolumns, ngrows, mxDOUBLE_CLASS);
         mwArray mwGen(ngrows, ngcolumns, mxDOUBLE_CLASS);
         // BRANCH DATA MATRIX DEFINITION
         // branch matrix dimensions, and total number of elements
         // int nbrrows = 9, nbrcolumns = 13, nbrelem = nbrrows * nbrcolumns;
         int nbrelem = nbrrows * nbrcolumns;
         double branch[nbrelem];
         mwArray mwBranchT(nbrcolumns, nbrrows, mxDOUBLE_CLASS);
         mwArray mwBranch(nbrrows, nbrcolumns, mxDOUBLE_CLASS);
         // AREA DATA MATRIX DEFINITION
         // area matrix dimensions, and total number of elements
         // int narows = 1, nacolumns = 2, naelem = narows * nacolumns;
         int naelem = narows * nacolumns;
         double area[naelem];
         mwArray mwAreaT(nacolumns, narows, mxDOUBLE_CLASS);
         mwArray mwArea(narows, nacolumns, mxDOUBLE_CLASS);
         // GENERATOR COST DATA MATRIX DEFINTION
         // generator cost matrix dimensions, and total number of elements
         // int ncrows = 3, nccolumns = 7, ncelem = ncrows * nccolumns;
         int ncelem = ncrows * nccolumns;
         double costs[ncelem];
         mwArray mwCostsT(nccolumns, ncrows, mxDOUBLE_CLASS);
         mwArray mwCosts(ncrows, nccolumns, mxDOUBLE_CLASS);
         // SUBSTATION BUS NUMBERS AND NAMES FOR FNCS COMMUNICATION
         // the bus numbers names where the substations are connected to, and the corresponding real and imaginary power
         int bus_num[nFNCSelem];
         double bus_valueReal[nFNCSelem], bus_valueIm[nFNCSelem];
         // substation names
         char sub_name[nFNCSelem][15];
         // market names
         char market_name[nFNCSelem][11];
         // static active and reactive power at the buses that are connected to substations
         double static_pd[nFNCSelem], static_qd[nFNCSelem];
         // calculated real and imaginary voltage at the buses that are connected to substations
         double sendValReal[nFNCSelem], sendValIm[nFNCSelem];
         // calculated LMP values at the buses that are connected to substations
         double realLMP[nFNCSelem], imagLMP[nFNCSelem];
         // bus index in the MATPOWER bus matrix corresponding to the buses connected to substations
         int modified_bus_ind[nFNCSelem];
         int mesgc[nFNCSelem]; // synchronization only happens when at least one value is received
         bool mesg_rcv = false; // if at least one message is passed between simulators, set the message received flag to TRUE
         bool mesg_snt = false; // MATPOWER is now active, it will send a message that major changes happened at transmission level
         bool solved_opf = false; // activates only when OPF is solved to be able to control when price is sent to GLD
         bool topology_changed = false; // activates only if topology changed, like if a generator is turned off form on, or vice-versa, for example.
         // Generator bus matrix consisting of bus numbers corresponding to the generators that could become out-of service,
         // allowing us to set which generators get off-line, in order to simulate a reduction in generation capacity.
         // MATPOWER should reallocate different generation needs coming from the on-line generators to cover for the lost ones, since load stays constant
         // for MATPOWER: value >  0 means generator is in-service
         //                     <= 0 means generator is out-of-service
         // number of rows and columns in the MATPOWER structure, and the total number of buses
         // int noffgrows = 1, noffgcolumns = 1, noffgelem = noffgrows*noffgcolumns;
         int offline_gen_bus[noffgelem], offline_gen_ind[noffgelem];
         // times recorded for visualization purposes
         int curr_time = 0; // current time in seconds
         int curr_hours = 0, curr_minutes = 0, curr_seconds = 0; // current time in hours, minutes, seconds
         int delta_t[nFNCSelem], prev_time[nFNCSelem]; // for each substation, we save the time between 2 consecutive received messages
         for (int i = 0; i < nFNCSelem; i++) {
            delta_t[i] = 0;
            prev_time[i] = 0;
            realLMP[i] = 0;
            imagLMP[i] = 0;
         }
         // output files for saving results
         char subst_output_file_name[nFNCSelem][18]; // one file for each substation
         char gen_output_file_name[ngrows][17]; // one file for each generator; there are ngrows generators in this case; needs to be changed for general purpose, maybe using ngrows
         ofstream subst_output_file, gen_output_file;
         for (int i = 0; i < sizeof(subst_output_file_name)/sizeof(subst_output_file_name[0]); i++) {
            snprintf(subst_output_file_name[i], sizeof(subst_output_file_name[i]), "Substation_%d.csv", i+1);
            ofstream subst_output_file(subst_output_file_name[i], ios::out);
            subst_output_file << "Time (seconds), Real Power Demand - PD (MW), Reactive Power Demand (MVAr), Substation V real (V), Substation V imag (V), LMP ($/kWh), LMP ($/kVArh)" << endl;
         }
         // Turning off the Generator file creation for a while. Uncomment the lines below to have them created again.
         // for (int i = 0; i < sizeof(gen_output_file_name)/sizeof(gen_output_file_name[0]); i++) {
         //    snprintf(gen_output_file_name[i], sizeof(gen_output_file_name[i]), "Generator_%d.csv", i+1);
         //    ofstream gen_output_file(gen_output_file_name[i], ios::out);
         //    gen_output_file << "Time (seconds), STATUS, PMAX (MW), PMIN (MW), Real power output - PG (MW), QMAX (MVAr), QMIN (MVAr), Reactive power output - QG (MVAr)" << endl;
         // }

// ========================================================================================================================
         // Creating the MPC structure that is going to be used as input for OPF function
         const char *fields[] = {"baseMVA", "bus", "gen", "branch", "areas", "gencost"};
         mwArray mpc(1, 1, 6, fields);
         // Variable that will hold both the mpc structure and the options for the opf function
         mwArray opfIn(1, 2, mxCELL_CLASS);

         // Creating the variable that would set the options for the OPF solver
         mwArray mpopt(124, 1, mxDOUBLE_CLASS); // there are 124 options that can be set
         mwArray mpoptNames(124, 18, mxCHAR_CLASS); // there are 124 option names and the maximum length is 18, but made it to 20
         cout << "=================================================" << endl;
         cout << "========= SETTING UP THE OPTIONS !!!!!===========" << endl;
         cout << "Setting initial options" << endl;
         mpoption(2, mpopt, mpoptNames); // initialize powerflow options to DEFAULT ones
         cout << "Finished setting the initial options" << endl;
         // cout << "mpopt = " << mpopt << endl;
         // cout << "mpoptNames = " << mpoptNames << endl;
         mwArray optIn(1, 3, mxCELL_CLASS); // this holds the initial option vector, the property name that will be set up, and the new value for that property
         optIn.Get(1, 1).Set(mpopt);
         optIn.Get(1, 2).Set(mwArray("PF_DC")); // name of the option that could be modified, e.g. PF_DC
         optIn.Get(1, 3).Set(mwArray(1)); // value of the modified option, e.g. 0 or 1 for false or true
         mpoption(2, mpopt, mpoptNames, optIn); //, optionName, optionValue); // Setting up the DC Power Flow
         optIn.Get(1, 1).Set(mpopt); // Update the option vector to the one with one property changed. Problem is, we have to do this avery time we change one option.
         optIn.Get(1, 2).Set(mwArray("VERBOSE"));
         optIn.Get(1, 3).Set(mwArray(0)); // Setting the VERBOSE mode OFF, so we do not see all the steps on the terminal
         mpoption(2, mpopt, mpoptNames, optIn); //, optionName, optionValue);
         optIn.Get(1, 1).Set(mpopt); // Update the option vector to the one with one property changed. Problem is, we have to do this avery time we change one option.
         optIn.Get(1, 2).Set(mwArray("OUT_ALL"));
         optIn.Get(1, 3).Set(mwArray(0)); // Setting the OUT_APP mode OFF, so we do not see all the results printed at the terminal
         mpoption(2, mpopt, mpoptNames, optIn); //, optionName, optionValue);
         optIn.Get(1, 1).Set(mpopt); // Update the option vector to the one with one property changed. Problem is, we have to do this avery time we change one option.
         opfIn.Get(1, 2).Set(mpopt); // Setting up the second input argument of the opf function representing the power flow options
// ================================ END OF VARIABLE DECLARATION AND INITIALIZATION =============================================
// =============================================================================================================================
         // get the MATPOWER model data
         read_model_data(file_name, nbrows, nbcolumns, ngrows, ngcolumns, nbrrows, nbrcolumns, narows, nacolumns,
                  ncrows, nccolumns, nFNCSelem, noffgelem, &baseMVA, bus, gen,
                  branch, area, costs, bus_num, sub_name, market_name, offline_gen_bus, &amp_fact, &NS3_flag);
         mwBusT.SetData(bus, nbelem);
         // Transposing mwBusT to get the correct bus matrix
         // Careful: it is 1-base indexing because we are working with MATLAB type array mwArray
         mwBus = mwArrayTranspose(nbrows, nbcolumns, mwBusT);

         mwArray mwBusDim = mwBus.GetDimensions();
         cout << "============= TESTING THROUGH VISUALIZATION... HAHAHAHAHAHAHA =============" << endl;
         cout << "baseMVA = " << baseMVA << endl;
         cout << "=================================" << endl;
         cout << "NS3 flag = " << NS3_flag << endl;
         cout << "==================================" << endl;
         // cout << "mwBus = " << mwBus << endl;
         // cout << "==================================" << endl;

         mwGenT.SetData(gen, ngelem);
         mwGen = mwArrayTranspose(ngrows, ngcolumns, mwGenT);

         cout << "==================================" << endl;
         cout << "mwGen = " << mwGen << endl;
         cout << "==================================" << endl;

         mwBranchT.SetData(branch, nbrelem);
         mwBranch = mwArrayTranspose(nbrrows, nbrcolumns, mwBranchT);

         cout << "==================================" << endl;
         cout << "mwBranch = " << mwBranch << endl;
         cout << "==================================" << endl;

         mwAreaT.SetData(area, naelem);
         mwArea = mwArrayTranspose(narows, nacolumns, mwAreaT);

         cout << "==================================" << endl;
         cout << "mwArea = " << mwArea << endl;
         cout << "==================================" << endl;

         mwCostsT.SetData(costs, ncelem);
         mwCosts = mwArrayTranspose(ncrows, nccolumns, mwCostsT);

         cout << "==================================" << endl;
         cout << "mwCosts = " << mwCosts << endl;
         cout << "==================================" << endl;

         cout << "==================================" << endl;
         cout << "mpc.SubBusFNCS = ";
         for (int sub_ind = 0; sub_ind < sizeof(bus_num)/sizeof(bus_num[0]); sub_ind++){
            cout << "\t" << bus_num[sub_ind];
         }
         cout << endl;
         cout << "==================================" << endl;

         cout << "===========solvedcase=======================" << endl;
         cout << "mpc.SubNameFNCS = ";
         for (int sub_ind = 0; sub_ind < sizeof(bus_num)/sizeof(bus_num[0]); sub_ind++){
            cout << "\t" << sub_name[sub_ind];
         }
         cout << endl;
         cout << "==================================" << endl;

         cout << "==================================" << endl;
         cout << "mpc.MarketNameFNCS = ";
         for (int sub_ind = 0; sub_ind < sizeof(bus_num)/sizeof(bus_num[0]); sub_ind++){
            cout << "\t" << market_name[sub_ind];
         }
         cout << endl;
         cout << "==================================" << endl;

         cout << "==================================" << endl;
         cout << "mpc.offlineGenBus = ";
         for (int off_ind = 0; off_ind < sizeof(offline_gen_bus)/sizeof(offline_gen_bus[0]); off_ind++){
            cout << "\t" << offline_gen_bus[off_ind];
         }
         cout << endl;
         cout << "==================================" << endl;

         cout << "==================================" << endl;
         cout << "mpc.ampFactor = " << amp_fact << endl;
         cout << "==================================" << endl;

         // Initialize the MPC structure with the data read from the file
         mpc.Get("baseMVA", 1, 1).Set((mwArray) baseMVA);
         mpc.Get("bus", 1, 1).Set(mwBus);
         mpc.Get("gen", 1, 1).Set(mwGen);
         mpc.Get("branch", 1, 1).Set(mwBranch);
         mpc.Get("areas", 1, 1).Set(mwArea);
         mpc.Get("gencost", 1, 1).Set(mwCosts);
         cout << "=========================================" << endl;
         cout << "===== Initial MPC structure created =====\n" << mpc << endl;
         cout << "=========================================" << endl;

         for (int sub_ind = 0; sub_ind < sizeof(bus_num)/sizeof(bus_num[0]); sub_ind++) {
            // The index of the bus in the bus matrix could be different from the number of the bus
            // because buses do not have to be numbered consecutively, or be the same as the index
            for (int ind = 1; ind <= nbrows; ind++) {
               // ind is an index in MATLAB, that is it shouldstart at 1
               // In mpc.Get("bus", 1, 1).Get(2, ind, 1), the 2 in the second Get represents the number of indeces the array has
               if ((int) mpc.Get("bus", 1, 1).Get(2, ind, 1) == bus_num[sub_ind]) {
                  modified_bus_ind[sub_ind] = ind;
                  static_pd[sub_ind] = mpc.Get("bus", 1, 1).Get(2, ind, 3);
                  static_qd[sub_ind] = mpc.Get("bus", 1, 1).Get(2, ind, 4);
                  cout << sub_name[sub_ind] << " IS AT BUS NUMBER " << bus_num[sub_ind] << ", ";
                  cout << "WHICH IS AT LOCATION " << modified_bus_ind[sub_ind] << " IN THE MATPOWER BUS MATRIX." << endl;
                  cout << "Initially, the static ACTIVE power at bus " << bus_num[sub_ind] << " is " << static_pd[sub_ind] << "." << endl;
                  cout << "Initially, the static REACTIVE power at bus " << bus_num[sub_ind] << " is " << static_qd[sub_ind] << "." << endl;
               }
            }
         }

         // Find the index in the MATPOWER generator matrix corresponding to the buses that could be turned off
         // The bus number and the actual index in the MATPOWER matrix may not coincideprint(fig_handle, '-dpdf', '-r600', ['comparative_freq_res.pdf'])
         for (int off_ind = 0; off_ind < sizeof(offline_gen_bus)/sizeof(offline_gen_bus[0]); off_ind++){
            for (int gen_ind = 1; gen_ind <= ngrows; gen_ind++){ // in MATLAB indexes start from 1
               if((int) mpc.Get("gen", 1, 1).Get(2, gen_ind, 1) == offline_gen_bus[off_ind]){
                  offline_gen_ind[off_ind] = gen_ind; // index of the bus in the MATPOWER matrix
                  cout << "GENERATOR AT BUS " << mpc.Get("gen", 1, 1).Get(2, offline_gen_ind[off_ind], 1) << " MIGHT BECOME OFF-LINE!!!!" << endl;
               }
            }
         }

          // ==========================================================================================================
          // Uncomment the line below when running with FNCS
          initn(bus_num, &nFNCSelem); // initialize the bus number for FNCS
          // ==========================================================================================================
         
          do {
            // Start every time assuming no message is received or sent
            mesg_rcv = false;
            mesg_snt = false;
            solved_opf = false;
            topology_changed = false;
            // ==========================================================================================================
            // Uncomment the line below when running with FNCS
            startcalculation();
            // ==========================================================================================================
            // =============== CURRENT SIMULATION TIME ==================================================================
            // Uncomment the line below when running with FNCS
            curr_time = getCurrentTime();
            // ==========================================================================================================
            curr_hours = curr_time/3600;
            curr_minutes = (curr_time - 3600*curr_hours)/60;
            curr_seconds = curr_time - 3600*curr_hours - 60*curr_minutes;
            // Setting the load at the load buses based on the load profiles
            // In this case, the model has 6 load buses, out of which only 3 had non-zero values originally; so we stick to only those 
            // getting in a one-day long profile. WARNING: if the model is changed these need to be readjusted
            if (curr_time % 300 == 0) {
               /*
               cout << "\033[2J\033[1;1H"; // Just a trick to clear the screen before pritning the new results at the terminal
               cout << "================== It has been " << curr_hours << " hours, " << curr_minutes << " minutes, and ";
               cout << curr_seconds << " seconds. ========================" << endl;
               cout << "index -->> " << 12 * (curr_hours % 24) + curr_minutes / 5 << endl;
               */
               for (int sub_ind = 0; sub_ind < sizeof(bus_num)/sizeof(bus_num[0]); sub_ind++) {
                  mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 3).Set((mwArray) real_power_demand[sub_ind][12 * (curr_hours % 24) + curr_minutes / 5]);
                  static_pd[sub_ind] = real_power_demand[sub_ind][12 * (curr_hours % 24) + curr_minutes / 5];
                  // cout << "@ feeder " << sub_ind+1 << " -->> " << static_pd[sub_ind] << endl;
                  // mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 4).Set((mwArray) reactive_power_demand[curr_hours % 24 + curr_minutes / 5][sub_ind]);
                  // static_qd[sub_ind] = reactive_power_demand[curr_hours % 24 + curr_minutes / 5][sub_ind];
               }
            }
            // Setting up the status of the generators, based on the current time
            // Turning some generators out-of-service between certain time preiods in the simulation 
            // every day between 6 and 7 or 18 and 19 !!! WARNING !!! These hours are hard coded assuming we run more than 24 hours
            if ((curr_hours % 24 >= 6 && curr_hours % 24 < 7) || (curr_hours % 24 >= 18 && curr_hours % 24 < 19)){ 
               for (int off_ind = 0; off_ind < sizeof(offline_gen_ind)/sizeof(offline_gen_ind[0]); off_ind++){
                  if ((double) mpc.Get("gen", 1, 1).Get(2, offline_gen_ind[off_ind], 8) == 1){ // if generator is ON
                     mpc.Get("gen", 1, 1).Get(2, offline_gen_ind[off_ind], 8).Set((mwArray) 0); // turn generator OFF, and set flag that topology has changed
                     topology_changed = topology_changed || true; // signal that at least one generator changed its state
//                     mesg_snt = mesg_snt || true; // signal that at least one generator changed its state
                     // Uncomment after testing the load profile loading correctly
                     cout << "============ Generator at bus " << mpc.Get("gen", 1, 1).Get(2, offline_gen_ind[off_ind], 1);
                     cout << " is put OUT-OF-SERVICE. ==================" << endl;
                  }
               }
            }
            else {
               for (int off_ind = 0; off_ind < sizeof(offline_gen_ind)/sizeof(offline_gen_ind[0]); off_ind++){
                  if ((double) mpc.Get("gen", 1, 1).Get(2, offline_gen_ind[off_ind], 8) == 0){ // if generator is OFF
                     mpc.Get("gen", 1, 1).Get(2, offline_gen_ind[off_ind], 8).Set((mwArray) 1); // turn generator ON, and set flag that topology changed
                     topology_changed = topology_changed || true;// signal that at least one generator changed its state
//                     mesg_snt = mesg_snt || true; // signal that at least one generator changed its state
                     // Uncomment after testing the load profile loading correctly
                     cout << "============ Generator at bus " << mpc.Get("gen", 1, 1).Get(2, offline_gen_ind[off_ind], 1);
                     cout << " is brought back IN-SERVICE. ==================" << endl;
                  }
               }
            }
            for (int sub_ind = 0; sub_ind < sizeof(bus_num)/sizeof(bus_num[0]); sub_ind++) {
               // ==========================================================================================================
               // Uncomment the line below when running with FNCS
               getpower(&bus_num[sub_ind], &mesgc[sub_ind], &bus_valueReal[sub_ind], &bus_valueIm[sub_ind]);
               // ==========================================================================================================
               // Uncomment the line below when working off-line from FNCS
               // cout << "Simulate whether " << sub_name[sub_ind] << " received message or not. Enter 0 for NO, and 1 for YES." << endl;
               // cout << "YES [1] or NO [0]?\t";
               // cin >> mesgc[sub_ind];
               mesg_rcv =  mesg_rcv || (bool) mesgc[sub_ind];
               if (mesgc[sub_ind] == 1) {
                  delta_t[sub_ind] = curr_time - prev_time[sub_ind]; // number of seconds between 2 consecutive received messages
                  // If it is simulated that there's an incoming message, than we need to give a value.
                  // These are constant values used to test the code when running without FNCS and GLD.
                  // Need to be commented when the code is used in FNCS, and uncommented when running off-line from FNCS
                  // cout << "WHAT IS REAL LOAD CHANGE FROM " << sub_name[sub_ind] << " ?" << endl;
                  // cout << "Active power in MW:\t";100
                  // cin >> bus_valueReal[sub_ind];
                  // cout << "WHAT IS IMAGINARY LOAD CHANGE FROM " << sub_name[sub_ind] << " ?" << endl;
                  // cout << "Reactive power in MVAr:\t";
                  // cin >> bus_valueIm[sub_ind];
                  // It is assumed that the load at the bus consists of the initial constant load plus a controllable load coming from distribution (GridLAB-D)
                  // To simulate the idea of having a more substantial change in load at the substantion level, consider we have amp_fact similar models at on node
                  // That is why I multiply by amp_fact below.
                  mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 3).Set((mwArray) (static_pd[sub_ind] + amp_fact*bus_valueReal[sub_ind]));
                  mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 4).Set((mwArray) (static_qd[sub_ind] + amp_fact*bus_valueIm[sub_ind]));
               } // end IF(mesgc)
            } // end FOR(sub_ind)

            if (mesg_rcv){
               // ==========================================================================================================
               // Uncomment after testing the load profile loading correctly
               cout << "\033[2J\033[1;1H"; // Just a trick to clear the screen before pritning the new results at the terminal
               cout << "================== It has been " << curr_hours << " hours, " << curr_minutes << " minutes, and ";
               cout << curr_seconds << " seconds. ========================" << endl;
               cout << "================== GLD initiating message after changes at the distribution level. ==================" << endl;
               for (int sub_ind = 0; sub_ind < sizeof(bus_num)/sizeof(bus_num[0]); sub_ind++) {
                  if (mesgc[sub_ind] == 1) {
                     // Uncomment after testing the load profile loading correctly
                     cout << "================== NEW LOAD AT " << sub_name[sub_ind] << " AT BUS " << bus_num[sub_ind] << " =====================" << endl;
                     cout << "ACTIVE power required at the bus: " << mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 3) << " MW." << endl;
                     cout << "REACTIVE power required at the bus: " << mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 4) << " MW." << endl;
                     cout << "I've got the NEW POWER after " << delta_t[sub_ind] << " seconds." << endl;
                     prev_time[sub_ind] = curr_time;
                  }
                  else {
                     // Uncomment after testing the load profile loading correctly
                     cout << "================== NO LOAD CHANGE AT " << sub_name[sub_ind] << " AT BUS " << bus_num[sub_ind] << " =====================" << endl;
                  } // end IF(mesgc)
               } // end FOR(sub_ind)
            } // end IF(mesg_rcv)
            
            // Running the actual transmission simulator, by solving the power flow, or the optimal power flow
            if (curr_time % 300 == 295 || topology_changed){
               // Call OPF with nargout = 0 (first argument), and all results are going to be printed at the console
               // Call OPF with nargout = 7, and get all the output parameters up to et
               // Call OPF with nargout = 11, and get a freaking ERROR.... AVOID IT!
               // cout << "================= Solving the OPTIMAL POWER FLOW. ==================" << endl;
               opfIn.Get(1, 1).Set(mpc); // Setting up the first input parameter for opf function as the actual MPC model
               // opf(7, mwBusOut, mwGenOut, mwBranchOut, f, success, info, et, g, jac, xr, pimul, opfIn);
               runopf(7, mwMVAbase, mwBusOut, mwGenOut, mwGenCost, mwBranchOut, f, success, et, mpc, mpopt, printed_results, saved_results);
               mpc.Get("gen", 1, 1).Set(mwGenOut);
               mpc.Get("bus", 1, 1).Set(mwBusOut);
               mpc.Get("branch", 1, 1).Set(mwBranchOut);
               mpc.Get("gencost", 1, 1).Set(mwGenCost);
               solved_opf = true;
               mesg_snt = mesg_snt || true;
            }            
            else {
               // cout << "================= Solving the POWER FLOW. ==================" << endl;
               // opfIn.Get(1, 1).Set(mpc);
               runpf(6, mwMVAbase, mwBusOut, mwGenOut, mwBranchOut, success, et, mpc, mpopt, printed_results, saved_results);
               // Bring system in the new state by replacing the bus, generator, branch and generator cost matrices with the calculated ones
               mpc.Get("gen", 1, 1).Set(mwGenOut);
               mpc.Get("bus", 1, 1).Set(mwBusOut);
               mpc.Get("branch", 1, 1).Set(mwBranchOut);
               // runpf(6, mwMVAbase, mwBusOut, mwGenOut, mwBranchOut, success, et, opfIn.Get(1, 1), opfIn.Get(1, 2), printed_results, saved_results);
            }

            if (mesg_rcv || mesg_snt) {
               if (mesg_snt) { // only cleaning the screen when MATPOWER initiates the message transfer; otherwise is cleaned when message is received
                  // Uncomment after testing the load profile loading correctly
                  cout << "\033[2J\033[1;1H"; // Just a trick to clear the screen before pritning the new results at the terminal
                  if (curr_time % 300 == 295 && !topology_changed) {
                     cout << "================== It has been " << curr_hours << " hours, " << curr_minutes << " minutes, and ";
                     cout << curr_seconds << " seconds. ========================" << endl;
                     cout << "================== MATPOWER initiating message after dispatching new generation profile. ==================" << endl;
                  }
                  else {
                     cout << "================== It has been " << curr_hours << " hours, " << curr_minutes << " minutes, and ";
                     cout << curr_seconds << " seconds. ========================" << endl;
                     cout << "================== MATPOWER initiating message after topology has been changed. ==================" << endl;
                  }
               }
               for (int sub_ind = 0; sub_ind < sizeof(bus_num)/sizeof(bus_num[0]); sub_ind++) {
                  sendValReal[sub_ind] =  (double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 8)*cos((double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 9) * PI / 180)*(double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 10)*1000; // real voltage at the bus based on the magnitude (column 8 of the output bus matrix) and angle in degrees (column 9 of the output bus matrix)
                  sendValIm[sub_ind] = (double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 8)*sin((double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 9) * PI / 180)*(double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 10)*1000; // imaginary voltage at the bus based on the magnitude (column 8 of the output bus matrix) and angle in degrees (column 9 of the output bus matrix)
                  cout << "================== SENDING OUT NEW VOLTAGE TO " << sub_name[sub_ind];
                  // Using the MATLAB access at the bus number vor verification purposes
                  cout << " AT BUS " << mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 1) << " =====================" << endl;
                  cout << "Voltage REAL part -->> " << sendValReal[sub_ind] << " V." << endl;
                  cout << "Voltage IMAGINARY part -->> " << sendValIm[sub_ind] << " V." << endl;
                  cout << "=================================================================" << endl;
                  // output_file << (double) mwBusOut.Get(2, modified_bus_ind, 8) << ", " << (double) mwBusOut.Get(2, modified_bus_ind, 9) << ", ";
                  // =========================================================================================================================
                  // Uncomment the line below when running with FNCS
                  sendvolt(&bus_num[sub_ind], sub_name[sub_ind], &sendValReal[sub_ind], &sendValIm[sub_ind]);
                  // =========================================================================================================================
                  // Price will be sent only when an OPF has been solved
                  if (solved_opf) {
                     realLMP[sub_ind] = (double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 14)/1000; // local marginal price based on the Lagrange multiplier on real power mismatch (column 14 of the output bus matrix
                     // =========================================================================================================================
                     if (NS3_flag == 1) {
                        // If NS3 is envolved, call this SENDPRICE function
                        // Uncomment the line below when running with FNCS and with NS3
                        printf("Price sent through NS3!!!!!!!!!!\n");
                        sendprice(&realLMP[sub_ind], market_name[sub_ind]);
                     }
                     else if (NS3_flag == 0){
                        // Uncomment the line below when running with FNCS and without NS3
                        printf("Price sent directly to Gridlab-D!!!!! No NS3 involved.\n");
                        sendprice_noNS3(&realLMP[sub_ind], &bus_num[sub_ind], market_name[sub_ind]);
                        // =========================================================================================================================
                     }
                     else {
                        printf("Communication flag is not properly set in the model file. It should be 0 or 1, not %d !!!!!\n", NS3_flag);
                        exit(EXIT_FAILURE);
                     }
                     imagLMP[sub_ind] = (double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 15)/1000; // local marginal price based on the Lagrange multiplier on reactive power mismatch (column 14 of the output bus matrix
                     cout << "================== SENDING OUT THE LMP TO " << sub_name[sub_ind];
                     cout << " AT BUS " << mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 1) << " =====================" << endl;
                     cout << "LMP (Lagrange multiplier on real power mismatch) -->> " << realLMP[sub_ind] << " $/kWh." << endl;
                     cout << "LMP (Lagrange multiplier on reactive power mismatch) -->> " << imagLMP[sub_ind] << " $/kVArh." << endl;
                     cout << "=================================================================" << endl;
                  }
               }
               // Saving the data of each time when at least one message had been exchanged to the corresponding CSV file
               for (int sub_ind = 0; sub_ind < nFNCSelem; sub_ind++){
                  ofstream subst_output_file(subst_output_file_name[sub_ind], ios::app);
                  subst_output_file << curr_time << "," << (double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 3) << "," << (double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 4) << ", " << sendValReal[sub_ind] << ", " << sendValIm[sub_ind] <<  ", " << realLMP[sub_ind] << ", " << imagLMP[sub_ind] << endl;               
               }

               for (int gen_ind = 0; gen_ind < ngrows; gen_ind++){ // in C indexes start from 0, but from MATLAB variables index needs to start from 1
                  ofstream gen_output_file(gen_output_file_name[gen_ind], ios::app);
                  gen_output_file << curr_time << "," << (int) mwGenOut.Get(2, gen_ind + 1, 8) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 9) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 10) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 2) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 4) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 5) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 3) << endl;
               }

            }
            // Line Below is from when running off-line, without FNCS
            // curr_time = curr_time + 1;
         }while(synchronize(!mesg_rcv || !mesg_snt));
          // use while(!mesg_rcv || !mesg_snt); // when running off-line from FNCS
          // OR
          // while(curr_time < 400);
          // use while(synchronize(!mesg_rcv || !mesg_snt)); // when involving FNCS

         cout << "Just executed the MATLAB function from the shared library." << endl;
         subst_output_file.close();
         // gen_output_file.close();

         // mxDestroyArray(data_file);
         //mxFree(file_name);
         //
         /*
         mwDestroyArray(mwBusOut);
         mxDestroyArray(mwGenOut);
         mxDestroyArray(mwBranchOut);
         mxDestroyArray(f);
         mxDestroyArray(success);
         mxDestroyArray(info);
         mxDestroyArray(et);
         mxDestroyArray(g);
         mxDestroyArray(jac);
         mxDestroyArray(xr);
         mxDestroyArray(pimul);
         */
         }
      catch (const mwException& e) {
         cerr << e.what() << endl;
         cout << "Caught an error!!!" << endl;
         return -2;
         }
      catch (...) {
         cerr << "Unexpected error thrown" << endl;
         return -3;
         }
   libopfTerminate();
   librunpfTerminate();
   librunopfTerminate();
   libmpoptionTerminate();
   }
   mclTerminateApplication();
   return 0;
}

/* ==================================================================================================================
====================== MAIN PART ====================================================================================
===================================================================================================================*/
int main(int argc, char* argv[]) {	
   mclmcrInitialize();
   // return mclRunMain((mclMainFcnType) run_main, 0, NULL);
   cout << argc << endl;
   cout << argv[0] << endl;
   cout << argv[1] << endl;
   cout << argv[2] << endl;
   return mclRunMain((mclMainFcnType) run_main(argc, argv), 0, NULL);
}
