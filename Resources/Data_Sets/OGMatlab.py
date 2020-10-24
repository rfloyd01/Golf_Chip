from mat4py import loadmat

data = loadmat(R"C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/ExampleData.mat")

#print(data['Gyroscope'][0])
#print(data['Accelerometer'][0])
#print(data['Magnetometer'][0])
#print(data['time'][0])

file1 = open(R"C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/MatlabData.txt","w") 
sample_size = 125
location = 0
start_location = 0
time_count = 0
time_cutoff = 30
total_data_points = 6959

for count in range(1):
	for i in range(len(data['Gyroscope'])):
		if(data['time'][time_count][0] > time_cutoff): break
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
		time_count += 1
		location += 1

for i in range(location, len(data['Gyroscope'])):
		file1.write(str(data['time'][time_count][0]))
		file1.write("    ")
		file1.write(str(0))
		file1.write("    ")
		file1.write(str(0))
		file1.write("    ")
		file1.write(str(0))
		file1.write("    ")
		file1.write(str(data['Accelerometer'][location][0]))
		file1.write("    ")
		file1.write(str(data['Accelerometer'][location][1]))
		file1.write("    ")
		file1.write(str(data['Accelerometer'][location][2]))
		file1.write("    ")
		file1.write(str(data['Magnetometer'][location][0]))
		file1.write("    ")
		file1.write(str(data['Magnetometer'][location][1]))
		file1.write("    ")
		file1.write(str(data['Magnetometer'][location][2]))
		file1.write("\n")
		time_count += 1

print("Data transfer is complete, closing file.")
file1.close()
