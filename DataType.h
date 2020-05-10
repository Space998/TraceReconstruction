#ifndef DATATYPE_H
#define DATATYPE_H

#include <algorithm>
#include <fstream>

/*
type:	0 - For point generation
		1 - For line generation

take:	Number of takes, "number of time the program was launched before" -> Need to find a way to do it. Maybe will use a config file. COUNTED FROM 1

number: Number of intersection beetween a ray and the detector. COUNTED FROM 1

point: 	Number of intersection beetween a ray and the three plates of the detector. COUNTED FROM 1
*/

struct beginType				//total 88
{
	uint8_t head : 2;  			//Characterize begin/end
	uint8_t _blank : 1;
	uint8_t type : 1;			//Type of generator: single point or line
	uint8_t _blank1 : 3;
	int take : 17; 				//Number of takes
	uint64_t data : 64;

	beginType(uint8_t ty, uint64_t d, int t = 1) : head{3}, _blank{0}, type{ty}, _blank1{0}, take{t}, data{d} {}
};

struct headerType				//total 24
{
	uint8_t type : 2;  			//Characterize header
	uint8_t _blank : 5;
	int number : 17; 			//Relevation number
	
	headerType(int n) : type{1}, _blank{0}, number{n} {}
};

struct dataType					//total 24
{
	uint8_t type: 2; 			//Characterize data
	uint8_t rivelatore : 2;  	//Plane of the detector that made the measurement
	int valore : 20; 			//Detected value
	
	dataType(uint8_t r = 0, int v = 0) : type{0}, rivelatore{r}, valore{v} {}
};

struct trailerType				//total 24
{
	uint8_t type : 2;  			//Characterize trailer
	uint8_t point : 2;  		//Number of d point taken during this relevation
	uint8_t _blank : 3;
	int number : 17; 			//Relevation number
	
	trailerType(int n, uint8_t p = 0) : type{2}, point{p}, _blank{0}, number{n} {}
};

struct endType 					//total 88
{
	uint8_t head : 2;  			//Characterize begin/end
	uint8_t _blank : 1;
	uint8_t type : 1;			//Type of generator: single point or line
	uint8_t _blank1 : 3;
	int number : 17; 			//Relevation number
	uint64_t data : 64;
	
	endType(uint8_t ty, uint64_t d, int n) : head{3}, _blank{0}, type{ty}, _blank1{0}, number{n}, data{d} {}
};

#endif
