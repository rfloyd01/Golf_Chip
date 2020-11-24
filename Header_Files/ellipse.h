#pragma once

#include <vector>
#include <Header_Files/glm.h>
#include <Eigen/Dense.h>
#include <Header_Files/gnuplot.h>

void eTest();

void matrixMultiply(float* m1, int rows1, int columns1, float* m2, int rows2, int columns2, float* prod);
std::vector<double> getEllipsePoint(float roll, float pitch, float yaw, float xr, float yr, float zr, float x_off, float y_off, float z_off, float u, float v);
void ellipseBestFit(std::vector<float>& x_data, std::vector<float>& y_data, std::vector<float>& z_data, std::vector<std::vector<double> >& RUV, float* mag_off, float* mag_gain);

//Functions Used for Newton_Gauss best fit method
void convertCartesianToSpherical(std::vector<float>& x, std::vector<float>& y, std::vector<float>& z, Eigen::MatrixXf& ruv_values);
std::vector<double> convertSphericalToCartesian(std::vector<double>& spherical);
double calculateRSquared(double u, double v, Eigen::MatrixXf& b);
std::vector<double> calculateXYZ(double u, double v, Eigen::MatrixXf& b);
Eigen::MatrixXf createJacobian(Eigen::MatrixXf b, Eigen::MatrixXf& uv, std::vector<float>& x, std::vector<float>& y, std::vector<float>& z);
double derivative(double u, double v, double x, double y, double z, Eigen::MatrixXf b, int bIndex);
double residualError(Eigen::MatrixXf& res);
void updateParameters(Eigen::MatrixXf& b, Eigen::MatrixXf& res, Eigen::MatrixXf& J);
double calculateResidual(double x, double y, double z, double xr, double yr, double zr);
double calculateGeometricDistance(double x, double y, double z, Eigen::MatrixXf& b);

//Graphing Functions
void graphCompare(std::vector<float>& x, std::vector<float>& y, std::vector<float>& z, Eigen::MatrixXf& b);