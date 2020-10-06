
#include "message.h"

static const int MSGFJG_DEFAULT = 0666;

struct Msgbuf {
    long mtype;
};

void CreateProcesses(size_t nProcesses, pid_t* pid, size_t* numProcess);

void    SendMessage(int msqid, long mtype);
void ReceiveMessage(int msqid, long mtype);

//=============================================================================

void Print_NumChildProcesses(const size_t nProcesses)
{

    int id_MsgQueue = msgget(IPC_PRIVATE, MSGFJG_DEFAULT | IPC_CREAT);
    if(id_MsgQueue < 0) {
        perror("Error msgget()");
        exit(EXIT_FAILURE);
    }

    pid_t pid = 0;
    size_t numProcess = 0;
    CreateProcesses(nProcesses, &pid, &numProcess);

    if (pid > 0)
        for (size_t i_numProcess = 1; i_numProcess <= nProcesses; i_numProcess++) {
//            printf("Parent %zu\n", i_numProcess);
            SendMessage   (id_MsgQueue, i_numProcess);
            ReceiveMessage(id_MsgQueue, nProcesses + 1);
        }

    else {
//        printf("Child %zu [%d]\n", numProcess, getpid());

        ReceiveMessage(id_MsgQueue, numProcess);
        printf("Child %zu\n", numProcess);
        SendMessage   (id_MsgQueue, nProcesses + 1);
        exit(EXIT_SUCCESS);
    }

//    errno = 0;
//    int ret_msgctl = msgctl(id_MsgQueue, IPC_RMID, NULL);
//    if (ret_msgctl < 0) {
//        perror("Error msgctl()");
//        exit(EXIT_FAILURE);
//    }
}

void CreateProcesses(size_t nProcesses, pid_t* pid, size_t* numProcess)
{
    assert(nProcesses > 0);
    assert(pid);
    assert(numProcess);

    for (size_t i_numProcess = 1; i_numProcess <= nProcesses; i_numProcess++) {
        errno = 0;
        switch (*pid = fork()) {
            case -1:
                perror("Error fork()");
                exit(EXIT_FAILURE);
            case 0:
                *numProcess = i_numProcess;
                break;
            default:
                continue;
        }

        if(*numProcess > 0)
            break;
    }
}


void SendMessage(int msqid, long mtype)
{
    assert(msqid >= 0);
    assert(mtype >= 0);

    struct Msgbuf msgbuf = {mtype};

    //Check size queue???????
    errno = 0;
    int ret_msgsnd = msgsnd(msqid, &msgbuf, 0, 0);
    if (ret_msgsnd < 0) {
        perror("Error msgsnd()");
        exit(EXIT_FAILURE);
    }
}


void ReceiveMessage(int msqid, long mtype)
{
    assert(msqid >= 0);
    assert(mtype >= 0);

    struct Msgbuf msgbuf = {0};

    errno = 0;
    int res_msgrcv = msgrcv(msqid, &msgbuf, 0, mtype, 0);
    if (res_msgrcv < 0) {
        perror("Error msgrcv()");
        exit(EXIT_FAILURE);
    }
}
