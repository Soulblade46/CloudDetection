#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

#define L15_PATH "U-MARF/MSG/Level1.5/"

const  char* fid = "img.h5";

struct calibration
{
    double slope[12];
    double offset[12];

};
struct KeyValue
{
    char name[80];
    char value[80]; 
};

char *ChannelPath[] = {
        L15_PATH "DATA/Channel 04/IMAGE_DATA",
        L15_PATH "DATA/Channel 09/IMAGE_DATA",
};

static hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(struct calibration));
static hid_t h5c80_t = H5Tcreate(H5T_STRING, 80);
static hid_t h5keyval_t = H5Tcreate(H5T_COMPOUND, sizeof(value));

int main()
{
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

    if (!setup) {
        // setup the h5cal_t type
        H5Tinsert(h5cal_t, "Cal_Slope",
            HOFFSET(calibration, slope),
            H5T_NATIVE_DOUBLE);
        H5Tinsert(h5cal_t, "Cal_Offset",
            HOFFSET(calibration, offset),
            H5T_NATIVE_DOUBLE);

        // setup the h5keyval_t type
        H5Tinsert(h5keyval_t, "EntryName", HOFFSET(KeyValue, name), h5c80_t);
        H5Tinsert(h5keyval_t, "Value", HOFFSET(KeyValue, value), h5c80_t);
    }

    clog << "Loading '" << fname << "'" << endl;

    Data data;

    hid_t fid = H5Fopen(fname.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

    if (fid < 0)
        throw runtime_error("could not open '" + fname + "'");

    set_region(data, fid, fname);
    data.rows = data.subset.north - data.subset.south + 1;
    data.cols = data.subset.west - data.subset.east + 1;

    read_calibration(data, fid, fname);

    // TODO this is a bit of a hack, it would be nice to find a more
    // robust way to map channel paths to the channeldata in the struct
    ChannelData<uint16_t, double> *channel_data = &(data.ch04);

    herr_t status;

    for (auto path : ChannelPath) {
        hid_t dataset = H5Dopen2(fid, path, H5P_DEFAULT);
        if (dataset < 0)
            no_dataset(fname, path);

        hid_t datatype = H5Dget_type(dataset);

        // // Little-Endian or Big-Endian?
        // H5T_order_t order = H5Tget_order(datatype);
        H5T_class_t h_class = H5Tget_class(datatype);
        size_t h_size = H5Tget_size(datatype);

        if (h_class != H5T_INTEGER)
            throw runtime_error(string(path) + " does not have integer type");

        if (h_size != 2)
            throw runtime_error(string(path) + " does not have 16-bit size");

        hid_t dataspace = H5Dget_space(dataset);
        hsize_t dims[2];
        if (H5Sget_simple_extent_dims(dataspace, dims, NULL) != 2)
            throw runtime_error(string(path) + " is not a 2D dataset");

        if (dims[0] != data.rows) {
            stringstream err_msg;
            err_msg << "line mismatch: expected " << data.rows
                << ", found " << dims[1];
            throw runtime_error(err_msg.str());
        }

        if (dims[1] != data.cols) {
            stringstream err_msg;
            err_msg << "column mismatch: expected " << data.cols
                << ", found " << dims[1];
            throw runtime_error(err_msg.str());
        }

        channel_data->raw_data.resize(data.rows*data.cols);
        status = H5Dread(dataset, H5T_NATIVE_USHORT, H5S_ALL, dataspace,
            H5P_DEFAULT, channel_data->raw_data.data());
        if (status < 0)
            throw runtime_error("error reading dataset '" + string(path) + "'");

        H5Dclose(dataset);
        ++channel_data;
    }

    H5Fclose(fid);
}