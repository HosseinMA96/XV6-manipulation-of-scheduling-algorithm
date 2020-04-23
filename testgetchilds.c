#include "types.h"
#include "user.h"


 

int temp(void)
{
 int ans=0;
  for (int i=0;i<11111111;i++)
  {
	 ans++;	
	ans%=1000;
  }
 

return ans;

}

int dosomework(void)
{
 int n=0,a;

  for(int i=0;i<10;i++){
   a=temp();
   n=n+a;
	}
  
    printf(1, "%d\n",n%1000 );
  return n;

  
}

int main (void) {

    changepolicy(2);
    int n1,n2,n3,n4;

   
    n1 = fork();
	if(n1==0)    
	dosomework(); 

   // if(n1==0)
	//sleep(5);
  

    n2 = fork();
	if(n2==0)
	dosomework(); 

     //if(n2==0)
	//sleep(5);


    n3 = fork();
        if(n3==0)
	dosomework(); 

  //  if(n3==0)
	//sleep(5);

    
    n4 = fork(); 
	if(n4==0)
	dosomework(); 

   // if(n4==0)
	//sleep(5);
  
  
  

    if (n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) { 
      //  printf(1,"parent\n"); 
         getchilds(); 
	 
	 //while(waitforchilds() != -1) { }
	waitshowaverage();
    } //when a program calls waitforchilds, it's a curproc. make a system call to run waitforchilds

    /* wait for all child to terminate */
    while(wait() != -1) { }

    /* give time to parent to reach wait clause */
   // sleep(10);

    exit();
}
