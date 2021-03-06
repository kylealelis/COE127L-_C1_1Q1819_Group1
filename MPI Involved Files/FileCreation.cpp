// FileCreation.cpp : This file contains the 'main' function. Program execution begins and ends there.
// This code takes the SPLIT_FILE_NAME and divides it to 4 separate .csv files to be used
// by the MS MPI implementation of the code.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

const char *SPLIT_FILE_NAME = "winequality-red.csv";

int main(int argc, char** argv)
{
	ifstream readFile(SPLIT_FILE_NAME);
	//int skipNum = atoi(argv[1]);
	int skipNum = 1;
	vector<float> data;

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

	int fileSize = data.size() / 4;
	for (int i = 0; i < 4; i++)
	{
		string fileName = "input_" + to_string(i) + ".csv";
		ofstream writeFile(fileName.c_str());
		for (int j = fileSize * i; j < fileSize * (i + 1); j++)
		{
			if (j == fileSize * (i + 1) - 1)
				writeFile << data[j];
			else
				writeFile << data[j] << endl;
		}
		writeFile.close();
	}
	return 0;
}