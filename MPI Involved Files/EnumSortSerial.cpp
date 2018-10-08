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

int main()
{
	ifstream readFile("winequality-red.csv");
	vector<float> acidity;
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
			int i = 0;
			while (readStr[i] != ',' && readStr[i] != NULL)
			{
				if (readStr[i] == NULL)
					value = false;
				else
				{
					appStr.append(1, readStr[i]);
					i++;
				}
			}
			if (value)
			{
				float app = stof(appStr);
				acidity.push_back(app);
			}
		}
	}

	// Sorting
	cout << "Data Values Retrieved, will now sort." << endl;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	auto readDuration = duration_cast<microseconds>(t1 - t3).count();
	cout << "Duration of file reading: " << readDuration << " microseconds" << endl;
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	enumSort(acidity);
	high_resolution_clock::time_point t4 = high_resolution_clock::now();
	auto sortDuration = duration_cast<microseconds>(t4 - t2).count();
	cout << "Duration of sorting: " << sortDuration << " miseconds" << endl;
	return 0;
}

void enumSort(vector<float> &data)
{
	vector<int> rank;
	for (int i = 0; i < data.size(); i++)
		rank.push_back(0);
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
	vector<float> newData;
	for (int i = 0; i < data.size(); i++)
		newData.push_back(0);
	for (int i = 0; i < data.size(); i++)
		newData[rank[i]] = data[i];
	data = newData;
}