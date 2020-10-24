from mat4py import loadmat

data = loadmat(R"C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/ExampleData.mat")

#print(data['Gyroscope'][0])
#print(data['Accelerometer'][0])
#print(data['Magnetometer'][0])
#print(data['time'][0])

file1 = open(R"C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/MatlabData.txt","w") 
sample_size = 125
start_location = 1125
time_count = 0
total_data_points = 6959

for count in range(30):
	for i in range(sample_size):
		file1.write(str(data['time'][time_count][0]))
		file1.write("    ")
		file1.write(str(data['Gyroscope'][start_location + i][0]))
		file1.write("    ")
		file1.write(str(data['Gyroscope'][start_location + i][1]))
		file1.write("    ")
		file1.write(str(data['Gyroscope'][start_location + i][2]))
		file1.write("    ")
		file1.write(str(data['Accelerometer'][start_location + i][0]))
		file1.write("    ")
		file1.write(str(data['Accelerometer'][start_location + i][1]))
		file1.write("    ")
		file1.write(str(data['Accelerometer'][start_location + i][2]))
		file1.write("    ")
		file1.write(str(data['Magnetometer'][start_location + i][0]))
		file1.write("    ")
		file1.write(str(data['Magnetometer'][start_location + i][1]))
		file1.write("    ")
		file1.write(str(data['Magnetometer'][start_location + i][2]))
		file1.write("\n")
		time_count = time_count + 1

print("Data transfer is complete, closing file.")
file1.close()
