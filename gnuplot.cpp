#include "pch.h"
#include "gnuplot.h"

Gnuplot::Gnuplot()
{
    // with -persist option you will see the windows as your program ends
    //gnuplotpipe=_popen("gnuplot -persist","w");
    //without that option you will not see the window
    // because I choose the terminal to output files so I don't want to see the window
    gnuplotpipe = _popen("gnuplot -persist", "w");
    if (!gnuplotpipe)
    {
        cerr << ("Gnuplot not found !");
    }
}
Gnuplot::~Gnuplot() {
    fprintf(gnuplotpipe, "exit\n");
    _pclose(gnuplotpipe);
}
void Gnuplot::operator()(const string& command) {
    fprintf(gnuplotpipe, "%s\n", command.c_str());
    fflush(gnuplotpipe);
    // flush is necessary, nothing gets plotted else
};

/*
void Gnuplot::GraphData(std::vector<double> x_data, std::vector<std::vector<double> > y_data, bool cal = 0)
{
    //takes data to grpah from x_data and y_data and saves it into a file named data.dat. This file is then graphed using gnuplot
    //length of all data sets must be the same

    for (int i = 0; i < y_data.size(); i++)
        if (y_data[i].size() > x_data.size() || y_data[i].size() < x_data.size())
        {
            std::cout << "Data sets must be the same length to graph them." << std::endl;
            std::cout << "Y data size = " << y_data[i].size() << std::endl;
            std::cout << "X data size = " << x_data.size() << std::endl;
            return;
        }

    ofstream myFile;
    myFile.open("data.dat");
    for (int i = 0; i < x_data.size(); i++)
    {
        myFile << x_data[i] << "    ";
        for (int j = 0; j < y_data.size(); j++)
            myFile << y_data[j][i] << "    ";
        myFile << '\n';
    }
    myFile.close();

    std::string function = "plot ", relative = "[0:" + to_string((int)x_data.back() + 1) + "] ";
    relative = relative + " 0, " + relative + "9.80665, " + relative + "-9.80665";
    for (int i = 0; i < y_data.size(); i++)
    {
        std::string next_line = "'data.dat' using 1:" + to_string(i + 2) + " with lines, ";
        function = function + next_line;
    }
    function.pop_back(); function.pop_back(); //remove extra space and comma at end of string

    Gnuplot plot;
    if (cal) plot(function + ", " + relative);
    else plot(function);

    std::cout << "Plotting Done. Close graph to continue.\n" << std::endl;
}
*/
