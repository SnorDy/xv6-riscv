#ifndef _PROCINFO_H_
#define _PROCINFO_H_

struct procinfo {
    int pid;                
    char name[16];         
    int state;              
    int ppid;              
    char pname[16];         
};

#endif 