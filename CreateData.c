#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define DATASIZE 50000 

int main()
{
	FILE *fp = fopen("input-50000.txt","w");
	int i, j;
	float a, b, c;
	int ran;
	if(DATASIZE >= 100)
	 	ran = DATASIZE/10;
	 else
	 	ran = DATASIZE;
	srand(time(NULL));
	for(i = 0 ; i < DATASIZE ; i++){
		a = (rand()%ran+1);
		b = (rand()%ran+1);
	//	c = (rand()%ran+1);
		fprintf(fp, "%f %f\n", a, b);
		printf("%f %f\n", a, b);
	}
	printf("total = %d\n", DATASIZE);
	fclose(fp);
	return 0;
}


