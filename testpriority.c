#include "types.h"
#include "user.h"


int main()
{
	//quantum=8
	changequantum(0);
	changequantum(0);
	changequantum(0);

	int n[5];

	n[0]=fork();
	n[1]=fork();
	n[2]=fork();
	n[3]=fork();
	n[4]=fork();

	if(n[0]>0 && n[1]==0 && n[2]==0 && n[3]==0 && n[4]==0)
		changeCurrentPriority(1);
	
	if(n[0]==0 && n[1]>0 && n[2]==0 && n[3]==0 && n[4]==0)
		changeCurrentPriority(3);

	if(n[0]==0 && n[1]==0 && n[2]>0 && n[3]==0 && n[4]==0)
		changeCurrentPriority(5);

	if(n[0]==0 && n[1]==0 && n[2]==0 && n[3]>0 && n[4]==0)
		changeCurrentPriority(7);

	if(n[0]==0 && n[1]==0 && n[2]==0 && n[3]==0 && n[4]>0)
		changeCurrentPriority(9);

	//do some work
	for(int i=0;i<100;i++)
		printf(1, "%d\n",i);

	waitshowaverage();
	
	if(n[0]>0 && n[1]>0 && n[2]>0 && n[3]>0 && n[4]>0)
		result();

	exit();
	
	



}


	
	
	
		

	
	

