#include "types.h"
#include "user.h"


int main()
{


	int n[5];
	
	//first, set quantum to 8
	changequantum();
	changequantum();
	changequantum();

	n[0]=fork();
	n[1]=fork();
	n[2]=fork();
	n[3]=fork();
	n[4]=fork();
	
	
  	//Since each process is initialized to be at queue number 2 (Default algorithm), and each changepolicy() call increments the queue number in a circular manner (that is 1->2->3->1), hence, two consequtive changepolicy() calls sets current process' queue to number 1 whichi is priority queue, and a signle changepolicy() call sets a process' queue from default queue to default with quantum


	//lets have 4 process with quantum policy	
	if(n[0]>0 && n[1]==0 && n[2]==0 && n[3]==0 && n[4]==0)
	{
	   changepolicy();
	   changepolicy();
	}
	   
	
	if(n[0]==0 && n[1]>0 && n[2]==0 && n[3]==0 && n[4]==0)
	{
	   changepolicy();
	   changepolicy();

	}

	if(n[0]==0 && n[1]==0 && n[2]>0 && n[3]==0 && n[4]==0)
	{
	   changepolicy();
	   changepolicy();

	}

	if(n[0]==0 && n[1]==0 && n[2]==0 && n[3]>0 && n[4]==0)
	{
	   changepolicy();
	   changepolicy();
	}
	
	//lets have 10 process with priority policies and different priorityies
	if(n[0]==0 && n[1]==0 && n[2]==0 && n[3]>0 && n[4]>0)
	{
	   changepolicy();
	   changeCurrentPriority(1);
	}
	
	if(n[0]==0 && n[1]==0 && n[2]>0 && n[3]>0 && n[4]==0)
	{
	   changepolicy();
	   changeCurrentPriority(3);
	}

	
	if(n[0]>0 && n[1]>0 && n[2]==0 && n[3]==0 && n[4]==0)
	{
	   changepolicy();
	   changeCurrentPriority(5);
	}

	if(n[0]>0 && n[1]==0 && n[2]==0 && n[3]==0 && n[4]>0)
	{
	   changepolicy();
	   changeCurrentPriority(7);
	}

	if(n[0]>0 && n[1]==0 && n[2]==0 && n[3]>0 && n[4]==0)
	{
	   changepolicy();
	   changeCurrentPriority(9);
	}

	if(n[0]>0 && n[1]==0 && n[2]>0 && n[3]==0 && n[4]==0)
	{
	   changepolicy();
	   changeCurrentPriority(11);
	}

	if(n[0]==0 && n[1]>0 && n[2]==0 && n[3]==0 && n[4]>0)
	{
	   changepolicy();
	   changeCurrentPriority(13);
	}

	if(n[0]>0 && n[1]>0 && n[2]==0 && n[3]>0 && n[4]==0)
	{
	   changepolicy();
	   changeCurrentPriority(15);
	}

	if(n[0]==0 && n[1]==0 && n[2]>0 && n[3]>0 && n[4]>0)
	{
	   changepolicy();
	   changeCurrentPriority(17);
	}
	
	//Do some work
	for (int i=0;i<50;i++)
	printf(1, "%d\n", i);


	waitshowaverage();
	
	if(n[0]>0 && n[1]>0 && n[2]>0 && n[3]>0 && n[4]>0)
		result();

	exit();
	
	



}


	
	
	
		

	
	

