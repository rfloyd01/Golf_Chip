#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

class Gnuplot
{
public:
    Gnuplot();
    ~Gnuplot();
    void operator ()(const string& command); // send any command to gnuplot

protected:
    FILE* gnuplotpipe;
};

//Tester Functions
void graphFromFile(std::string file_location, int data_sets);
void ellipsePlot(std::vector<double>& x_data, std::vector<double>& y_data, std::vector<double>& z_data);