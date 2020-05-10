#include <iostream>
#include "Simulazione.h"
#include "DataType.h"
#include <optional>
#include <ctime>
#include <chrono>
#include <random>

int main(){
	//std::cout << "Test" << std::endl;
    Simulazione r(5);
    r.SimulatePoint(0.7,1,0,0);

/*
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
