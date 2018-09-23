// This code contains sample functions that make use of MPI functionalities
// including a processing technique using MPI that will be based off from for
// our project. Refer to further comments

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <time.h>
#include <vector>
using namespace std;

typedef struct {
	int location;
	int num_steps_left_in_walk;
} Walker;

void PingPong(int, char**);
void Ring();
void CheckStatus();
void Probe();
void decompose_domain(int, int, int, int*, int*);
void initialize_walkers(int, int, int, vector<Walker>*);
void walk(Walker*, int, int, int, vector<Walker>*);
void send_outgoing_walkers(vector<Walker>*, int, int);
void receive_incoming_walkers(vector<Walker>*, int, int);

int main(int argc, char** argv)
{
	int domain_size;
	int max_walk_size;
	int num_walkers_per_proc;

	if (argc < 4) {
		cerr << "Usage: random_walk domain_size max_walk_size "
			<< "num_walkers_per_proc" << endl;
		exit(1);
	}
	domain_size = atoi(argv[1]);
	max_walk_size = atoi(argv[2]);
	num_walkers_per_proc = atoi(argv[3]);
	MPI_Init(NULL, NULL);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	srand(time(NULL) * world_rank);
	int subdomain_start, subdomain_size;
	vector<Walker> incoming_walkers, outgoing_walkers;

	// Find your part of the domain
	decompose_domain(domain_size, world_rank, world_size,
		&subdomain_start, &subdomain_size);
	// Initialize walkers in your subdomain
	initialize_walkers(num_walkers_per_proc, max_walk_size, subdomain_start,
		&incoming_walkers);

	cout << "Process " << world_rank << " initiated " << num_walkers_per_proc
		<< " walkers in subdomain " << subdomain_start << " - "
		<< subdomain_start + subdomain_size - 1 << endl;

	// Determine the maximum amount of sends and receives needed to
	// complete all walkers
	int maximum_sends_recvs = max_walk_size / (domain_size / world_size) + 1;
	for (int m = 0; m < maximum_sends_recvs; m++) {
		// Process all incoming walkers
		for (int i = 0; i < incoming_walkers.size(); i++) {
			walk(&incoming_walkers[i], subdomain_start, subdomain_size,
				domain_size, &outgoing_walkers);
		}
		cout << "Process " << world_rank << " sending " << outgoing_walkers.size()
			<< " outgoing walkers to process " << (world_rank + 1) % world_size
			<< endl;
		if (world_rank % 2 == 0) {
			// Send all outgoing walkers to the next process.
			send_outgoing_walkers(&outgoing_walkers, world_rank,
				world_size);
			// Receive all the new incoming walkers
			receive_incoming_walkers(&incoming_walkers, world_rank,
				world_size);
		}
		else {
			// Receive all the new incoming walkers
			receive_incoming_walkers(&incoming_walkers, world_rank,
				world_size);
			// Send all outgoing walkers to the next process.
			send_outgoing_walkers(&outgoing_walkers, world_rank,
				world_size);
		}
		cout << "Process " << world_rank << " received " << incoming_walkers.size()
			<< " incoming walkers" << endl;
	}
	cout << "Process " << world_rank << " done" << endl;
	MPI_Finalize();
    return 0;
}

void PingPong(int argc, char** argg)
{
	const int PING_PONG_LIMIT = 10;

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if (world_size != 2)
	{
		fprintf(stderr, "World size must be two for %s\n", argg[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	int ping_pong_count = 0;
	int partnerRank = (world_rank + 1) % 2;

	while (ping_pong_count < PING_PONG_LIMIT)
	{
		if (world_rank == ping_pong_count % 2)
		{
			ping_pong_count++;
			MPI_Send(&ping_pong_count, 1, MPI_INT, partnerRank, 0, MPI_COMM_WORLD);
			printf("%d sent ping pong count %d to %d\n", world_rank, ping_pong_count, partnerRank);
		}
		else
		{
			MPI_Recv(&ping_pong_count, 1, MPI_INT, partnerRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("%d received ping pong count %d from %d\n", world_rank, ping_pong_count, partnerRank);
		}
	}
}

void Ring()
{
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	int token;

	if (world_rank != 0)
	{
		MPI_Recv(&token, 1, MPI_INT, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Process %d received token %d from process %d\n", world_rank, token, world_rank - 1);
	}
	else
	{
		token = -1;
	}
	MPI_Send(&token, 1, MPI_INT, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);

	if (world_rank == 0)
	{
		MPI_Recv(&token, 1, MPI_INT, world_size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Process %d received token %d from process %d\n", world_rank, token, world_size - 1);
	}
}

void CheckStatus()
{
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	if (world_size != 2)
	{
		fprintf(stderr, "Must use two processes for this example\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	const int MAX_NUMBERS = 100;
	int numbers[MAX_NUMBERS];
	int number_amount;
	if (world_rank == 0)
	{
		srand(time(NULL)); 
		number_amount = (rand() / (float)RAND_MAX) * MAX_NUMBERS;
		// Send the amount of integers to process one
		MPI_Send(numbers, number_amount, MPI_INT, 1, 0, MPI_COMM_WORLD);
		printf("0 sent %d numbers to 1\n", number_amount);
	}
	else if (world_rank == 1) {
		MPI_Status status;
		// Receive at most MAX_NUMBERS from process zero
		MPI_Recv(numbers, MAX_NUMBERS, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		// After receiving the message, check the status to determine how many
		// numbers were actually received
		MPI_Get_count(&status, MPI_INT, &number_amount);
		// Print off the amount of numbers, and also print additional information
		// in the status object
		for (int i = 0; i < number_amount; i++)
		{
			cout << numbers[i] << " ";
		}
		cout << endl;
		printf("1 received %d numbers from 0. Message source = %d, tag = %d\n",
			number_amount, status.MPI_SOURCE, status.MPI_TAG);
	}
	MPI_Barrier(MPI_COMM_WORLD);
}

void Probe()
{
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	if (world_size != 2) {
		fprintf(stderr, "Must use two processes for this example\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	int number_amount;
	if (world_rank == 0) {
		const int MAX_NUMBERS = 100;
		int numbers[MAX_NUMBERS];
		// Pick a random amont of integers to send to process one
		srand(time(NULL));
		number_amount = (rand() / (float)RAND_MAX) * MAX_NUMBERS;
		// Send the amount of integers to process one
		MPI_Send(numbers, number_amount, MPI_INT, 1, 0, MPI_COMM_WORLD);
		printf("0 sent %d numbers to 1\n", number_amount);
	}
	else if (world_rank == 1) {
		MPI_Status status;
		// Probe for an incoming message from process zero
		MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
		// When probe returns, the status object has the size and other
		// attributes of the incoming message. Get the size of the message.
		MPI_Get_count(&status, MPI_INT, &number_amount);
		// Allocate a buffer just big enough to hold the incoming numbers
		int* number_buf = (int*)malloc(sizeof(int) * number_amount);
		// Now receive the message with the allocated buffer
		MPI_Recv(number_buf, number_amount, MPI_INT, 0, 0, MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
		printf("1 dynamically received %d numbers from 0.\n",
			number_amount);
		free(number_buf);
	}
}

// The following functions are part of the technique mentioned earlier

void decompose_domain(int domain_size, int world_rank, int world_size, int* subdomain_start, int* subdomain_size)
{
	if (world_size > domain_size)
		MPI_Abort(MPI_COMM_WORLD, 1);
	*subdomain_start = domain_size / world_size * world_rank;
	*subdomain_size = domain_size / world_size;
	if (world_rank == world_size - 1)
		*subdomain_size += domain_size % world_size;
}

void initialize_walkers(int num_walkers_per_proc, int max_walk_size, int subdomain_start, vector<Walker>* incoming_walkers)
{
	Walker walker;
	for (int i = 0; i < num_walkers_per_proc; i++)
	{
		walker.location = subdomain_start;
		walker.num_steps_left_in_walk = (rand() / (float)RAND_MAX) * max_walk_size;
		incoming_walkers->push_back(walker);
	}
}

void walk(Walker* walker, int subdomain_start, int subdomain_size, int domain_size, vector<Walker>* outgoing_walkers) {
	while (walker->num_steps_left_in_walk > 0) {
		if (walker->location == subdomain_start + subdomain_size) {
			// Take care of the case when the walker is at the end
			// of the domain by wrapping it around to the beginning
			if (walker->location == domain_size) {
				walker->location = 0;
			}
			outgoing_walkers->push_back(*walker);
			break;
		}
		else {
			walker->num_steps_left_in_walk--;
			walker->location++;
		}
	}
}

void send_outgoing_walkers(vector<Walker>* outgoing_walkers, int world_rank, int world_size) {
	// Send the data as an array of MPI_BYTEs to the next process.
	// The last process sends to process zero.
	MPI_Send((void*)outgoing_walkers->data(),
		outgoing_walkers->size() * sizeof(Walker), MPI_BYTE,
		(world_rank + 1) % world_size, 0, MPI_COMM_WORLD);

	// Clear the outgoing walkers
	outgoing_walkers->clear();
}

void receive_incoming_walkers(vector<Walker>* incoming_walkers, int world_rank, int world_size) {
	MPI_Status status;

	// Receive from the process before you. If you are process zero,
	// receive from the last process
	int incoming_rank =
		(world_rank == 0) ? world_size - 1 : world_rank - 1;
	MPI_Probe(incoming_rank, 0, MPI_COMM_WORLD, &status);

	// Resize your incoming walker buffer based on how much data is
	// being received
	int incoming_walkers_size;
	MPI_Get_count(&status, MPI_BYTE, &incoming_walkers_size);
	incoming_walkers->resize(incoming_walkers_size / sizeof(Walker));
	MPI_Recv((void*)incoming_walkers->data(), incoming_walkers_size, MPI_BYTE, incoming_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}