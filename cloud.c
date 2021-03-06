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
#define DATA_PER_DAY 5
#define TOT_DATA (DATA_PER_DAY*NUM_DAYS)

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
int north[TOT_DATA];
int east[TOT_DATA];
int south[TOT_DATA];
int west[TOT_DATA];

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

char* fname_input[NUM_DAYS*DATA_PER_DAY];
char* fname_mask[NUM_DAYS*DATA_PER_DAY];
//char* fname[NUM_DAYS*DATA_PER_DAY]={"img-mezzogiorno.h5"};
hid_t  fid;

#define DIM 3712
#define DIM_HD_X 5568
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
	H5Fclose(fid);
}

void read_data_hd(int num)
{
	hid_t dataset, dataspace;
	herr_t status;

	fid = H5Fopen(fname_input[num] , H5F_ACC_RDONLY , H5P_DEFAULT );
	dataset = H5Dopen2(fid, ChannelPathHd, H5P_DEFAULT);    
	dataspace = H5Dget_space(dataset);

	data_hd[num]=(unsigned short*)malloc(sizeof(unsigned short)*DIM_HD_X*DIM_HD_Y);
	status = H5Dread(dataset, H5T_NATIVE_USHORT, H5S_ALL, dataspace, H5P_DEFAULT, data_hd[num]);
	H5Dclose(dataset);
	H5Fclose(fid);
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

	fid = H5Fopen(fname_input[num] , H5F_ACC_RDONLY , H5P_DEFAULT );
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
			printf("prova\n");
			len=strlen(dir->d_name);
			printf("provina\n");
			if ((strstr(dir->d_name,".h5")!=NULL) && (strstr(dir->d_name,".gz")==NULL))
			{
				printf("provona\n");
				strcpy(direct,directory);
				strcat(direct,dir->d_name);
				fname_input[last_file]=malloc(sizeof(char)*strlen(direct)+1);
				
				strcpy(fname_input[last_file],direct);
				++last_file;
			}
			//printf("%s\n", fname_input[last_file-1]);        
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
	printf("okprova2\n");
	if (d)
	{
		printf("okprova1\n");
		while ((dir = readdir(d)) != NULL && (last_file<NUM_DAYS*DATA_PER_DAY))
		{
			len=strlen(dir->d_name);
			printf("okprova\n");
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

unsigned short median(unsigned short* x, int n) {
	unsigned short temp;
	int i, j;
	// the following two loops sort the array x in ascending order
	/*for(i=0; i<n-1; i++) {
		for(j=i+1; j<n; j++) {
			if(x[j] < x[i]) {
				// swap elements
				temp = x[i];
				x[i] = x[j];
				x[j] = temp;
			}
			printf("Mediana n.%d\n", i);
		}
	}*/

	qsort(x,n,sizeof(unsigned short),compare);

	if(n%2==0) {
		// if there is an even number of elements, return mean of the two elements in the middle
		return((x[n/2] + x[n/2 - 1]) / 2);
	} else {
		// else return the element in the middle
		return x[n/2];
	}
}

#define DIM_REGION 192
unsigned short* t_median(unsigned short* mat[],int dimX,int dimY,int n_files)
{
	unsigned short* t_mediana;
	t_mediana=malloc(sizeof(unsigned short)*dimX*dimY);
	for (int i = 0; i < dimX*dimY; ++i)
	{
		unsigned short* values_files;
		values_files=malloc(sizeof(unsigned short)*n_files);
		for (int j = 0; j < n_files; ++j)
		{
			values_files[j]=mat[j][i];
		}
		t_mediana[i]=median(values_files,n_files);
	}
	return t_mediana;
}

#if 1
unsigned short* s_median(unsigned short* mat[],int dimX,int dimY,int n_files)
{
	int num_square_max=0,pixel_zero=0,num_square=0,start_x=0,start_y=0;
	unsigned short* s_mediana;
	s_mediana=malloc(sizeof(unsigned short)*n_files);
	num_square_max=dimX/DIM_REGION;
	
	printf("median_s\n");
	int l=0;

	for (int k = 0; k < n_files; ++k)
	{
		unsigned short* values_files;
		values_files=malloc(sizeof(unsigned short)*DIM_REGION*DIM_REGION);
		for (pixel_zero = 0; num_square< num_square_max; ++num_square)
		{
			for (int i = pixel_zero; (i < DIM_REGION+pixel_zero) && (l<DIM_REGION*DIM_REGION); ++i)
			{
				for (int j = pixel_zero; (j < DIM_REGION+pixel_zero) && (l<DIM_REGION*DIM_REGION); ++j)
				{
					values_files[l]=mat[k][i+j];
					printf("l=%d\n",l);
					++l;
				}
			}
			l=0;
			//pixel_zero=;
			printf("ok_prima median\n");
			//exit(2);
			s_mediana[num_square]=median(values_files,DIM_REGION*DIM_REGION);
			pixel_zero=pixel_zero+(DIM_REGION*DIM_REGION);
		}//da vedere e incompleto
		printf("Sto per mediare\n");
		s_mediana[k]=median(values_files,DIM_REGION*DIM_REGION);
		printf("File n.%d mediato\n", k);
	}
	return s_mediana;
}
#else
unsigned short* s_median(unsigned short* mat[],int dimX,int dimY,int n_files)
{
	unsigned short* s_mediana;
	s_mediana=malloc(sizeof(unsigned short)*n_files);
	for (int i = 0; i < n_files; ++i)
	{
		unsigned short* values_files;
		values_files=malloc(sizeof(unsigned short)*dimX*dimY);
		for (int j = 0; j < dimX*dimY; ++j)
		{
			values_files[j]=mat[i][j];
		}
		printf("Sto per mediare\n");
		s_mediana[i]=median(values_files,dimX*dimY);
		printf("File n.%d mediato\n", i);
	}
	return s_mediana;
}
#endif

unsigned short s_t_median(unsigned short* mat[],int dimX,int dimY,int n_files)
{
	unsigned short s_t_mediana;
	unsigned short* s_mediana;

	/*t_mediana=t_median(mat,dimX,dimY,n_files);
	s_t_mediana=median(t_mediana,dimX*dimY);*/
	s_mediana=s_median(mat,dimX,dimY,n_files);
	s_t_mediana=median(s_mediana,n_files);
	return s_t_mediana;
}

unsigned short* clear[TOT_DATA];

#if 1
unsigned short** clear_sky(unsigned short* mat[],int dimX,int dimY,int n_files)
{
	//unsigned short* clear[n_files];
	for (int i = 0; i < n_files; ++i)
	{
		clear[i]=malloc(sizeof(unsigned short)*dimX*dimY);
	}
	/*for (int i = 0; i < dimX*dimY; ++i)
	{
		for (int j = 0; j < n_files; ++j)
		{
			clear[i]=mat[j][i];
		}		
	}*/
	for (int j = 0; j < n_files; ++j)
	{
		for (int i = 0; i < dimX*dimY; ++i)
		{
			clear[j][i]=mat[j][i]-  (   (  *(   t_median(mat,dimX,dimY,n_files)+i   )    )   -  s_t_median(mat,dimX,dimY,n_files));
		}
	}
	return clear;
}
#else
#define DIM_REGION 192
unsigned short** clear_sky(unsigned short* mat[],int dimX,int dimY,int n_files)
{
	int start_x=0,start_y=0;
	int pixel_zero=0;
	int num_square_max=0;
	for (int i = 0; i < n_files; ++i)
	{
		clear[i]=malloc(sizeof(unsigned short)*dimX*dimY);
	}
	/*for (int j = 0; j < n_files; ++j)
	{
		for (int i = 0; i < dimX*dimY; ++i)
		{
			clear[j][i]=mat[j][i]-  (   (  *(   t_median(mat,dimX,dimY,n_files)+i   )    )   -  s_t_median(mat,dimX,dimY,n_files));
		}
	}*/
	num_square_max=dimX/DIM_REGION;
	for (num_square = 0; pixel_zero< num_square_max; num_square=pixel_zero+DIM_REGION)
	{
		for (int i = start_x; i < DIM_REGION+pixel_zero; ++i)
		{
			for (int j = start_y; j < DIM_REGION+pixel_zero; ++j)
			{
				clear[j][i]=mat[j][i]-  (   (  *(   t_median(mat,dimX,dimY,n_files)+i   )    )   -  s_t_median(mat,dimX,dimY,n_files));				
			}
		}
	}
	return clear;
}
#endif
/*#define DIM_REGION 192
void region(unsigned short* mat[],int dimX,int dimY,int n_files)
{
	int start_x=0,start_y=0;
	int pixel_zero=0;
	int num_square_max=0;
	for (num_square = 0; pixel_zero< num_square_max; num_square=pixel_zero+DIM_REGION)
	{
		for (int i = start_x; i < DIM_REGION+pixel_zero; ++i)
		{
			for (int j = start_y; j < DIM_REGION+pixel_zero; ++j)
			{
				mat[i][y];				
			}
		}
	}
	return;
}*/


#if 0
unsigned short* cut_and_shape(unsigned short matrix[],int len,int dimx,int north,int east,int south,int west,int dimX_shaped,int dimY_shaped,int token)
{
	//da sud est a nord ovest
	unsigned short* shaped;
	unsigned short* cut;

	int south_east = east + south*dimx;
	int south_west = west + south*dimx;
	int north_east = east + north*dimx;
	int north_west = west + north*dimx;
	int dim=0;

	shaped=malloc(sizeof(unsigned short)*dimX_shaped*dimY_shaped);
	
	/*shaped=malloc(sizeof(unsigned short)*len);
	for (int i = south_east; i < north_west; ++i)
	{
		shaped[i]=matrix[i];
		//funziona male
	}*/
	/*for (int i = west; i < east; ++i)
	{
		for (int j = north; j < south; ++j)
		{
			int loc = i + j*dimx;
			shaped[loc]=matrix[loc];
			++dim;
		}
	}*///corretto

	for (int i = west; i < east; ++i)
	{
		for (int j = north; j < south; ++j)
		{
			int loc = i + j*dimx;
			shaped[dim]=matrix[loc];
			++dim;
		}
	}

	cut=malloc(sizeof(unsigned short)*dim);

	dim=0;
	for (int i = 0; i < len; ++i)
	{
		if (shaped[i]!=token)
		{
			cut[dim]=shaped[i];
			++dim;
		}
	}
	return shaped;
}

#else
unsigned short* cut_and_shape(int flag,unsigned short matrix[],int len,int dimx,int north,int east,int south,int west,int dimX_shaped,int dimY_shaped,unsigned short token)
{
	//da sud est a nord ovest
	unsigned short* shaped;
	unsigned short* cut;

	int south_east = east + south*dimx;
	int south_west = west + south*dimx;
	int north_east = east + north*dimx;
	int north_west = west + north*dimx;
	int dim=0;
	int k=0;

	shaped=malloc(sizeof(unsigned short)*len);
	
	/*shaped=malloc(sizeof(unsigned short)*len);
	for (int i = south_east; i < north_west; ++i)
	{
		shaped[i]=matrix[i];
		//funziona male
	}*/
	for (int i = 0; i < len; ++i)
	{
		shaped[i]=token;
	}
	for (int i = west; i < east; ++i)
	{
		for (int j = north; j < south; ++j)
		{
			int loc = i + j*dimx;
			shaped[loc]=matrix[loc];
			++dim;
		}
	}
	cut=malloc(sizeof(unsigned short)*dim);

	for (int i = 0; i < dim; ++i)
	{
		cut[i]=token;
	}

	for (int i = 0; i < len; ++i)
	{
		if (shaped[i]!=token)
		{
			cut[k]=shaped[i];
			++k;
		}
	}
	if (flag) return cut;
	else return shaped;
}

#endif

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

/*//ITALIA

#define PADDING_NORTH 400
#define PADDING_EAST 1312
#define PADDING_SOUTH 3045
#define PADDING_WEST 1970

#define PADDING_HD_NORTH PADDING_NORTH*3
#define PADDING_HD_EAST PADDING_EAST
#define PADDING_HD_SOUTH 0
#define PADDING_HD_WEST PADDING_WEST*/

#define PADDING_NORTH_HD 1200
#define PADDING_EAST_HD 2000
#define PADDING_SOUTH_HD 900
#define PADDING_WEST_HD 2400

#define PADDING_NORTH_MASK_HD 0
#define PADDING_EAST_MASK_HD 0
#define PADDING_SOUTH_MASK_HD 0
#define PADDING_WEST_MASK_HD 0

unsigned short * rotate(unsigned short original[],int rows,int cols)
{
	unsigned short* rotated;
	int row,col,loc;
	
	rotated=malloc(sizeof(unsigned short)*rows*cols);

	for(row = 0; row < rows; row++)
	{
		for(col = 0; col < cols; col++ )
		{
			int o = (row*cols) + col;
			int n = ((rows-row-1)*cols) + (cols-col-1);
			rotated[n] = original[o];
		}
	}
	return rotated;
}

int get_dimension(unsigned short* matrix)
{
	int len=(sizeof(matrix)/sizeof(unsigned short));
	return len;
}


unsigned short *shaped_hd[TOT_DATA];
int dimY_shaped;
int dimX_shaped;

void read_all()
{
	for (int i = 0; i < TOT_DATA; ++i)
	{
		read_calibration(i);
		//read_data(i);
		//int dimY_shaped=DIM_HD_Y - south[0] - PADDING_SOUTH_HD - (0 + PADDING_NORTH_HD);
		//int dimX_shaped=DIM_HD_X - east[0] - PADDING_EAST_HD - (0 + (DIM_HD_X-west[0]) + PADDING_WEST_HD)+1;
		printf("ok_dim\n");
		read_data_hd(i);
		shaped_hd[i]=cut_and_shape(1,data_hd[i],DIM_HD_X*DIM_HD_Y,DIM_HD_X, 0 + PADDING_NORTH_HD , DIM_HD_X - PADDING_EAST_HD ,DIM_HD_Y - south[0] - PADDING_SOUTH_HD ,0 + PADDING_WEST_HD,dimY_shaped,dimX_shaped,0);
		free(data_hd[i]);
		printf("Letto file n.%d\n", i);
	}
}

void main()
{
	unsigned short* maschera[TOT_DATA];
	unsigned short* rotated[TOT_DATA];
	//unsigned short *shaped_mask[TOT_DATA],*shaped_hd[TOT_DATA];
	unsigned short *shaped_mask[TOT_DATA];
	for (int i=0;i<NUM_CHANNEL;++i)
		data_true[i]=(double*)malloc(sizeof(double)*DIM*DIM);
	printf("ok1\n");
	get_input_files();
	printf("ok2\n");
/*	read_calibration(0);
	read_calibration(50);
	printf("%s\n",fname_input[100]);
	read_data(0);
	read_data_hd(0);
	read_data(50);
	read_data_hd(50);*/

	/*read_calibration(0);
	read_data(0);
	read_data_hd(0);
	read_calibration(50);
	read_data(50);
	read_data_hd(50);*/

	get_coordinates(0);
	dimY_shaped=DIM_HD_Y - south[0] - PADDING_SOUTH_HD - (0 + PADDING_NORTH_HD);
	dimX_shaped=DIM_HD_X - east[0] - PADDING_EAST_HD - (0 + (DIM_HD_X-west[0]) + PADDING_WEST_HD)+1;
	printf("ok_get_cordinates\n");
	read_all();
	printf("Lettura ok\n");
	
	//printf("pixel_hd_before=%d\n",pixel_hd(7000,3000,0));
	
	//toRadiance_hd(0);
	//printf("pixel=%d\n",pixel(3000,2104,0,0));
	//printf("pixel_hd_after=%f\n",pixel_hd_true(7000,3000,0));
	printf("%f\n",cal[0].slope);
	//get_coordinates(0);
	printf("ok\n");

	maschera[0]=get_eumetsat_mask("mask.grb",TOT_DATA);
	zoom_mask[0]=zoom(maschera[0],DIM,DIM,DIM*3,DIM*3);
	rotated[0]=rotate(zoom_mask[0],DIM*3,DIM*3);

	printf("north=%d\n",north[0]);
	printf("east=%d\n",east[0]);
	printf("south=%d\n",south[0]);
	printf("west=%d\n",west[0]);

	//printh5(data[0][6],DIM,DIM);
	//shaped=cut_and_shape(data[0][6],DIM*DIM,DIM,0 + PADDING_NORTH, DIM - PADDING_EAST ,DIM - PADDING_SOUTH ,0 + PADDING_WEST);
	//shaped_mask[0]=cut_and_shape(maschera,DIM*DIM,DIM,0 + PADDING_NORTH , DIM - PADDING_EAST  ,DIM - PADDING_SOUTH ,0 + PADDING_WEST);
	
	//shaped_hd[0]=cut_and_shape(data_hd[0],DIM_HD_X*DIM_HD_Y,DIM_HD_X,north[0]-DIM_HD_Y + PADDING_HD_NORTH , west[0]-east[0]- PADDING_HD_EAST ,DIM_HD_Y - south[0] - PADDING_HD_SOUTH ,0 + PADDING_HD_WEST);
	
	//Questo v------------------------------- funziona bene ma mancano alcuni parametri
	#if 0
	shaped_hd[0]=cut_and_shape(data_hd[0],DIM_HD_X*DIM_HD_Y,DIM_HD_X, 0 + PADDING_NORTH_HD , DIM_HD_X - PADDING_EAST_HD ,DIM_HD_Y - south[0] - PADDING_SOUTH_HD ,0 + PADDING_WEST_HD);
	printh5(shaped_hd[0],DIM_HD_Y,DIM_HD_X);

	#else 
	/*int dimY_shaped=DIM_HD_Y - south[0] - PADDING_SOUTH_HD - (0 + PADDING_NORTH_HD);
	int dimX_shaped=DIM_HD_X - east[0] - PADDING_EAST_HD - (0 + (DIM_HD_X-west[0]) + PADDING_WEST_HD)+1;*///+1
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////+1
	//shaped_mask[0]=cut_and_shape(1,rotated[0],DIM_HD_Y*DIM_HD_Y,DIM_HD_Y, 0 + PADDING_NORTH_HD , DIM_HD_Y - east[0] +1 - PADDING_EAST_HD ,DIM_HD_Y - south[0] - PADDING_SOUTH_HD ,0 + (DIM_HD_Y-west[0]) + PADDING_WEST_HD,dimY_shaped,dimX_shaped,3);
	/////FUNZIONA ^^^^^

	//ok maschera ----^OK
	//originale che non funziona bene --V
	//shaped_hd[0]=cut_and_shape(1,data_hd[0],DIM_HD_X*DIM_HD_Y,DIM_HD_X, 0 + PADDING_NORTH_HD , DIM_HD_X - PADDING_EAST_HD ,DIM_HD_Y - south[0] - PADDING_SOUTH_HD ,0 + PADDING_WEST_HD,dimY_shaped,dimX_shaped,3);
	
	//shaped_hd[0]=cut_and_shape(1,data_hd[0],DIM_HD_X*DIM_HD_Y,DIM_HD_X, 0 + PADDING_NORTH_HD , DIM_HD_X - PADDING_EAST_HD ,DIM_HD_Y - south[0] - PADDING_SOUTH_HD ,0 + PADDING_WEST_HD,dimY_shaped,dimX_shaped,0);
	//FUNZIONA ^^^^^^^

	//shaped_mask[0]=cut_and_shape(rotated,dimX_shaped*dimY_shaped,dimX_shaped, 0 + PADDING_NORTH_HD , DIM_HD_Y - east[0] - PADDING_EAST_HD ,DIM_HD_Y - south[0] - PADDING_SOUTH_HD ,0 + (DIM_HD_Y-west[0]) + PADDING_WEST_HD,dimX_shaped,dimY_shaped,3);


	//printf("dim=%d\n",get_dimension(shaped_mask[0]));

	//printf("dimX_shaped= %d , dimY_shaped= %d\n",dimX_shaped,dimY_shaped);

	/*shaped_hd[0]=cut_and_shape(data_hd[0],DIM_HD_X*DIM_HD_Y,DIM_HD_X, 0 + PADDING_NORTH_HD , DIM_HD_X - PADDING_EAST_HD ,DIM_HD_Y - south[0] - PADDING_SOUTH_HD ,0 + PADDING_WEST_HD,dimX_shaped,dimY_shaped,3);
	printh5(shaped_hd[0],DIM_HD_Y,DIM_HD_X);*/

	/*shaped_mask[0]=cut_and_shape(maschera,DIM*3,DIM*3, 0 + PADDING_NORTH_HD , DIM_HD_X - PADDING_EAST_HD ,DIM_HD_Y - south[0] - PADDING_SOUTH_HD ,0 + PADDING_WEST_HD,dimX_shaped,dimY_shaped,0);
	printh5(shaped_mask[0],DIM*3,DIM*3);*/
	//printh5(shaped_mask[0],DIM_HD_Y,DIM_HD_Y);
	//printh5(shaped_mask[0],get_dimension(shaped_mask[0]),get_dimension(shaped_mask[0]));
	//printh5(rotated,DIM,DIM);
	
	unsigned short** clear;
	clear=clear_sky(shaped_hd,dimX_shaped,dimY_shaped,TOT_DATA);
	
	//STAMPE
	//printh5(shaped_mask[0],dimY_shaped,dimX_shaped);//ok maschera OK (col +1 sulla X NON funziona bene)
	printh5(clear[130],dimY_shaped,dimX_shaped);//ok hd OK quasi (col +1 sulla X funziona bene)
	//printh5(shaped_hd[0],DIM_HD_Y,DIM_HD_X);//ok con shaped
	//printh5(shaped_mask[0],DIM_HD_Y,DIM_HD_Y);//ok con shaped
	//printh5(data_hd[130],DIM_HD_Y,DIM_HD_X);
	
	//printf("Sto per stampare\n");
	//printh5(clear[0],DIM_HD_Y,DIM_HD_X);
	#endif
	
	//clear_sky();
	//get_input_files();
	//printh5((unsigned short*)mask_final_eumet);
}
/*Nota per me:
Devo prendere solo la parte dell'Italia sia dalla maschera (algoritmo gia' pronto) che dal canale hd.
*/
/*OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
OOOOOOOOOOIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOOOOOOOO
OOOOOOOOOIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOOOOOO
OOOOOOOOIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOOOOOO
OOOOOOOIIOOOOOOOOOOOOOOOOOOOOOOOOOOIIOOOOOOOOOOO
OOOOOOIIOOOOOOOOOOOOOOOOOOOOOOOOOOOOIIOOOOOOOOOO
OOOOOIIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOIIOOOOOOOOO
OOOOIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOIOOOOOOOO
OOOOIOOOOOOOOIIIIOOOOOOOOOIIIIOOOOOOOOOIOOOOOOOO
OOOOIOOOOOOOOIOOIOOOOOOOOOIOOIOOOOOOOOOIIOOOOOOO
OOOOIOOOOOOOOIIIIOOOOOOOOOIIIIOOOOOOOOOOIOOOOOOO
OOOIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOIOOOOOOO
OOOOIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOIOOOOOOOO
OOOOOIOOOOOOOOOOOOOOIIIOOOOOOOOOOOOOOOIOOOOOOOOO
OOOOOOIOOOOOOOOOOOOIOO0IOOOOOOOOOOOOIOOOOOOOOOO0
OOOOOOOIOOOOOOOOOOOOIIIOOOOOOOOOOOOOIOOOOOOOOOOO
OOOOOOOOIOOOOOOOOOOOOOOOOOOOOOOOOOOIOOOOOOOOOOOO
OOOOOOOOOIOOOOOOOOOOOOOOOOOOOOOOOOIOOOOOOOOOOOOO
OOOOOOOOOOIOOOOOOOOOOOOOOOOOOOOOOIOOOOOOOOOOOOOO
OOOOOOOOOOOIOOOOOOIIIIIIIOOOOOOOIOOOOOOOOOOOOOOO
OOOOOOOOOOOOIOOOOOIOOOOOIOOOOOOIOOOOOOOOOOOOOOOO
OOOOOOOOOOOOIOOOOOIIIIIIIOOOOOOIOOOOOOOOOOOOOOOO
OOOOOOOOOOOOIOOOOOOOOOOOOOOOOOOIOOOOOOOOOOOOOOOO
OOOOOOOOOOOOOIOOOOOOOOOOOOOOOOIOOOOOOOOOOOOOOOOO
OOOOOOOOOOOOOOIOOOOOOOOOOOOOOIOOOOOOOOOOOOOOOOOO
OOOOOOOOOOOOOOOIOOOOOOOOOOOOIOOOOOOOOOOOOOOOOOOO
OOOOOOOOOOOOOOOIOOOOOOOOOOOOIOOOOOOOOOOOOOOOOOOO
OOOOOOOOOOOOOOOOIOOOOOOOOOOIOOOOOOOOOOOOOOOOOOOO
OOOOOOOOOOOOOOOOOIOOOOOOOOIOOOOOOOOOOOOOOOOOOOOO
OOOOOOOOOOOOOOOOOOIOOOOOOIOOOOOOOOOOOOOOOOOOOOOO
OOOOOOOOOOOOOOOOOOOIOOOOIOOOOOOOOOOOOOOOOOOOOOOO*/


