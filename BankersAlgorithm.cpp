#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

int noOfProcess,noOfResources;
int *resourcesCount;
int *safeSequence;
int **allocatedResourcesCount;
int **maxRequiredResoucesCount;
int **needResourcesCount;
int noOfProcessComplete = 0;
pthread_mutex_t lock;
pthread_cond_t cond;

void* processCode(void* );
void calculateNeedResources();
bool getSafeProcessSequence(int *,int **,int **);

//Main function
int main(){
	printf("\nEnter Number Of Process Count: ");
	scanf("%d",&noOfProcess);
	
	printf("\nEnter Number Of Resources Count: ");
	scanf("%d",&noOfResources);
	
	resourcesCount = (int *)malloc(noOfResources * sizeof(*resourcesCount));
	printf("\nEnter Currently Available resources Count(R1 R2 R3...)? ");
    for(int i=0; i<noOfResources; i++)
        scanf("%d", &resourcesCount[i]);
    
	maxRequiredResoucesCount = (int **)malloc(noOfProcess * sizeof(*maxRequiredResoucesCount));
	for(int i=0; i<noOfProcess; i++)
		maxRequiredResoucesCount[i] = (int *)malloc(noOfResources * sizeof(**maxRequiredResoucesCount));
	
    for(int i=0; i<noOfProcess; i++){
		printf("\nEnter Maximum resource required by process %d (R1 R2 R3...)? ", i+1);
		for(int j=0; j<noOfResources; j++)
			scanf("%d", &maxRequiredResoucesCount[i][j]);
    }
	printf("\n");
		    
    allocatedResourcesCount = (int **)malloc(noOfProcess * sizeof(*allocatedResourcesCount));
    for(int i=0; i<noOfProcess; i++)
		allocatedResourcesCount[i] = (int *)malloc(noOfResources * sizeof(**allocatedResourcesCount));
	
    for(int i=0; i<noOfProcess; i++) {
        printf("\nEnter Resource allocated to process %d (R1 R2 R3...)? ", i+1);
        for(int j=0; j<noOfResources; j++)
            scanf("%d", &allocatedResourcesCount[i][j]);
    }
    printf("\n");	
	calculateNeedResources();    
	safeSequence = (int *)malloc(noOfProcess * sizeof(*safeSequence));
	int safeOrNot = getSafeProcessSequence(resourcesCount,allocatedResourcesCount,needResourcesCount);
	if(safeOrNot){
		printf("Safe Sequence :");
		for(int i=0; i<noOfProcess; i++){
			printf("%d ",safeSequence[i]);
		}
	}
	
	printf("\nRunning Processes...\n\n"); 
	pthread_t processes[noOfProcess];
    	pthread_attr_t attribute;
    	pthread_attr_init(&attribute);

	int processNumber[noOfProcess];
	for(int i=0; i<noOfProcess; i++)
		 processNumber[i] = i;

    for(int i=0; i<noOfProcess; i++)
        pthread_create(&processes[i], &attribute, processCode, (void *)(&processNumber[i]));

    for(int i=0; i<noOfProcess; i++)
        pthread_join(processes[i], NULL);

    printf("\nAll Process Finished");	
}

//Calculate the need request based on max resource needed and allocated resource
void calculateNeedResources(){
	needResourcesCount = (int **)malloc(noOfProcess * sizeof(*needResourcesCount));
    for(int i=0; i<noOfProcess; i++)
        needResourcesCount[i] = (int *)malloc(noOfResources * sizeof(**needResourcesCount));

    for(int i=0; i<noOfProcess; i++)
        for(int j=0; j<noOfResources; j++)
            needResourcesCount[i][j] = maxRequiredResoucesCount[i][j] - allocatedResourcesCount[i][j];
}

//Find safe sequence using banker's algorithm.
bool getSafeProcessSequence(int *resourcesCount,int **allocatedResourcesCount,int **needResourcesCount) {
    int temporaryResources[noOfResources];
    for(int i=0; i<noOfResources; i++)
		temporaryResources[i] = resourcesCount[i];	
    bool finished[noOfProcess];
    for(int i=0; i<noOfProcess; i++)
		finished[i] = false;
    int noOfFinished=0;
    while(noOfFinished < noOfProcess) {
        bool safeState = false;
        for(int i=0; i<noOfProcess; i++) {
            if(!finished[i]) {
                bool possible = true;
                for(int j=0; j<noOfResources; j++){				
                    if(needResourcesCount[i][j] > temporaryResources[j]){
                        possible = false;
                        break;
                    }
                }
                if(possible){
                    for(int j=0; j<noOfResources; j++){
                        temporaryResources[j] += allocatedResourcesCount[i][j];
                    }
                    safeSequence[noOfFinished] = i;
                    finished[i] = true;
                    noOfFinished=+1;
                    safeState = true;
                }
            }
        }
        if(!safeState){
            for(int k=0; k<noOfProcess; k++)
				safeSequence[k] = -1;
            return false;
        }
	}
    return true;
}

//Simulate the execution of process using multithreading
void* processCode(void *arg) {
    int p = *((int *) arg);
    pthread_mutex_lock(&lock);
    
    while(p != safeSequence[noOfProcessComplete]){
        pthread_cond_wait(&cond, &lock);
    }

    printf("\n--> Process %d", p+1);
    printf("\n\tAllocated : ");
    for(int i=0; i<noOfResources; i++)
        printf("%3d", allocatedResourcesCount[p][i]);

    printf("\n\tNeeded    : ");
    for(int i=0; i<noOfResources; i++)
        printf("%3d", needResourcesCount[p][i]);

    printf("\n\tAvailable : ");
    for(int i=0; i<noOfResources; i++)
        printf("%3d", resourcesCount[i]);

    printf("\n"); 
    printf("\Allocating Resource...");
	sleep(1);
    printf("\tResource Allocated!");
    printf("\n");
	sleep(1);
    printf("Process Running...");
    printf("\n");
	sleep(rand()%3);
    printf("Process Completed...");
    printf("\n");
	sleep(1);
    printf("Releasing Resource...");
    printf("\n");
	sleep(1);
    printf("Resources Released!");

	for(int i=0; i<noOfResources; i++)
        resourcesCount[i] += allocatedResourcesCount[p][i];

    printf("\nNow Available : ");
    for(int i=0; i<noOfResources; i++)
        printf("%3d", resourcesCount[i]);
    printf("\n\n");
    sleep(1);

    noOfProcessComplete+=1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}
