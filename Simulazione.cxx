#include "Simulazione.h"
#include <iostream>
#include "DataType.h"
#include <fstream>
#include <ctime>
#include <chrono>
#include <bitset>

//Constructor for Simulation class
Simulazione::Simulazione(int num) : m_num(num) {;}

//Function for the evaluation of m and q end similar
float mLine(const float y1, const float x1, const float y, const float x)
{
    return (y - y1)/(x - x1);
}

float qLine(const float y1, const float x1, const float y, const float x)
{
    return y1 - x1 * (y - y1);
}

float RandomFloat(float min, float max) 
{
    return  (max - min) * ((((float) rand()) / (float) RAND_MAX)) + min ;
}

auto time()
{
    auto data = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(data);  
    return std::ctime(&end_time);
}

int mBorders(float y, float x, float (&array1)[2], float (&array2)[2])             //Function that return the value of m1 and m2 between with the m of the random line needs to be generated in case of a measure with limits
{
    float m1_1 = mLine(0,0,y,x);
    float m1_2 = mLine(0,-2,y,x);
    float m2_1 = mLine(1,0,y,x);
    float m2_2 = mLine(1,-2,y,x);
    //std::cout << m1_2 << ", " << m2_1 << ", " << m1_1 << ", " << m2_2 << ", " << std::endl;

    if (m2_1 > m1_2 || m2_2 > m1_1)
    {
        std::cerr << "ERROR: Impossible to guarantee three hits" << std::endl;
        return 1;
    }
    else if (m1_1 > m1_2 && m2_2 > m2_1)
    {
        array1[0] = m1_2;
        array2[0] = m2_2;
    }
    else if (m1_1 > m1_2 && m2_1 > m2_2)
    {
        array1[0] = m1_2;
        array2[0] = m2_1;            
    }
    else if (m1_2 > m1_1 && m2_2 > m2_1)
    {
        array1[0] = m2_2;
        array2[0] = m1_2;
    }
}

//Printing dataTypes

void print_beginType(std::ofstream &file, bool type, uint64_t time, int take)
{
    int out = 0b11 << 22;                                    //Set head = 11
    if(type) {out = out | 0b1<<20; }                         //Set type = 0 for point geration, 1 for line generation                    
    out = out | take;
	file.write(reinterpret_cast<char*>(&out), 3);
    file.write(reinterpret_cast<char*>(&time), 8);
}

void print_headerType(std::ofstream &file, int num)
{
    int out = 0b01 << 22;                                    //Set head = 01                  
    out = out | num;
	file.write(reinterpret_cast<char*>(&out), 3);
}

void print_dataType(std::ofstream &file, int plate, int value)
{
    int out = 0b00 << 22;                                    //Set head = 00                 
    out = out | plate<< 20;
    out = out | value;
	file.write(reinterpret_cast<char*>(&out), 3);
}

void print_trailerType(std::ofstream &file, int point, int num)
{
    int out = 0b10 << 22;                                    //Set head = 10   
    out = out | point<<20;               
    out = out | num;
	file.write(reinterpret_cast<char*>(&out), 3);
}

void print_endType(std::ofstream &file, bool type, uint64_t time, int num)
{
    int out = 0b11 << 22;                                    //Set head = 11 
    if(type) {out = out | 0b1<<20; }                         //Set type = 0 for point geration, 1 for line generation                  
    out = out | num;
	file.write(reinterpret_cast<char*>(&out), 3);
    file.write(reinterpret_cast<char*>(&time), 8);
}

//Method of Simulate class

int Simulazione::SimulatePoint(const float y, const float x, const bool limit, const bool noise)
{
    //std::cout << "Rivelazione di " << m_rivelatore.m_plate << " punti" << std::endl;
    //std::cout << x << y << std::endl;

    float mq[2] = {0,0};        //{m,q}

    float mq1[2] = {0,0};       //{m1,q1}
    float mq2[2] = {0,0};       //{m2,q2}    

    if (limit)
    {
        mq1[0] = mLine(0,0,y,x);
        mq2[0] = mLine(1,0,y,x);
    }
    else
    {
        mBorders(y,x,mq1,mq2);
    }

    bool type = 0;
    int num = 0;                                                    //Also the number of "data" (in the sense of detector takes) starts counting from 1
    int take = 42;                                                  //Need to write program to determine take -- need to add control (no biger than 17 bit)

    auto file1 = "Simulazione.bin";                                 //Binary file to store all the data, "official file"
    std::ofstream datafile(file1, std::ios::binary);    

    auto file2 = "Original.txt";                                    //File to contain the generated value of m and q for comparison
    std::ofstream originaldatafile(file2);                 

    std::cout << "Starting point simulation\n";
    std::cout << "Take number: " << take << "\n";
    std::cout << "Point to simulate: " << m_num << "\n";
    std::cout << "Output of generated numbers: " << file2 << "\n";
    std::cout << "Output of data: " << file1 << "\n";

    auto instant1 = time();
    print_beginType(datafile, type, uint64_t(instant1), take);
    originaldatafile << "Take number\n";
    originaldatafile << take << "\n";
    originaldatafile << "Begin time\n";
    originaldatafile << instant1;
    originaldatafile << "y\tx\n";
    originaldatafile << y << "\t" << x << "\n";
    originaldatafile << "m\tq\n";

    for (int i = 0; i < m_num; i++)
    {    
        num ++;
        //std::cout << num << std::endl;

        print_headerType(datafile, num);

        int plate = 0;
        float yLine = 0;

        mq[0] = RandomFloat(mq1[0],mq2[0]);
        mq[1] = y - mq[0]*x;
        originaldatafile << mq[0] << "\t" << mq[1] << "\n";
        //std::cout << "y = " << mq[0] << "x + " << mq[1] << std::endl;

        for (int j = 0; j < 3; j++)
        {
            yLine = mq[0]*(-j) + mq[1];
            if (!(y > 1 || y < 0)) 
            {                        
                print_dataType(datafile, plate, int(y*1e6));        //Check the int isn't longer than 20 bit
                plate ++;
            }
        }

        print_trailerType(datafile, plate, num);                    //Plate are conunted from 1 to 3, NOT COUNTED FROM ZERO
    }

    if (noise)
    {
        std::cout << "Noise not impeltemted" << std::endl;
    }

    auto instant2 = time();
    print_endType(datafile, type, uint64_t(instant2), num);
    originaldatafile << "Number of m and q generated\n";
    originaldatafile << num << "\n";
    originaldatafile << "End time\n";
    originaldatafile << instant2;
    
    std::cout << "Time needed for the simulation: " << instant2 - instant1 << "\n";
    std::cout << "End of point simulation\n" << std::endl;

    datafile.close();
    originaldatafile.close();

    // Test function to read -- NON CORRECT
    /*std::ifstream in("Simulazione.bin", std::ios::binary);
    while( !in.eof())
    {
        uint32_t w;
        uint64_t g;
        in.read( (char*) &w, 3);
        std::cout << w << std::endl;
        in.read( (char*) &g, 8);
        std::cout << g << std::endl;
    }
    */
}

int Simulazione::SimulateLine()
{
    std::cout << "Rivelazione di " << m_rivelatore.m_plate << " punti su linea" << std::endl;
}
