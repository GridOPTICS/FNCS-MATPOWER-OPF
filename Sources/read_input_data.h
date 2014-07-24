

void read_load_profile(char *file_name, double load_profile[][288], int subst_num);

void read_model_dim(char *file_name, int *nbrows, int *nbcolumns, int *ngrows, int *ngcolumns,
              int *nbrrows, int *nbrcolumns, int *narows, int *nacolumns,
              int *ncrows, int *nccolumns, int *nFNCSSub, int *noffGen);

void read_model_data(char *file_name, int nbrows, int nbcolumns, int ngrows, int ngcolumns,
              int nbrrows, int nbrcolumns, int narows, int nacolumns,
              int ncrows, int nccolumns, int nFNCSelem, int noffgelem,
              double *baseMVA, double *bus, double *gen,
              double *branch, double *area, double *costs, int *SubBusFNCS,
              char SubNameFNCS[][15], char MarketNameFNCS[][11],
              int *offline_gen_bus, double *ampFactor, int *NS3_flag);
