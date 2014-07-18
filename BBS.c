/**************************************************************************
*	BBS.c
*
*	AUTHOR : Hung, Liu - original code
*
*	PURPOSE : compute Skyline with BBS algorithm
*
*	COPYRIGHT : (C) 2014 by the Neal Development Team
*
*	Implement BBS Algorithm with R tree in MBR.h. It's need to modify 
*	dimensions "DIM", size of MBR "MAXENTRY", and file name "FILENAME" 
*	in MBR.h. Data size divided by 100 is MAXENTRY, if size is more than 
*	1,000. 
*
*	DATE : 07/18/2014 by Neal, Liu (hunter0hunter04@hotmail.com.tw)
***************************************************************************/

#include<stdio.h>
#include"MBR.h"
#define STACK_SIZE TotalLine
#define STACK_EMPTY -1
#define SKYLINE_SIZE TotalLine*DIM

void ReadData(MBRBRANCH *);
void BBS(MBRBRANCH *);
void push(MBRBRANCH *[], MBRBRANCH *, int *);
void pop(int *);
int CheckSkyline(float [], MBRBRANCH *);
void InitSkyline(float *);
int TotalLine = 0;


int main(void)
{
	MBRBRANCH *mbr = (MBRBRANCH *) malloc(sizeof(MBRBRANCH));

    InitMbr(mbr);
    ReadData(mbr);	
    PrintMBR(mbr, 0);
    BBS(mbr);

    return 0;
}

/* Read the input file. */
void ReadData(MBRBRANCH *mbr)
{
	FILE *fp = fopen(FILENAME_INPUT,"r");
	float data[DIM]; int line = 1, i, j;
	
	if(fp == NULL)
    	printf("File does not exist\n");
    else{
    	while(feof(fp) == 0){
	   		for(i=0; i < DIM; i++){
	   			fscanf(fp, "%f", &data[i]);
	    		if(i == DIM-1)
	   				TotalLine++;
	   		}
    	}
    }

    rewind(fp);
    DataSet = (int **)malloc(TotalLine*sizeof(int *));
	datasetx = (int *)malloc(TotalLine*DIM*sizeof(int));

   	for(i=0; i < TotalLine; i++){
   		DataSet[i] = datasetx;
		datasetx += DIM;
   		for(j=0; j < DIM; j++){
   			fscanf(fp, "%f", &data[j]);
   			DataSet[i][j] = data[j];
   			if(j == DIM-1){
				RectInsertData(mbr, &data[0], line);
	  			line++;
	  		}
	//		printf("%d ", DataSet[i][j]);		
	  	}	
	//	printf("\n");
	}	

    fclose(fp);
}

/* Implement BBS algorithm */
void BBS(MBRBRANCH *mbr)
{
	FILE *fp = fopen(FILENAME_OUTPUT,"w");
	float skyline[TotalLine*DIM];					// up to 50 skyline data.
	float MiniBound1 = 0, MiniBound2 = 0;
	int s_top = STACK_EMPTY;						// stack top intialize as empty.
	int i, x=0, k=0;
	MBRBRANCH *queue[STACK_SIZE];				// Queue
	MBRBRANCH *temp;

	InitSkyline(&skyline[0]);
	push(queue, mbr, &s_top);

	while(queue != NULL){
		temp = queue[s_top];

		if(CheckSkyline(&skyline[0],temp) == -1){
			printf("dominates \n");
			pop(&s_top);
			if(s_top == STACK_EMPTY)
				break;
			else
				continue;
		}

		/* Calculate the minimum distance from origin, and sort it 
			from large to small into Queue. */
		if(queue[s_top]->isNode){
			pop(&s_top);

			for(i=0 ; i<DIM ; i++){
				MiniBound1 += temp->next1->mbr.bound[i];
				MiniBound2 += temp->next2->mbr.bound[i];
			}
			if(MiniBound2 >= MiniBound1){
				push(queue, temp->next2, &s_top);
				push(queue, temp->next1, &s_top);
	//			printf("%f, push next2\n", MiniBound2);
			}
			else{
				push(queue, temp->next1, &s_top);
				push(queue, temp->next2, &s_top);
	//			printf("%f, push next1\n", MiniBound1);
			}
		}
		/* It's only one index in a mbr, it means 
		this mbr also means point. */
		else if(queue[s_top]->mbr.DataIndex[1] == 0){
			printf("skyline is %d\n", queue[s_top]->mbr.DataIndex[0]);
			GetLineData(queue[s_top]->mbr.DataIndex[0], &skyline[k]);
			pop(&s_top);
			if(s_top == STACK_EMPTY)
				break;

			k+= DIM;

		}
		/* It's a rectangle, and we need to split
		 and push data into Queue. */
		else{
			pop(&s_top);
			for(i=0 ; i<MAXENTRY ; i++){
				if(temp->mbr.DataIndex[i] != 0){
					MBRBRANCH *a = CreateNode(temp->mbr.DataIndex[i]);
			//		printf("index %d\n", a->mbr.DataIndex[0]);
					push(queue, a, &s_top);
				}
			}
		}
	}
	printf("Skyline are :\n");
	for(i=0 ; i < SKYLINE_SIZE ; i++){
		if(skyline[i] != 0){
			fprintf(fp, "%f ", skyline[i]);	
			printf("%f ", skyline[i]);	
		}
		if((i+1)%DIM == 0){
			fprintf(fp,"\n");
			printf("\n");
		}
		if(skyline[i] == 0 && skyline[i+1] == 0)
			break;
	}

	fclose(fp);
}
/* push mbr or data into Queue. */
void push(MBRBRANCH *queue[], MBRBRANCH *mbr, int *top)
{
	int i, TopQ;
	float Btop = 0, Bdown = 0;
	MBRBRANCH *temp;
	printf("push %d\n", *top);
	if((*top) >= STACK_SIZE-1){
		printf("stack is full !\n");
		return ;
	}
	(*top)++;
	TopQ = *top;
	queue[*top] = mbr;
	printf("topQ = %d\n", TopQ);
	while(TopQ != 0){
		Btop = Bdown = 0;
		for(i=0 ; i<DIM ; i++){
			Btop += queue[TopQ]->mbr.bound[i];
			Bdown += queue[TopQ-1]->mbr.bound[i];
		}
		if(Btop > Bdown){
			temp = queue[TopQ];
			queue[TopQ] = queue[TopQ-1];
			queue[TopQ-1] = temp;
			TopQ--;
		}
		else break;
	}

}
/* pop data or mbr from Queue. */
void pop(int *top)
{
	printf("pop %d\n", *top);
	(*top)--;
}

/* Check skyline result can or can not 
		dominates other objects in Queue. */
int CheckSkyline(float *skyline, MBRBRANCH *temp)
{
	int i, j, Dom;

	if(skyline[0] == 0)
		return 0;

	for(i=0; i<TotalLine ; i++){
		Dom = 0;
		for(j=0 ; j<DIM ; j++){
	//		printf("temp is %f , sky is %f \n", temp->mbr.bound[j], skyline[i*DIM + j]);
			if(temp->mbr.bound[j] > skyline[i*DIM + j] ){
				Dom++;
			}
			if(Dom == DIM)
				return -1;
			if(skyline[i*DIM + j] == 0)
				return 0;
		}
	}
	return 0;
}


void InitSkyline(float *skyline)
{
	int i, j ;
	for(i=0 ; i<SKYLINE_SIZE ; i++)
		skyline[i] = 0;
	for(i-0 ; i<STACK_SIZE ; i++){
		for(j=0 ; j<DIM ; j++)
			DataSet[i][j] = 0;
	}
}