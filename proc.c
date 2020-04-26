#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"



struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);
int quantum=1,counter=0,violate=0,firstTime=1,keepid=-1,avgWT=0,avgSLP=0,avgTRT=0,avgRNT=0,childs=0,lastPid,index=0,QQQ=1;
int record[10000][5]; //An array to record waiting time,termination time, priority and process idfor finished processes so that we can see the impact of priority on waiting time

//hold last processes of default algorithm and default algorithm + quantum so that next time on these queues, we know which process to run next

struct proc *p1;
struct proc *p0;
struct proc *p2;
struct proc *highP;
int first1=1,first2=1,l1=0,l2=0;


static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
 //Initialize the process attributes
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->priority=10;
  p->cpriority=0;
  p->creationTime=ticks;
  p->queue=2;



  p->sleepingTime=0;
	
  p->waitingTime=0;
  p->runningTime=0;
  p->lastSLeepTick=0;



  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{

  struct proc *curproc = myproc();
  //struct proc *p2=curproc;

  //record
  //record[index][0]=p2->pid;
  //record[index][1]=p2->waitingTime;
  //record[index++][2]=p2->priority;

 
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  curproc->terminationTime=ticks;

  
 
  

  
  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  
  panic("zombie exit");
 
}


	

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

int
waitforchilds(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids++;

      

      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
	//c=pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
	record[index][0]=p->pid;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);

//	cprintf("%s%d%s%d%s%d%s%d%s%d%s%d%s%d\n\n","This process had a child with id : ",c," creation tick: ",p->creationTime," termination tick: ",p->terminationTime," waitingTicks/QUANTUM : ",p->waitingTime/QUANTUM," runningTicks/QUANTUM : ",p->runningTime/QUANTUM," waitingTicks/QUANTUM : ",p->waitingTime/QUANTUM," sleepingTicks/QUANTUM : ",p->sleepingTime/QUANTUM);
	
	
	int dif=p->terminationTime-p->creationTime;
	
       // avgWT+=p->waitingTime;
	//cprintf("%d\n",avgWT);
	
	//avgSLP+=p->sleepingTime;
	avgTRT+=dif;
	//avgRNT+=p->runningTime;
	
	//childs++;
	//cprintf("%d\n",childs);
	//record
 	
  	record[index][1]=p->waitingTime;
  	record[index][2]=p->priority;
	record[index][3]=p->terminationTime;
	record[index++][4]=p->queue;
	//p2->terminationTime=ticks;

//	cprintf("%s%d\t","Quantum : ",QUANTUM);
	//cprintf("%s%d\t","Waiting ticks : ",p->waitingTime);
	//cprintf("%s%d\t","Sleeping ticks : ",p->sleepingTime);
	//cprintf("%s%d\t","Creation Time : ",p->creationTime);	
	//cprintf("%s%d\t","Termination time : ",p->terminationTime);
	//cprintf("%s%d\t","Running time : ",p->runningTime);
//	cprintf("%s%d\t","zombie  : ",p->zombieTime);
	//cprintf("%s%d\t","Turnaround time : ",dif);	
	//cprintf("%s%d\t","sum  : ",sum);	
	//cprintf("%s%d\t","\n\n");
        return pid;
      }
    }
	
  //  cprintf("%s%d\n","\nhaveKids=  ",havekids);
    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
    //  cprintf("%s\n","I'm quiting");
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
	return 2;
}


//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  int empty0=1,empty1=1,empty2=1;

  
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.

    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);


    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state != RUNNABLE)
           continue;
	
	
	//cprintf("%s%d\n\n","My queue is =",p->queue);
	//int aktiv=0;

	//for(p0 = ptable.proc; p0 < &ptable.proc[NPROC]; p0++)
	//    if(p0->pid>=1)
	//	aktiv++;

	
	//cprintf("%s%d\n","aktive=",aktiv);
//	cprintf("%s%d\n","Starting Q=",QQQ);
	

        empty0=1;
	empty1=1;
	empty2=1;
	
	if(QQQ==1)
	{
	 //s   struct proc *temp;
	
            
	    for(highP = ptable.proc; highP < &ptable.proc[NPROC]; highP++)
      			if((highP->state == RUNNABLE) && (highP->queue==1))
			{
			
				empty0=0;
				break;
			}

         
	    if(empty0==0)
		{
	
		   for(p0 = ptable.proc; p0 < &ptable.proc[NPROC]; p0++)
		     if(p0->state == RUNNABLE && p0->queue==1 && p0->cpriority<highP->cpriority)
			highP=p0;
			
		   p=highP;
		   p->cpriority+=p->priority;


		  
		
				
		}
	  
		
	    if(empty0==1)
	    {
			QQQ=2;
			continue;
	    }
			

		
	}
	
	if(QQQ==2)
	{
	//   int u=l1;
	   struct proc *temp=ptable.proc;
	   for(int k=0;k<l1;k++)
		temp++;
		
	   for (p1=temp;p1<&ptable.proc[NPROC];p1++)
		if((++l1)>0 && p1->state==RUNNABLE && p1->queue==2)
		  {
			empty1=0;
			p=p1;

			if(l1==NPROC)
			  l1=0;

			break;
		  }
		
	    if(empty1==1)
	    {
		l1=0;

		for(p1 = ptable.proc; p1<temp; p1++)
			if((++l1)>0 && p1->state==RUNNABLE && p1->queue==2)
			{
				empty1=0;
				p=p1;

				if(l1==NPROC)
			  	  l1=0;

				break;
			}
	     
				
		if(empty1==1)
		{
			QQQ=3;
			continue;
		}
	    }
		
	}
	
        if(QQQ==3)
	{
		//Choose a new process
		if(counter==0)
		{
		
	             struct proc *temp=ptable.proc;
	  	     for(int k=0;k<l2;k++)
		       temp++;

		     for (p2=temp;p2<&ptable.proc[NPROC];p2++)
		       if((++l2)>0 && p2->state==RUNNABLE && p2->queue==3)
			{
				empty2=0;
				p=p2;
				if(l2==NPROC)
			  	   l2=0;
				lastPid=p2->pid;
				break;
			}
			
                     if(empty2==1)
		     {
			l2=0;
			for(p2 = ptable.proc; p2<temp; p2++)
			  if((++l2)>0 && p2->state==RUNNABLE && p2->queue==3)
			  {
				empty2=0;
				p=p2;
				if(l2==NPROC)
			  	  l2=0;
				lastPid=p2->pid;
				break;
			  }
		     }

		    if(empty2==1)
		    {
			QQQ=1;
			continue;
		    }
		}

		//Already have a process
		else	
		{
			 for(p2 = ptable.proc; p2 < &ptable.proc[NPROC]; p2++)
				 if(p2->state==RUNNABLE && (p2->pid==lastPid))
			 {
				p=p2;
				empty2=0;
				break;
			 }
			if(empty2==1)
			{
		 	 QQQ=1;
			// cprintf("%s%d\n\n","Refused, looking pid=",lastPid);
		
		//	 for(p2 = ptable.proc; p2 < &ptable.proc[NPROC]; p2++)
			  //	if(p2->state==RUNNABLE)
				//	cprintf("%s%d%s%d\n","queue=",p2->queue,"\tpid=",p2->pid);
			//cprintf("%s","\n\n");

			 counter=0;
		 	 continue;
			}
		}
		
		
			
		counter=(counter+1)%quantum;
		//cprintf("%s%d%s%d\n\n","counter=","counter","\tlastPid=","lastPid","\tPid=",p->pid);
		//cprintf("%s%d%s%d\n\n","counter=",counter,"\tlastPid=",lastPid);
		
	}
	
     // if(QQQ==3)
    //  cprintf("%s%d\n","Ending Q=",QQQ); 
	//if(sq!=QQQ)
	//cprintf("%s\n\n\n","CAPRUTED!!!");
			 
		
      switch (QQQ)
      {
	case 1:
	 QQQ=2;
         break;

	case 2:
	 QQQ=3;
	 break;

	case 3:
	 if(counter==0)
	   QQQ=1;
	 break;

	default:
	 break;
     }
     

      
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.

      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;
        

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;

//    p->terminationTime=ticks;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;
  p->lastSLeepTick=ticks;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan){
      p->state = RUNNABLE;
      avgSLP+=(ticks-p->lastSLeepTick);
	}
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{	 
  //cprintf("%s\n\n","Getting killed!!");
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;

      // Wake process from sleep if necessary.
      if(p->state == SLEEPING){
        p->state = RUNNABLE;
	avgSLP+=(ticks-p->lastSLeepTick);
	}
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

int
getchilds(void)
{
  struct proc *p;
  int havekids;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);

    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      
	  if(havekids==0)
	  {
	  	havekids++;
	  	cprintf("%d",p->pid);
		//cprintf("%s","first\n\n");
	  }
	  
	  else
	  {
	  	havekids++;
	  	cprintf("%d",0);
	  	cprintf("%d",p->pid);
		//cprintf("%s","second\n\n");
	  }
    }
	cprintf("\n");
	cprintf("%s\t","totally had : ");
	cprintf("%d",havekids);
	cprintf("%s","childs\n\n");
        release(&ptable.lock);
	return 22;
}

//Change the s queue of this process to the next process. 0=Default scheduler, 1=
int
changepolicy (void)
{
  struct proc *curproc = myproc();
   
  switch(curproc->queue)
  {
	case 1:
		curproc->queue=2;
		break;

	case 2:
		curproc->queue=3;
		break;

	case 3:
		curproc->queue=1;
		break;
  }
	

  return 23;
}

//Change priority of current process
int
changeCurrentPriority(int p)
{
	struct proc *curproc = myproc();
	curproc->priority=p;
	return 24;
}



//calculate statistics
int 
waitshowaverage(void)
{

       	while(waitforchilds()!=-1){}

	
	//cprintf("%s%d%s%d%s%d%s%d%s%d","WT=",avgWT,"\tSLP=",avgSLP,"\tTR=",avgTRT,"\tRNT=",avgRNT,"\tchld=",childs);
	//cprintf("%s","\n");
	
	
	
	return 27;
}

int
changequantum(void)
{

   quantum=quantum*2;
     // cprintf("%);	
   //cprintf("%s%d%s%d%s%d%s%d%s%d","WT=",avgWT,"\tSLP=",avgSLP,"\tTR=",avgTRT,"\tRNT=",avgRNT,"\tchld=",childs);
   //avgWT=0;
  // avgSLP=0;
  // avgTRT=0;
 //  avgRNT=0;
//   childs=0;

   return 26;
}


//After all processes are finished, print out records for all processes to compare priority effect on processes
int 
result(void)
{
 
	//cprintf("%s%d%s%d%s%d%s%d%s%d","WT=",avgWT,"\tSLP=",avgSLP,"\tTR=",avgTRT,"\tRNT=","\tchld=",childs);
	//cprintf("%s%d%s%d%s%d%s%d%s%d","WT=",avgWT,"\tSLP=",avgSLP,"\tTR=",avgTRT,"\tRNT=",avgRNT,"\tquantum=",quantum);
	for (int i=0;i<index;i++)
		cprintf("%s%d%s%d%s%d%s%d%s%d\n","pid: ",record[i][0],"\tqueue: ",record[i][4],"\twaiting time: ",record[i][1],"\tpriority: ",record[i][2],"\tTermination: ",record[i][3]);
  
	
	

return 28;
}

void
update(void)
{
  struct proc *p;
  sti();
  acquire(&ptable.lock);
  for (p=ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
   // if (p -> state == SLEEPING){
   //   p -> sleepingTime++;
   //   check++;
    // }//
     if (p -> state == RUNNING){
      p -> runningTime++;
      avgRNT++;
      }
     if (p -> state == RUNNABLE){
      p -> waitingTime ++;
      avgWT++;
      }
  }
  release(&ptable.lock);


	//cprintf("%s","\nUpdate\n");
}



