/*
Launching the MATPOWER Optimal Power Flow solver
  - MATPOWER OPF resides as a shared object in libopf.so, after being compiled on a computer with MATLAB isntalled.
  - Running the MATPOWER OPF requires that at least Matlab Compiler Runtime (downloaded for free from MATHWORKS webpage) is installed.
  - The code below reads the data file that resides in a .m file (the MATPOWER case file).
  - It creates the data structure needed by the OPF solver, calls the solver, and returns whatever it is desired.
  - Files needed for deployment (for this case, at least, in order to be able to compile): start_MATPOWER.cpp, libopf.h, libopf.so, libmpoption.so, libmpoption.h, case9.m, matpowerintegrator.h, matpowerintegrator.c.
==========================================================================================
Copyright (C) 2013, Battelle Memorial Institute
Written by Laurentiu Dan Marinovici, Pacific Northwest National Laboratory
Last modified: 03/05/2014
Reason for change: Implement possibility of running it with multiple instances of GridLAB-D
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
#include "libmpoption.h"
#include "/usr/local/matlab_2013a/extern/include/mclmcrrt.h"
#include "/usr/local/matlab_2013a/extern/include/mclcppclass.h"
#include "/usr/local/matlab_2013a/extern/include/matrix.h"

# define PI 3.14159265

#include "matpowerintegrator.h"

// Transposing matrices; wee need this function becuase the way the file reading is done leads to the transpose of the necessary matrix
// Careful: it is 1-base indexing because we are working with MATLAB type array mwArray
mwArray mwArrayTranspose(int nrows, int ncolumns, mwArray matrix_in) {
   mwArray matrix_out(nrows, ncolumns, mxDOUBLE_CLASS);
   for (int ind_row = 1; ind_row <= nrows; ind_row++) {
   for (int ind_col = 1; ind_col <= ncolumns; ind_col++) {
      matrix_out(ind_row, ind_col) = matrix_in(ind_col, ind_row); } }
   return matrix_out; }

char *nextline(char *buf) {
   char *tmp;
   tmp = strchr(buf, '\n');
   if (!tmp) {
      cout << "ERROR MOVING TO THE NEXT LINE!!!!!!!!!!" << endl; }
   return tmp + 1; }

/*
   - get_data function reads the file that has the transmission system model
   - nbrows, nbcolumns = number of rows and columns for the bus matrix (mpc.bus)
   - ngrows, ngcolumns = number of rows and columns for generator matrix (mpc.gen)
   - nbrrows, nbrcolumns = number of rows and columns for branch matrix (mpc.branch)
   - narows, nacolumns = number of rows and columns for area matrix (mpc.areas)
   - ncrows, nccolumns = number of rows and columns for the generator cost matrix (mpc.gencost)
   - baseMVA = usually 100, but given in the input file (mpc.baseMVA)
   - bus = the BUS matrix
   - gen = the GENERATOR matrix
   - branch = the BRANCH matrix
   - area = the AREA matrix
   - costs = the GENERATOR COSTS matrix
*/

void get_data(char *file_name, int nbrows, int nbcolumns, int ngrows, int ngcolumns,
              int nbrrows, int nbrcolumns, int narows, int nacolumns,
              int ncrows, int nccolumns, int nFNCSrows, int nFNCScolumns,
              int noffgrows, int noffgcolumns,
              double *baseMVA, double *bus, double *gen,
              double *branch, double *area, double *costs, int *SubBusFNCS,
              char SubNameFNCS[][15], char MarketNameFNCS[][11],
              int *offline_gen_bus)
{
// Open the file with the name given by the file name
ifstream data_file(file_name, ios::in);
bool read_bus = 0, read_line = 0, read_gen = 0, read_areas = 0, read_gencost = 0;
bool read_SubBusFNCS = 0, read_SubNameFNCS = 0, read_MarketNameFNCS = 0, read_offlineGenBus = 0;
int ind_row, ind_col, ind = 0;
string curr_line; // string holding the line that I scurrently read
if (data_file.is_open()) {
   cout << "======== Starting reading the data file. ======" << endl;
   while (data_file.good()) { // this will test the EOF mark
      data_file >> ws;
      getline(data_file, curr_line);
      if (curr_line[0] != '%') {
         // ================== READING BASE MVA =========================================
         if (strncmp(&curr_line[0], "mpc.baseMVA =", 13) == 0) {
            cout << "Reading BASE MVA ......................." << endl;
            sscanf(&curr_line[0], "%*s = %lf %*s", baseMVA);
         }
         // ================== READING NOMINAL FREQUENCY =========================================
//         if (strncmp(&curr_line[0], "sys_freq", 8) == 0) {
//            cout << "Reading NOMINAL FREQUENCY ......................." << endl;
//            sscanf(&curr_line[0], "%*s = %lf %*s", nomfreq); }
         // ================== READING BUS DATA =========================================
         if (strncmp(&curr_line[0], "mpc.bus =", 9) == 0) {
            cout << "Reading BUS DATA ...................." << endl;
            read_bus = 1;
         }
         if (read_bus == 1 & strncmp(&curr_line[0], "mpc.bus =", 9) != 0) {
            if (ind < nbrows*nbcolumns) {
               read_bus = 1;
               sscanf(&curr_line[0], "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %*s", &bus[ind], &bus[ind + 1], &bus[ind + 2], &bus[ind + 3],
                      &bus[ind + 4], &bus[ind + 5], &bus[ind + 6], &bus[ind + 7], &bus[ind + 8],
                      &bus[ind + 9], &bus[ind + 10], &bus[ind + 11], &bus[ind + 12]);
               ind += nbcolumns;
            }
            else {
               read_bus = 0;
               ind = 0;
            }
         }
         // ================== READING BRANCH DATA =========================================
         if (strncmp(&curr_line[0], "mpc.branch =", 12) == 0) {
            cout << "Reading BRANCH DATA ...................." << endl;
            read_line = 1;
         }
         if (read_line == 1 & strncmp(&curr_line[0], "mpc.branch =", 12) != 0) {
            if (ind < nbrrows*nbrcolumns) {
               read_line = 1;
               sscanf(&curr_line[0], "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %*s", &branch[ind], &branch[ind + 1], &branch[ind + 2], &branch[ind + 3],
                      &branch[ind + 4], &branch[ind + 5], &branch[ind + 6], &branch[ind + 7], &branch[ind + 8],
                      &branch[ind + 9], &branch[ind + 10], &branch[ind + 11], &branch[ind + 12]);
               ind += nbrcolumns;
            }
            else {
               read_line = 0;
               ind = 0;
            }
         }
         // ================== READING GENERATOR DATA =========================================              
         if (strncmp(&curr_line[0], "mpc.gen =", 9) == 0) {
            cout << "Reading GENERATOR DATA ...................." << endl;
            read_gen = 1;
         }
         if (read_gen == 1 & strncmp(&curr_line[0], "mpc.gen =", 9) != 0) {
            if (ind < ngrows*ngcolumns){
               read_gen = 1;
               sscanf(&curr_line[0], "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %*s", &gen[ind], &gen[ind + 1],
                      &gen[ind + 2], &gen[ind + 3], &gen[ind + 4], &gen[ind + 5], &gen[ind + 6], &gen[ind + 7], &gen[ind + 8],
                      &gen[ind + 9], &gen[ind + 10], &gen[ind + 11], &gen[ind + 12], &gen[ind + 13], &gen[ind + 14],
                      &gen[ind + 15], &gen[ind + 16], &gen[ind + 17], &gen[ind + 18], &gen[ind + 19], &gen[ind + 20]);
               ind += ngcolumns; 
            }
            else {
               read_gen = 0;
               ind = 0;
            }
         }
         // ================== READING AREAS DATA =========================================              
         if (strncmp(&curr_line[0], "mpc.areas =", 11) == 0) {
            cout << "Reading AREAS DATA ...................." << endl;
            read_areas = 1; }
         if (read_areas == 1 & strncmp(&curr_line[0], "mpc.areas =", 11) != 0){
            if (ind < narows*nacolumns){
               read_areas = 1;
               sscanf(&curr_line[0], "%lf %lf %*s", &area[ind], &area[ind + 1]);
               ind += nccolumns;
            }
            else {
               read_areas = 0;
               ind = 0;
            }
         }
         // ================== READING GENERATOR COST DATA =========================================              
         if (strncmp(&curr_line[0], "mpc.gencost =", 13) == 0) {
            cout << "Reading GENERATOR COST DATA ...................." << endl;
            read_gencost = 1; }
         if (read_gencost == 1 & strncmp(&curr_line[0], "mpc.gencost =", 13) != 0){
            if (ind < ncrows*nccolumns){
               read_gencost = 1;
               sscanf(&curr_line[0], "%lf %lf %lf %lf %lf %lf %lf %*s", &costs[ind], &costs[ind + 1], &costs[ind + 2], &costs[ind + 3],
                  &costs[ind + 4], &costs[ind + 5], &costs[ind + 6]);
               ind += nccolumns;
            }
            else {
               read_gencost = 0;
               ind = 0;
            }
         }
         // ================== READING BUS NUMBERS FOR FNCS COMMUNICATION =========================================              
         if (strncmp(&curr_line[0], "mpc.SubBusFNCS =", 16) == 0) {
            cout << "Reading FNCS SUBSTATION BUSES ...................." << endl;
            read_SubBusFNCS = 1; }
         if (read_SubBusFNCS == 1 & strncmp(&curr_line[0], "mpc.SubBusFNCS =", 16) != 0){
            if (ind < nFNCSrows*nFNCScolumns){
               read_SubBusFNCS = 1;
               sscanf(&curr_line[0], "%d %d %d %*s", &SubBusFNCS[ind], &SubBusFNCS[ind + 1], &SubBusFNCS[ind + 2]);
               ind += nFNCScolumns;
            }
            else {
               read_SubBusFNCS = 0;
               ind = 0;
            }
         }
         // ================== READING SUBSTATION NAMES FOR FNCS COMMUNICATION =========================================              
         if (strncmp(&curr_line[0], "mpc.SubNameFNCS =", 17) == 0) {
            cout << "Reading FNCS SUBSTATION NAMES ...................." << endl;
            read_SubNameFNCS = 1; }
         if (read_SubNameFNCS == 1 & strncmp(&curr_line[0], "mpc.SubNameFNCS =", 17) != 0){
            if (ind < nFNCSrows*nFNCScolumns){
               read_SubNameFNCS = 1;
               sscanf(&curr_line[0], "%s %s %s %*s", SubNameFNCS[ind], SubNameFNCS[ind + 1], SubNameFNCS[ind + 2]);
               ind += nFNCScolumns;
            }
            else {
               read_SubNameFNCS = 0;
               ind = 0;
            }
         }
         // ================== READING MARKET NAMES FOR FNCS COMMUNICATION =========================================              
         if (strncmp(&curr_line[0], "mpc.MarketNameFNCS =", 20) == 0) {
            cout << "Reading FNCS MARKET NAMES ...................." << endl;
            read_MarketNameFNCS = 1; }
         if (read_MarketNameFNCS == 1 & strncmp(&curr_line[0], "mpc.MarketNameFNCS =", 20) != 0){
            if (ind < nFNCSrows*nFNCScolumns){
               read_MarketNameFNCS = 1;
               sscanf(&curr_line[0], "%s %s %s %*s", MarketNameFNCS[ind], MarketNameFNCS[ind + 1], MarketNameFNCS[ind + 2]);
               ind += nFNCScolumns;
            }
            else {
               read_MarketNameFNCS = 0;
               ind = 0;
            }
         }
         // ================== READING BUS NUMBERS FOR POSSIBLE OFF-LINE GENERATORS FOR FNCS COMMUNICATION =========================================              
         if (strncmp(&curr_line[0], "mpc.offlineGenBus =", 19) == 0) {
            cout << "Reading OFFLINE GENERATOR BUSES ...................." << endl;
            read_offlineGenBus = 1; }
         if (read_offlineGenBus == 1 & strncmp(&curr_line[0], "mpc.offlineGenBus =", 19) != 0){
            if (ind < noffgrows*noffgcolumns){
               read_offlineGenBus = 1;
               sscanf(&curr_line[0], "%d %*s", &offline_gen_bus[ind]); // go with only 1 generator switching off-line
               ind += noffgcolumns;
            }
            else {
               read_offlineGenBus = 0;
               ind = 0;
            }
         }
      } // END OF if (curr_line[0] != '%')
   } // END OF while (data_file.good())
   cout << "Reached the end of the file!!!!!!!!" << endl;
   cout << "======== Done reading the data file!!!!!!!!! ====================" << endl;
   data_file.close(); } // END OF if (data_file.is_open())
else {
   std:cout << "Unable to open file" << endl;
   data_file.close(); }
} // END OF get_data function

int run_main(int argc, char **argv) {
   if (!mclInitializeApplication(NULL, 0)) {
      cerr << "Could not initialize the application properly !!!" << endl;
      return -1;
      }
   if (!libopfInitialize() & !libmpoptionInitialize()) {
      cerr << "Could not initialize one or more libraries properly !!!" << endl;
      return -1;
      }
   else {
      try {
// ================================ VARIABLE DECLARATION AND INITIALIZATION =============================================
         char file_name[] = {"case9.m"};
         double baseMVA, nomfreq;
         // The powerflow solution is going to be calculated in the following variables
         mwArray mwBusOut, mwGenOut, mwBranchOut, f, success, info, et, g, jac, xr, pimul;
//         double mwBusOut_copy[9];
         int repeat = 1;
         // matrix dimensions based on test case; they need to be revised if other case is used
         // for C code we need the total number of elements, while the matrices will be passed to MATLAB as mwArray with rows and columns
         // BUS DATA MATRIX DEFINITION
         // bus matrix dimensions, and total number of elements
         int nbrows = 9, nbcolumns = 13, nbelem = nbrows * nbcolumns;
         double bus[nbelem];
         mwArray mwBusT(nbcolumns, nbrows, mxDOUBLE_CLASS); // given the way we read the file, we initially get the transpose of the matrix
         mwArray mwBus(nbrows, nbcolumns, mxDOUBLE_CLASS);
         // GENERATOR DATA MATRIX DEFINITION
         // generator matrix dimensions, and total number of elements
         int ngrows = 3, ngcolumns = 21, ngelem = ngrows * ngcolumns;
         double gen[ngelem];
         mwArray mwGenT(ngcolumns, ngrows, mxDOUBLE_CLASS);
         mwArray mwGen(ngrows, ngcolumns, mxDOUBLE_CLASS);
         // BRANCH DATA MATRIX DEFINITION
         // branch matrix dimensions, and total number of elements
         int nbrrows = 9, nbrcolumns = 13, nbrelem = nbrrows * nbrcolumns;
         double branch[nbrelem];
         mwArray mwBranchT(nbrcolumns, nbrrows, mxDOUBLE_CLASS);
         mwArray mwBranch(nbrrows, nbrcolumns, mxDOUBLE_CLASS);
         // AREA DATA MATRIX DEFINITION
         // area matrix dimensions, and total number of elements
         int narows = 1, nacolumns = 2, naelem = narows * nacolumns;
         double area[naelem];
         mwArray mwAreaT(nacolumns, narows, mxDOUBLE_CLASS);
         mwArray mwArea(narows, nacolumns, mxDOUBLE_CLASS);
         // GENERATOR COST DATA MATRIX DEFINTION
         // generator cost matrix dimensions, and total number of elements
         int ncrows = 3, nccolumns = 7, ncelem = ncrows * nccolumns;
         double costs[ncelem];
         mwArray mwCostsT(nccolumns, ncrows, mxDOUBLE_CLASS);
         mwArray mwCosts(ncrows, nccolumns, mxDOUBLE_CLASS);
         // SUBSTATION BUS NUMBERS AND NAMES FOR FNCS COMMUNICATION
         int nFNCSrows = 1, nFNCScolumns = 3, nFNCSelem = nFNCSrows*nFNCScolumns;
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
         // Generator bus matrix consisting of bus numbers corresponding to the generators that could become out-of service,
         // allowing us to set which generators get off-line, in order to simulate a reduction in generation capacity.
         // MATPOWER should reallocate different generation needs coming from the on-line generators to cover for the lost ones, since load stays constant
         // for MATPOWER: value >  0 means generator is in-service
         //                     <= 0 means generator is out-of-service
         // number of rows and columns in the MATPOWER structure, and the total number of buses
         int noffgrows = 1, noffgcolumns = 1, noffgelem = noffgrows*noffgcolumns;
         int offline_gen_bus[noffgelem], offline_gen_ind[noffgelem];
         // times recorded for visualization purposes
         int curr_time; // current time in seconds
         int curr_hours, curr_minutes, curr_seconds; // current time in hours, minutes, seconds
         int delta_t[nFNCSelem], prev_time[nFNCSelem]; // for each substation, we save the time between 2 consecutive received messages
         // output files for saving results
         char subst_output_file_name[nFNCSelem][17]; // one file for each substation
         char gen_output_file_name[ngrows][16]; // one file for each generator; there are ngrows generators in this case; needs to be changed for general purpose, maybe using ngrows
         ofstream subst_output_file, gen_output_file;
         for (int i = 0; i < sizeof(subst_output_file_name)/sizeof(subst_output_file_name[0]); i++) {
            snprintf(subst_output_file_name[i], sizeof(subst_output_file_name[i]), "Substation_%d.csv", i+1);
            ofstream subst_output_file(subst_output_file_name[i], ios::out);
            subst_output_file << "Time (seconds), Real Power Demand - PD (MW), Reactive Power Demand (MVAr), Substation V real (V), Substation V imag (V), LMP ($/MWh), LMP ($/MVAr)" << endl;
         }
         for (int i = 0; i < sizeof(gen_output_file_name)/sizeof(gen_output_file_name[0]); i++) {
            snprintf(gen_output_file_name[i], sizeof(gen_output_file_name[i]), "Generator_%d.csv", i+1);
            ofstream gen_output_file(gen_output_file_name[i], ios::out);
            gen_output_file << "Time (seconds), STATUS, PMAX (MW), PMIN (MW), Real power output - PG (MW), QMAX (MVAr), QMIN (MVAr), Reactive power output - QG (MVAr)" << endl;
         }

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
         mpoption(2, mpopt, mpoptNames); // initialize powerflow options to DEFAULT ones
         // cout << "mpopt = " << mpopt << endl;
         // cout << "mpoptNames = " << mpoptNames << endl;
         mwArray optIn(1, 3, mxCELL_CLASS);
         optIn.Get(1, 1).Set(mpopt);
         optIn.Get(1, 2).Set(mwArray("PF_DC")); // name of the option that could be modified, e.g. PF_DC
         optIn.Get(1, 3).Set(mwArray(1)); // value of the modified option, e.g. 0 or 1 for false or true
         optIn.Get(1, 2).Set(mwArray("VERBOSE"));
         optIn.Get(1, 3).Set(mwArray(0));
         mpoption(2, mpopt, mpoptNames, optIn); //, optionName, optionValue);
         opfIn.Get(1, 2).Set(mpopt); // Setting up the second input argument of the opf function representing the power flow options
         // cout << "mpopt = " << mpopt << endl;
// ================================ END OF VARIABLE DECLARATION AND INITIALIZATION =============================================

         cout << "Just entered the MAIN function of the driver application." << endl;
         get_data(file_name, nbrows, nbcolumns, ngrows, ngcolumns, nbrrows, nbrcolumns, narows, nacolumns,
                  ncrows, nccolumns, nFNCSrows, nFNCScolumns, noffgrows, noffgcolumns, &baseMVA, bus, gen,
                  branch, area, costs, bus_num, sub_name, market_name, offline_gen_bus);
         mwBusT.SetData(bus, nbelem);
         // Transposing mwBusT to get the correct bus matrix
         // Careful: it is 1-base indexing because we are working with MATLAB type array mwArray
         mwBus = mwArrayTranspose(nbrows, nbcolumns, mwBusT);

         mwArray mwBusDim = mwBus.GetDimensions();
         cout << "============= TESTING THROUGH VISUALIZATION... HAHAHAHAHAHAHA =============" << endl;
         cout << "baseMVA = " << baseMVA << endl;
         cout << "=================================" << endl;
         // cout << "mwBusT = " << mwBusT << endl;
         // cout << "==================================" << endl;
         cout << "mwBus = " << mwBus << endl;
         cout << "==================================" << endl;

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

         cout << "==================================" << endl;
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
         // The bus number and the actual index in the MATPOWER matrix may not coincide
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
            // Start every time assuming no message it is received
            mesg_rcv = false;
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
            // Setting up the status of the generators, based on the current time
            // Turning some generators out-of-service between certain time preiods in the simulation 
            if ((curr_hours >= 1 && curr_hours < 2) || (curr_hours >= 3 && curr_hours < 4)){
               for (int off_ind = 0; off_ind < sizeof(offline_gen_ind)/sizeof(offline_gen_ind[0]); off_ind++){
                  mpc.Get("gen", 1, 1).Get(2, offline_gen_ind[off_ind], 8).Set((mwArray) 0);
               }
            }
            else {
               for (int off_ind = 0; off_ind < sizeof(offline_gen_ind)/sizeof(offline_gen_ind[0]); off_ind++){
                  mpc.Get("gen", 1, 1).Get(2, offline_gen_ind[off_ind], 8).Set((mwArray) 1);
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
               mesg_rcv =  mesg_rcv || mesgc[sub_ind];
               if (mesgc[sub_ind] == 1) {
                  delta_t[sub_ind] = curr_time - prev_time[sub_ind]; // number of seconds between 2 consecutive received messages
                  // If it is simulated that there's an incoming message, than we need to give a value.
                  // These are constant values used to test the code when running without FNCS and GLD.
                  // Need to be commented when the code is used in FNCS, and uncommented when running off-line from FNCS
                  // cout << "WHAT IS REAL LOAD CHANGE FROM " << sub_name[sub_ind] << " ?" << endl;
                  // cout << "Active power in MW:\t";
                  // cin >> bus_valueReal[sub_ind];
                  // cout << "WHAT IS IMAGINARY LOAD CHANGE FROM " << sub_name[sub_ind] << " ?" << endl;
                  // cout << "Reactive power in MVAr:\t";
                  // cin >> bus_valueIm[sub_ind];
                  // It is assumed that the load at the bus consists of the initial constant load plus a controllable load coming from distribution (GridLAB-D)
                  mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 3).Set((mwArray) (static_pd[sub_ind] + bus_valueReal[sub_ind]));
                  mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 4).Set((mwArray) (static_qd[sub_ind] + bus_valueIm[sub_ind]));
               } // end IF(mesgc)
            } // end FOR(sub_ind)

            if (mesg_rcv){
               // ==========================================================================================================
               cout << "\033[2J\033[1;1H"; // Just a trick to clear the screen before pritning the new results at the terminal
               for (int sub_ind = 0; sub_ind < sizeof(bus_num)/sizeof(bus_num[0]); sub_ind++) {
                  if (mesgc[sub_ind] == 1) {
                     cout << "It has been " << curr_hours << " hours, " << curr_minutes << " minutes, and ";
                     cout << curr_seconds << " seconds." << endl;
                     cout << "================== NEW LOAD AT " << sub_name[sub_ind] << " AT BUS " << bus_num[sub_ind] << " =====================" << endl;
                     cout << "ACTIVE power required at the bus: " << mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 3) << " MW." << endl;
                     cout << "REACTIVE power required at the bus: " << mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 4) << " MW." << endl;
                     cout << "I've got the POWER after " << delta_t[sub_ind] << " seconds." << endl;
                     prev_time[sub_ind] = curr_time;
                  }
                  else {
                     cout << "================== NO LOAD CHANGE AT " << sub_name[sub_ind] << " AT BUS " << bus_num[sub_ind] << " =====================" << endl;
                  } // end IF(mesgc)
               } // end FOR(sub_ind)
            } // end IF(mesg_rcv)
            
            // Call OPF with nargout = 0 (first argument), and all results are going to be printed at the console
            // Call OPF with nargout = 7, and get all the output parameters up to et
            // Call OPF with nargout = 11, and get a freaking ERROR.... AVOID IT!
            opfIn.Get(1, 1).Set(mpc); // Setting up the first input parameter for opf function as the actual MPC model
            opf(7, mwBusOut, mwGenOut, mwBranchOut, f, success, info, et, g, jac, xr, pimul, opfIn);
            if (mesg_rcv) {
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
                  realLMP[sub_ind] = (double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 14); // local marginal price based on the Lagrange multiplier on real power mismatch (column 14 of the output bus matrix
                  sendprice(&realLMP[sub_ind], market_name[sub_ind]);
                  imagLMP[sub_ind] = (double) mwBusOut.Get(2, modified_bus_ind[sub_ind], 15); // local marginal price based on the Lagrange multiplier on reactive power mismatch (column 14 of the output bus matrix
                  cout << "================== SENDING OUT THE LMP TO " << sub_name[sub_ind];
                  cout << " AT BUS " << mpc.Get("bus", 1, 1).Get(2, modified_bus_ind[sub_ind], 1) << " =====================" << endl;
                  cout << "LMP (Lagrange multiplier on real power mismatch) -->> " << realLMP[sub_ind] << endl;
                  cout << "LMP (Lagrange multiplier on reactive power mismatch) -->> " << imagLMP[sub_ind] << endl;
                  cout << "=================================================================" << endl;
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
         }while(synchronize(!mesg_rcv));
          // use while(!mesg_rcv); when running off-line from FNCS
          // use while(synchronize(!mesg_rcv)); when involving FNCS

         cout << "Just executed the MATLAB function from the shared library." << endl;
         subst_output_file.close();
         gen_output_file.close();

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
         return -2;
         }
      catch (...) {
         cerr << "Unexpected error thrown" << endl;
         return -3;
         }
   libopfTerminate();
   libmpoptionTerminate();
   }
   mclTerminateApplication();
   return 0;
}

int main(int argc, char* argv[]) {	
   mclmcrInitialize();
   return mclRunMain((mclMainFcnType) run_main, 0, NULL);
}
