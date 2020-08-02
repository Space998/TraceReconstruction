#include "WriteRead.h"
#include "DataType.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <typeinfo>

int checkWriteFile(std::string &filename, std::string &file2)
{
    int take = 1;
    //So che è un brutto nest di if, se riesco a trovare un modo migliore lo sostituirò, al momento non sono ancora riuscito a trovare un modo migliore per farlo
    if (filename == "auto")         //Check if user defined a specific file for the output, if passed auto an automatic file is created in teh directory Simulation/
    {
        if(!std::filesystem::is_directory("Simulation/")) //Check if directory exist, if not creates it
            std::filesystem::create_directory("./Simulation/");
        take += howMany();         //Checks how many "Simulation*.bin" are alreaady in the directory
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
        std::string directory = file.at(0) + std::string("/");
        if(!std::filesystem::is_directory(directory)) //Check if directory where the file shoud be created exist, if not creates it
            std::filesystem::create_directory(directory);
        std::cout << file.at(0);
        file2 = directory + std::string("Original_") + file.at(1) + std::string(".txt");    
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
    }
    return take;
}

std::string existanceReadFile(std::string namefile)
{
    if(!std::filesystem::is_directory("Simulation/")) //Check if directory exist
    {
        std::cerr << "No file found to read, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    if(std::filesystem::is_empty("Simulation/"))     //Check if directory is empty
    {
        std::cerr << "No file found to read, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    if(namefile == "auto")     //If passed "auto" reads the last Simulation-"x".bin created
    {
        int count = 0;
        for (auto& p : std::filesystem::directory_iterator("Simulation/")) 
        {
            size_t found;
            size_t div;
            size_t point;
            found = p.path().string().find_last_of("/\\");
            //std::cout << typeid(p.path()).name() << std::endl;
            if (p.path().string().substr(found+1).find("Simulation") != std::string::npos)
                {
                div = p.path().string().substr(found+1).find_first_of("_");
                point = p.path().string().substr(found+1).substr(div+1).find_first_of(".");
                //std::cout << p.path().string().substr(found+1).substr(div+1).substr(0,point) << std::endl;
                if(std::stoi(p.path().string().substr(found+1).substr(div+1).substr(0,point)) > count)
                    count = std::stoi(p.path().string().substr(found+1).substr(div+1).substr(0,point));
                }
        }
        namefile = std::string("Simulation/Simulation_") + std::to_string(count) + std::string(".bin");
        return namefile;
    }
    else if (!std::filesystem::exists(namefile))
    {
        std::cerr << "No file found to read, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    else //If no problem return the original name passed
    {
        return namefile;
    }
    
}

void readFile(std::string namefile)
{
    namefile = existanceReadFile(namefile);

    std::vector<int> xValue;        //Vector to store all the x values of an event aka the affected pixel 
    std::vector<int> yValue;        //Vector to store all the y values of an event

    //std::cout << "File size: " << int(std::filesystem::file_size(namefile)) << std::endl;
    //int num = 0;
    std::ifstream in(namefile, std::ios::binary);
    while(!in.eof())       //Temporary
    {
        unsigned int w;
        in.read((char*) &w, 4);
        if (w == 0x4EADE500)
        {   
            xValue.clear();
            yValue.clear();
            int eventCount;     
            int eventNum;
            in.read((char*) &eventCount, 4);
            in.read((char*) &eventNum, 4);
            std::cout << "Event number: " << eventNum << "\n";
            std::cout << "Point number: " << eventCount << "\n";
            for (int i = 0; i < eventCount; i++)
            {
                in.read((char*) &w, 4);
                if (w == 0XDADADADA)
                {
                	int time;
	                int plate; 
	                int value;
                    in.read((char*) &time, 4);
                    in.read((char*) &plate, 4);
                    in.read((char*) &value, 4);

                    xValue.push_back(value);
                    yValue.push_back(plate);
                }
            }
            for (int i = 0; i < int(xValue.size()); i++)
            {
                std::cout << "( " << yValue.at(i) << " , " << xValue.at(i) << " )\n";
            }

            /*
            std::cout << "0x" << std::hex << w << " : " << std::dec << structLenght[w]/4 << std::endl;
            //num += structLenght[w];
            in.read( (char*) &w, 4);
            std::cout << w << "\n";
            */
        }
        /*
        else if (structLenght.contains(w))
        {
            std::cout << "0x" << std::hex << w << " : " << std::dec << structLenght[w]/4 << std::endl;
            //num += structLenght[w];
        }
        */
    }
    //std::cout << num << std::endl;
}