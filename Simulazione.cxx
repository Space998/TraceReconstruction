#include "DataType.h"
#include "Handler.h"
#include "WriteRead.h"
#include "Simulazione.h"
#include "HoughFunctions.h"

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

//Random generators
std::random_device rd;
std::mt19937 gen(rd());

//Poisson
int poisson(const float mean)
{
    std::poisson_distribution<int> pois(mean);
    return pois(gen);
}

//Random int
int randomInt(const int &min, const int &max)
{
    std::uniform_int_distribution<int> uni(min,max);
    return uni(gen);
}

//Random Float
float randomFloat(const float &min, const float &max) 
{
    std::uniform_real_distribution<> flo(min, max);
    return flo(gen);
}

//Function for the evaluation of m and q end similar
void mBorders(const Rivelatore &rivelatore, const float y, const float x, float &m1, float &m2) 
{
    float m1_1 = mLine(0,0,y,x);
    float m1_2 = mLine(0,-rivelatore.m_width,y,x);
    float m2_1 = mLine(rivelatore.m_lenght,0,y,x);
    float m2_2 = mLine(rivelatore.m_lenght,-rivelatore.m_width,y,x);
    //std::cout << m1_2 << ", " << m2_1 << ", " << m1_1 << ", " << m2_2 << ", " << std::endl;

    if (m2_1 > m1_2 || m2_2 > m1_1)
    {
        std::cerr << "ERROR: Impossible to guarantee three hits" << std::endl;
        exit(1);
    }
    else if (m1_1 > m1_2 && m2_2 > m2_1)
    {
        m1 = m1_2;
        m2 = m2_2;
    }
    else if (m1_1 > m1_2 && m2_1 > m2_2)
    {
        m1 = m1_2;
        m2 = m2_1;
    }
    else if (m1_2 > m1_1 && m2_2 > m2_1)
    {
        m1 = m1_1;
        m2 = m2_2;
    }
}

//Method of Simulate class
void SimulatePoint(std::string filename,const Rivelatore &rivelatore, const int num, const float y, const float x, const bool limit, const bool noise)
{
    std::signal(SIGINT, sig_handler);

    //Calculation of m that characterize lines corresponding to lind angle of the detector
    double mMin = tan(degRad(90+rivelatore.m_angle));
    double mMax = tan(degRad(90-rivelatore.m_angle));

    if (x < 0)
    {
        std::cerr << "Incorrect point passed since x values must be grater than zero\n";
        exit(7);
    }
    else if (y >= ((mMax*x)+rivelatore.m_lenght) || y <= (mMin*x))
    {
        std::cerr << "The point specified will not give rise to any hit, since it is supposed that any hitting track must have must have a slope greater than two degree from the detector plate in zero\n";
        exit(7);
    }
    else if(rivelatore.m_distance < (rivelatore.m_lenght/mMax))
    {
        std::cerr << "It is supposed that the blind angle of the detectro cannot prevent hits\n";
        exit(7);
    }
    //std::cout << mMin << " , " << mMax << std::endl;

    float mq[2] = {0,0};        //{m,q} of the generated trace

    float m1 = 0;       //{m1}   maximum and minumum  values of mq for the generation (in this way only track that intersect the detector are generated)
    float m2 = 0;       //{m2}    

    unsigned int track = 0;              //Number of the generated event
    std::string originalFile;   //File to contain the original data genereted by the algorithm
    unsigned int take = checkWriteFile(filename, originalFile);  

    std::ofstream datafile(filename, std::ios::binary);             //Opens binary file to store all the data, "official file"  
    std::ofstream originaldatafile(originalFile);                   //Opens the file to store all the generated m and q values 

    //Writing on terminal the conditin of the simulation
    std::cout << "Starting point simulation\n";
    std::cout << "Take number: " << take << "\n";       //In case of user defined file the take number will always be one
    std::cout << "Tracks to simulate: " << num << "\n";
    std::cout << "Output of generated numbers: " << originalFile << "\n";
    std::cout << "Output of data: " << filename << "\n";

    auto instant1 = time();         //Determines the time when the simulation begins
    write(datafile, fileHeader(rivelatore, take, int64_t(reinterpret_cast<char*>(&instant1))));  //Writing the header of the file for the simulation in the Simulation.bin file

    if (limit)
    {
        mBorders(rivelatore,y,x,m1,m2);
    }
    else
    {
        m1 = mLine(0,-(rivelatore.m_lenght/mMax),y,x);
        m2 = mLine(rivelatore.m_lenght,rivelatore.m_lenght/mMin,y,x);  
    }
    //std::cout << m1 << " , " << m2 << std::endl;

    originaldatafile << "Original data file\n";
    originaldatafile << "Take number\n";
    originaldatafile << take << "\n";
    originaldatafile << "Point to generate\n";
    originaldatafile << num << "\n";
    originaldatafile << "Type of simulation\n";
    originaldatafile << "Point simulation ";
    if (limit) 
    {
        originaldatafile << "with limits";
    }
    else
    {
        originaldatafile << "without limits";
    }
    if (noise)
    {
        originaldatafile << " and noise\n";
    }
    else
    {
        originaldatafile << " and without noise\n";
    }
    originaldatafile << "Begin time\n";
    originaldatafile << instant1  << "\n";
    originaldatafile << "y\tx\n";
    originaldatafile << y << "\t" << x << "\n";
    originaldatafile << "m\tq\tNoise points\n";

    float yLine = 0;        //y value of hit
    std::vector<dataType> values;   //vector to store all the data
    std::vector<int> temp;          //Temporary vector to deterim date - noise order
    std::vector<float> yvalue;      //vector to store all yLine
    int real = 0;                   //int to take into account number of real data obtained
    std::chrono::high_resolution_clock::time_point time1 = std::chrono::high_resolution_clock::now(); //Determines the moment from which is calculated time passed from beginning of simulation for hit points

    for (int i = 0; i < num; i++)
    {    
        real = 0;
        values.clear();
        yvalue.clear();
        temp.clear();

        mq[0] = randomFloat(m2,m1);
        mq[1] = qLine(y,x,mq[0]);

        originaldatafile << mq[0] << "\t" << mq[1];
        //std::cout << "y = " << mq[0] << "x + " << mq[1] << std::endl;
        
        for (int j = 0; j < rivelatore.m_plate; j++)
        {
            yLine = mq[0]*(-(j*rivelatore.m_distance)) + mq[1];             //Calculate intersection between generated trace with x=0,1,2,.....
            if (yLine < rivelatore.m_lenght && yLine > 0)       //Determines if the particles hits a plate
            {
                yvalue.push_back(yLine);
                temp.push_back(rivelatore.m_plate);
            }

            if(noise)
            {
                for (int n = 0; n < poisson(rivelatore.m_errorMean); n++)
                    temp.push_back(j);
            }
        }

        int64_t triggertime = duration(time1);
        std::random_shuffle(temp.begin(), temp.end());
        for (int n = 0; n < int(temp.size()); n++)
        {
            if(temp.at(n) == rivelatore.m_plate)
            {
                values.push_back(dataType(duration(time1), real, pixel(rivelatore, yvalue.at(real))));
                real++; 
            }
            else
            {
                int value = randomInt(0,rivelatore.m_number);
                values.push_back(dataType(duration(time1), temp.at(n), value));
                originaldatafile << "\t( " << temp.at(n) << " , " << value << ")";   
            }   
        }
        originaldatafile << "\n";

        writeData(datafile, headerType(triggertime, track, int(temp.size())), values);

        track ++;

        if(killed)
        {
            datafile.close();
            originaldatafile.close();
            exit(666);
        }
    }
    originaldatafile << "Number of m and q generated\n";
    originaldatafile << track << "\n";

    //Writing on terminal the condition of the end of the simulation
    std::cout << "Number of tracks simulated: " << track << "\n";
    std::cout << "Time needed for the simulation: " << duration(time1) << " ns\n";
    std::cout << "End of point simulation\n" << std::endl;

    datafile.close();
    originaldatafile.close();
}

