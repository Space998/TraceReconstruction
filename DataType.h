#ifndef DATATYPE_H
#define DATATYPE_H

#include <algorithm>
#include <fstream>
#include <vector>
#include <memory>
#include <map>
#include "Rivelatore.h"

/*
type:	0xAAAA0000 - For point generation without limit
		0xBBBB0000 - For point generation with limit
		0xAAAAFFFF - For line generation without limit
		0XBBBBFFFF - For line generation with limit

detector: type of detector used - "name of the detector"

take:	Number of takes, "number of time the program was launched before" -> Need to find a way to do it. Maybe will use a config file. COUNTED FROM 1

number: Number of intersection beetween a ray and the detector. COUNTED FROM 1

point: 	Number of intersection beetween a ray and the three plates of the detector. COUNTED FROM 1
*/

/*
struct fileHeader		//Struct for the file header
{
	unsigned int checkWord : 32;
	unsigned int detector : 32;
	unsigned int type : 32;
	int take : 32;
	int date1 : 32;		//the data value was divided into two parts to maintain 32 bits allignement in accordance with the rest of the program
	int date2 : 32;

	fileHeader(unsigned int ty, int t, int64_t date) : checkWord{0xF0CAFACE}, detector{0x1234ABCD}, type{ty}, take{t}, date1{int(date >> 32)}, date2{int(date)} {}

	int64_t date()		//Method to return the complite data value					
	{
		return (int64_t(date1) << 32) + date2;
	}
};
static_assert(sizeof(fileHeader) == 6*4);
*/

struct fileHeader		//Struct for the file header
{
	unsigned int checkWord : 32;
	Rivelatore detector;		//Detector used
	int take : 32;
	int date1 : 32;		//the data value was divided into two parts to maintain 32 bits allignement in accordance with the rest of the program
	int date2 : 32;

	fileHeader(Rivelatore det, int t, int64_t date) : checkWord{0xF0CAFACE}, detector{det}, take{t}, date1{int(date >> 32)}, date2{int(date)} {}

	int64_t date()		//Method to return the complite data value					
	{
		return (int64_t(date1) << 32) + date2;
	}
};
static_assert(sizeof(fileHeader) == 12*4);

struct headerType		//Struct fot header of data			
{
	unsigned int checkWord : 32;
	int number : 32;				//Nuber of event -> start counting from 1			
	int dimension : 32;				//dimension of the event, number of value taken during this event (from 1 to number of plates of the detector) + noise -> starts counting from 1

	headerType(int n, int d) : checkWord{0x4EADE500}, number{n}, dimension{d} {}		//0x4EADE500 = HEADER00
};
static_assert(sizeof(headerType) == 3*4);

struct dataType			//Struct for data
{
	unsigned int checkWord : 32;
	int time1 : 32; 				//Time passed from the start of the simulation fin ns	
	int time2 : 32; 				//Time passed from the start of the simulation fin ns
	int plate : 32;				//Plate hitten -> counted from zero to two
	int value : 32;				//Number of the pixel hit
	
	dataType(int64_t t, int p, int v = 0) : checkWord{0xDADADADA}, time1{int(t >> 32)}, time2{int(t)}, plate{p}, value{v} {}				//0xDADADADA = DATADATAù

	int64_t time()		//Method to return the complite data value					
	{
		return (int64_t(time1) << 32) + time2;
	}
};
static_assert(sizeof(dataType) == 4*5);
 
struct outFileHeader		//Struct for the file header of the analised data file
{
	unsigned int checkWord : 32;
	Rivelatore detector;		//Detector used
	int take : 32;
	int date1 : 32;		//the data value was divided into two parts to maintain 32 bits allignement in accordance with the rest of the program
	int date2 : 32;

	outFileHeader(Rivelatore det, int t, int64_t date) : checkWord{0xF0CADEAD}, detector{det}, take{t}, date1{int(date >> 32)}, date2{int(date)} {}

	int64_t date()		//Method to return the complite data value					
	{
		return (int64_t(date1) << 32) + date2;
	}
};
static_assert(sizeof(outFileHeader) == 12*4);

struct outHeaderType
{
	unsigned int checkWord : 32;
	int num : 32; 					//Number of events analysed
	int found : 32;					//Number of track reconstructed -> doesn't cout the case with significance 1

	outHeaderType(int n, int f) : checkWord{0x0074EADE}, num{n}, found{f} {} //0x0074EADE  = 0xOUTHEADE
};
static_assert(sizeof(outHeaderType) == 3*4);

struct outDataType
{
	unsigned int checkWord : 32;
	int eventNum : 32;
	float mValue;
	float qValue;

	outDataType(int n, float m, float q) : checkWord{0x007DADAD}, eventNum{n}, mValue{m}, qValue{q} {} //0x007DADAD = 0xOUTDATAD
};
static_assert(sizeof(outDataType) == 4*4);

#endif
