#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <stdbool.h>
#include "printh5.c"
#include "hrv.c"
#include <math.h>
//#include <grib_api.h>
#include <dirent.h>

#define L15_PATH "U-MARF/MSG/Level1.5/"

int *mask1A,*mask1B,*mask1C,*mask2A,*mask3C,*mask4A,*mask4B,*mask4C,*mask4D,*mask4F,*mask5B,*mask5F;

#define NUM_CHANNEL 9
#define NUM_DAYS 16
#define DATA_PER_DAY 40
#define TOT_DATA 40*9

char *ChannelPath[] = {
    L15_PATH "DATA/Channel 01/IMAGE_DATA",
    L15_PATH "DATA/Channel 02/IMAGE_DATA",
    L15_PATH "DATA/Channel 03/IMAGE_DATA",
    L15_PATH "DATA/Channel 04/IMAGE_DATA",
    L15_PATH "DATA/Channel 05/IMAGE_DATA",
    L15_PATH "DATA/Channel 06/IMAGE_DATA",
    L15_PATH "DATA/Channel 07/IMAGE_DATA",
    L15_PATH "DATA/Channel 09/IMAGE_DATA",
    L15_PATH "DATA/Channel 11/IMAGE_DATA",
};

char * ChannelPathHd = L15_PATH "DATA/Channel 12/IMAGE_DATA";

int num[NUM_CHANNEL]={1,2,3,4,5,6,7,9,11};

#define META_PATH L15_PATH "METADATA/"
#define HEADER_PATH META_PATH "HEADER/"

char * AttributePath   = HEADER_PATH "RadiometricProcessing/Level15ImageCalibration_ARRAY";
char * DescriptionPath = HEADER_PATH "ImageDescription/ImageDescription_DESCR";
char * SubsetPath = META_PATH "SUBSET";
int north[NUM_DAYS];
int east[NUM_DAYS];
int south[NUM_DAYS];
int west[NUM_DAYS];

struct calibration
{
    double slope;
    double offset;
};

struct description
{
    char name[80];
    char value[80];
};

char fname_input[NUM_DAYS*DATA_PER_DAY][50];
char fname_mask[NUM_DAYS*DATA_PER_DAY][50];
//char* fname[NUM_DAYS*DATA_PER_DAY]={"img-mezzogiorno.h5"};
hid_t  fid;

#define DIM 3712
#define DIM_HD_X 11136
//#define DIM_HD_Y 5568
#define DIM_HD_Y 11136

unsigned short* data[NUM_DAYS*DATA_PER_DAY][NUM_CHANNEL];
unsigned short* data_hd[NUM_DAYS*DATA_PER_DAY];
double* data_hd_true[NUM_DAYS*DATA_PER_DAY];
//unsigned short (*data)[DIM] [NUM_CHANNEL];
struct calibration *cal;
struct description *descr;
double* data_true[NUM_CHANNEL];
bool* mask[NUM_CHANNEL];
bool* final_mask;
unsigned short* zoom_mask[TOT_DATA];

void read_data(int num)//num=numero del file di input da considerare
{
    hid_t dataset, dataspace,dataset_img, dataspace_img;
    hssize_t num_entries;
    herr_t status;

    /*hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(struct calibration));
    H5Tinsert(h5cal_t, "Cal_Slope", HOFFSET(struct calibration, slope),
            H5T_NATIVE_DOUBLE);
    H5Tinsert(h5cal_t, "Cal_Offset",
            HOFFSET(struct calibration, offset),
            H5T_NATIVE_DOUBLE);
 
    fid = H5Fopen(fname_input[num] , H5F_ACC_RDONLY , H5P_DEFAULT );
    dataset = H5Dopen2(fid, AttributePath, H5P_DEFAULT);    
    dataspace = H5Dget_space(dataset);
    num_entries = H5Sget_simple_extent_npoints(dataspace);
    cal=(struct calibration*)malloc(sizeof(struct calibration)*num_entries);
    status = H5Dread(dataset, h5cal_t, H5S_ALL, dataspace, H5P_DEFAULT, cal[num]);
    H5Dclose(dataset);*/

    for (int i=0;i<NUM_CHANNEL;++i)
    {
        dataset_img = H5Dopen2(fid, ChannelPath[i], H5P_DEFAULT);    
        dataspace_img = H5Dget_space(dataset_img);

        data[num][i]=(unsigned short*)malloc(sizeof(unsigned short)*DIM*DIM);
        printf("ok\n");

        status = H5Dread(dataset_img, H5T_NATIVE_USHORT, H5S_ALL, dataspace_img, H5P_DEFAULT, data[num][i]);
        H5Dclose(dataset_img);
    }
}

void read_calibration(int num)
{
    hid_t dataset, dataspace,dataset_img, dataspace_img;
    hssize_t num_entries;
    herr_t status;

    hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(struct calibration));
    H5Tinsert(h5cal_t, "Cal_Slope", HOFFSET(struct calibration, slope),
            H5T_NATIVE_DOUBLE);
    H5Tinsert(h5cal_t, "Cal_Offset",
            HOFFSET(struct calibration, offset),
            H5T_NATIVE_DOUBLE);
 
    fid = H5Fopen(fname_input[num] , H5F_ACC_RDONLY , H5P_DEFAULT );
    dataset = H5Dopen2(fid, AttributePath, H5P_DEFAULT);    
    dataspace = H5Dget_space(dataset);
    num_entries = H5Sget_simple_extent_npoints(dataspace);
    cal=(struct calibration*)malloc(sizeof(struct calibration)*num_entries);
    status = H5Dread(dataset, h5cal_t, H5S_ALL, dataspace, H5P_DEFAULT, cal);
    H5Dclose(dataset);
}

void read_data_hd(int num)
{
    hid_t dataset, dataspace;
    herr_t status;

    dataset = H5Dopen2(fid, ChannelPathHd, H5P_DEFAULT);    
    dataspace = H5Dget_space(dataset);

    data_hd[num]=(unsigned short*)malloc(sizeof(unsigned short)*DIM_HD_X*DIM_HD_Y);
    status = H5Dread(dataset, H5T_NATIVE_USHORT, H5S_ALL, dataspace, H5P_DEFAULT, data_hd[num]);
    H5Dclose(dataset);
}

unsigned short pixel(int x,int y,int channel,int file)
{
    int loc = x + y*DIM;
    unsigned short this=data[file][channel][loc];
    return this;
}

double pixel_true(int x,int y,int channel)
{
    int loc = x + y*DIM;
    double this=data_true[channel][loc];   
    return this;
}

bool pixel_mask(int x,int y,int channel)
{
    int loc = x + y*DIM;
    bool this=mask[channel][loc];   
    return this;
}
double pixel_hd_true(int x,int y,int file)
{
    int loc = x + y*DIM_HD_X;
    double this=data_hd_true[file][loc];   
    return this;
}
unsigned short pixel_hd(int x,int y,int file)
{
    int loc = x + y*DIM_HD_X;
    double this=data_hd[file][loc];   
    return this;
}

void toRadiance_hd(int num)
{
    data_hd_true[num]=malloc(sizeof(double)*DIM_HD_X*DIM_HD_Y);
    for (int j=0;j<DIM_HD_X*DIM_HD_Y;++j)
    {
        data_hd_true[num][j]=((double)data_hd[num][j])*cal[11].slope+cal[11].offset;
    }
}

/*void toRadiance()
{
    for (int i=0;i<TOT_DATA;++i)
    {
        for (int j=0;j<DIM_HD_X*DIM_HD_Y;++j)
        {
            data_hd_true[i][j]=((double)data_hd[i][j])*cal[num[i]-1].slope+cal[num[i]-1].offset;
        }
    }
}*/

void toBrightness()
{
    //costants
    double A[]={0.9959,0.9963,0.9991,0.9996,0.9983,0.9981};
    double B[]={3.471,2.219,0.485,0.181,0.627,0.576};
    double Vc[]={2569.094,1598.566,1362.142,1149.083,930.659,752.381};
    double C1=1.19104*pow(10,-5);
    double C2=1.43877;

    //printf("log=%f",log(5));

    for (int i = 3; i < NUM_CHANNEL; ++i)
    {
        for (int j = 0; j < DIM*DIM; ++j)
        {
            //printf("before=%lf\n",data_true[i][j]);
            if (data_true[i][j]<0)
                data_true[i][j]=0;
            else 
                data_true[i][j]=((C2*Vc[i-3])/(log((C1*pow(Vc[i-3],3)/data_true[i][j])+1)-B[i-3]))/A[i-3];
            //printf("after=%lf\n",data_true[i][j]);
        }
    }
}

void setup_final_mask()
{
    final_mask=(bool*)malloc(sizeof(bool)*DIM*DIM);
    for (int i = 0; i < DIM*DIM; ++i)
    {    
        final_mask[i]=true;
    } 
}

#define FROM_X 0
#define TO_X DIM
#define FROM_Y 0
#define TO_Y DIM

void sim()
{
    setup_final_mask();
    for (int i = 0; i < NUM_CHANNEL; ++i)
    {
        mask[i]=(bool*)malloc(sizeof(int)*DIM*DIM);
    }
    for (int l = 0; l < NUM_CHANNEL; ++l)
    {    
        for (int i = FROM_X; i < TO_X; ++i)
        {
            for (int j = FROM_Y; j < TO_Y; ++j)
            {
                double max=-100.0,min=2000.0;
                for (int k1 = i-7; k1 < i+7; ++k1)
                {
                    for (int k2 = j-7; k2 < j+7; ++k2)
                    {
                        if ((!((k1)<0)) && (!((k2)<0)) && (!((k1)>DIM)) && (!((k2)>DIM)))
                        {
                            if (pixel_true(k1,k2,l) < min)
                            {
                                min=pixel_true(k1,k2,l);
                            }
                            if (pixel_true(k1,k2,l) > max)
                            {
                                max=pixel_true(k1,k2,l);
                            }
                        }
                    }
                }
                if ((max-min)>4) mask[l][i + j*DIM]=false;
                /*printf("max=%lf\n",max);
                printf("min=%lf\n",min);
                printf("pixel(%d,%d)\n",i,j);
                printf("canale=%d\n",l);*/
            }
        }
    }
    final_mask=(bool*)malloc(sizeof(int)*DIM*DIM);
    for (int l = 0; l < NUM_CHANNEL; ++l)
    {
        for (int i = 0; i < DIM*DIM; ++i)
        {    
            if (mask[l][i]==0)
            final_mask[i]=0;
        }        
    }
}

void get_coordinates(int num)
{
    hid_t dataset, dataspace;
    herr_t status;

    dataset = H5Dopen2(fid, DescriptionPath, H5P_DEFAULT);    
    dataspace = H5Dget_space(dataset);

    hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(struct description));
    hid_t h5c80_t = H5Tcreate(H5T_STRING, 80);
    H5Tinsert(h5cal_t, "EntryName", HOFFSET(struct description, name),h5c80_t);
    H5Tinsert(h5cal_t, "Value",HOFFSET(struct description, value),h5c80_t);
    descr=(struct description*)malloc(sizeof(struct description)*27);
    status = H5Dread(dataset, h5cal_t, H5S_ALL, dataspace, H5P_DEFAULT, descr);
    for (int i = 20; i < 24; ++i)
    {
        printf("%s\n", descr[i].value);
        switch (i)
        {
            case 20:
                south[num]=atoi(descr[i].value);
                break;
            case 21:
                north[num]=atoi(descr[i].value);
                break;
            case 22:
                east[num]=atoi(descr[i].value);
                break;
            case 23:
                west[num]=atoi(descr[i].value);
                break;
        }
    }
    H5Dclose(dataset);
}


void get_input_files()
{
    DIR * d;
    FILE * pFile;
    int len;
    int last_file=0;
    struct dirent *dir;
    char direct[100];
    char* directory="/home/studenti/dagostino/fulldisk-201501/";
    d = opendir(directory);
    if (d)
    {
        while ((dir = readdir(d)) != NULL && (last_file<NUM_DAYS*DATA_PER_DAY))
        {
            len=strlen(dir->d_name);
            if ((strstr(dir->d_name,".h5")!=NULL) && (strstr(dir->d_name,".gz")==NULL))
            {
                strcpy(direct,directory);
                strcat(direct,dir->d_name);
                strcpy(fname_input[last_file],direct);
                ++last_file;
            }         
        }
    closedir(d);
    }
}

void get_mask_files()
{
    DIR * d;
    FILE * pFile;
    int len;
    int last_file=0;
    struct dirent *dir;
    char direct[100];
    char* directory="/home/studenti/dagostino/fulldisk-201501/cloudmask";
    d = opendir(directory);
    if (d)
    {
        while ((dir = readdir(d)) != NULL && (last_file<NUM_DAYS*DATA_PER_DAY))
        {
            len=strlen(dir->d_name);
            if ((strstr(dir->d_name,".grb")!=NULL))
            {
                strcpy(direct,directory);
                strcat(direct,dir->d_name);
                strcpy(fname_mask[last_file],direct);
                ++last_file;
            }         
        }
    closedir(d);
    }
}

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

double median(int loc)
{
    double* values;

    values=malloc(sizeof(double)*TOT_DATA);
    for (int j = 0; j < TOT_DATA; ++j)
    {       
        values[j]=data_hd_true[j][loc];             
    }
    qsort (values, TOT_DATA, sizeof(double), compare);
    return values[TOT_DATA/2]; 
}

void clear_sky()
{
    for (int i = 0; i < DIM_HD_X*DIM_HD_Y; ++i)
    {
        median(data_hd_true[i]);
    }
}

unsigned short* cut_and_shape(unsigned short matrix[],int len,int dimx,int north,int east,int south,int west)
{
    //da sud est a nord ovest
    unsigned short* shaped;

    int south_east = east + south*dimx;
    int south_west = west + south*dimx;
    int north_east = east + north*dimx;
    int north_west = west + north*dimx;
    int i;
    
    shaped=malloc(sizeof(unsigned short)*len);
    /*for (int i = south_east; i < north_west; ++i)
    {
        shaped[i]=matrix[i];
        //funziona male
    }*/
    for (int i = west; i < east; ++i)
    {
        for (int j = north; j < south; ++j)
        {
            int loc = i + j*dimx;
            shaped[loc]=matrix[loc];
        }
    }
    return shaped;
}

void print_matrix(double matrix[],int dim,int flag)//0 --> unsigned short , 1 --> double
{
    if (!flag)
    {
        for (int i = 0; i < dim; ++i)
        {
            printf("%d\n",(unsigned short)matrix[i]);
        }
    }
    else
    {
        for (int i = 0; i < dim; ++i)
        {
            printf("%f\n",matrix[i]);
        }        
    }
}

#define PADDING_NORTH 400
#define PADDING_EAST 1312
#define PADDING_SOUTH 3045
#define PADDING_WEST 1970

void main()
{
    unsigned short* maschera;
    unsigned short* shaped;
    for (int i=0;i<NUM_CHANNEL;++i)
        data_true[i]=(double*)malloc(sizeof(double)*DIM*DIM);
    get_input_files();
    read_calibration(0);
    printf("%s\n",fname_input[0]);
    read_data(0);
    read_data_hd(0);
    printf("pixel_hd_before=%d\n",pixel_hd(7000,3000,0));
    toRadiance_hd(0);
    printf("pixel=%d\n",pixel(3000,2104,0,0));
    printf("pixel_hd_after=%f\n",pixel_hd_true(7000,3000,0));
    printf("%f\n",cal[0].slope);
    get_coordinates(0);
    maschera=get_eumetsat_mask("mask.grb",TOT_DATA);
    #if 1
    zoom_mask[0]=zoom(maschera,DIM,DIM,DIM*3,DIM*3);
    //printh5(zoom_mask[0],DIM*3,DIM*3);
    #else

    for(int j=0;j < TOT_DATA;++j)
    {
        zoom_mask[j]=malloc(sizeof(unsigned short)*DIM*3*DIM*3);
    }

    zoom_mask[0]=zoom(maschera,DIM,DIM*3,DIM*3);
    printh5(zoom_mask[0],DIM*3,DIM*3);
    #endif

    printf("north=%d\n",north[0]);
    printf("east=%d\n",east[0]);
    printf("south=%d\n",south[0]);
    printf("west=%d\n",west[0]);

    //printh5(data[0][6],DIM,DIM);
    //shaped=cut_and_shape(data[0][6],DIM*DIM,DIM,0 + PADDING_NORTH, DIM - PADDING_EAST ,DIM - PADDING_SOUTH ,0 + PADDING_WEST);
    shaped=cut_and_shape(data[0][6],DIM*DIM,DIM,0 + PADDING_NORTH , DIM - PADDING_EAST  ,DIM - PADDING_SOUTH ,0 + PADDING_WEST);
    printh5(shaped,DIM,DIM);
    //clear_sky();
    //get_input_files();
    //printh5((unsigned short*)mask_final_eumet);
}
/*Nota per me:
D+evo prendere solo la parte dell'Italia sia dalla maschera(gia" fatto) che dal canale hd.
*/