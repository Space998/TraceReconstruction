#ifndef DATATYPE_H
#define DATATYPE_H

#include <algorithm>
#include <fstream>
#include <vector>
#include <memory>
#include <map>

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

struct headerType		//Struct fot header of data			
{
	unsigned int checkWord : 32;
	int dimension : 32;				//dimension of the event, number of value taken during this event (from 1 to 3) + noise -> starts counting from 1
	int number : 32;				//Nuber of event -> start counting from 1			

	headerType(int d, int n) : checkWord{0x4EADE500}, dimension{d}, number{n} {}		//0x4EADE500 = HEADER00
};
static_assert(sizeof(headerType) == 3*4);

struct dataType			//Struct for data
{
	unsigned int checkWord : 32;
	int time : 32; 				//Time passed from the start of the simulation fin ns	
	int plate : 32;				//Plate hitten -> counted from zero to two
	int value : 32;				//Number of the pixel hit
	
	dataType(int t, int p, int v = 0) : checkWord{0XDADADADA}, time{t}, plate{p}, value{v} {}				//0XDADADADA = DATADATA
};
static_assert(sizeof(dataType) == 4*4);
 
//Defining a vector that contains all the data type 
inline std::vector<std::string> dataStuct{"fileHeader", "fileEnd", "headerType", "dataType"};
inline std::map<int, int> structLenght{
	{0xF0CAFACE, sizeof(fileHeader)},
	{0x4EADE500, sizeof(headerType)},
	{0XDADADADA, sizeof(dataType)}	
};

#endif
