#include "WriteRead.h"
#include "DataType.h"
#include "Rivelatore.h"
#include "Simulazione.h"
#include "Hough.h"
#include "HoughFunctions.h"

#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <typeinfo>
#include <cmath>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>

//Inclusions for ROOT
#include <TROOT.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TView.h>
#include <TGraph.h>				//Classe TGraph
#include <TGraphErrors.h>		//Classe TGraphErrors
#include <TAxis.h>
#include <TImage.h>				//Libreria per la gestione dell'output come immagine
#include <TLine.h>
#include <TH1.h>
#include <TH2.h>
#include <TStyle.h>
#include <TExec.h>

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
        //std::cout << file.at(0);
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
        else
        {
            std::cout << "In function existanceFile the specified type is not it is not among the types provided" << std::endl;
            exit(3);
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
    else
    {
        std::cout << "In function existanceFile the specified type is not it is not among the types provided" << std::endl;
        exit(6);
    }
    
    return namefile;
}

/*
std::string existanceFile(std::string namefile, std::string type)
{
    if(namefile == "auto")     //If passed "auto" reads the last Simulation-"x".bin created
    {
        if(!std::filesystem::is_directory("Simulation/") || std::filesystem::is_empty("Simulation/")) //Check if directory exist or if it is empty
        {
            std::cerr << "No file found to read, make a simulation or check the file name (Simulation direcctory doesn't exits or is empty" << std::endl;
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
        namefile = std::string("Simulation/Simulation_") + std::to_string(count) + std::string(".bin");
        return namefile;
    }
    else if(!std::filesystem::exists(namefile))
    {
        std::cerr << "No file found to analize, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    else //If no problem return the original name passed
    {
        return namefile;
    }
    
}
*/

void checkCorrectness(std::string original, std::string analysis, std::vector<int64_t> times, const bool interactiveImg)
{   
    std::cout << "Initialisyng evalution of algorithm performance\n";

    original = existanceFile(original, "Original");
    analysis = existanceFile(analysis, "Analysis");

    //std::cout << original << std::endl;
    //std::cout << analysis << std::endl;

    int events = 0;
    std::vector<std::string> lines;

    //Reads origin file  
    FILE* fp = fopen(original.c_str(), "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    char* line = NULL;
    size_t len = 0;
    while ((getline(&line, &len, fp)) != -1) 
    {    
        if(strcmp(line, "Point to generate\n") == 0)
        {              
            getline(&line, &len, fp);
            events = atoi(line);
        }
        if(strcmp(line, "m	q	Noise points\n") == 0)
        {
            for(int i = 0; i < events; i++)
            {
                getline(&line, &len, fp);
                //std::cout << line << "\n";
                lines.push_back(line);
            }
        }
    }
    fclose(fp);
    if (line)
        free(line);
/*
    for(int i = 0; i < lines.size(); i++)
    {std::cout << "- " << lines.at(i) << std::endl;}
*/

    //Definid variable to read take characteristics
    Rivelatore detector;
    int take;

    //Read analysis file
    unsigned int w;

    //Header elements
    int eventAnalysed;
    int tracks;

    //Data elements
    int eventNum;
    float m;
    float q;

    //vector for m and q value difference
    std::vector<float> mVec;
    std::vector<float> qVec;

    std::ifstream in(analysis, std::ios::binary);

    while(!in.eof())
    {
        in.read((char*) &w, 4);
        
        if(w == 0xF0CADEAD)
        {
            in.read(reinterpret_cast<char *>(&detector), sizeof(Rivelatore));
            in.read((char*) &take, 4);
            std::cout << "Take number: " << take << "\n";
        }

        if (w == 0x0074EADE)
        {   
            in.read((char*) &eventAnalysed, 4);
            in.read((char*) &tracks, 4);
            int i = 0;
            int j = 0;
            while (i < tracks && j < 10) //Substantially if the program doen't find a new expected value goes on
            {
                j++;

                in.read((char*) &w, 4);
                if (w == 0x007DADAD)
                {
                    in.read((char*) &eventNum, 4);
                    in.read((char*) &m, 4);
                    in.read((char*) &q, 4);

                    i++;
                    j = 0;

                    std::stringstream ss(lines.at(eventNum));

                    float temp;
                    ss >> temp;
                    //std::cout << m << " , " << temp << "\n";
                    mVec.push_back(absFloat(m-temp));
                    ss >> temp;
                    //std::cout << q << " , " << temp << "\n";
                    qVec.push_back(absFloat(q-temp));
                } 

                if (j == 10)
                {
                    std::cout << "Problem with take: " << take << ", found only " << i << " values of the expected " << tracks << " analysed events.\n";
                    std::cout << "Would you like to continue the analysis? (y/n)\n";
                    
                    std::string response;
                    std::cin >> response;
                    if (response == std::string("n"))
                    {
                        std::cerr << "Ok, terminating program" << std::endl;
                        exit(5);
                    }
                    else if (response != std::string("y"))
                    {
                        std::cerr << "Unexpeted user response" << std::endl;
                        exit(3);
                    } 
                } 
            }  

            //for (int i = 0; i < mVec.size(); i++)
            //    std::cout << "- " << mVec.at(i) << " , " << qVec.at(i) << std::endl;

            std::sort(mVec.begin(), mVec.end());
            std::sort(qVec.begin(), qVec.end());

            //for (int i = 0; i < mVec.size(); i++)
            //    std::cout << "-- " << mVec.at(i) << " , " << qVec.at(i) << std::endl;

            int nbins = int(((mVec.at(int(mVec.size())-1))/0.0001)+0.5);
            float xMax = nbins*0.0001;

            //std::cout << nbins << std::endl;
            //std::cout << xMax << std::endl;

            std::string imgDirectory = SplitFilename(analysis).at(0) + std::string("/") + std::string("fitImages_") + std::to_string(take); 

            if(std::filesystem::is_directory(imgDirectory) && !std::filesystem::is_empty("Simulation/") ) //Check if directory where the image should be saved exists
            {
                std::cout << "Problem with the directory to store analysis images relative to take: " << take << ", it already exists and it is not empty.\n";
                std::cout << "Would you like to continue and overwrite the files? (y/n)\n";
                
                std::string response;
                std::cin >> response;
                if (response == std::string("n"))
                {
                    std::cerr << "Ok, terminating program" << std::endl;
                    exit(5);
                }
                else if (response != std::string("y"))
                {
                    std::cerr << "Unexpeted user response" << std::endl;
                    exit(3);
                }    
            }
            std::filesystem::create_directory(imgDirectory);

            // Creazione dello screen
            TCanvas *c2 = new TCanvas("c2","",0,0,800,600); 
            //Img output
            TImage *img = TImage::Create();
        
            // Creazione dello histo 
            TH1F *histo1 = new TH1F("m Histo","m error",nbins,0,xMax);
            histo1->GetXaxis()->SetTitle("Difference (m)");
            histo1->GetXaxis()->CenterTitle();

            for (int i = 0; i < int(mVec.size()); i++)
            {
                histo1->Fill(mVec.at(i));
            }

            histo1->SetFillColor(42);  // Colore dell'istogramma
            histo1->SetMinimum(0);     // Minimo per il Draw
            histo1->Draw();
            
            //Output immagine histo
            img->FromPad(c2);
            std::string imgOut = imgDirectory + std::string("/mAnalysis.png");
            //std::cout << imOut << std::endl;
            img->WriteImage(imgOut.c_str());

            if(interactiveImg)
            {
                usleep(1e6);
            }


            std::cout << "Histogram for m values difference got " << histo1->GetEntries() << " entries, mean value: " << histo1->GetMean() << " +/- " << histo1->GetMeanError() << "\n";

            c2->Clear();

            nbins = int(((qVec.at(int(mVec.size())-1))/0.0001)+0.5);
            xMax = nbins*0.0001;

            TH1F *histo2 = new TH1F("q Histo","q error",nbins,0,xMax);
            histo2->GetXaxis()->SetTitle("Difference (m)");
            histo2->GetXaxis()->CenterTitle();

            for (int i = 0; i < int(qVec.size()); i++)
            {
                histo2->Fill(qVec.at(i));
            }

            histo2->SetFillColor(42);  // Colore dell'istogramma
            histo2->SetMinimum(0);     // Minimo per il Draw
            histo2->Draw();
            
            if(interactiveImg)
            {
                c2->Modified();
                c2->Update();
                usleep(1e6);
            }

            //Output immagine histo
            img->FromPad(c2);
            imgOut = imgDirectory + std::string("/qAnalysis.png");
            //std::cout << imOut << std::endl;
            img->WriteImage(imgOut.c_str());

            std::cout << "Histogram for q values difference got " << histo2->GetEntries() << " entries, mean value: " << histo2->GetMean() << " +/- " << histo2->GetMeanError() << "\n";

            delete histo1;
            delete histo2;
            delete c2;
            delete img;
            
            std::cout << "Output of images (directory): " << imgDirectory << "\n";
            std::cout << "Mean time for Hough trasformation's algorithm execution: " << mean(times) << " ns\n";
            std::cout << "End data analysis evaluation\n\n";
        }
    } 
}

void readFile(std::string namefile, const float rhoPrecision, const float thetaPrecision, const bool terminalOutput, const bool images, const bool interactiveImg, const bool check)
{
    TApplication *myApp;
    if (interactiveImg)
    {
        int argc = 0; 
        char* argv[1];
        //Inizializzo l'interfaccia con root (e' necessario)
        myApp = new TApplication("App", &argc, argv);
    }   

    // Creazione dello screen
    TCanvas *c1 = new TCanvas("c1","",0,0,800,600); 
    //Img output
    TImage *img = TImage::Create();

    namefile = existanceFile(namefile, "Simulation");

    std::vector<float> xValueFloat; //Vector to store all the x values
    std::vector<float> yValueFloat; //Vector to store all the y values

    //Definid variable to read take characteristics
    Rivelatore detector;
    int take;
    int64_t init;

    //Definid variable to read event characteristics
    int pointNum;     
    int eventNum;

    //Defining variable relative to value read
    int64_t time;
    int plate; 
    int value;

    //General variable for reading
    unsigned int w;

    //Vector to store events succefully reconstructed
    std::vector<struct outDataType> mqValues;

    //Creating output file
    std::string directory = SplitFilename(namefile).at(0) + std::string("/");
    std::string outFile;
    std::ofstream datafile;
    
    //Defining string to store directory name of where to save images from ROOT
    std::string imgDirectory = "none";

    //Vector to store executions times
    std::vector<int64_t> times;
    
    std::cout << "Starting data analysis\n";
    std::cout << "Analysing file: " << namefile << "\n";
    
    std::ifstream in(namefile, std::ios::binary);
    while(!in.eof())       //Temporary
    {
        in.read((char*) &w, 4);
        if (w == 0xF0CAFACE)
        {
            in.read(reinterpret_cast<char *>(&detector), sizeof(Rivelatore));
            in.read((char*) &take, 4);
            in.read((char*) &init, 8);

            std::cout << "Take number: " << take << "\n";
            std::cout << "Using detector: " << std::hex << detector.name << "\n";
            std::cout << "\t Plate number: " << detector.m_plate << "\n";
            std::cout << "\t Plate distance: " << detector.m_distance << " m\n";
            std::cout << "\t Pixel number per plate: " << detector.m_number << "\n";
            std::cout << "\t Pixel dimension: " << detector.m_dimension << " m\n\n";
            //std::cout << std::hex << riv.name << " , " << riv.m_plate << std::endl;

            //Defining name of outFile
            outFile = directory + std::string("Analysis_") + std::to_string(take) + std::string(".bin"); 
            if(std::filesystem::exists(outFile))
            {
                std::cout << "Output file to store m and q values (analysed data) realtive to take: " << take << " already exist.\n";
                std::cout << "Would you like to continue and overwrite the files? (y/n)\n";
                std::string response;
                std::cin >> response;
                if (response == std::string("n"))
                {
                    std::cerr << "Ok, terminating program" << std::endl;
                    exit(5);
                }
                else if (response != std::string("y"))
                {
                    std::cerr << "Unexpeted user response" << std::endl;
                    exit(3);
                }    
            }
            datafile = std::ofstream(outFile, std::ios::binary);

            //Creates directory to store the images
            if(images)
            {
                imgDirectory = directory + std::string("images_") + std::to_string(take);
                if(std::filesystem::is_directory(imgDirectory) && !std::filesystem::is_empty("Simulation/") ) //Check if directory where the image should be saved exists
                {
                    std::cout << "Problem with the directory to store images relative to take: " << take << ", it already exists and it is not empty.\n";
                    std::cout << "Would you like to continue and overwrite the files? (y/n)\n";
                    
                    std::string response;
                    std::cin >> response;
                    if (response == std::string("n"))
                    {
                        std::cerr << "Ok, terminating program" << std::endl;
                        exit(5);
                    }
                    else if (response != std::string("y"))
                    {
                        std::cerr << "Unexpeted user response" << std::endl;
                        exit(3);
                    }    
                }
                std::filesystem::create_directory(imgDirectory);
            }
        }
        else if (w == 0x4EADE500)
        {   
            xValueFloat.clear();            //Creation of vector to store x values (correct values not descrete ones)
            yValueFloat.clear();            //Creation of vector to store y values (correct values not descrete ones)
            
            std::vector<std::vector<std::vector<int>>> values(int(180/thetaPrecision)-1); //The position in the first vector identifies the angle, the vector in the third inclusion are [values] and [repetition of this values]

            in.read((char*) &eventNum, 4);
            in.read((char*) &pointNum, 4);
            int i = 0;
            int j = 0;
            while (i < pointNum && j < 10) //Substantially if the program doen't find a new expected value goes on
            {
                j++;

                in.read((char*) &w, 4);
                if (w == 0XDADADADA)
                {
                    in.read((char*) &time, 8);
                    in.read((char*) &plate, 4);
                    in.read((char*) &value, 4);

                    xValueFloat.push_back(xValueCor(detector,plate));
                    yValueFloat.push_back(yValueCor(detector,value));

                    i++;
                    j = 0;
                }
            }
            if (j == 10)
            {
                std::cout << "Problem with event number: " << eventNum << ", found only " << i << " values of the expected " << pointNum << ".\n";
                std::cout << "Would you like to continue the analysis? (y/n)\n";
                
                std::string response;
                std::cin >> response;
                if (response == std::string("n"))
                {
                    std::cerr << "Ok, terminating program" << std::endl;
                    exit(5);
                }
                else if (response != std::string("y"))
                {
                    std::cerr << "Unexpeted user response" << std::endl;
                    exit(3);
                } 
            }   
            std::vector<int> max = {0,0,0,0};

            std::chrono::high_resolution_clock::time_point time1 = std::chrono::high_resolution_clock::now();
            calculateRho(values, max, yValueFloat, xValueFloat, thetaPrecision, rhoPrecision, detector.m_lenght, true);
            if(check)
                times.push_back(duration(time1));

            int maxRho = max.at(3);
            bool fit;
            if (max.at(2) == 1)
            {
                fit = false;
            }
            else
            {
                fit = true;
                mqValues.push_back(outDataType(eventNum, mReconstructed(max.at(0), thetaPrecision),qReconstructed(max.at(0), max.at(1), thetaPrecision, rhoPrecision)));
            }
            
            
            //std::cout << "-- " << maxRho << " , " << maxRho*rhoPrecision << std::endl;
            if (terminalOutput)
            {
                std::cout << "Event number: " << std::dec << eventNum << "\n";
                std::cout << "Point number: " << std::dec << pointNum << "\n";
                for (int i = 0; i < int(xValueFloat.size()); i++)
                {
                    std::cout << "( " << yValueFloat.at(i) << " , " << xValueFloat.at(i) << " ) \n";
                }

                //std::cout << "Angle: " << (float(max.at(0)+1))*thetaPrecision << "\n";
                //std::cout << "Rho: " << max.at(1)*rhoPrecision << "\n";
                std::cout << "Significance: " << max.at(2) << std::endl;
                if (max.at(2) != 1)
                {
                    std::cout << "m: " << mReconstructed(max.at(0), thetaPrecision) 
                                << " q: " << qReconstructed(max.at(0), max.at(1), thetaPrecision, rhoPrecision) 
                                << " ( Angle: " << (float(max.at(0)+1))*thetaPrecision << " , Rho: " << max.at(1)*rhoPrecision << " )\n";
                }
                else
                {
                    std::cout << "Impossible to calculate line equation\n";
                }
                std::cout << "\n";
            }   
            
            if (images || interactiveImg)
            {
                //Error vector
                std::vector<float> yErr;
                yErr.resize(xValueFloat.size());
                std::fill(yErr.begin(), yErr.end(), detector.m_dimension/sqrt(12));     

                c1->Clear();
                
                //Crea grafico con barre di errore
                TGraphErrors *gr = new TGraphErrors(xValueFloat.size(), &(xValueFloat[0]) , &(yValueFloat[0]), 0, &(yErr[0]));
                gr->SetMarkerStyle(20);					// Seleziona il marker rotondo
                gr->SetMarkerSize(1);
                gr->SetTitle("Detector hits");					// Titolo del grafico
                
                gStyle->SetTitleX(0.5f);
                gStyle->SetTitleW(0.8f);
                //gr->CenterTitle();

                gr->Draw("APE");					// Plot del grafico

                //Creo asse X
                TAxis *xaxis = gr->GetXaxis();
                xaxis->SetLimits(-detector.m_width-0.2,0);
                //xaxis->SetRangeUser(0,-detector.m_width);
                xaxis->SetTitle("x (m)");				//Titole asse X
                xaxis->CenterTitle();

                //Creo asse y
                TAxis *yaxis = gr->GetYaxis();
                yaxis->SetRangeUser(0,detector.m_lenght);
                yaxis->SetTitle("y (m)");				//Titolo asse Y
                yaxis->CenterTitle();    


                //Drawing lines to indicate detector plate
                std::vector<TLine> lines;
                //Determining the lines
                for (int l=0; l < detector.m_plate; l++)
                {
                    lines.push_back(TLine(-l*detector.m_distance,0,-l*detector.m_distance,detector.m_lenght));
                }

                //Drawing the lines
                for (int l=0; l < detector.m_plate; l++)
                {
                    lines.at(l).SetLineColor(kRed);
                    lines.at(l).Draw();
                }

                if(interactiveImg)
                {
                    c1->Modified();
                    c1->Update();
                    usleep(0.5e6);
                }

                std::string imOut;
                if(images)
                {
                    //Output immagine
                    img->FromPad(c1);
                    imOut = imgDirectory + std::string("/Img") + std::to_string(eventNum) + std::string(".png");
                    //std::cout << imOut << std::endl;
                    img->WriteImage(imOut.c_str());    
                }

                if (fit)
                {
                    gr->SetTitle("Detector hits with linear fit");					// Titolo del grafico
                    //gr->CenterTitle();

                    //Drawing the fit line
                    TLine line(0,qReconstructed(max.at(0), max.at(1), thetaPrecision, rhoPrecision),-detector.m_width,(mReconstructed(max.at(0), thetaPrecision)*-detector.m_width)+qReconstructed(max.at(0), max.at(1), thetaPrecision, rhoPrecision));
                    line.Draw();

                    if(interactiveImg)
                    {
                        c1->Modified();
                        c1->Update();
                        usleep(1e6);
                    }

                    if(images)
                    {
                        //Output immagine con fit
                        img->FromPad(c1);
                        imOut = imgDirectory + std::string("/Img") + std::to_string(eventNum) + std::string("Fit.png");
                        //std::cout << imOut << std::endl;
                        img->WriteImage(imOut.c_str());
                    }
                }

                delete gr;
                c1->Clear();

                // Creazione dello histo 
                TH2F *histo = new TH2F("Histo","Hough space",int(180/thetaPrecision)-1,thetaPrecision,180-thetaPrecision,2*maxRho,-maxRho*rhoPrecision,maxRho*rhoPrecision);
                histo->GetXaxis()->SetTitle("Theta (degree)");
                histo->GetXaxis()->CenterTitle();
                histo->GetYaxis()->SetTitle("Rho (m)");
                histo->GetYaxis()->CenterTitle();

                for (int i = 0; i < int(values.size()); i++)
                {
                    for (int j = 0; j < int(values.at(i).at(0).size()); j++)
                    {
                        for (int n = 0; n < values.at(i).at(1).at(j); n++)
                            histo->Fill((float(i+1))*thetaPrecision,values.at(i).at(0).at(j)*rhoPrecision);
                    }
                    /*
                    for(auto const& [key, val] : values.at(i))  //Cicles over the elements of the map to print the values detected with the corresponding detector plate
                    {
                        for (int p = 0; p < val; p++)
                            histo->Fill((float(i+1))*thetaPrecision,key*rhoPrecision);
                    }
                    */
                }

                gStyle->SetPalette(kBird);
                //gStyle->SetOptStat(0);
                histo->SetStats(0);
                histo->Draw("COLZ");
                
                if(interactiveImg)
                {
                    c1->Modified();
                    c1->Update();
                    usleep(1e6);
                }                

                if(images)
                {
                    //Output immagine histo
                    img->FromPad(c1);
                    imOut = imgDirectory + std::string("/Img") + std::to_string(eventNum) + std::string("Histo.png");
                    //std::cout << imOut << std::endl;
                    img->WriteImage(imOut.c_str());
                }

                delete histo;
                c1->Clear();
                //delete c1;
            }
        }
    }
    write(datafile, outFileHeader(detector, take, int64_t(reinterpret_cast<char*>(&init))));  //Writing the header of the file for the simulation in the Simulation.bin file
    writeData(datafile, outHeaderType(eventNum, int(mqValues.size())), mqValues);

    
    datafile.close();

    std::cout << "Number of events analysed: " << std::dec << eventNum + 1 << "\n";
    std::cout << "Number of events succesfully reconstructed: " << mqValues.size() << "\n";
    std::cout << "Output of analysed data: " << outFile << "\n";
    if(images)
        std::cout << "Output of images (directory): " << imgDirectory << "\n";
    std::cout << "End data analysis\n\n";

    delete c1;
    delete img;

    if(check)
    {
        std::string original = directory + std::string("Original_") + std::to_string(take) + std::string(".txt");
        checkCorrectness(original, outFile, times, interactiveImg);
    }

    if(interactiveImg)
    {
        myApp->Terminate();
        myApp->Run();  // Passaggio del controllo a root
    }
    //std::cout << num << std::endl;
        
}

