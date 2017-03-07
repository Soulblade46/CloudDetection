/* Support for SEVIRI data in HDF5 format */

#include "seviri-hdf5.hh"

#include <hdf5.h>

#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

namespace Seviri
{

namespace HDF5
{

/* Information about regions etc is stored in key-value tables where both keys and
 * values are 80-char strings
 */
struct KeyValue {
	char name[80];
	char value[80];
};

static hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(CalibrationData<double>));
static hid_t h5c80_t = H5Tcreate(H5T_STRING, 80);
static hid_t h5keyval_t = H5Tcreate(H5T_COMPOUND, sizeof(KeyValue));
static bool setup = false;

#define L15_PATH "U-MARF/MSG/Level1.5/"

static const char *ChannelPath[] = {
	L15_PATH "DATA/Channel 04/IMAGE_DATA",
	L15_PATH "DATA/Channel 09/IMAGE_DATA",
};

#define META_PATH L15_PATH "METADATA/"
#define HEADER_PATH META_PATH "HEADER/"

static const char *AttributePath   = HEADER_PATH "RadiometricProcessing/Level15ImageCalibration_ARRAY";
static const char *DescriptionPath = HEADER_PATH "ImageDescription/ImageDescription_DESCR";
static const char *SubsetPath = META_PATH "SUBSET";

// Note the whitespace here -----------------------------------------v---
static const string fulldisk_lines_key("ReferenceGridVIS_IR-NumberOfLines ");
static const string fulldisk_cols_key("ReferenceGridVIS_IR-NumberOfColumns");
static const string subset_north_key("VIS_IRNorthLineSelectedRectangle");
static const string subset_south_key("VIS_IRSouthLineSelectedRectangle");
static const string subset_east_key("VIS_IREastColumnSelectedRectangle");
static const string subset_west_key("VIS_IRWestColumnSelectedRectangle");

// throw an error about a missing dataset `path` in file `fname`
void no_dataset(string const& fname, string const& path)
{
	stringstream err_msg;
	err_msg << "could not open dataset '" << path << "' in '" << fname << "'";
	throw runtime_error(err_msg.str());
}

// set the fulldisk and subset regions for Data from the HDF5 file fname
// opened as `fid`
void set_region(Data &data, hid_t fid, string const& fname)
{
	hid_t dataset, dataspace;
	hssize_t num_entries;
	herr_t status;
	vector<KeyValue> pairs;

	// Read fulldisk lines and columns from image description table
	dataset = H5Dopen2(fid, DescriptionPath, H5P_DEFAULT);

	if (dataset < 0)
		no_dataset(fname, DescriptionPath);

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

// read into `data` the calibration info for the wanted channes
// from HDF5 file `fname` opened as `fid`
void read_calibration(Data &data, hid_t fid, string const& fname)
{
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

// Sanity checks on the loaded data
void verify(Data const& data)
{
	Region const& full(data.fulldisk);
	Region const& sub(data.subset);

	clog << "Full disk:\t" <<
		"lines [" << full.south << "," << full.north << "], " <<
		"columns [" << full.east << "," << full.west << "]\n" <<
		"Subset   :\t"
		"lines [" << sub.south << "," << sub.north << "], " <<
		"columns [" << sub.east << "," << sub.west << "]" << endl;

	if (full.north != 3712 || full.west != 3712)
		throw invalid_argument("wrong fulldisk extrema");
	if (sub.south < full.south || sub.north > full.north ||
		sub.east < full.east || sub.west > full.west)
		throw invalid_argument("wrong subset extrema");

	clog << "Channel 4: " << data.ch04.calibration.offset
		<< " + P*" << data.ch04.calibration.slope
		<< "\tO/S=" << data.ch04.calibration.offset/data.ch04.calibration.slope
		<< endl;
	clog << "Channel 9: " << data.ch09.calibration.offset
		<< " + P*" << data.ch09.calibration.slope
		<< "\tO/S=" << data.ch09.calibration.offset/data.ch09.calibration.slope
		<< endl;
}

Data load_image(string const& fname)
{
	if (!setup) {
		// setup the h5cal_t type
		H5Tinsert(h5cal_t, "Cal_Slope",
			HOFFSET(CalibrationData<double>, slope),
			H5T_NATIVE_DOUBLE);
		H5Tinsert(h5cal_t, "Cal_Offset",
			HOFFSET(CalibrationData<double>, offset),
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

	verify(data);

	clog << "'" << fname << "' loaded" << endl;

	return data;
}

}
}
