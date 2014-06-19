#include<stdio.h>
#include<math.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
using namespace std;

void read_load_profile(char *file_name, double load_profile[][3])
{
   ifstream data_file(file_name, ios::in);
   int ind = 0;
   string curr_line;
   if (data_file.is_open()){
      cout << "======== Starting reading the load profile data. ========" << endl;
      while (data_file.good()){
         getline(data_file, curr_line);
         sscanf(&curr_line[0], "%lf %lf %lf", &load_profile[ind][0], &load_profile[ind][1], &load_profile[ind][2]);
         ind = ind + 1;
      }
      cout << "Reached the end of the file!!!!!!!!" << endl;
      cout << "======== Done reading the load profile file!!!!!!!!! ====================" << endl;
      data_file.close(); } // END OF if (data_file.is_open())
   else {
      cout << "Unable to open load profile file." << endl;
      data_file.close(); }
} // END OF get_load_profile function
