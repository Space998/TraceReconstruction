#include "WriteRead.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <filesystem>

unsigned int checkWriteFile(std::string &filename, std::string &file2)
{
    int take = 1;
    if (filename == "auto")         //Check if user defined a specific file for the output, if passed auto an automatic file is created in teh directory Simulation/
    {
        if(!std::filesystem::is_directory("Simulation/")) //Check if directory exist, if not creates it
        {
            std::filesystem::create_directory("./Simulation/");
        }
        else
        {
            take += howMany();         //Checks how many "Simulation*.bin" are alreaady in the directory            
        }
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

std::string existanceFile(std::string namefile, std::string type)
{
    if(!(type == "Simulation" || type == "Original" || type == "Analysis"))
    {
        std::cerr << "In function existanceFile the specified type is not it is not among the types provided\n";
        exit(6);
    }
    
    if(namefile == "auto")     //If passed "auto" reads the last Simulation-"x".bin created
    {
        if(!std::filesystem::is_directory("Simulation/") || std::filesystem::is_empty("Simulation/")) //Check if directory exist or if it is empty
        {
            std::cerr << "No file found to read, simulation directory doesn't exits or is empty" << std::endl;
            exit(4);
        }

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

        if(type == "Simulation")
        {    
            namefile = std::string("Simulation/Simulation_") + std::to_string(count) + std::string(".bin");
        }
        else if(type == "Original")
        {
            namefile = std::string("Simulation/Original_") + std::to_string(count) + std::string(".txt");
        }
        else if(type == "Analysis")
        {
            namefile = std::string("Simulation/Analysis_") + std::to_string(count) + std::string(".bin");
        }
    }
    
    if(!std::filesystem::exists(namefile))
    {
        std::cerr << "No file found to analize, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    
    bool find = false;
    unsigned int w;

    if(type == "Simulation")
    {    
        std::ifstream in(namefile, std::ios::binary);
        while(!in.eof() && !find)
        {
            in.read((char*) &w, 4);
            if (w == 0xF0CAFACE)
                find = true;
        }

        in.close();

        if(!find)
        {
            std::cout << "Even though the file passed is indicated as a Simulation file inside of it doesn't apper the corresponding keyword\n";
            exit(6);
        }
    }
    else if(type == "Original")
    {
        FILE* fp = fopen(namefile.c_str(), "r");
        if (fp == NULL)
            exit(EXIT_FAILURE);

        char* line = NULL;
        size_t len = 0;
        while ((getline(&line, &len, fp)) != -1 && !find) 
        {    
            if(strcmp(line, "Original data file\n") == 0)
                find = true;
        }
        fclose(fp);
        if (line)
            free(line);
        
        if(!find)
        {
            std::cout << "Even though the file passed is indicated as a Simulation file inside of it doesn't apper the corresponding keyword\n";
            exit(6);
        }
    }
    else if(type == "Analysis")
    {
        std::ifstream in(namefile, std::ios::binary);
        while(!in.eof() && !find)
        {
            in.read((char*) &w, 4);
            if (w == 0xF0CADEAD)
                find = true;
        }

        in.close();

        if(!find)
        {
            std::cout << "Even though the file passed is indicated as a Analysis file inside of it doesn't apper the corresponding keyword\n";
            exit(6);
        }
    }
    
    return namefile;
}