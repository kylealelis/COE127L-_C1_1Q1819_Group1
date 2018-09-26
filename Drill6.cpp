#include <iostream>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <queue>
#include <iostream>
#include <math.h>

using namespace std;

struct DiskQueue {
	int request;
	int headStart;
	DiskQueue *next;
};

bool initDiskQueue(DiskQueue *&,int *,int, int *);
bool deleteQueue(DiskQueue *&);
bool doFCFS(DiskQueue *&);
bool doSSTF(DiskQueue *&);

int main()
{
	DiskQueue *queueStart;
	int queue[8]={98,153,37,122,14,124,65,67};
	int head = 53;
	cout<<"Note: This example is taken from the book for comparison purposes -> Operating System Concepts 7th Ed. \n";
	cout<<"\nHead Start: 53\n";
	cout<<"Disk Queue: 98 153 37 122 14 124 65 67\n";
	initDiskQueue(queueStart,(int *)&queue,(sizeof(queue)/sizeof(int)),(int *)&head);
	printf("\nFirst-Come First-Served Scheduling (FCFS) Algorithm\n");
	doFCFS(queueStart);
	initDiskQueue(queueStart,(int *)&queue,(sizeof(queue)/sizeof(int)),(int *)&head);
	printf("\nShortest Seek Time First (SSTF) Scheduling Algorithm\n");
	doSSTF(queueStart);
	system("pause");
	system("cls");
	return 0;
 } 
 
 bool deleteQueue(DiskQueue *& queueStart)
 {
 	DiskQueue *queueDel = NULL;
 	do{
 		queueDel = queueStart;
 		queueStart = queueStart ->next;
 		delete queueDel;
	 }while (queueStart != NULL);
	 return true;
 }
 
 bool initDiskQueue(DiskQueue*& queueStart, int *req, int nReq, int *headS){
 	DiskQueue* queues = new DiskQueue;
 	int j = 0;
 	memset(queues,0,sizeof(DiskQueue));
 	queueStart = queues;
 	for (int i =0;i<nReq;i+=1){
 		queues->next = new DiskQueue;
 		memset(queues->next,0,sizeof(DiskQueue));
 		queues = queues->next;
 		queues->request = *(req+i);
 		queues->headStart = *(headS);
 		queues->next = NULL;
	 }
	 return true;
 }
