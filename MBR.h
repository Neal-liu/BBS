/**************************************************************************
*	MBR.h
*
*	AUTHOR : Hung, Liu - original code
*
*	PURPOSE : build Rtree with MBR to compute skyline by BBS
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
#include<stdlib.h>
#include<stdbool.h>

#define DIM 2
#define SIDES_NUM 2*DIM
#define MAXENTRY 5
#define FILENAME_INPUT "assignment-1-input.txt"
#define FILENAME_OUTPUT "assignment-1-output.txt"

/* typedef float REALTYPE; */
typedef double REALTYPE;

typedef struct _MBR
{
    REALTYPE bound[SIDES_NUM];	/* xmin,ymin,...,xmax,ymax,... */
    int DataIndex[MAXENTRY+1];
}MBR;

typedef struct _MBRBRANCH
{
	bool isNode;					/* it have children or not */
    MBR mbr;
    struct _MBRBRANCH *next1;		/* mbr's child */
    struct _MBRBRANCH *next2;
}MBRBRANCH;


void RectInsertData(MBRBRANCH *, float *, int);
void InitMbr(MBRBRANCH *);
void RectSeed(MBRBRANCH *, int);
void FarDist(float *, int *, int *);
void RectSplit(MBRBRANCH *, int, int);
MBRBRANCH *CreateNode(int);
void GetLineData(int, float *);
int RectChoose(MBRBRANCH *);
void PrintMBR(MBRBRANCH *, int);
int **DataSet = NULL;
int *datasetx = NULL;


/* insert new data to MBR */
void RectInsertData(MBRBRANCH *nd, float *NewData, int CurrentLine)			/* nd means new data */
{
	int rect=0, i;
	if(!nd->isNode){
		/* calculate maximum and minimum bounds */
		if(nd->mbr.bound[0] == 0){
			for(i=0 ; i < DIM ; i++){
				nd->mbr.bound[i+DIM] = nd->mbr.bound[i] = NewData[i];
			}
		}
		else{
			for(i=0 ; i < DIM ; i++){
				if(nd->mbr.bound[i] > NewData[i])
					nd->mbr.bound[i] = NewData[i];
				if(nd->mbr.bound[i+DIM] < NewData[i])
					nd->mbr.bound[i+DIM] = NewData[i];
			}
		}

		for(i=0 ; i < MAXENTRY+1 ; i++){
			if(nd->mbr.DataIndex[i] == 0){
				nd->mbr.DataIndex[i] = CurrentLine;

				if(i == MAXENTRY){
					RectSeed(nd, CurrentLine);
					nd->mbr.DataIndex[i] = 0;
				}
				break;
			}
		}
	}
	else{
		rect = RectChoose(nd);

		if(rect == 1)
			RectInsertData(nd->next1, &NewData[0], CurrentLine);
		else if(rect == 2)
			RectInsertData(nd->next2, &NewData[0], CurrentLine);
		else{
			printf("Error for insert data\n");
			exit(1);
		}
	}
}
/* Choose the less points MBR to insert the new data. */
int RectChoose(MBRBRANCH *nd)
{
	int indexSize1=0, indexSize2=0, i;
	for(i=0 ; i < MAXENTRY+1 ; i++){
		if(nd->next1->mbr.DataIndex[i] > 0)
			indexSize1++;
		if(nd->next2->mbr.DataIndex[i] > 0)
			indexSize2++;
	}

	if(indexSize2 >= indexSize1)
		return 1;
	else
		return 2;

}

/* MBR is already full, so we need to split it. */
void RectSeed(MBRBRANCH *nd, int OverflowLine)
{
	int SizeTemp = DIM*(MAXENTRY+1);
	float f[50] = {0};
	float temp[SizeTemp];				//temporary
	int temp_k = 0, i, j, a;
	int far1, far2; 					//far line index
	int line1, line2;					//far line

	/* initialize temp[] */
	for(i=0 ; i < SizeTemp ; i++)
		temp[i] = 0;


	/* read the line number and get the correct data to calculate */
	for(i=0 ; i < MAXENTRY ; i++){
		for( temp_k=0 ; temp_k < SizeTemp ; temp_k=temp_k+DIM){
			if(temp[temp_k] == 0)
				break;
		}
		a = temp_k;
		GetLineData(nd->mbr.DataIndex[i], &temp[a]);
	}

	/* read overflowline data */
		GetLineData(OverflowLine, &temp[SizeTemp-2]);

	FarDist(&temp[0], &far1, &far2);
//	printf("\nfar1 = %d, far2 = %d\n", far1, far2);

	line1 = nd->mbr.DataIndex[far1-1];
	line2 = nd->mbr.DataIndex[far2-1];
	RectSplit(nd, line1, line2);

}

/* find two points are the farest distance in this rectangle. */
void FarDist(float *temp, int *far1, int *far2)
{
	int SizeTemp = DIM*(MAXENTRY+1) ;
	float td[DIM], distance, farD = 0 ;
	int unCal = 0, neCal = -1, count=0, i, j;

	for(i=0 ; i<SizeTemp ; i++)
	{
		td[i%DIM] = ( temp[i] - temp[i+DIM+unCal] ) * ( temp[i] - temp[i+DIM+unCal] );

		if( (i+1) % DIM == 0 ){
			distance = 0;
			for(j=0; j<DIM ; j++)
				 distance += td[j];

			if(distance > farD){
				farD = distance;
				*far1 = (i/DIM) + 1;
				*far2 = ( (i+DIM+unCal)/DIM ) + 1;
			}

			if( (unCal+DIM+neCal+1) != (DIM*(MAXENTRY)) ){
				i = neCal;
				unCal += DIM;
			}
			else{
				unCal = 0;
				neCal += DIM;
				i = neCal;
				count++;
				if(count == MAXENTRY)
					break;
			}
		}
	}
}
/* One MBR split it into two, depends on two seeds far1 and far2,
and allocate others points to one. */
void RectSplit(MBRBRANCH *nd, int far1, int far2)
{
	FILE *fptr = fopen(FILENAME_INPUT,"r");
	int i,j, k=0, Far;
	float temps[2*DIM]; 			// seeds
	float f[DIM]= {0};
	float lineData[DIM], dis[2]={0};


	/* root node connect with leaf nodes */
	nd->isNode = true;
	MBRBRANCH *a = CreateNode(far1);
	MBRBRANCH *b = CreateNode(far2);
	a->next2 = a->next1 = nd->next1;
	nd->next1 = a;
	b->next2 = b->next2 = nd->next2;
	nd->next2 = b;

	if(far1 > far2)
		Far = far1;
	else Far = far2;

	for(i=0 ; i < Far ; i++){
		for(j=0 ; j < DIM ; j++){
			fscanf(fptr, "%f", &f[j]);

			if( (i+1) == far1 ){
				temps[k++] = f[j];
		//		printf("\nseed is %f", f[j]);
			}
			else if( (i+1) == far2 ){
				temps[k++] = f[j];
		//		printf("\nseed is %f", f[j]);
			}
			else
				continue;
		}
	}

	for(i=0 ; i < MAXENTRY+1 ; i++){
		if( (nd->mbr.DataIndex[i] != far1) && (nd->mbr.DataIndex[i] != far2) ){
			GetLineData(nd->mbr.DataIndex[i], &lineData[0]);
			dis[0] = 0;
			dis[1] = 0;

			for(j=0 ; j<DIM ; j++){
				dis[0] += (lineData[j]-temps[j])*(lineData[j]-temps[j]);
				dis[1] += (lineData[j]-temps[j+DIM])*(lineData[j]-temps[j+DIM]);
			}
			if(dis[0] <= dis[1])
				RectInsertData(a, &lineData[0], nd->mbr.DataIndex[i]);
			else
				RectInsertData(b, &lineData[0], nd->mbr.DataIndex[i]);
		}
	}

	fclose(fptr);
}
/* Give line number and ouput line data. */
void GetLineData(int line, float *lineData)
{
	int i, j;
	float temps[DIM];
	for(j=0; j<DIM ; j++){
		lineData[j] = DataSet[line-1][j];
	}
}

/* create new node for splitting old node */
MBRBRANCH *CreateNode(int far)
{
	int i, j;
	float f[DIM];
	MBRBRANCH *mbrNew = (MBRBRANCH *) malloc(sizeof(MBRBRANCH));

	InitMbr(mbrNew);
	GetLineData(far, &f[0]);
	RectInsertData(mbrNew, &f[0], far);

	return mbrNew;
}

/* initialize the MBR  */
void InitMbr(MBRBRANCH *init)
{
    int i;

	init->next1 = NULL;
	init->next2 = NULL;
	init->isNode = false;
	for(i=0 ; i < SIDES_NUM ; i++){
		init->mbr.bound[i] = 0;
	}
	for(i=0 ; i < MAXENTRY+1 ; i++){
		init->mbr.DataIndex[i] = 0;
	}
}

void PrintMBR(MBRBRANCH *nd, int level)
{
	int i;
	if(nd->isNode){
		printf("%d [ N ]\n", level);
	}

	printf("%d [ ", level);
	for(i=0; i<MAXENTRY+1 ; i++){
		printf(" %d ", nd->mbr.DataIndex[i]);
	}
	printf("]\n");

	if(nd->next1 != NULL){
		PrintMBR(nd->next1, level+1);
	}
	if(nd->next2 != NULL){
		PrintMBR(nd->next2, level+1);
	}
}



