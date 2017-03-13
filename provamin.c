#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <stdbool.h>

#define L15_PATH "U-MARF/MSG/Level1.5/"

#define NUM_CHANNEL 5

char *ChannelPath[] = {
    L15_PATH "DATA/Channel 01/IMAGE_DATA",
    L15_PATH "DATA/Channel 02/IMAGE_DATA",
    L15_PATH "DATA/Channel 09/IMAGE_DATA",
    L15_PATH "DATA/Channel 11/IMAGE_DATA",
    L15_PATH "DATA/Channel 04/IMAGE_DATA",
};

int num[NUM_CHANNEL]={1,2,9,11,4};

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

//OK singolo
    /*dataset_img = H5Dopen2(fid, ChannelPath[0], H5P_DEFAULT);    
    dataspace_img = H5Dget_space(dataset_img);
    data=(unsigned short*)malloc(sizeof(unsigned short)*DIM*DIM);
    status = H5Dread(dataset_img, H5T_NATIVE_USHORT, H5S_ALL, dataspace_img, H5P_DEFAULT, data);
    H5Dclose(dataset_img);


    for (int i=300000;i<=400000;++i)
    {
        printf("value=%u\n",data[i]);
    }*/

    /*for (int i=0;i<NUM_CHANNEL;++i)
    {
        for (int j=300000;j<=400000;++j)
        printf("canale %d ,value=%u\n",i,data[i][j]);
    }*/
}

/*void set_region()
{
    hid_t dataset, dataspace;
    hssize_t num_entries;
    herr_t status;
    struct KeyValue *keys;
    hid_t h5keyval_t = H5Tcreate(H5T_COMPOUND, sizeof(struct KeyValue));
    dataset = H5Dopen2(fid, DescriptionPath, H5P_DEFAULT);

    if (dataset < 0)
        no_dataset(fname, DescriptionPath);

    dataspace = H5Dget_space(dataset);
    if (H5Sget_simple_extent_ndims(dataspace) != 1)
        fprintf(stderr ,"too many dimensions in description dataset");

    num_entries = H5Sget_simple_extent_npoints(dataspace);

    keys=(struct KeyValue*)malloc(sizeof(struct KeyValue)*num_entries);

    status = H5Dread(dataset, h5keyval_t, H5S_ALL, dataspace, H5P_DEFAULT, keys);
    if (status < 0 )
        fprintf(stderr,"failed to read image description data");
    H5Dclose(dataset);

    data.fulldisk.south = 1;
    data.fulldisk.east = 1;
    for (auto const& kv : pairs) {
        if (kv.name == fulldisk_lines_key)
            data.fulldisk.north = atoi(kv.value);
        else if (kv.name == fulldisk_cols_key)
            data.fulldisk.west = atoi(kv.value);tatic hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(CalibrationData<double>));
        // TODO early bailout
    }


    // Now check if there is a subset table, and use it to set the region

    htri_t subset_exists = H5Lexists(fid, SubsetPath, H5P_DEFAULT);

    if (subset_exists < 0)
        throw runtime_error("error looking for subset in '" + string(fname) + "'");

    if (subset_exists == 0) {
        data.subset = data.fulldisk;
        return;
    }

    // We get here if the subset table exists. Read it.
    dataset = H5Dopen2(fid, SubsetPath, H5P_DEFAULT);

    if (dataset < 0)
        no_dataset(fname, SubsetPath);

    dataspace = H5Dget_space(dataset);
    if (H5Sget_simple_extent_ndims(dataspace) != 1)
        throw runtime_error("too many dimensions in subset dataset");

    num_entries = H5Sget_simple_extent_npoints(dataspace);

    pairs.clear();
    pairs.resize(num_entries);

    status = H5Dread(dataset, h5keyval_t, H5S_ALL, dataspace, H5P_DEFAULT, pairs.data());
    if (status < 0 )
        throw runtime_error("failed to read image subset data");
    H5Dclose(dataset);

    for (auto const& kv : pairs) {
        if (kv.name == subset_north_key)
            data.subset.north = atoi(kv.value);
        else if (kv.name == subset_south_key)
            data.subset.south = atoi(kv.value);
        else if (kv.name == subset_east_key)
            data.subset.east = atoi(kv.value);
        else if (kv.name == subset_west_key)
            data.subset.west = atoi(kv.value);
        // TODO early bailout
    }
}*/

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

void pre()
{
    for (int i=0;i<NUM_CHANNEL;++i)
    {
        for (int j=0;j<DIM*DIM;++j)
        {
            data_true[i][j]=((double)data[i][j])*cal[num[i]].slope+cal[num[i]].offset;
        }
    }
    for (int i = 0; i < DIM*DIM; ++i)
    {
        data_true[4][i]=data_true[2][i]-data_true[4][i];
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

#define FROM_X 1500
#define TO_X 2000
#define FROM_Y 1500
#define TO_Y 2000

void sim()
{
    setup_final_mask();
    for (int i = 0; i < NUM_CHANNEL; ++i)
    {
        mask[i]=(bool*)malloc(sizeof(bool)*DIM*DIM);
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
                if ((max-min)>4) mask[l][i + j*DIM]=true;
                printf("max=%lf\n",max);
                printf("min=%lf\n",min);
                printf("pixel(%d,%d)\n",i,j);
                printf("canale=%d\n",l);
            }
        }
    }
    final_mask=(bool*)malloc(sizeof(bool)*DIM*DIM);
    for (int l = 0; l < NUM_CHANNEL; ++l)
    {
        for (int i = 0; i < DIM*DIM; ++i)
        {    
            if (mask[l][i]==false)
            final_mask[i]=false;
        }        
    }
}

void main()
{
    for (int i=0;i<NUM_CHANNEL;++i)
        data_true[i]=(double*)malloc(sizeof(double)*DIM*DIM);
    read_data();
    printf("pixel=%d\n",pixel(3000,2104,0));
    pre();
    sim();
    printf("pixel=%lf\n",pixel_true(3000,2104,0));
    printf("%lf\n",data_true[0][3000 + 2104*DIM]);
}