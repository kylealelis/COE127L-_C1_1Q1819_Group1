#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <cstring>
#include <chrono>

using namespace std;
using namespace std::chrono;

void enumSort(vector<float>&);

int main(int argc, char** argv)
{
	ifstream readFile("winequality-red.csv");
	vector<float> data;
	int skipNum = atoi(argv[1]);
	high_resolution_clock::time_point t3 = high_resolution_clock::now();

	// File Reading and vector populating
	if (readFile.good())
	{
		string readStr("");
		getline(readFile, readStr);
		while (!readFile.eof())
		{
			string appStr("");
			bool value = true;
			getline(readFile, readStr);
			int readIndex = 0;
			for (int i = 0; i < skipNum - 1; i++)
			{
				while (readStr[readIndex] != ',' && readStr[readIndex] != NULL)
					readIndex++;
				readIndex++;
			}
			while (readStr[readIndex] != ',' && readStr[readIndex] != NULL)
			{
				if (readStr[readIndex] == NULL)
				{
					value = false;
				}
				else
				{
					appStr.append(1, readStr[readIndex]);
					readIndex++;
				}
			}
			if (value)
			{
				float app = stof(appStr);
				data.push_back(app);
			}
		}
	}

	// Sorting
	cout << "Data Values Retrieved, will now sort." << endl;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	auto readDuration = duration_cast<microseconds>(t1 - t3).count();
	cout << "Duration of file reading: " << readDuration << " microseconds" << endl;
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	enumSort(data);
	high_resolution_clock::time_point t4 = high_resolution_clock::now();
	auto sortDuration = duration_cast<microseconds>(t4 - t2).count();
	cout << "Duration of sorting: " << sortDuration << " microseconds" << endl;
	return 0;
}

void enumSort(vector<float> &data)
{
	vector<int> rank;
	rank.assign(data.size(), 0);

	// Computes for the rank of each member of the vector
	for (int i = 0; i < data.size(); i++)
	{
		for (int j = i + 1; j < data.size(); j++)
		{
			if (data[i] >= data[j])
				rank[i]++;
			else
				rank[j]++;
		}
	}

	// Assigns each member of data to its corresponding rank position on newData
	// and equates newData to data
	vector<float> newData;
	for (int i = 0; i < data.size(); i++)
		newData.push_back(0);
	for (int i = 0; i < data.size(); i++)
		newData[rank[i]] = data[i];
	data = newData;
}