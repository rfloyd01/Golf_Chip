#include "pch.h"
#include <Header_Files/gnuplot.h>

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

//Tester Functions
void graphFromFile(std::string file_location, int data_sets)
{
	//this function assumes that the first column of data will always be time
	//ofstream myFile;
	//myFile.open(file_location);
	//for (int i = 0; i < mx.size(); i++) myFile << mx[i] << "    " << my[i] << "    " << mz[i] << '\n';
	//myFile.close();

	//std::string function = "plot 'mag.dat' using 1:2, 'mag.dat' using 1:3, 'mag.dat' using 2:3";
	std::string function = "plot ";

	for (int i = 0; i < data_sets - 1; i++)
	{
		function += "'" + file_location + "' using 1:" + to_string(i + 2) + " with lines, ";
	}
	function += "'" + file_location + "' using 1:" + to_string(data_sets + 1) + " with lines";
	Gnuplot plot;
	plot(function);
}
void ellipsePlot(std::vector<double>& x_data, std::vector<double>& y_data, std::vector<double>& z_data) //this is being used as a test
{
    int axes_maximum = 50, axes_minimum = -50;

    //write all data to file
    ofstream myFile;
    myFile.open("ellipse.dat");
    for (int i = 0; i < x_data.size(); i++)
    {
        myFile << x_data[i] << "    " << y_data[i] << "    " << z_data[i] << '\n';
    }
    myFile.close();

    std::string function = "splot 'ellipse.dat' using 1:2:3";

    Gnuplot plot;
    plot("set terminal wxt size 1000,1000"); //make the gnuplot window a square so that all axes tics have the same physical distance between them, otherwise will get ellipse instead of sphere
    plot("set xrange [" + to_string(axes_minimum) + ":" + to_string(axes_maximum) + "]");
    plot("set yrange [" + to_string(axes_minimum) + ":" + to_string(axes_maximum) + "]");
    plot("set zrange [" + to_string(axes_minimum) + ":" + to_string(axes_maximum) + "]");
    plot("set ticslevel 0"); //make sure that axes aren't offset at all
    plot(function);
}