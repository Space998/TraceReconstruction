#include "DataType.h"
#include "WriteRead.h"
#include "Simulazione.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "Hough.h"

//Random generators
std::random_device rd;
std::mt19937 gen(rd());

//Poisson
int poisson(Rivelatore &rivelatore)
{
    std::poisson_distribution<int> pois(rivelatore.m_errorMean);
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
    //return  (max - min) * ((((float) rand()) / (float) RAND_MAX)) + min ;
}

//Function for the evaluation of m and q end similar
int mBorders(Rivelatore &rivelatore, float y, float x, float &m1, float &m2) 
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
        return 0;
    }
    else if (m1_1 > m1_2 && m2_1 > m2_2)
    {
        m1 = m1_2;
        m2 = m2_1;
        return 0;            
    }
    else if (m1_2 > m1_1 && m2_2 > m2_1)
    {
        m1 = m2_2;
        m2 = m1_2;
        return 0;
    }

    return 0;
}

//Method of Simulate class
int SimulatePoint(std::string filename, Rivelatore rivelatore, int num, const float y, const float x, const bool limit, const bool noise)
{
    //std::cout << "Rivelazione di " << m_rivelatore.m_plate << " punti" << std::endl;
    //std::cout << x << y << std::endl;

    float mq[2] = {0,0};        //{m,q} of the generated trace

    float m1 = 0;       //{m1}   maximum and minumum  values of mq for the generation (in this way only track that intersect the detector are generated)
    float m2 = 0;       //{m2}    

    int track = 0;              //Number of event generated, starts counting from 1
    std::string originalFile;   //File to contain the original data genereted by the algorithm
    int take = checkWriteFile(filename, originalFile);  

    std::ofstream datafile(filename, std::ios::binary);             //Opens binary file to store all the data, "official file"  
    std::ofstream originaldatafile(originalFile);                   //Opens the file to store all the generated m and q values 

    //Writing on terminal the conditin of the simulation
    std::cout << "Starting point simulation\n";
    std::cout << "Take number: " << take << "\n";       //In case of user defined file the take number will always be one
    std::cout << "Tracks to simulate: " << num << "\n";
    std::cout << "Output of generated numbers: " << originalFile << "\n";
    std::cout << "Output of data: " << filename << "\n";

    auto instant1 = time();         //Determines the time when the simulation begins
    std::chrono::high_resolution_clock::time_point time1 = std::chrono::high_resolution_clock::now();

    if (limit)
    {
        mBorders(rivelatore,y,x,m1,m2);
        //head.type = 0xBBBB0000;
    }
    else
    {
        m1 = mLine(0,0,y,x);
        m2 = mLine(rivelatore.m_lenght,0,y,x);  
        //head.type = 0xAAAA0000;
    }
    
    write(datafile, fileHeader(rivelatore, take, int64_t(reinterpret_cast<char*>(&instant1))));  //Writing the header of the file for the simulation in the Simulation.bin file

    originaldatafile << "Point to generaate\n";
    originaldatafile << num << "\n";
    originaldatafile << "Take number\n";
    originaldatafile << take << "\n";
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
        originaldatafile << " without noise\n";
    }
    originaldatafile << "Begin time\n";
    originaldatafile << instant1  << "\n";
    originaldatafile << "y\tx\n";
    originaldatafile << y << "\t" << x << "\n";
    originaldatafile << "m\tq\tNoise points\n";

    //int hit = 0;
    float yLine = 0;
    std::vector<dataType> values;
    std::vector<int> temp;
    std::vector<float> yvalue;

    //int point = 0;
    int real = 0;

    for (int i = 0; i < num; i++)
    {    
        //hit = 0;
        real = 0;
        values.clear();
        yvalue.clear();
        temp.clear();

        mq[0] = randomFloat(m1,m2);
        mq[1] = qLine(y,x,mq[0]);

        originaldatafile << mq[0] << "\t" << mq[1];
        //std::cout << "y = " << mq[0] << "x + " << mq[1] << std::endl;
        
        for (int j = 0; j < rivelatore.m_plate; j++)
        {
            yLine = mq[0]*(-(j*rivelatore.m_distance)) + mq[1];             //Calculate intersection between generated trace with x=0,1,2,.....
            if (yLine < rivelatore.m_lenght && yLine > 0)
            {
                yvalue.push_back(yLine);
                temp.push_back(rivelatore.m_plate);
            }

            if(noise)
            {
                for (int n = 0; n < poisson(rivelatore); n++)
                    temp.push_back(j);
            }
            //std::cout << yLine << " : " << -j << std::endl;


/*
            point = 0;
            real = 0;

            if (noise)
                point += poisson(rivelatore);
             
            if (yLine < rivelatore.m_lenght && yLine > 0)     
            {
                point++;
                real = randomInt(0,point-1);
            }

            for (int n = 0; n <= point; n++) //This was made to mix the real data between the noise, in this way all the noise points aren't generated after the real point makin that using the time stamp was possible to go back to the real point (first in the list from a temporal prospective)
            {
                if (n == real)
                    values.push_back(dataType(duration(time1), j, pixel(rivelatore, yLine)));
                else
                {
                    int value = randomInt(0,rivelatore.m_number);
                    values.push_back(dataType(duration(time1), j, value));
                    originaldatafile << "\t( " << j << " , " << value << ")";
                }      
            }
            hit += point; 
*/
            /*
            if (yLine < rivelatore.m_lenght && yLine > 0) 
            {          
                //std::cout << yLine << " : " << pixel(rivelatore, yLine) << "\n";            
                values.push_back(dataType(duration(time1), j, pixel(rivelatore, yLine)));           //Calculate the pixel that got hit
                hit ++;
            }
            if(noise)
            {
                for (int n = 0; n < poisson(); n++)
                {
                    //std::cout << "noise" << std::endl;
                    values.push_back(dataType(duration(time1), j, randomInt(0,rivelatore.m_number)));       //Value of the noise is the generated with a uniform distribution over the plate lenght
                    hit++;
                }
            }
            */
        }
        
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

        writeData(datafile, headerType(track, int(temp.size())), values);

        /*
        write(datafile, headerType(hit, track));
        for(auto const& el: values) 
        {
            write(datafile, el);
        }
        */

        /*
        for(auto const& [key, val] : values)  //Cicles over the elements of the map to print the values detected with the corresponding detector plate
        {
            write(datafile, dataType(key,val));
        }
        */

        track ++;
    }
    originaldatafile << "Number of m and q generated\n";
    originaldatafile << track << "\n";

    //Writing on terminal the condition of the end of the simulation
    std::cout << "Number of tracks simulated: " << track << "\n";
    std::cout << "Time needed for the simulation: " << duration(time1) << " ns\n";
    std::cout << "End of point simulation\n" << std::endl;

    datafile.close();
    originaldatafile.close();
    
    return 0;
}

/*
int SimulateLine(std::string filename, Rivelatore rivelatore, int num)
{
    std::cout << "Rivelazione di " << rivelatore.m_plate << " punti su linea" << std::endl;
    return 0;
}
*/
