#include "WriteRead.h"
#include "DataType.h"
#include "Handler.h"
#include "Rivelatore.h"
#include "Simulazione.h"
#include "Hough.h"
#include "HoughFunctions.h"
#include "DataTypeOut.h"

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
#include <algorithm>

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

void checkCorrectness(std::string original, std::string analysis, std::vector<int64_t> times, const bool interactiveImg)
{   
    std::signal(SIGINT, sig_handler);
    
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
        if(killed)
        {
            fclose(fp);
            exit(666);
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
        else if(strcmp(line, "Point to generate\n") == 0)
        {              
            getline(&line, &len, fp);
            events = atoi(line);
        }
        else if(strcmp(line, "Type of simulation\n") == 0)
        {
            getline(&line, &len, fp);
            std::cout << line;
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
    unsigned int take;

    //Read analysis file
    unsigned int w;

    //Header elements
    unsigned int eventAnalysed;
    unsigned int tracks;

    //Data elements
    unsigned int eventNum;
    float m;
    float q;

    //vector for m and q value difference
    std::vector<float> mVec;
    std::vector<float> qVec;

    std::ifstream in(analysis, std::ios::binary);

    while(!in.eof())
    {
        if(killed)
        {
            in.close();
            exit(666);
        }

        in.read((char*) &w, 4);
        
        if (w == 0x4EADDEAD)
        {   
            in.read((char*) &eventAnalysed, 4);
            in.read((char*) &tracks, 4);
            unsigned int i = 0;
            int j = 0;
            while (i < tracks && j < 10 && !in.eof()) //Substantially if the program doen't find a new expected value goes on
            {
                j++;

                in.read((char*) &w, 4);
                if (w == 0xDADADEAD)
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

            int nbins = int(((mVec.at(int(mVec.size())-1))/0.0001)+0.5) - int(((mVec.at(0))/0.0001)+0.5);

            //std::cout << nbins << std::endl;
            //std::cout << xMax << std::endl;

            std::string imgDirectory = SplitFilename(analysis).at(0) + std::string("/") + std::string("fitImages_") + std::to_string(take); 

            if(std::filesystem::is_directory(imgDirectory) && !std::filesystem::is_empty(imgDirectory) ) //Check if directory where the image should be saved exists
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
            if(!(std::filesystem::is_directory(imgDirectory)))
                std::filesystem::create_directory(imgDirectory);

            // Creazione dello screen
            TCanvas *c2 = new TCanvas("c2","",0,0,800,600); 
            //Img output
            TImage *img = TImage::Create();
        
            // Creazione dello histo 
            TH1F *histo1 = new TH1F("m Histo","m error",nbins,mVec.at(0),mVec.at(int(mVec.size())-1));
            histo1->GetXaxis()->SetTitle("Difference (m)");
            histo1->GetXaxis()->CenterTitle();

            for (int i = 0; i < int(mVec.size()); i++)
            {
                histo1->Fill(mVec.at(i));
            }

            histo1->SetFillColor(42);  // Colore dell'istogramma
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

            nbins = int(((qVec.at(int(qVec.size())-1))/0.0001)+0.5) - int(((qVec.at(0))/0.0001)+0.5) + 2;

            TH1F *histo2 = new TH1F("q Histo","q error",nbins,qVec.at(0),qVec.at(int(mVec.size())-1));
            histo2->GetXaxis()->SetTitle("Difference (m)");
            histo2->GetXaxis()->CenterTitle();

            for (int i = 0; i < int(qVec.size()); i++)
            {
                histo2->Fill(qVec.at(i));
            }

            histo2->SetFillColor(42);  // Colore dell'istogramma
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

            c2->Clear();

            std::sort(times.begin(), times.end());
            nbins = times.at(int(times.size())-1)*1e-6 - times.at(0)*1e-6 + 2;

            TH1C *histo3 = new TH1C("times Histo","Times",nbins,(times.at(0)*1e-6)-1,(times.at(int(times.size())-1)*1e-6)+1);
            histo3->GetXaxis()->SetTitle("Algorith's execution time (ms)");
            histo3->GetXaxis()->CenterTitle();

            for (int i = 0; i < int(times.size()); i++)
            {
                histo3->Fill(times.at(i)*1e-6);
            }

            histo3->SetFillColor(42);  // Colore dell'istogramma
            histo3->Draw();

            if(interactiveImg)
            {      
                c2->Modified();
                c2->Update();
                usleep(1e6);
            }

            //Output immagine histo
            img->FromPad(c2);
            imgOut = imgDirectory + std::string("/timeAnalysis.png");
            //std::cout << imOut << std::endl;
            img->WriteImage(imgOut.c_str());

            std::cout << "Mean time for Hough trasformation's algorithm execution: " << histo3->GetMean() << " ms, evaluated over " << histo3->GetEntries() << " entries\n";

            delete histo1;
            delete histo2;
            delete histo3;
            delete c2;
            delete img;
            
            std::cout << "Output of images (directory): " << imgDirectory << "\n";
            std::cout << "End data analysis evaluation\n\n";
        }
        else if(w == 0xF0CADEAD)
        {
            in.read(reinterpret_cast<char *>(&detector), sizeof(Rivelatore));
            in.read((char*) &take, 4);
            std::cout << "Take number: " << take << "\n";
        }
    } 
    in.close();
}

void Analisys(std::string namefile, const float rhoPrecision, const float thetaPrecision, const bool terminalOutput, const bool images, const bool interactiveImg, const bool check, const bool costrain)
{
    std::signal(SIGINT, sig_handler);

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


    //Check file existance/correctness
    namefile = existanceFile(namefile, "Simulation");


    std::vector<float> xValueFloat; //Vector to store all the x values
    std::vector<float> yValueFloat; //Vector to store all the y values

    //Definid variable to read headerFile characteristics
    Rivelatore detector;
    unsigned int take;
    int64_t initTime;

    //Definid variable to read event characteristics
    int64_t triggerTime;
    unsigned int pointNum;     
    unsigned int eventNum;

    //Defining variable relative to read value
    int64_t time;
    unsigned int plate; 
    unsigned int value;

    //General variable for reading
    unsigned int w;

    //Vector to store events succefully reconstructed
    std::vector<struct outDataType> mqValues;

    //Creating output file
    std::string directory = SplitFilename(namefile).at(0) + std::string("/");
    std::string outFile;
    std::ofstream datafile; //Initializind ofstream element for outputfile
    
    //Defining string to store directory name of where to save images from ROOT
    std::string imgDirectory = "none";

    //Vector to store executions times
    std::vector<int64_t> times;

    std::cout << "Starting data analysis\n";
    std::cout << "Analysing file: " << namefile << "\n";
    
    //OpeningFile
    std::ifstream in(namefile, std::ios::binary);
    while(!in.eof())       //Temporary
    {
        in.read((char*) &w, 4);
        if (w == 0x4EADFACE) //Found an event
        {   
            xValueFloat.clear();            //Creation of vector to store x values (correct values not descrete ones)
            yValueFloat.clear();            //Creation of vector to store y values (correct values not descrete ones)
            
            //Vector to store all the analysed data
            std::vector<std::vector<std::vector<int>>> values(int((180-(2*detector.m_angle))/thetaPrecision)+1); //The position in the first vector identifies the angle, the vector in the third inclusion are [values] and [repetition of this values]

            //Vector to store final data
            std::vector<int> max = {0,0,0,0}; //angle, rho, significance, maxRho
            //std::vector<int> max1 = {0,0,0,0}; //angle, rho, significance, maxRho

            //Value to determine if track reconstruction was successfull
            bool fit = false;

            in.read((char*) &triggerTime, 8);
            in.read((char*) &eventNum, 4);
            in.read((char*) &pointNum, 4);
            unsigned int i = 0;
            int j = 0;

            while (i < pointNum && j < 10 && !in.eof()) //Substantially if the program doen't find a new expected value goes on
            {
                j++;

                in.read((char*) &w, 4);
                if (w == 0xDADAFACE)
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

            std::chrono::high_resolution_clock::time_point time1 = std::chrono::high_resolution_clock::now(); 
            calculateRho(values, max, yValueFloat, xValueFloat, thetaPrecision, rhoPrecision, detector.m_lenght, costrain, detector.m_angle);
            //Hough(values, max, yValueFloat, xValueFloat, thetaPrecision, rhoPrecision, detector.m_lenght, costrain, detector.m_angle);
            cudaDeviceReset();
            if(check)
                times.push_back(duration(time1));

            int maxRho = max.at(3);
            float angle = (max.at(0)*thetaPrecision)+detector.m_angle;
            float rho = (max.at(1)*rhoPrecision)+(rhoPrecision/2);
/*
            float angle1 = (max1.at(0)*thetaPrecision)+detector.m_angle;
            float rho1 = (max1.at(1)*rhoPrecision)+(rhoPrecision/2);
*/

            if (max.at(2) != 1) //Exclude tha case in which only one point was found on the fit line
            {
                fit = true;
                mqValues.push_back(outDataType(eventNum, mReconstructed(angle),qReconstructed(angle,rho)));
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

                std::cout << "Significance: " << max.at(2) << std::endl;
                if (max.at(2) != 1)
                {
                    std::cout << "m: " << mReconstructed(angle) 
                                << " q: " << qReconstructed(angle, rho) 
                                << " ( Angle: " << angle << " , Rho: " << rho << " )\n";
                }
                else
                {
                    std::cout << "Impossible to calculate line equation\n";
                }
                std::cout << "\n";
/*
                std::cout << "Significance: " << max1.at(2) << std::endl;
                if (max1.at(2) != 1)
                {
                    std::cout << "m: " << mReconstructed(angle1) 
                                << " q: " << qReconstructed(angle1, rho1) 
                                << " ( Angle: " << angle1 << " , Rho: " << rho1 << " )\n";
                }
                else
                {
                    std::cout << "Impossible to calculate line equation\n";
                }
                std::cout << "\n";
*/
            }   
            
            if (images || interactiveImg)
            {
                std::string imOut; //Name of output image

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

                if (fit)
                {
                    gr->SetTitle("Detector hits with linear fit");					// Titolo del grafico
                    //gr->CenterTitle();

                    //Drawing the fit line
                    TLine line(0,qReconstructed(angle, rho),-detector.m_width,(mReconstructed(angle)*-detector.m_width)+qReconstructed(angle, rho));
                    line.Draw();

                    c1->Modified();
                    c1->Update();

                    if(interactiveImg)
                    {
                        usleep(0.5e6);
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
                else
                {
                    if (images)
                    {
                        gr->SetTitle("Detector hits (fit not possible)");					// Titolo del grafico
                        
                        c1->Modified();
                        c1->Update();

                        //Output immagine
                        img->FromPad(c1);
                        imOut = imgDirectory + std::string("/Img") + std::to_string(eventNum) + std::string(".png");
                        //std::cout << imOut << std::endl;
                        img->WriteImage(imOut.c_str());    
                    }
                }

                delete gr;
                c1->Clear();

                // Creazione dello histo 
                TH2F *histo = new TH2F("Histo","Hough space",int((180-(2*detector.m_angle))/thetaPrecision)+1,detector.m_angle,180-detector.m_angle,2*maxRho,-((maxRho*rhoPrecision)+(rhoPrecision/2)),(maxRho*rhoPrecision)+(rhoPrecision/2));
                histo->GetXaxis()->SetTitle("Theta (degree)");
                histo->GetXaxis()->CenterTitle();
                histo->GetYaxis()->SetTitle("Rho (m)");
                histo->GetYaxis()->CenterTitle();

                for (int i = 0; i < int(values.size()); i++)
                {
                    for (int j = 0; j < int(values.at(i).at(0).size()); j++)
                    {
                        for (int n = 0; n < values.at(i).at(1).at(j); n++)
                            histo->Fill((i*thetaPrecision)+detector.m_angle,(values.at(i).at(0).at(j)*rhoPrecision)+(rhoPrecision/2));
                    }
                }

                gStyle->SetPalette(kBird);
                histo->SetStats(0);
                histo->Draw("COLZ");

                c1->Modified();
                c1->Update();

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
            }
        }
        else if (w == 0xF0CAFACE)
        {
            in.read(reinterpret_cast<char *>(&detector), sizeof(Rivelatore));
            in.read((char*) &take, 4);
            in.read((char*) &initTime, 8);

            std::cout << "Take number: " << take << "\n";
            std::cout << "Using detector: " << std::hex << detector.name << "\n";
            std::cout << "\t Plate number: " << detector.m_plate << "\n";
            std::cout << "\t Plate distance: " << detector.m_distance << " m\n";
            std::cout << "\t Pixel number per plate: " << detector.m_number << "\n";
            std::cout << "\t Pixel dimension: " << detector.m_dimension << " m\n\n";

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
                if(std::filesystem::is_directory(imgDirectory) && !std::filesystem::is_empty(imgDirectory) ) //Check if directory where the image should be saved exists
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
                if(!std::filesystem::is_directory(imgDirectory))
                    std::filesystem::create_directory(imgDirectory);
            }
        }
        
        if(killed)
        {
            //Writes what calculated till now
            write(datafile, outFileHeader(detector, take, int64_t(reinterpret_cast<char*>(&initTime))));  //Writing the header of the file for the simulation in the Simulation.bin file
            writeData(datafile, outHeaderType(eventNum, int(mqValues.size())), mqValues);
            in.close();
            datafile.close();
            exit(666);
        }
    }
    write(datafile, outFileHeader(detector, take, int64_t(reinterpret_cast<char*>(&initTime))));  //Writing the header of the file for the simulation in the Simulation.bin file
    writeData(datafile, outHeaderType(eventNum, int(mqValues.size())), mqValues);

    in.close();
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
    
    if(killed)
        exit(666);
    /*
    if(interactiveImg)
    {
        myApp->Terminate();
        myApp->Run();  // Passaggio del controllo a root
    }*/        
}

