#include <iostream>
#include <chrono>
#include <vector>
#include <mpi.h>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <cmath>
#include "EnumSortParallel2.h"

#define INCREMENTTAG 1
#define REQUESTTAG 2
#define SENDTAG 3
#define DATATAG 4
#define RANKTAG 5

using namespace std;
using namespace std::chrono;

int main()
{
	int world_rank;
	int world_size;
	MPI_Status status;
	vector<float> procData;
	vector<float> arranged;
	vector<int> procRank;
	float compare;
	int swapCount = 0;
	int placeCount = 0;
	int requestCount = 0;
	int reqCount[3] = { 0,0,0 };
	int indexStart, indexEnd;
	string readString;


	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	//File input
	high_resolution_clock::time_point IStart = high_resolution_clock::now();
	string fileName = "input_" + to_string(world_rank) + ".csv";
	ifstream dataFile(fileName.c_str());
	if (dataFile.good())
	{
		while (!dataFile.eof())
		{
			getline(dataFile, readString);
			float app = stof(readString);
			procData.push_back(app);
		}
	}
	high_resolution_clock::time_point IEnd = high_resolution_clock::now();

	if (world_rank == 0)
	{
		auto fileDuration = duration_cast<microseconds>(IEnd - IStart).count();
		cout << "Duration of File Reading: " << fileDuration << " us" << endl;
	}

	//switch (world_rank)
	//{
	//case 0:
	//	procData = { 4,1,10 };
	//	break;
	//case 1:
	//	procData = { 0,6,129 };
	//	break;
	//case 2:
	//	procData = { 7,3,23 };
	//	break;
	//case 3:
	//	procData = { 5,298,3 };
	//	break;
	//}
	procRank.assign(procData.size(), 0);

	// Printing unsorted values
	/*cout << "Data values of process " << world_rank << " unsorted with ranks: ";
	for (int i = 0; i < procData.size(); i++)
	{
		cout << procData[i] << " " << procRank[i] << " ";
	}
	cout << endl;*/

	// Sorting proper

	high_resolution_clock::time_point sortStart = high_resolution_clock::now();
	// Process local rank computation
	for (int i = 0; i < procData.size(); i++)
		for (int j = i + 1; j < procData.size(); j++)
		{
			if (procData[i] >= procData[j])
				procRank[i]++;
			else
				procRank[j]++;
		}

	// Interprocess rank computation through communication
	if (world_rank != world_size - 1)
	{
		for (int i = 0; i < procData.size(); i++)
			for (int j = world_rank + 1; j < world_size; j++)
			{
				int temp = i;
				MPI_Send(&procData[i], 1, MPI_FLOAT, j, temp, MPI_COMM_WORLD);
				//cout << "Process " << world_rank << " sent " << procData[i] << " to process " << j << endl;
			}
	}
	if (world_rank != 0)
	{
		for (int i = 0; i < world_rank * procData.size(); i++)
		{
			MPI_Recv(&compare, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			int comRank = status.MPI_TAG;
			//cout << "Process " << world_rank << " received " << compare << " from process " << status.MPI_SOURCE << endl;
			for (int j = 0; j < procData.size(); j++)
			{
				if (compare >= procData[j])
				{
					MPI_Send(&comRank, 1, MPI_INT, status.MPI_SOURCE, SENDTAG, MPI_COMM_WORLD);
					//cout << "Process " << world_rank << " sent " << SENDTAG << " for increment of " << compare << " to process " << status.MPI_SOURCE << endl;
					reqCount[status.MPI_SOURCE]++;
				}
				else
					procRank[j]++;
			}
		}
		for (int i = 0; i < world_rank; i++)
			MPI_Send(&reqCount[i], 1, MPI_INT, i, REQUESTTAG, MPI_COMM_WORLD);
	}
	//cout << "Req Count for process " << world_rank << ": " << reqCount[0] << " " << reqCount[1] << " " << reqCount[2] << endl;
	MPI_Barrier(MPI_COMM_WORLD);
	if (world_rank != 3)
	{
		int temp;
		for (int i = world_rank + 1; i < world_size; i++)
		{
			MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, REQUESTTAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			requestCount += temp;
		}
		while (requestCount != 0)
		{
			requestCount--;
			MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			procRank[temp]++;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);

	indexStart = procData.size() * world_rank;
	indexEnd = (procData.size() * (world_rank + 1)) - 1;
	arranged.assign(procData.size(), 0);
	for (int i = 0; i < procData.size(); i++)
	{
		if (procRank[i] >= indexStart && procRank[i] <= indexEnd)
		{
			int destIndex = procRank[i] % procData.size();
			arranged[destIndex] = procData[i];
			placeCount++;
		}
		else
		{
			int dest = procRank[i] / procData.size();
			MPI_Send(&procData[i], 1, MPI_FLOAT, dest, procRank[i], MPI_COMM_WORLD);
		}
	}

	for (int i = 0; i < procData.size() - placeCount; i++)
	{
		float temp;
		MPI_Recv(&temp, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		int destIndex = status.MPI_TAG % procData.size();
		arranged[destIndex] = temp;
	}
	//cout << "Process " << world_rank << " has " << placeCount << " arranged items, expecting " << procData.size() - placeCount << " receives.";

	MPI_Barrier(MPI_COMM_WORLD);

	//cout << "Swap count for process " << world_rank << ": " << swapCount;

	MPI_Barrier(MPI_COMM_WORLD);
	high_resolution_clock::time_point sortEnd = high_resolution_clock::now();
	//For verifying data in each process
	//cout << "Contents of Data for process " << world_rank << ": ";
	//for (int i = 0; i < procData.size(); i++)
	//{
	//	cout << procData[i] << " " << procRank[i] << " ";
	//}

	// Vector verification
	//cout << "Contents of Arranged Data for process " << world_rank << ": ";
	//for (int i = 0; i < arranged.size(); i++)
	//{
	//	cout << arranged[i] << " ";
	//}

	MPI_Barrier(MPI_COMM_WORLD);
	// Printing total process time
	if (world_rank == 0)
	{
		auto sortDuration = duration_cast<microseconds>(sortEnd - sortStart).count();
		cout << endl <<"Duration of Sorting: " << sortDuration << " us";
	}
	MPI_Finalize();
	return 0;
}