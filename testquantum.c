#include "types.h"
#include "user.h"


 

int temp(void)
{
 int ans=0;
  for (int i=0;i<1111111;i++)
  {
	for(int j=0;j<11111111;j++)
		for(int q=0;q<11111111;q++)
		ans++;
	 

     ans%=1000;
  }
 

return ans;

}

int dosomework(int k)
{
 int n=0,a;

  for(int i=0;i<k*k+2;i++){
   a=temp();
   n=n+a;
	}
  
    //printf(1, "%d\n",n%1000 );
  return n;

}


int pow2(int k)
{
  int ans=1;

  for (int i=0;i<k;i++)
	ans*=2;

  return ans;
}

int main()
{
      changepolicy(2);
	
    

  
   
   int n[10];
	
   for (int i=0;i<10;i++)
   {
	n[i]=fork();
	
	if(n[i]==0)
	dosomework(i);
	
	
	waitforchilds();
    }

    int tell=1;

    for (int i=0;i<10;i++)
    	if(n[i]==0)
	{
		tell=0;
		break;
	}
	

	
    if(tell==1)	
    {
	while(wait() != -1)
	{}
	
	
	result();
	  
    }
    
  
  
    exit();
}
