#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"
uint64 acquire_freemem();
uint64 acquire_nproc();
uint64
sys_exit(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if (argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// 加入一个 trace function
uint64
sys_trace(void)
{
  int mask; // 用户态传过来的入参
  if (argint(0, &mask) < 0)
    return -1;
  printf("sys_trace hi: n is %d\n", mask);
  struct proc *p = myproc();
  p->trace_mask = mask; // 以共享内存的方式传输过去了

  return 0;
}
uint64
sys_sysinfo(void)
{
  printf("sys_sysinfo hi\r\n");
  struct sysinfo info;
  struct proc *p = myproc();
  uint64 addr;
  info.nproc = acquire_nproc();     // 当前进程的数
  info.freemem = acquire_freemem(); // 空闲内存大小
  if (argaddr(0, &addr) < 0)        // 用这个接收入参
    return -1;
  if (copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0) // 将结果放到addr里面
    return -1;
  return 0;
}
