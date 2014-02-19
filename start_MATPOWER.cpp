/*
Launching the MATPOWER Optimal Power Flow solver
  - MATPOWER OPF resides as a shared object in libopf.so, after being compiled on a computer with MATLAB isntalled.
  - Running the MATPOWER OPF requires that at least Matlab Compiler Runtime (downloaded for free from MATHWORKS webpage) is installed.
  - The code below reads the data file that resides in a TXT file (the MATPOWER case files stripped of the unnecessary comments and text).
  - It creates the data structure needed by the OPF solver, calls the solver, and returns whatever it is desired.
  - Files needed for deployment (for this case, at least, in order to be able to compile): start_MATPOWER.cpp, libopf.h, libopf.so, libmpoption.so, libmpoption.h, case9.m, matpowerintegrator.h, matpowerintegrator.c.
==========================================================================================
Copyright (C) 2013, Battelle Memorial Institute
Written by Laurentiu Dan Marinovici, Pacific Northwest National Laboratory
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
#include "mclmcrrt.h"
#include "mclcppclass.h"
#include "matrix.h"

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
              int ncrows, int nccolumns, double *baseMVA, double *bus, double *gen,
              double *branch, double *area, double *costs)
{
// Open the file with the name given by the file name
ifstream data_file(file_name, ios::in);
bool read_bus = 0, read_line = 0, read_gen = 0, read_areas = 0, read_gencost = 0;
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
         char file_name[] = {"case9.m"};
         char output_file_name[] = {"MATPOWER_output.csv"};
         char gen_output_file_name[3][16]; // one file for each generator; there are 3 generators in this case; needs to be changed for general purpose, maybe using ngrows
         ofstream gen_output_file;
         ofstream output_file(output_file_name, ios::out);
         output_file << "Time (seconds), Real Power Demand - PD (MW), Reactive Power Demand (MVAr), Substation V real (V), Substation V imag (V), LMP ($/MWh), LMP ($/MVAr)" << endl;

         for (int i = 0; i < sizeof(gen_output_file_name)/sizeof(gen_output_file_name[0]); i++) {
            snprintf(gen_output_file_name[i], sizeof(gen_output_file_name[i]), "Generator_%d.csv", i+1);
            ofstream gen_output_file(gen_output_file_name[i], ios::out);
            gen_output_file << "Time (seconds), PMAX (MW), PMIN (MW), Real power output - PG (MW), QMAX (MVAr), QMIN (MVAr), Reactive power output - QG (MVAr)" << endl;
            }
         double baseMVA, nomfreq;
         // The powerflow solution is going to be calculated in the following variables
         mwArray mwBusOut, mwGenOut, mwBranchOut, f, success, info, et, g, jac, xr, pimul;
//         double mwBusOut_copy[9];
         double bus_num = 7, bus_valueReal, bus_valueIm; // the bus number where the substation is connected to, and the corresponding real and imaginary power
         int repeat = 1, modified_bus_ind; // modified_bus_ind
         bool gotValue = false; // synchronization only happens when a value is received
         int curr_time; // current time in seconds
         int curr_hours, curr_minutes, curr_seconds, delta_t, prev_time = 0;
         double static_pd, static_qd;
         init((int)bus_num); // I presume this is to initialize the bus number for FNCS
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

         // Creating the MPC structure that is going to be used as input for OPF function
         const char *fields[] = {"baseMVA", "bus", "gen", "branch", "areas", "gencost"};
         mwArray mpc(1, 1, 6, fields);
         // Variable that will hold both the mpc structure and the options for the opf function
         mwArray opfIn(1, 2, mxCELL_CLASS);

         // Creating the variable that would set the options for the OPF solver
         mwArray mpopt(124, 1, mxDOUBLE_CLASS); // there are 124 options that can be set
         mwArray mpoptNames(124, 18, mxCHAR_CLASS); // there are 124 option names and the maximum length is 18, but made it to 20
         cout << "=================================================" << endl;
         cout << "==== TRYING TO SET UP THE OPTIONS !!!!!===========" << endl;
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

         cout << "Just entered the MAIN function of the driver application." << endl;
         get_data(file_name, nbrows, nbcolumns, ngrows, ngcolumns, nbrrows, nbrcolumns, narows, nacolumns,
                  ncrows, nccolumns, &baseMVA, bus, gen, branch, area, costs);
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
         cout << "SUBSTATION BUS IS BUS NUMBER " << bus_num << endl;
         //cin >> bus_value;
         for (int ind = 1; ind <= nbrows; ind++) {
            // In mpc.Get("bus", 1, 1).Get(2, ind, 1), the 2 in the second Get represents the number of indeces the array has
            if ((int) mpc.Get("bus", 1, 1).Get(2, ind, 1) == bus_num) {
               modified_bus_ind = ind;
               // the index of the bus in the bus matrix could be different form the number of the bus
               // because busses do not have to be numbered consecutively
               cout << "FOUND THE SUBSTATION BUS AT LOCATION "  << modified_bus_ind << " IN THE BUS MATRIX." << endl;
            }
         }
         static_pd = mpc.Get("bus", 1, 1).Get(2, modified_bus_ind, 3);
         static_qd = mpc.Get("bus", 1, 1).Get(2, modified_bus_ind, 4);
         cout << "Initially, the static ACTIVE power at bus " << bus_num << " is " << static_pd << "." << endl;
         cout << "Initially, the static REACTIVE power at bus " << bus_num << " is " << static_qd << "." << endl;

         do {
            startcalculation();
            //cout << "===== OPF number " << repeat << " started. =====" << endl;
            //cout << "Bus number where load is changed ---->>>>> ";
            gotValue=getpower((int)bus_num,&bus_valueReal,&bus_valueIm);
            if(gotValue) {
               curr_time = getCurrentTime();
               curr_hours = curr_time/3600;
               curr_minutes = (curr_time - 3600*curr_hours)/60;
               curr_seconds = curr_time - 3600*curr_hours - 60*curr_minutes;
               delta_t = curr_time - prev_time; // number of seconds between 2 consecutive synchronizations
               prev_time = curr_time;
               cout << "\033[2J\033[1;1H"; // Just a trick to clear the screen before pritning the new results at the terminal
               cout << "It has been " << curr_hours << " hours, " << curr_minutes << " minutes, and " << curr_seconds << " seconds." << endl;
               // bus_valueReal = 10;
               // bus_valueIm = 20;
               // It is assumed that the load at the bus consists of the initial constant load plus a controllable load coming from distribution (GridLAB-D)
               mpc.Get("bus", 1, 1).Get(2, modified_bus_ind, 3).Set((mwArray) (static_pd + bus_valueReal));
               mpc.Get("bus", 1, 1).Get(2, modified_bus_ind, 4).Set((mwArray) (static_qd + bus_valueIm));
               cout << "================== NEW LOAD AT THE SUBSTATION BUS =====================" << endl;
               cout << "ACTIVE power required at the bus: " << mpc.Get("bus", 1, 1).Get(2, modified_bus_ind, 3) << " MW." << endl;
               cout << "REACTIVE power required at the bus: " << mpc.Get("bus", 1, 1).Get(2, modified_bus_ind, 4) << " MW." << endl;
               cout << "I've got the POWER after " << delta_t << " seconds." << endl;
               // cout << "The modified bus \n" << mpc.Get("bus", 1, 1) << endl;
               // Call OPF with nargout = 0 (first argument), and all results are going to be printed at the console
               // Call OPF with nargout = 7, and get all the output parameters up to et
               // Call OPF with nargout = 11, and get a freaking ERROR.... AVOID IT!
               opfIn.Get(1, 1).Set(mpc); // Setting up the first input parameter for opf function as the actual MPC model
               opf(7, mwBusOut, mwGenOut, mwBranchOut, f, success, info, et, g, jac, xr, pimul, opfIn);
               double sendValReal =  (double) mwBusOut.Get(2, modified_bus_ind, 8)*cos((double) mwBusOut.Get(2, modified_bus_ind, 9) * PI / 180)*(double) mwBusOut.Get(2, modified_bus_ind, 10)*1000; // real voltage at the bus based on the magnitude (column 8 of the output bus matrix) and angle in degrees (column 9 of the output bus matrix)
               double sendValIm = (double) mwBusOut.Get(2, modified_bus_ind, 8)*sin((double) mwBusOut.Get(2, modified_bus_ind, 9) * PI / 180)*(double) mwBusOut.Get(2, modified_bus_ind, 10)*1000; // imaginary voltage at the bus based on the magnitude (column 8 of the output bus matrix) and angle in degrees (column 9 of the output bus matrix)
               cout << "================== SENDING OUT THIS VOLTAGE =====================" << endl;
               cout << "Voltage REAL part -->> " << sendValReal << " V." << endl;
               cout << "Voltage IMAGINARY part -->> " << sendValIm << " V." << endl;
               cout << "=================================================================" << endl;
               // output_file << (double) mwBusOut.Get(2, modified_bus_ind, 8) << ", " << (double) mwBusOut.Get(2, modified_bus_ind, 9) << ", ";
               double realLMP = (double) mwBusOut.Get(2, modified_bus_ind, 14); // local marginal price based on the Lagrange multiplier on real power mismatch (column 14 of the output bus matrix
               double imagLMP = (double) mwBusOut.Get(2, modified_bus_ind, 15); // local marginal price based on the Lagrange multiplier on reactive power mismatch (column 14 of the output bus matrix
               cout << "================================== LMPs =====================================" << endl;
               cout << "LMP (Lagrange multiplier on real power mismatch) -->> " << realLMP << endl;
               cout << "LMP (Lagrange multiplier on reactive power mismatch) -->> " << imagLMP << endl;
               cout << "=================================================================" << endl;
               output_file << curr_time << "," << (double) mwBusOut.Get(2, modified_bus_ind, 3) << "," << (double) mwBusOut.Get(2, modified_bus_ind, 4) << ", " << sendValReal << ", " << sendValIm <<  ", " << realLMP << ", " << imagLMP << endl;
               for (int gen_ind = 0; gen_ind < ngrows; gen_ind++){ // in C indexes start from 0, but from MATLAB variables index needs to start from 1
                  ofstream gen_output_file(gen_output_file_name[gen_ind], ios::app);
                  gen_output_file << curr_time << "," << (double) mwGenOut.Get(2, gen_ind + 1, 9) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 10) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 2) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 4) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 5) << "," << (double) mwGenOut.Get(2, gen_ind + 1, 3) << endl;
               }
               // cout << "===== OPF number " << repeat << " ended. =====" << endl;
               sendvolt((int)bus_num,sendValReal,sendValIm);
            }
         }while(synchronize(!gotValue));

         cout << "Just executed the MATLAB function from the shared library." << endl;
         output_file.close();
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
