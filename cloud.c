#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <stdbool.h>
#include "printh5.c"
//#include "tests.c"
#include "hrv.c"
#include <math.h>

#define L15_PATH "U-MARF/MSG/Level1.5/"

int *mask1A,*mask1B,*mask1C,*mask2A,*mask3C,*mask4A,*mask4B,*mask4C,*mask4D,*mask4F,*mask5B,*mask5F;

#define NUM_CHANNEL 9

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

int num[NUM_CHANNEL]={1,2,3,4,5,6,7,9,11};

#define META_PATH L15_PATH "METADATA/"
#define HEADER_PATH META_PATH "HEADER/"

char * AttributePath   = HEADER_PATH "RadiometricProcessing/Level15ImageCalibration_ARRAY";
char * DescriptionPath = HEADER_PATH "ImageDescription/ImageDescription_DESCR";
char * SubsetPath = META_PATH "SUBSET";

struct calibration
{
    double slope;
    double offset;
};

struct KeyValue
{
    char name[80];
    char value[80];
};

char* fname ="img-mezzogiorno.h5";
hid_t  fid;

#define DIM 3712

unsigned short* data[NUM_CHANNEL];
//unsigned short (*data)[DIM] [NUM_CHANNEL];
double* data_true[NUM_CHANNEL];
struct calibration *cal;
bool* mask[NUM_CHANNEL];
bool* final_mask;

void read_data()
{
    hid_t dataset, dataspace,dataset_img, dataspace_img;
    hssize_t num_entries;
    herr_t status;
    //struct calibration *cal;
    struct KeyValue *keys;
    //unsigned short (*data)[DIM] = malloc(sizeof(unsigned short[DIM][DIM]));

    hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(struct calibration));
    H5Tinsert(h5cal_t, "Cal_Slope", HOFFSET(struct calibration, slope),
            H5T_NATIVE_DOUBLE);
    H5Tinsert(h5cal_t, "Cal_Offset",
            HOFFSET(struct calibration, offset),
            H5T_NATIVE_DOUBLE);
 
    fid = H5Fopen(fname , H5F_ACC_RDONLY , H5P_DEFAULT );
    dataset = H5Dopen2(fid, AttributePath, H5P_DEFAULT);    
    dataspace = H5Dget_space(dataset);
    H5Sget_simple_extent_ndims(dataspace);
    num_entries = H5Sget_simple_extent_npoints(dataspace);
    cal=(struct calibration*)malloc(sizeof(struct calibration)*num_entries);
    status = H5Dread(dataset, h5cal_t, H5S_ALL, dataspace, H5P_DEFAULT, cal);   
    H5Dclose(dataset);
    /////////////////////////////////////////////////////////////////////////////////////////
    hid_t h5keyval_t = H5Tcreate(H5T_COMPOUND, sizeof(struct KeyValue));
    hid_t h5c80_t = H5Tcreate(H5T_STRING, 80);

    H5Tinsert(h5keyval_t, "EntryName", HOFFSET(struct KeyValue, name), h5c80_t);
    H5Tinsert(h5keyval_t, "Value", HOFFSET(struct KeyValue, value), h5c80_t);

    for (int i=0;i<NUM_CHANNEL;++i)
    {
        dataset_img = H5Dopen2(fid, ChannelPath[i], H5P_DEFAULT);    
        dataspace_img = H5Dget_space(dataset_img);

        data[i]=(unsigned short*)malloc(sizeof(unsigned short)*DIM*DIM);
        //*data[DIM] [NUM_CHANNEL]= malloc(sizeof(unsigned short[DIM][DIM]));

        status = H5Dread(dataset_img, H5T_NATIVE_USHORT, H5S_ALL, dataspace_img, H5P_DEFAULT, data[i]);
        H5Dclose(dataset_img);
    }
}

unsigned short pixel(int x,int y,int channel)
{
    int loc = x + y*DIM;
    unsigned short this=data[channel][loc];
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

void toRadiance()
{
    for (int i=0;i<NUM_CHANNEL;++i)
    {
        for (int j=0;j<DIM*DIM;++j)
        {
            data_true[i][j]=((double)data[i][j])*cal[num[i]-1].slope+cal[num[i]-1].offset;
        }
    }
}

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

void main()
{
    for (int i=0;i<NUM_CHANNEL;++i)
        data_true[i]=(double*)malloc(sizeof(double)*DIM*DIM);
    read_data();
    printf("pixel=%d\n",pixel(3000,2104,0));
    toRadiance();
    //toBrightness();
    sim();
    //printf("pixel=%lf\n",pixel_true(3000,2104,0));
    //printf("%lf\n",data_true[0][3000 + 2104*DIM]);
    zoom();
    printh5((unsigned short*)mask_final_eumet);
}