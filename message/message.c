
#include "message.h"

static const int MSGFJG_DEFAULT = 0666;

struct Msgbuf {
    long mtype;
};

int CreateProcesses(size_t nProcesses, pid_t* pid, size_t* numProcess,
                    pid_t* pidsChild);

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
    pid_t* pidsChild = (pid_t*)calloc(nProcesses + 1, sizeof(*pidsChild));
    int ret_CreateProcesses = CreateProcesses(nProcesses, &pid,
                                              &numProcess, pidsChild);
    if (ret_CreateProcesses < 0) {
        errno = 0;
        int ret_msgctl = msgctl(id_MsgQueue, IPC_RMID, NULL);
        if (ret_msgctl < 0)
            perror("Error msgctl()");
        exit(EXIT_FAILURE);
    }


    if (pid > 0)
        for (size_t i_numProcess = 1; i_numProcess <= nProcesses; i_numProcess++) {
//          printf("Parent %zu\n", i_numrocess);
            SendMessage   (id_MsgQueue, i_numProcess);
            ReceiveMessage(id_MsgQueue, nProcesses + 1);
//          wait(&pidsChild[i_numProcess]);
        }

    else {
//      printf("Child %zu [%d]\n", numProcess, getpid());
        ReceiveMessage(id_MsgQueue, numProcess);
        printf("Child %zu\n", numProcess);
        fflush(stdout);
        SendMessage(id_MsgQueue, nProcesses + 1);
        exit(EXIT_SUCCESS);
    }

    errno = 0;
    int ret_msgctl = msgctl(id_MsgQueue, IPC_RMID, NULL);
    if (ret_msgctl < 0) {
        perror("Error msgctl()");
        exit(EXIT_FAILURE);
    }

    free(pidsChild);
}

//-----------------------------------------------------------------------------

int CreateProcesses(size_t nProcesses, pid_t* pid, size_t* numProcess,
                     pid_t* pidsChild)
{
    assert(pid);
    assert(numProcess);
    assert(pidsChild);

    pid_t* cur_pidsChild = pidsChild + 1;
    for (size_t i_numProcess = 1; i_numProcess <= nProcesses; i_numProcess++) {
        pid_t pid_parent = getpid();
        errno = 0;
        switch (*pid = fork()) {
            case -1:
                perror("Error fork()\n");
                return -1;
            case 0:
                *numProcess = i_numProcess;
                if(getppid() != pid_parent)
                    exit(EXIT_FAILURE);
                break;
            default:
                *cur_pidsChild = *pid;
                cur_pidsChild++;
                continue;
        }

        if(*numProcess > 0)
            break;
    }

    return 0;
}


void SendMessage(int msqid, long mtype)
{
    assert(msqid >= 0);
    assert(mtype >= 0);

    struct Msgbuf msgbuf = {mtype};

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
    ssize_t res_msgrcv = msgrcv(msqid, &msgbuf, 0, mtype, 0);
    if (res_msgrcv < 0) {
        perror("Error msgrcv()");
        exit(EXIT_FAILURE);
    }
}
