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

    int point = 0;                                                  //Number of event generated, starts counting from 1
    int take = 1;                                                   

    std::string file2;   //File to contain data in binary form    

    //So che è un brutto nest di if, se riesco a trovare un modo migliore lo sostituirò, al momento non sono ancora riuscito a trovare un modo migliore per farlo
    if (filename == "auto")         //Check if user defined a specific file for the output, if passed auto an automatic file is created in teh directory Simulation/
    {
        if(!std::filesystem::is_directory("Simulation/")) //Check if directory exist, if not creates it
            std::filesystem::create_directory("./Simulation/");
        int hm = howMany();         //Checks how many "Simulation*.bin" are alreaady in the directory
        take += hm;
        std::string take_str = std::to_string(take);       
        filename = std::string("Simulation/Simulation_") + take_str + std::string(".bin");    //File to contain data in binary form
        file2 = std::string("Simulation/Original_") + take_str + std::string(".txt");         
        while (std::filesystem::exists(filename) || std::filesystem::exists(file2))     //If a SImulation"x".bin or Orginal"x".txt already exist checks recursively for x++
        {
            take++;
            take_str = std::to_string(take);       
            filename = std::string("Simulation/Simulation_") + take_str + std::string(".bin");
            file2 = std::string("Simulation/Original_") + take_str + std::string(".txt");
        }
    }
    else
    {
        std::vector<std::string> file = SplitFilename(filename);
        file2 = file.at(0) + std::string("/") + std::string("Original_") + file.at(1) + std::string(".txt");    
        if (std::filesystem::exists(filename) || std::filesystem::exists(file2))        //Checks if the file passed by the user alredy exists
        {
            std::cout << "File passed to the simulation or the Original_filename.txt alredy exists, overwrite? [y/n]" << std::endl;
            std::string response;
            std::cin >> response;
            if (response == std::string("n"))
            {
                std::cerr << "Ok, terminating program" << std::endl;
                exit(2);
            }
            else if (response != std::string("y"))
            {
                std::cerr << "Unexpeted user response" << std::endl;
                exit(3);
            } 
        }
        else if (!std::filesystem::is_directory(file.at(0)))
        {
            std::cerr << "Can't find the directory of the passed file " << std::endl;
            exit(4);
        }   
    }
    
    std::ofstream datafile(filename, std::ios::binary);             //Opens binary file to store all the data, "official file"  
    std::ofstream originaldatafile(file2);                          //Opens the file to store all the generated m and q values


    //Writing on terminal the conditin of the simulation
    std::cout << "Starting point simulation\n";
    std::cout << "Take number: " << take << "\n";
    std::cout << "Point to simulate: " << num << "\n";
    std::cout << "Output of generated numbers: " << file2 << "\n";
    std::cout << "Output of data: " << filename << "\n";

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
            yLine = mq[0]*(-j) + mq[1];             //Calculate intersection between generated trace with x=0,1,2
            //std::cout << yLine << " : " << -j << std::endl;
            if (!(yLine > rivelatore.m_lenght || yLine < 0)) 
            {          
                //std::cout << yLine << " : " << pixel(rivelatore, yLine) << "\n";            
                values[j] = pixel(rivelatore, yLine);           //Calculate the pixel that got hit
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
    std::cout << "Number of point simulated: " << point << "\n";
    std::cout << "Time needed for the simulation: " << instant2 - instant1 << "\n";
    std::cout << "End of point simulation\n" << std::endl;

    datafile.close();
    originaldatafile.close();
    
    return 0;
}

int SimulateLine(std::string filename, Rivelatore rivelatore, int num)
{
    std::cout << "Rivelazione di " << rivelatore.m_plate << " punti su linea" << std::endl;
    return 0;
}
