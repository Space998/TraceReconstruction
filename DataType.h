#ifndef DATATYPE_H
#define DATATYPE_H

#include "Rivelatore.h"
#include <cstdint>

struct fileHeader		//Struct for the file header
{
	unsigned int checkWord : 32;
	Rivelatore detector;		//Detector used
	unsigned int take : 32;		//"number of time the program was launched before" -> starts counting from 1
	int date1 : 32;		//the data value was divided into two parts to maintain 32 bits allignement in accordance with the rest of the program
	int date2 : 32;

	fileHeader(Rivelatore det, unsigned int t, int64_t date) : checkWord{0xF0CAFACE}, detector{det}, take{t}, date1{int(date >> 32)}, date2{int(date)} {}

	int64_t date()		//Method to return the complite data value					
	{
		return (int64_t(date1) << 32) + date2;
	}
};
static_assert(sizeof(fileHeader) == (4+9)*4);

struct headerType		//Struct fot header of data			
{
	unsigned int checkWord : 32;
	int time1 : 32; 			//Moment in time the hit was registered by the trigger form the start of the start of the simulation in ns	
	int time2 : 32;				//This time determines the overall time associated to the event -> can be used to determine rate of events					
	unsigned int number : 32;				//Nuber of event -> start counting from 1			
	unsigned int dimension : 32;				//dimension of the event, number of value taken during this event (from 1 to number of plates of the detector) + noise -> starts counting from 1

	headerType(int64_t t, unsigned int n, unsigned int d) : checkWord{0x4EADFACE}, time1{int(t >> 32)}, time2{int(t)}, number{n}, dimension{d} {}		//0x4EADE500 = HEADER00
	
	int64_t time()		//Method to return the complite data value					
	{
		return (int64_t(time1) << 32) + time2;
	}

};
static_assert(sizeof(headerType) == 5*4);
struct dataType			//Struct for data
{
	unsigned int checkWord : 32;
	int time1 : 32; 			//Moment in time the hit was registered form the start of the start of the simulation in ns	
	int time2 : 32; 				
	unsigned int plate : 32;				//Plate hitten -> counted from zero to two
	unsigned int value : 32;				//Number of the pixel hit
	
	dataType(int64_t t = 0, unsigned int p = 0, unsigned int v = 0) : checkWord{0xDADAFACE}, time1{int(t >> 32)}, time2{int(t)}, plate{p}, value{v} {}				//0xDADADADA = DATADATAù

	int64_t time()		//Method to return the complite data value					
	{
		return (int64_t(time1) << 32) + time2;
	}
};
static_assert(sizeof(dataType) == 5*4);
 
#endif
