#include "types.h"
#include "user.h"


int main()
{


	int n[5];
	
	changequantum();
	changequantum();
	changequantum();

	n[0]=fork();
	n[1]=fork();
	n[2]=fork();
	n[3]=fork();
	n[4]=fork();
	
	
  

	if(n[0]>0 && n[1]==0 && n[2]==0 && n[3]==0 && n[4]==0)
	{
	//   changepolicy();
	//   changepolicy();
	}
	   
	
	if(n[0]==0 && n[1]>0 && n[2]==0 && n[3]==0 && n[4]==0)
	{
	//   changepolicy();
	//   changepolicy();

	}

	if(n[0]==0 && n[1]==0 && n[2]>0 && n[3]==0 && n[4]==0)
	{
	//   changepolicy();
	//   changeCurrentPriority(3);
	   
	}

	if(n[0]==0 && n[1]==0 && n[2]==0 && n[3]>0 && n[4]==0)
	{
	  // changepolicy();
	  // changeCurrentPriority(2);
	}

	if(n[0]==0 && n[1]==0 && n[2]==0 && n[3]==0 && n[4]>0)
	{
	//   changepolicy();
	 //  changeCurrentPriority(1);
	}

	//do some work
	//for(int i=0;i<100;i++)
	//s	printf(1, "%d\n",i);

	waitshowaverage();
	
	if(n[0]>0 && n[1]>0 && n[2]>0 && n[3]>0 && n[4]>0)
		result();

	exit();
	
	



}


	
	
	
		

	
	

