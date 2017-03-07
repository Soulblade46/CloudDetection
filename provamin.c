#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

#define L15_PATH "U-MARF/MSG/Level1.5/"

const  char* fid = "img.h5";

struct calibration
{
    double cal_slope[12];
    double cal_offset[12];

};
struct data
{
    uint16_t 
};

int main()
{

    char *ChannelPath[] = {
    	L15_PATH "DATA/Channel 04/IMAGE_DATA",
    	L15_PATH "DATA/Channel 09/IMAGE_DATA",
    };

    #define META_PATH L15_PATH "METADATA/"
    #define HEADER_PATH META_PATH "HEADER/"

    char * AttributePath   = HEADER_PATH "RadiometricProcessing/Level15ImageCalibration_ARRAY";
    char * DescriptionPath = HEADER_PATH "ImageDescription/ImageDescription_DESCR";
    char * SubsetPath = META_PATH "SUBSET";

    // Note the whitespace here -----------------------------------------v---
    char* fulldisk_lines_key("ReferenceGridVIS_IR-NumberOfLines ");
    char* fulldisk_cols_key("ReferenceGridVIS_IR-NumberOfColumns");
    char* subset_north_key("VIS_IRNorthLineSelectedRectangle");
    char* subset_south_key("VIS_IRSouthLineSelectedRectangle");
    char* subset_east_key("VIS_IREastColumnSelectedRectangle");
    char* subset_west_key("VIS_IRWestColumnSelectedRectangle");

    hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(struct calibration);
    hid_t h5c80_t = H5Tcreate(H5T_STRING, 80);
    hid_t h5keyval_t = H5Tcreate(H5T_COMPOUND, sizeof(struct calibration));

    dataset = H5Dopen2(fid, DescriptionPath, H5P_DEFAULT);










    hid_t dataset, dataspace;
    hssize_t num_entries;
    herr_t status;
    vector< CalibrationData<double> > cal;

    // Read fulldisk lines and columns from image description table
    dataset = H5Dopen2(fid, AttributePath, H5P_DEFAULT);

    if (dataset < 0)
        no_dataset(fname, DescriptionPath);

    dataspace = H5Dget_space(dataset);
    if (H5Sget_simple_extent_ndims(dataspace) != 1)
        throw runtime_error("too many dimensions in calibration dataset");

    num_entries = H5Sget_simple_extent_npoints(dataspace);

    if (num_entries != 12)
        throw runtime_error("wrong calibration data length");

    cal.resize(num_entries);

    status = H5Dread(dataset, h5cal_t, H5S_ALL, dataspace, H5P_DEFAULT, cal.data());
    if (status < 0 )
        throw runtime_error("failed to read image description data");
    H5Dclose(dataset);

    // channel numbering starts at 1, array indices at 0
    data.ch04.calibration = cal[3];
    data.ch09.calibration = cal[8];
}

void set_region(Data &data, hid_t fid, string const& fname)
{
    hid_t dataset, dataspace;
    hssize_t num_entries;
    herr_t status;
    vector<KeyValue> pairs;

    // Read fulldisk lines and columns from image description table
    dataset = H5Dopen2(fid, DescriptionPath, H5P_DEFAULT);

    dataspace = H5Dget_space(dataset);
    if (H5Sget_simple_extent_ndims(dataspace) != 1)
        throw runtime_error("too many dimensions in description dataset");

    num_entries = H5Sget_simple_extent_npoints(dataspace);

    pairs.resize(num_entries);

    status = H5Dread(dataset, h5keyval_t, H5S_ALL, dataspace, H5P_DEFAULT, pairs.data());
    if (status < 0 )
        throw runtime_error("failed to read image description data");
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


}