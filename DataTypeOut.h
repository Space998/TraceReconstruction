#ifndef DATATYPEOUT_H
#define DATATYPEOUT_H

#include "Rivelatore.h"
#include <cstdint>

struct outFileHeader		//Struct for the file header of the analised data file
{
	unsigned int checkWord : 32;
	Rivelatore detector;		//Detector used
	unsigned int take : 32;
	int date1 : 32;		//the data value was divided into two parts to maintain 32 bits allignement in accordance with the rest of the program
	int date2 : 32;

	outFileHeader(Rivelatore det, unsigned int t, int64_t date) : checkWord{0xF0CADEAD}, detector{det}, take{t}, date1{int(date >> 32)}, date2{int(date)} {}

	int64_t date()		//Method to return the complite data value					
	{
		return (int64_t(date1) << 32) + date2;
	}
};
static_assert(sizeof(outFileHeader) == (4+9)*4);

struct outHeaderType
{
	unsigned int checkWord : 32;
	unsigned int num : 32; 					//Number of events analysed
	unsigned int found : 32;				//Number of track reconstructed -> doesn't cout the case with significance 1

	outHeaderType(unsigned int n,unsigned int f) : checkWord{0x4EADDEAD}, num{n}, found{f} {} //0x0074EADE  = 0xOUTHEADE
};
static_assert(sizeof(outHeaderType) == 3*4);

struct outDataType
{
	unsigned int checkWord : 32;
	unsigned int eventNum : 32;
	float mValue;
	float qValue;

	outDataType(unsigned int n, float m, float q) : checkWord{0xDADADEAD}, eventNum{n}, mValue{m}, qValue{q} {} //0x007DADAD = 0xOUTDATAD
};
static_assert(sizeof(outDataType) == 4*4);

#endif