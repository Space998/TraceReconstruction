#include "DataType.h"
#include "WriteRead.h"
#include "Simulazione.h"
#include <iostream>
#include <fstream>
#include <map>

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

//Method of Simulate class
int SimulatePoint(Rivelatore rivelatore, int num, const float y, const float x, const bool limit, const bool noise)
{
    //std::cout << "Rivelazione di " << m_rivelatore.m_plate << " punti" << std::endl;
    //std::cout << x << y << std::endl;

    float mq[2] = {0,0};        //{m,q}

    float mq1[2] = {0,0};       //{m1,q1}
    float mq2[2] = {0,0};       //{m2,q2}    

    bool type = 0;
    int point = 0;                                                  //Also the number of "data" (in the sense of detector takes) starts counting from 1
    int take = 42;                                                  //Need to write program to determine take -- need to add control (no biger than 17 bit)

    auto file1 = "Simulazione.bin";                                 //Binary file to store all the data, "official file"
    std::ofstream datafile(file1, std::ios::binary);    

    auto file2 = "Original.txt";                                    //File to contain the generated value of m and q for comparison
    std::ofstream originaldatafile(file2);                 


    //Writing on terminal the conditin of the simulation
    std::cout << "Starting point simulation\n";
    std::cout << "Take number: " << take << "\n";
    std::cout << "Point to simulate: " << num << "\n";
    std::cout << "Output of generated numbers: " << file2 << "\n";
    std::cout << "Output of data: " << file1 << "\n";

    auto instant1 = time();         //Determines the time when the simulation begins
    fileHeader head(0,take,int64_t(instant1));

    if (limit)
    {
        mq1[0] = mLine(0,0,y,x);
        mq2[0] = mLine(1,0,y,x);
        head.type = 0xBBBB0000;
    }
    else
    {
        mBorders(y,x,mq1,mq2);
        head.type = 0xAAAA0000;
    }
    
    write(datafile, head);  //Writing the header of the file for the simulation in the Simulation.bin file

    //print_beginType(datafile, type, uint64_t(instant1), take);
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
    originaldatafile << instant1;
    originaldatafile << "y\tx\n";
    originaldatafile << y << "\t" << x << "\n";
    originaldatafile << "m\tq\n";

    for (int i = 0; i < num; i++)
    {    
        point ++;

        int plate = 0;
        float yLine = 0;
        std::map<int, int> values;

        mq[0] = RandomFloat(mq1[0],mq2[0]);
        mq[1] = y - mq[0]*x;
        originaldatafile << mq[0] << "\t" << mq[1] << "\n";
        //std::cout << "y = " << mq[0] << "x + " << mq[1] << std::endl;

        for (int j = 0; j < 3; j++)
        {
            yLine = mq[0]*(-j) + mq[1];
            //std::cout << yLine << " : " << -j << std::endl;
            if (!(yLine > 1 || yLine < 0)) 
            {                      
                values[j] = pixel(yLine);  
                plate ++;
            }
        }

        write(datafile, headerType(plate, point));
        for(auto const& [key, val] : values)  //Cicles over the elements of the map to print the values detected with the corresponding detector plate
        {
            write(datafile, dataType(key,val));
        }
        
    }

    if (noise)
    {
        std::cout << "Noise not impeltemted" << std::endl;
    }

    auto instant2 = time();
    write(datafile, fileEnd(int64_t(instant2)));

    originaldatafile << "Number of m and q generated\n";
    originaldatafile << point << "\n";
    originaldatafile << "End time\n";
    originaldatafile << instant2;
    
    //Writing on terminal the condition of the end of the simulation
    if(num == point)
    {
        std::cout << "Number of point simulated: " << point << "\n";
        std::cout << "Time needed for the simulation: " << instant2 - instant1 << "\n";
        std::cout << "End of point simulation\n" << std::endl;
    }
    else
    {
        std::cerr << "The number of generated point is different from the number passed by the user";
        std::cout << "Time needed for the simulation: " << instant2 - instant1 << "\n";
        std::cout << "End of point simulation\n" << std::endl;
        return 2;
    }

    datafile.close();
    originaldatafile.close();
}

int SimulateLine(Rivelatore rivelatore, int num)
{
    std::cout << "Rivelazione di " << rivelatore.m_plate << " punti su linea" << std::endl;
}
