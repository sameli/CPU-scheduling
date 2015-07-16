/*
 ============================================================================
 Name        : CPU_scheduling.c
 Author      : S. Ameli
 Date        : 01-MAR-2014
 Description : This program reads a process list from file 'input.txt' and schedules their jobs based on
 	 	 	 	 Shortest Job First Schedulling algorithm. Then it calculates the waiting times, turnaround, averages
 	 	 	 	 and also throughput.
 Notes:      : Please note that this program seperates the CPU burst list and IO burst list into two different lists
 	 	  	   but the calculation is done by assuming if they are in sequence. for example CPU burst 1 then IO burst 1...
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

// these constant numbers are used to define maximum size of arrays
#define MAX_LISTLEN 1024
#define MAX_PROCESSLEN 100

// this is a simple struct to store a list of integers with its size
typedef struct{
	int list[MAX_LISTLEN];
	int size;
} IntList;

//ProcessInfo is a struct which stores information about one process
typedef struct{
	int processId;
	int arrivalTime;
	IntList burstList;
	int isSubmitted;
	int isCompleted;
	int burstCompletedPos; // this indicates the position of up to which cpu burst value from burstList is done
	IntList ioList;

	int waitingTime;
	int turnaroundTime;
} ProcessInfo;

// ProcessList struct stores information about all processes
typedef struct{
	ProcessInfo processList[MAX_PROCESSLEN];
	int size;
	int avgWaitingTime;
	int avgTurnaroundTime;
	int throughput;
} ProcessList;


/*
 * this is the main function. the program starts from here.
 * this method:
 * 1- read process data from 'input.txt'
 * 2- calls another method to start the cpu scheduler
 * 3- calls another method to calculates totals and averages
 */
int main(void){
	ProcessList *processList = malloc(sizeof(ProcessList));

	int isFileExist = readDataFromFile("input.txt", processList);

	if(isFileExist != 1){
		return -1;
	}
	//printList(intList);
	printf("Here is list of processes and their information that have been loaded from 'input.txt':\n");
	printProcessList(processList);

	printf("\nNow we start the CPU scheduler using Shortest Job First Schedulling algorithm: \n");
	startCPUScheduler(processList);

	//printProcessList(processList);

	calculateTotals(processList);

	printf("Finished\n");

	return EXIT_SUCCESS;
}

/*
 * This method simulates the Shortest Job First Schedulling algorithm.
 * It accepts a ProcessList pointer and then accesses its data for simulation.
 */
void startCPUScheduler(ProcessList *processList){

	/* to calculate:
	 *
	 * for each process: waiting time, turnaround time
	 * for entire system: avg turnaround time, avg waiting time, throughput
	 */

	int totalCPUburst = 0;

	// initing the first burst from the first process:
	ProcessInfo *processInfoTmp1 = &processList->processList[0];
	processInfoTmp1->isSubmitted = 1;
	totalCPUburst = processInfoTmp1->burstList.list[processInfoTmp1->burstCompletedPos];
	processInfoTmp1->burstCompletedPos++;
	processInfoTmp1->turnaroundTime = totalCPUburst - processInfoTmp1->arrivalTime;
	processInfoTmp1->waitingTime = 0;
	printf("CPU bursts are ordered based on the algorithm:\n");
	printf("%d ", totalCPUburst);


	// a while loop to simulate time progress. this loop ends when all jobs are completed
	while(1){

		// finding smallest cpt burst:
		ProcessInfo *smallestProcessInfo; // a processInfo is a structure to hold one process data
		int smallestNextCPUburst = 9999999; // this is used to store smallest next CPU burst value

		int i;
		for(i=0; i<processList->size; i++){

			//for each process:

			ProcessInfo *processInfoTmp = &processList->processList[i];

			// the 'if' below means if the position of completed cpu bursts is less than or equal to availble cpu burst
			// list that is in this process info object and isn't completed yet, then proceed to get next cpu burst and
			// check if it is the smallest
			if(processInfoTmp->burstCompletedPos <= processInfoTmp->burstList.size
					&& processInfoTmp->isCompleted != 1){
				int nextCPUburst = processInfoTmp->burstList.list[processInfoTmp->burstCompletedPos];

				if(nextCPUburst <= smallestNextCPUburst){
					smallestNextCPUburst = nextCPUburst;
					smallestProcessInfo = processInfoTmp;
				}
			}

		}


		// from here we try to submit the chosen job from smallestProcessInfo
		smallestProcessInfo->isSubmitted = 1;
		smallestProcessInfo->waitingTime = totalCPUburst - smallestProcessInfo->arrivalTime;

		int currWaitingTime = smallestProcessInfo->burstList.list[smallestProcessInfo->burstCompletedPos];
		totalCPUburst += currWaitingTime;
		printf("%d ", currWaitingTime);


		// if there is io burst add them up till smallestProcessInfo->burstCompletedPos
		// and add the result to smallestProcessInfo->turnaroundTime
		int iowaitingTime = 0;

		//printf("start processid: %d\n",smallestProcessInfo->processId);

		// here we loop in the io burst list to add up values of the io burst list to the io waiting time.
		// this will be added to the thurnaround time
		for(i=0; i<smallestProcessInfo->ioList.size; i++){
			iowaitingTime += smallestProcessInfo->ioList.list[i];
			//printf("i: %d, iowaitingTime: %d\n", i, iowaitingTime);

			// we have to break here because we don't need to get io bursts of more than our position in the
			// completed cpu bursts
			if(i == smallestProcessInfo->burstCompletedPos){
				break;
			}
		}
		//printf("end processid: %d\n",smallestProcessInfo->processId);

		smallestProcessInfo->turnaroundTime = totalCPUburst - smallestProcessInfo->arrivalTime + iowaitingTime;

		smallestProcessInfo->burstCompletedPos++;
		if(smallestProcessInfo->burstCompletedPos == smallestProcessInfo->burstList.size){
			smallestProcessInfo->isCompleted = 1;
		}

		// check if all processes are complete. if so, break this loop (exit)
		int completedCount = 0;
		for(i=0; i<processList->size; i++){

			ProcessInfo *processInfoTmp = &processList->processList[i];
			if(processInfoTmp->isCompleted == 1){
				completedCount++;
			}
		}

		//printf("completedCount: %d\n", completedCount);
		if(completedCount == processList->size){
			break;
		}
	}

	printf("\n\n");

}

/*
 * This method calculates average turnaround time, average waiting time and throughput and prints them
 */
void calculateTotals(ProcessList *processList){

	printf("These are information after the scheduling:\n");

	float avgTurnaround = 0;
	float avgWaitingTime = 0;
	float throughput = 0;
	int totalCompletedJobs = 0;
	int totalTurnaroundTime = 0;
	int i;

	int processesSize = processList->size;
	for(i=0; i<processesSize; i++){

		//for each process:
		ProcessInfo *processTmp = &processList->processList[i];
		avgTurnaround += (float)processTmp->turnaroundTime;
		avgWaitingTime += (float)processTmp->waitingTime;

		totalTurnaroundTime += processTmp->turnaroundTime;

		if(processTmp->isCompleted == 1){
			totalCompletedJobs++;
		}


		printf("information for process id #%d :\n", processTmp->processId);

		printf("\t waiting Time: %d\n", processTmp->waitingTime);
		printf("\t turnaround Time: %d\n", processTmp->turnaroundTime);

	}

	avgTurnaround = avgTurnaround / processesSize;
	avgWaitingTime = avgWaitingTime / processesSize;

	throughput = (float)totalCompletedJobs / (float)totalTurnaroundTime;

	printf("Calculating averages and totals for all processes:\n");

	printf("avgTurnaround: %f\n", avgTurnaround);
	printf("avgWaitingTime: %f\n", avgWaitingTime);
	printf("throughput = total completed jobs: %d divided by total turnaround time: %d = %f\n",
			totalCompletedJobs, totalTurnaroundTime, throughput);

}

/*
 * This method accepts a file name and loads its content into the given ProcessList structure
 */
int readDataFromFile(const char* file_name, ProcessList *processList){
	FILE* file = fopen (file_name, "r");
	int i = 0;

	if(file == NULL){
		printf("You have to provide a file named 'input.txt' \n Exiting now.\n");
		return 0;
	}

	//fscanf (file, "%d", &i);
	//printf ("%d ", i);

	IntList *intList = malloc(sizeof(IntList));

	int lineCounter = 0; // counter to count lines in the input file
	int numCounter = 0; // counter for each number in a line
	while(1){
		fscanf (file, "%d", &i);


		if(i == -99){

			//printf("\n");
			// we have a line of data now which needs to be added to processList
			ProcessInfo *processTmp = &processList->processList[lineCounter];
			processTmp->processId = intList->list[0];
			processTmp->arrivalTime = intList->list[1];

			processTmp->burstCompletedPos = 0;
			processTmp->isSubmitted = 0;
			processTmp->isCompleted = 0;

			// from intList->list[2] to end each even num is cpu burst and odd ones are io burst
			if(intList->size < 2) printf("invalid input data");

			processTmp->burstList.size = 0;
			processTmp->ioList.size = 0;

			int i;
			for(i=2; i<intList->size; i++){
				if (i%2 == 0){
					addNumToList(&processTmp->burstList, intList->list[i]); //even = cpu burst
				}else{
					addNumToList(&processTmp->ioList, intList->list[i]); //odd = io burst
				}
			}

			processList->size++;
			lineCounter++;
			emptyList(intList);
		}else{
			//processList->processList[counter].
			addNumToList(intList, i);
			//printf ("%d ", i);
		}

		numCounter++;
		if(feof (file)) break;
	}
	fclose (file);

	return 1;
}

/*
 * This method adds a number to the given list structure
 */
void addNumToList(IntList *intList, int num){
	intList->list[intList->size] = num;
	intList->size++;
}

/*
 * This method clears the given list
 */
void emptyList(IntList *intList){
	int i;
	for(i=0; i<intList->size; i++){
		intList->list[i] = 0;
	}
	intList->size = 0;
}

/*
 * This method prints the given list
 */
void printList(IntList *intList){
	int i;
	for(i=0; i<intList->size; i++){
		printf("%d ", intList->list[i]);
	}
}

/*
 * this method prints all data stored in the given ProcessList structure
 */
void printProcessList(ProcessList * processList){
	int i;
	for(i=0; i<processList->size; i++){

		//for each process:

		ProcessInfo *processTmp = &processList->processList[i];
		printf("processId #%d:\n", processTmp->processId);
		printf("\tarrivalTime: %d \n", processTmp->arrivalTime);

		IntList *burstList = &processTmp->burstList;
		printf("\tcpu burst List: ");
		int j;
		for(j=0; j<burstList->size; j++){
			printf("%d ", burstList->list[j]);
		}

		IntList *ioList = &processTmp->ioList;
		printf("\n\tio burst List: ");

		for(j=0; j<ioList->size; j++){
			printf("%d ", ioList->list[j]);
		}
		printf("\n");

		//printf("\tburstCompletedPos: %d\n", processTmp->burstCompletedPos);
		printf("\twaitingTime: %d\n", processTmp->waitingTime);
		printf("\tturnaroundTime: %d\n", processTmp->turnaroundTime);


	}

}












