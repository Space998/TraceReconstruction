#include "DataType.h"
#include "WriteRead.h"
#include "Simulazione.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

//Function for the evaluation of m and q end similar
int mBorders(float y, float x, float (&array1)[2], float (&array2)[2]) 
{
    float m1_1 = mLine(0,0,y,x);
    float m1_2 = mLine(0,-2,y,x);
    float m2_1 = mLine(1,0,y,x);
    float m2_2 = mLine(1,-2,y,x);
    //std::cout << m1_2 << ", " << m2_1 << ", " << m1_1 << ", " << m2_2 << ", " << std::endl;

    if (m2_1 > m1_2 || m2_2 > m1_1)
    {
        std::cerr << "ERROR: Impossible to guarantee three hits" << std::endl;
        exit(1);
    }
    else if (m1_1 > m1_2 && m2_2 > m2_1)
    {
        array1[0] = m1_2;
        array2[0] = m2_2;
        return 0;
    }
    else if (m1_1 > m1_2 && m2_1 > m2_2)
    {
        array1[0] = m1_2;
        array2[0] = m2_1;
        return 0;            
    }
    else if (m1_2 > m1_1 && m2_2 > m2_1)
    {
        array1[0] = m2_2;
        array2[0] = m1_2;
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

    float mq1[2] = {0,0};       //{m1,q1}
    float mq2[2] = {0,0};       //{m2,q2}    

    int point = 0;      //Number of event generated, starts counting from 1
    std::string originalFile; //File to contain the original data genereted by the algorithm
    int take = checkWriteFile(filename, originalFile);  

    std::ofstream datafile(filename, std::ios::binary);             //Opens binary file to store all the data, "official file"  
    std::ofstream originaldatafile(originalFile);                          //Opens the file to store all the generated m and q values 

    //Writing on terminal the conditin of the simulation
    std::cout << "Starting point simulation\n";
    std::cout << "Take number: " << take << "\n";       //In case of user defined file the take number will always be one
    std::cout << "Point to simulate: " << num << "\n";
    std::cout << "Output of generated numbers: " << originalFile << "\n";
    std::cout << "Output of data: " << filename << "\n";

    auto instant1 = time();         //Determines the time when the simulation begins
    std::chrono::high_resolution_clock::time_point time1 = std::chrono::high_resolution_clock::now();
    fileHeader head(0,take,int64_t(reinterpret_cast<char*>(&instant1)));

    if (limit)
    {
        mBorders(y,x,mq1,mq2);
        head.type = 0xBBBB0000;
    }
    else
    {
        mq1[0] = mLine(0,0,y,x);
        mq2[0] = mLine(1,0,y,x) ;  
        head.type = 0xAAAA0000;
    }
    
    write(datafile, head);  //Writing the header of the file for the simulation in the Simulation.bin file

    originaldatafile << "Point to generaate\n";
    originaldatafile << num << "\n";
    originaldatafile << "Take number\n";
    originaldatafile << take << "\n";
    originaldatafile << "Type of simulatio\n";
    originaldatafile << "Point simulation ";
    if (limit) 
    {
        originaldatafile << "with limits\n";
    }
    else
    {
        originaldatafile << "without limits\n";
    }
    originaldatafile << "Begin time\n";
    originaldatafile << instant1  << "\n";
    originaldatafile << "y\tx\n";
    originaldatafile << y << "\t" << x << "\n";
    originaldatafile << "m\tq\n";

    for (int i = 0; i < num; i++)
    {    
        point ++;

        int plate = 0;
        float yLine = 0;
        std::vector<dataType> values;

        mq[0] = RandomFloat(mq1[0],mq2[0]);
        mq[1] = y - mq[0]*x;
        originaldatafile << mq[0] << "\t" << mq[1] << "\n";
        //std::cout << "y = " << mq[0] << "x + " << mq[1] << std::endl;

        for (int j = 0; j < 3; j++)
        {
            yLine = mq[0]*(-j) + mq[1];             //Calculate intersection between generated trace with x=0,1,2
            //std::cout << yLine << " : " << -j << std::endl;
            if (!(yLine > rivelatore.m_lenght || yLine < 0)) 
            {          
                //std::cout << yLine << " : " << pixel(rivelatore, yLine) << "\n";            
                values.push_back(dataType(duration(time1), j, pixel(rivelatore, yLine)));           //Calculate the pixel that got hit
                plate ++;
            }
        }

        write(datafile, headerType(plate, point));
        for(auto const& el: values) 
        {
            write(datafile, el);
        }
        /*
        for(auto const& [key, val] : values)  //Cicles over the elements of the map to print the values detected with the corresponding detector plate
        {
            write(datafile, dataType(key,val));
        }
        */
    }

    if (noise)
    {
        std::cout << " -- Noise not implemented --" << std::endl;
    }

    originaldatafile << "Number of m and q generated\n";
    originaldatafile << point << "\n";

    //Writing on terminal the condition of the end of the simulation
    std::cout << "Number of point simulated: " << point << "\n";
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
