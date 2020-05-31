
#include "DataType.h"
#include "WriteRead.h"
#include "Rivelatore.h"
#include "Simulazione.h"

#include <iostream>
#include <optional>
#include <ctime>
#include <chrono>
#include <random>
#include <fstream>

int main(){
	//std::cout << "Test" << std::endl;
    Rivelatore detector;
    SimulatePoint(detector, 10, 0.7,1,0,0);

    std::ifstream in("Simulazione.bin", std::ios::binary);
    readFile(in);
/*
    fileHeader test(0xFF, int64_t(0xAABBCCDD11223344));
    std::cout << std::hex << test.date1 << std::endl;
    std::cout << std::hex << test.date2 << std::endl;
    std::cout << std::hex << test.date() << std::endl;

    std::optional<int> x{};

    headerType test;
    std::cout << int(test.type);
    std::cout << std::endl;

    dataType mucca(1,int(0b1000000000000000000011));
    std::cout << int(mucca.valore);
    std::cout<<"size :"<<sizeof(mucca)<<"\n";

    auto mu = time();
    std::cout << "finished computation at " << mu << "s\n";
    std::cout<<"size :"<<sizeof(mu)<<"\n";
*/ 
	return 0;
}
