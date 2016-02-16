#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_wolfie(void)
{
  char *buffer;

  if(argstr(0, &buffer) < 0)
    return -1;

  // picture created on http://picascii.com/
  // image: http://grfx.cstv.com/schools/ston/graphics/ston-13-hdr-logo.png
  char wolfie_string[] = "\
  \n\
                                #++';;;;'++#                \n\
                               #            #               \n\
                     #;;;.....@              @.....;;;#     \n\
                     '       ..              ..       ;     \n\
                     #       #                #       #     \n\
                      '     +                  +     '      \n\
                      .     ,'''+''''++''''+''',     ,      \n\
                       #                            #       \n\
                       `+  +'':@@@#      #@@@:''+  +`       \n\
                         ##    #   '`  `'   #    ##         \n\
                         '#``  # :@ +::+ @: #  ``#'         \n\
                        `+` ., +,,'`    `',,+ ,. `+`        \n\
                      :+     `' #          @ ;`     +:      \n\
                     #         @            @         #     \n\
                    .          :            :          .    \n\
                    #          ,   `####`   :          #    \n\
                   `           `.``.@@@@,``..           .   \n\
                   #           #    @@@@    #           #   \n\
                   `           #      ,     #           `   \n\
                  #`           #      ,     #           `@  \n\
                    .+#''.     #  ; .;:, ;  #     ,''#+.    \n\
                          .;''+@,,'@ @' @',,@+'';.          \n\
                            # @ `: #`.:`# :` @ #            \n\
                            # `:@;@ '  ' @;@:` #            \n\
                            @  @`,        ,`@  @            \n\
                            ', ' @        @ ' ,'            \n\
                              ,++;        ;++,              \n\
                                ;`#,,,,,,#`'                \n\
                                 +        +                 \n\
                                 ``      ``                 \n\
  \n\
  ";

  char cur = wolfie_string[0];
  int i;
  for (i = 0; cur != '\0'; i++) {
    cur = wolfie_string[i];
    buffer[i] = cur;
  }
  buffer[i] = '\0';

  return 0;
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
