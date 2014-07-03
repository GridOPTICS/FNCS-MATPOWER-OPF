#include<stdio.h>
#include<string.h>
#include<math.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
using namespace std;

/*
   - get_dim function reads the file that has the transmission system model, just to get the dimensions of the models, such that we can later create the matrices without need of dynamic allocation
   - nbrows, nbcolumns = number of rows and columns for the bus matrix (mpc.bus)
   - ngrows, ngcolumns = number of rows and columns for generator matrix (mpc.gen)
   - nbrrows, nbrcolumns = number of rows and columns for branch matrix (mpc.branch)
   - narows, nacolumns = number of rows and columns for area matrix (mpc.areas)
   - ncrows, nccolumns = number of rows and columns for the generator cost matrix (mpc.gencost)
*/

void read_model_dim(char *file_name, int *nbrows, int *nbcolumns, int *ngrows, int *ngcolumns,
              int *nbrrows, int *nbrcolumns, int *narows, int *nacolumns,
              int *ncrows, int *nccolumns, int *nFNCSSub, int *noffGen)
{
// Open the file with the name given by the file name
ifstream data_file(file_name, ios::in);
bool read_bus = 0, read_line = 0, read_gen = 0, read_areas = 0, read_gencost = 0;
bool read_SubBusFNCS = 0, read_SubNameFNCS = 0, read_MarketNameFNCS = 0, read_offlineGenBus = 0;
int ind_row, ind_col, ind = 0;
string curr_line; // string holding the line that I scurrently read
if (data_file.is_open()) {
   cout << "======== Starting reading the data file, to get the transmission model size. ======" << endl;
   while (data_file.good()) { // this will test the EOF mark
      // data_file >> ws;
      getline(data_file, curr_line);
      if (curr_line[0] != '%') {
         // ================== READING BUS DATA SIZE =========================================
         if (strncmp(&curr_line[0], "mpc.busData =", 13) == 0) {
            cout << "Reading BUS DATA SIZE ...................." << endl;
            sscanf(&curr_line[0], "%*s = [ %d %d %*s", nbrows, nbcolumns);
         }
         // ================== READING BRANCH DATA SIZE =========================================
         if (strncmp(&curr_line[0], "mpc.branchData =", 16) == 0) {
            cout << "Reading BRANCH DATA SIZE ...................." << endl;
            sscanf(&curr_line[0], "%*s = [ %d %d %*s", nbrrows, nbrcolumns);
         }
         // ================== READING GENERATOR DATA SIZE =========================================              
         if (strncmp(&curr_line[0], "mpc.genData =", 13) == 0) {
            cout << "Reading GENERATOR DATA ...................." << endl;
            sscanf(&curr_line[0], "%*s = [ %d %d %*s", ngrows, ngcolumns);
         }
         // ================== READING AREAS DATA SIZE =========================================              
         if (strncmp(&curr_line[0], "mpc.areaData =", 14) == 0) {
            cout << "Reading AREAS DATA SIZE ...................." << endl;
            sscanf(&curr_line[0], "%*s = [ %d %d %*s", narows, nacolumns);
         }
         // ================== READING GENERATOR COST DATA =========================================              
         if (strncmp(&curr_line[0], "mpc.costData =", 14) == 0) {
            cout << "Reading GENERATOR COST DATA SIZE ...................." << endl;
            sscanf(&curr_line[0], "%*s = [ %d %d %*s", ncrows, nccolumns);
         }
         // ================== READING NUMBER OF FEEDERS FOR FNCS COMMUNICATION =========================================              
         if (strncmp(&curr_line[0], "mpc.SubBusFNCSNum =", 19) == 0) {
            cout << "Reading FNCS SUBSTATION NUMBER ...................." << endl;
            sscanf(&curr_line[0], "%*s = %d %*s", nFNCSSub);
         }
         // ================== READING NUMBER OF POSSIBLE OFF-LINE GENERATORS FOR FNCS COMMUNICATION =========================================              
         if (strncmp(&curr_line[0], "mpc.offlineGenNum =", 19) == 0) {
            cout << "Reading OFFLINE GENERATOR NUMBER  ...................." << endl;
            sscanf(&curr_line[0], "%*s = %d %*s", noffGen);
         }
      } // END OF if (curr_line[0] != '%')
   } // END OF while (data_file.good())
   cout << "Reached the end of the file!!!!!!!!" << endl;
   cout << "======== Done reading the data file!!!!!!!!! ====================" << endl;
   data_file.close(); } // END OF if (data_file.is_open())
else {
   std:cout << "Unable to open file" << endl;
   data_file.close(); }
} // END OF get_dim function
