#include "WriteRead.h"
#include "DataType.h"
#include "Rivelatore.h"
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <typeinfo>
#include <math.h>
#include <iterator>
#include "Hough.h"
#include "HoughFunctions.h"

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
#include <TH2.h>
#include <TStyle.h>

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
    if(!std::filesystem::is_directory("Simulation/") || std::filesystem::is_empty("Simulation/")) //Check if directory exist or if it is empty
    {
        std::cerr << "No file found to read, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    /*
    if(std::filesystem::is_empty("Simulation/"))     //Check if directory is empty
    {
        std::cerr << "No file found to read, make a simulation or check the file name" << std::endl;
        exit(4);
    }
    */
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

void readFile(std::string namefile, const float rhoPrecision, const float thetaPrecision, const bool terminalOutput, const bool images)
{
    //int argc = 0; 
    //char* argv[1];
    // Inizializzo l'interfaccia con root (e' necessario)
    //TApplication myApp("App", &argc, argv);

    namefile = existanceReadFile(namefile);

    //std::vector<int> xValue;        //Vector to store all the x values of an event aka the affacted plate
    //std::vector<int> yValue;        //Vector to store all the y values of an event aka the affected pixel
    std::vector<float> xValueFloat; //Vector to store all the x values
    std::vector<float> yValueFloat; //Vector to store all the y values

    //float rmax = sqrt(pow(detector.m_width,2) + pow(detector.m_lenght,2)); //maximum value possible of r -> diagonal of the detector

    //std::cout << "File size: " << int(std::filesystem::file_size(namefile)) << std::endl;
    //int num = 0;

    //Definid variable to read take characteristics
    Rivelatore detector;
    int take;

    //Definid variable to read event characteristics
    int pointNum;     
    int eventNum;

    //Defining variable relative to value read
    int time;
    int plate; 
    int value;

    //General variable for reading
    unsigned int w;

    //Vector to store events succefully reconstructed
    std::vector<struct outDataType> mqValues;

    //Defining string to store directory name of where to save images from ROOT
    std::string imgDirectory = "none";
    
    std::cout << "Starting data analysis\n";
    std::cout << "Analysing file: " << namefile << "\n";
    
    std::ifstream in(namefile, std::ios::binary);
    while(!in.eof())       //Temporary
    {
        in.read((char*) &w, 4);
        if (w == 0xF0CAFACE)
        {
            in.read(reinterpret_cast<char *>(&detector), sizeof(Rivelatore));

            std::cout << "Using detector: " << std::hex << detector.name << "\n";
            std::cout << "\t Plate number: " << detector.m_plate << "\n";
            std::cout << "\t Plate distance: " << detector.m_distance << " m\n";
            std::cout << "\t Pixel number per plate: " << detector.m_number << "\n";
            std::cout << "\t Pixel dimension: " << detector.m_dimension << " m\n\n";
            //std::cout << std::hex << riv.name << " , " << riv.m_plate << std::endl;

            in.read((char*) &take, 4);

            //Creates directory to store the images
            if(images)
            {
                imgDirectory = SplitFilename(namefile).at(0) + std::string("/images_") + std::to_string(take);
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
            //xValue.clear();
            //yValue.clear();
            xValueFloat.clear();            //Creation of vector to store x values (correct values not descrete ones)
            yValueFloat.clear();            //Creation of vector to store y values (correct values not descrete ones)
            
            //std::vector<std::map<int,int>> thetaValue(int(180/thetaPrecision)-1); //Vector to store the different values of rho associated with the same value of theta
            /*
            std::vector<std::vector<int>> rhovalueALL(int(180/thetaPrecision)-1);
            std::vector<std::vector<int>> rhovalue;
            std::vector<std::vector<int>> rhovalueOccurrency;
            */

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
                    in.read((char*) &time, 4);
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
            //VecToTrust(xValueFloat);

            //rhoAll(yValueFloat,xValueFloat,thetaValue,thetaPrecision,rhoPrecision);         

            std::vector<int> max = {0,0,0,0};

            /*
            for (int i = 0; i < int(thetaValue.size()); i++)
            {
                for(auto const& [key, val] : thetaValue.at(i))  //Cicles over the elements of the map to print the values detected with the corresponding detector plate
                {
                    //std::cout << key*rhoPrecision << std::endl; 
                    if (val > max.at(2) && key < rmax)
                    {
                        max.at(0) = i;
                        max.at(1) = key;
                        max.at(2) = val;
                    }    
                }
            }
            int maxRho = 0; //Maximum value of Rho observed
            for (int i = 0; i < int(thetaValue.size()); i++)
            {
                for(auto const& [key, val] : thetaValue.at(i))  //Cicles over the elements of the map to print the values detected with the corresponding detector plate
                {
                    //std::cout << i << " - " << key << " , " << abs(key) << std::endl;
                    if (abs(key) > maxRho)
                    {
                        maxRho = abs(key);
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
                }  }    
                }
            }
            */

            calculateRho(values, max, yValueFloat, xValueFloat, thetaPrecision, rhoPrecision, detector.m_lenght, true);
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
                std::cout << "Event number: " << eventNum << "\n";
                std::cout << "Point number: " << pointNum << "\n";
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
            /*
            std::cout << "0x" << std::hex << w << " : " << std::dec << structLenght[w]/4 << std::endl;
            //num += structLenght[w];
            in.read( (char*) &w, 4);
            std::cout << w << "\n";
            */
            
            if (images)
            {
                //Error vector
                std::vector<float> yErr;
                yErr.resize(xValueFloat.size());
                std::fill(yErr.begin(), yErr.end(), detector.m_dimension/sqrt(12));     

                // Creazione dello screen
                TCanvas *c1 = new TCanvas("c1","",0,0,800,600); 

                //Crea grafico con barre di errore
                TGraphErrors *gr = new TGraphErrors(xValueFloat.size(), &(xValueFloat[0]) , &(yValueFloat[0]), 0, &(yErr[0]));
                gr->SetMarkerStyle(20);					// Seleziona il marker rotondo
                gr->SetMarkerSize(1);
                gr->SetTitle("Detector hit");					// Titolo del grafico

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


                //gr->Draw("APE");					// Plot del grafico
                //c1->Modified();
                //c1->Update();

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

                //Output immagine
                TImage *img = TImage::Create();
                img->FromPad(c1);
                std::string imOut = imgDirectory + std::string("/Img") + std::to_string(eventNum) + std::string(".png");
                //std::cout << imOut << std::endl;
                img->WriteImage(imOut.c_str());
                
                if (fit)
                {
                    //Drawing the fit line
                    TLine line(0,qReconstructed(max.at(0), max.at(1), thetaPrecision, rhoPrecision),-detector.m_width,(mReconstructed(max.at(0), thetaPrecision)*-detector.m_width)+qReconstructed(max.at(0), max.at(1), thetaPrecision, rhoPrecision));
                    line.Draw();

                    //Output immagine con fit
                    img->FromPad(c1);
                    imOut = imgDirectory + std::string("/Img") + std::to_string(eventNum) + std::string("Fit.png");
                    //std::cout << imOut << std::endl;
                    img->WriteImage(imOut.c_str());
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
                gStyle->SetOptStat(0);
                histo->Draw("COLZ");

                //Output immagine histo
                img->FromPad(c1);
                imOut = imgDirectory + std::string("/Img") + std::to_string(eventNum) + std::string("Histo.png");
                //std::cout << imOut << std::endl;
                img->WriteImage(imOut.c_str());

                delete histo;
                delete c1;
            }
        }
        /*
        else if (structLenght.contains(w))
        {
            std::cout << "0x" << std::hex << w << " : " << std::dec << structLenght[w]/4 << std::endl;
            //num += structLenght[w];
        }
        */
    }
    std::cout << "Number of events analysed: " << std::dec << eventNum + 1 << "\n";
    std::cout << "Number of events succesfully reconstructed: " << mqValues.size() << "\n";
    std::cout << "Output of analysed data: " << "\n";
    if(images)
        std::cout << "Output of images: " << imgDirectory << "\n";
    std::cout << "End data analysis\n";


    //myApp.Run();  // Passaggio del controllo a root
    //std::cout << num << std::endl;
}