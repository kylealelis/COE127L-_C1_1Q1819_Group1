#include <iostream>
#include <chrono>
#include <vector>
#include <mpi.h>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <cmath>
 
#define INCREMENTTAG 1
#define REQUESTTAG 2

typedef struct {
	float value;
	int rank;
} RankedData;

using namespace std;
using namespace std::chrono;

void divideDomain(int, int, int, int*, int*);
void initializeDR(int, int, vector<float> data, vector<float>* procData, vector<int>* procRank);
void sendRequests(int, int, vector<float>, vector<float>, vector<int>&);
void computeRank(vector<float> data, vector<float> procData, vector<int>* procRank);
void buildCompiled(float procData, int procRank, vector<RankedData>*);

int main()
{
	vector<float> data;
	vector<float> procData;
	vector<int> procRank;
	vector<RankedData> compiled;
	MPI_Init(NULL, NULL);

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Request req;
	MPI_Status status;

	// File input
	high_resolution_clock::time_point IStart = high_resolution_clock::now();
	ifstream readFile("winequality-red.csv");
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
				data.push_back(app);
			}
		}
	}
	high_resolution_clock::time_point IEnd = high_resolution_clock::now();

	if (world_rank == 0)
	{
		auto fileDuration = duration_cast<microseconds>(IEnd - IStart).count();
		cout << "Duration of File Reading: " << fileDuration << " us" << endl;
	}
	// Printing unsorted values
	/*if (world_rank == 0)
	{
		cout << "Data values unsorted: ";
		for (int i = 0; i < data.size(); i++)
		{
			cout << data[i] << " ";
		}
		cout << endl;
	}*/
	
	// Sorting proper
	high_resolution_clock::time_point sortStart = high_resolution_clock::now(); // Retrieve time before sorting process
	int domainSize = data.size();
	int subdomainStart, subdomainSize;
	divideDomain(domainSize, world_size, world_rank, &subdomainStart, &subdomainSize);
	initializeDR(subdomainStart, subdomainSize, data, &procData, &procRank);
	sendRequests(world_rank, subdomainSize, data, procData, procRank);

	int requestCount = 0;
	if (world_rank != 0)
	{
		// Get amount of requests
		for (int i = 0; i < world_rank; i++)
		{
			int temp;
			MPI_Recv(&temp, 1, MPI_INT, i, REQUESTTAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			requestCount += temp;
		}
		// Increment requested ranks
		int requests = 0;
		while (requests < requestCount)
		{
			int index;
			MPI_Recv(&index, 1, MPI_INT, MPI_ANY_SOURCE, INCREMENTTAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			index %= subdomainSize;
			procRank[index]++;
			requests++;
		}

		for (int i = 0; i < procData.size(); i++)
		{
			MPI_Send(&procData[i], 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
			MPI_Send(&procRank[i], 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(world_rank == 0)
	{
		for (int i = subdomainStart; i < subdomainSize * 3; i++)
		{
			float tempData;
			int tempRank;
			MPI_Recv(&tempData, 1, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			MPI_Recv(&tempRank, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
			buildCompiled(tempData, tempRank, &compiled);
		}
		for (int i = 0; i < procData.size(); i++)
			buildCompiled(procData[i], procRank[i], &compiled);
	}

	if (world_rank == 0)
	{
		vector<float> temp;
		temp = data;
		for (int i = 0; i < compiled.size(); i++)
			temp[compiled[i].rank] = compiled[i].value;
		data = temp;
	}
	high_resolution_clock::time_point sortEnd = high_resolution_clock::now(); // Retrieve time after sorting process

	// For verifying data in each process
	/*cout << "Contents of Data for process " << world_rank << ": ";
	for (int i = 0; i < procData.size(); i++)
	{
		cout << procData[i] << " " << procRank[i] << " ";
	}*/

	// Vector verification
	/*if (world_rank == 0)
	{
		for (int i = 0; i < compiled.size(); i++)
		{
			cout << compiled[i].rank << " " << compiled[i].value << " ";
		}
	}*/

	// Printing total process time
	if (world_rank == 0)
	{
		auto sortDuration = duration_cast<microseconds>(sortEnd - sortStart).count();
		cout << "Duration of File Reading: " << sortDuration << " us";
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	// Printing sorted data
	/*if (world_rank == 0)
	{
		cout << endl << "Sorted data using enumeration sort: ";
		for (int i = 0; i < data.size(); i++)
		{
			cout << data[i] << " ";
		}
	}*/
	return 0;
}

void divideDomain(int domainSize, int worldSize, int worldRank, int *subdomainStart, int *subdomainSize)
{
	if (worldSize > domainSize) 
	{
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	*subdomainStart = domainSize / worldSize * worldRank;
	*subdomainSize = domainSize / worldSize;
	if (worldRank == worldSize - 1)
	{
		*subdomainSize += domainSize % worldSize;
	}
}

void initializeDR(int subdomainStart, int subdomainSize, vector<float> data, vector<float>* procData, vector<int>* procRank)
{
	float inData;
	int inRank = 0;
	for (int i = 0; i < subdomainSize; i++)
	{
		inData = data[subdomainStart + i];
		procData->push_back(inData);
		procRank->push_back(inRank);
	}
}

void sendRequests(int worldRank, int subdomainSize, vector<float> data, vector<float> procData, 
		vector<int> &procRank)
{
	int startIndex = worldRank * subdomainSize;
	int procSend[3] = { 0,0,0 };
	for (int i = 0; i < procData.size(); i++)
	{
		for (int j = i + 1; j < data.size() - startIndex; j++)
		{
			if (j > (subdomainSize - 1))
			{
				if (procData[i] >= data[j + startIndex])
					procRank[i]++;
				else
				{
					int dest = (j + startIndex) / subdomainSize ;
					int index = j + startIndex;
					MPI_Send(&index, 1, MPI_INT, dest, INCREMENTTAG, MPI_COMM_WORLD);
					procSend[dest - 1]++;
				}
			}
			else
			{
				if (procData[i] >= procData[j])
					procRank[i]++;
				else
					procRank[j]++;
			}
		}
	}
	for (int i = worldRank; i < (sizeof(procSend) / sizeof(int)); i++)
		MPI_Send(&procSend[i], 1, MPI_INT, i + 1, REQUESTTAG, MPI_COMM_WORLD);
}

void computeRank(vector<float> data, vector<float> procData, vector<int>* procRank)
{
	int *o = procRank->data();
	for (int i = 0; i < procData.size(); i++)
	{
		for (int j = 0; j < data.size(); j++)
		{ 
			if (procData[i] >= data[j])
				(*o)++;
		}
		o++;
	}
}

void buildCompiled(float procData, int procRank, vector<RankedData> *compiled)
{
	RankedData temp;
	temp.value = procData;
	temp.rank = procRank;
	compiled->push_back(temp);
}