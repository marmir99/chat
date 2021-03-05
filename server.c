#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

char names[32][15];
char passwords[32][15];
int logged[32];
int mid, msgmid;

void make_tabs()
{
    for (int i=0; i<32; i++)
    {
        logged[i] = 0;
    }
    FILE *file = fopen("users.txt", "r");
    char buf2[15];
    char buf[1];
    int ind = 0, work = 1;
    while (work == 1)
    {
        if (fscanf(file, "%[^ ]", buf2) == EOF) break;
        strcpy(names[ind], buf2);
        fscanf(file, "%[ ]", buf);
        fscanf(file, "%[^\n]", buf2);
        strcpy(passwords[ind], buf2);
        fscanf(file, "%[\n]", buf);
        ind++;
    }
    fclose(file);
}

void logowanie()
{
    struct msgbuf
    {
        long type;
        char data[2][15];
    }msg_log;
    struct msgbuf2
    {
        long type;
        int back;
    } msg_back;
    msg_back.type = 1035;
    int value;
    value = msgrcv(mid, &msg_log, 30, 1, IPC_NOWAIT);
    if (value == -1) ;
    else
    {
        for (int i=0; i<32; i++)
        {
            if (strcmp(msg_log.data[0], names[i]) == 0)
            {
                if (strcmp(msg_log.data[1], passwords[i]) == 0)
                {
                    msg_back.back = i+10;
                    logged[i] = 1;
                    msgsnd(mid, &msg_back, sizeof(msg_back.back), 0);
                    break;
                }
                else
                {
                    msg_back.back = -1;
                    msgsnd(mid, &msg_back, sizeof(msg_back.back), 0);
                }
            }
        }
    }
}

void wyloguj()
{
    struct msgbuf
    {
        long type;
        int me;
    } msg_logout;
    struct msgbuf2
    {
        long type;
        int good;
    } msg_logout_b;
    msg_logout_b.good = -1;
    int value;
    value = msgrcv(mid, &msg_logout, 12, 2, IPC_NOWAIT);
    if (value == -1) ;
    else
    {
        logged[msg_logout.me-10] = 0;
        msg_logout_b.type = msg_logout.me;
        msg_logout_b.good = 1;
        msgsnd(mid, &msg_logout_b, sizeof(msg_logout_b.good), 0);
    }
}

void logged_users()
{
    struct msgbuf
    {
        long type;
        int myType;
    } msg_list;
    struct msgbuf2
    {
        long type;
        char user[15];
    } msg_back;
    int value, type;
    value = msgrcv(mid, &msg_list, 12, 4, IPC_NOWAIT);
    if (value == -1) ;
    else
    {
        type = msg_list.myType;
        for (int i=0; i<32; i++)
        {
            if (logged[i] == 1)
            {
                msg_back.type = type;
                strcpy(msg_back.user, names[i]);
                msgsnd(mid, &msg_back, sizeof(msg_back.user), 0);
            }
        }
        msg_back.type = type;
        strcpy(msg_back.user, " ");
        msgsnd(mid, &msg_back, sizeof(msg_back.user), 0);
    }
}

void send_to_user()
{
    struct msgbuf
    {
        long type;
        char users[2][15];
        char message[64];
    } msg_send;
    struct msgbuf2
    {
        long type;
        int ok;
    } msg_sender;
    msg_sender.ok = -1;
    struct msgbuf3
    {
        long type;
        char user[15];
        char msg[64];
    } msg_go;
    int value, n;
    value = msgrcv(msgmid, &msg_send, 102, 3, IPC_NOWAIT);
    if (value == -1) ;
    else
    {
        strcpy(msg_go.user, msg_send.users[0]);
        strcpy(msg_go.msg, msg_send.message);
        for (int i=0; i<32; i++)
        {
            if (strcmp(msg_send.users[1], names[i]) == 0)
            {
                msg_go.type = i+10;
                break;
            }
        }
        n=msgsnd(msgmid, &msg_go, sizeof(msg_go.user)+sizeof(msg_go.msg),0);
        if (n==0)
            {
                msg_sender.ok = 1;
            }
        for (int i=0; i<32; i++)
        {
            if (strcmp(msg_send.users[0], names[i]) == 0)
            {
                msg_sender.type = i+10;
                msgsnd(msgmid, &msg_sender, sizeof(msg_sender.ok), 0);
                break;
            }
        }
    }
}

void send_to_group()
{
    struct msgbuf
    {
        long type;
        char names[2][15];
        char msg[64];
    } msg_send2;
    int value = msgrcv(msgmid, &msg_send2, 102, 7, IPC_NOWAIT);
    if (value != -1)
    {
        struct msgbuf2
        {
            long type;
            int back;
        } msg_back;
        msg_back.back = 1;
        struct msgbuf3
        {
            long type;
            char name[15];
            char msg[64];
        } msg_go;
        for (int i=0; i<32; i++)
        {
            if (strcmp(names[i], msg_send2.names[0]) == 0)
            {
                msg_back.type = i+10;
            }
        }
        char buf2[15];
        char buf[1];
        int check = 0;
        FILE * file = fopen("groups.txt", "r");
        while (fscanf(file, "%[^\n]", buf2)!=EOF)
        {
            if (strcmp(buf2, msg_send2.names[1])==0)
            {
                fscanf(file, "%[\n]", buf);
                while (fscanf(file, "%[^\n]", buf2)!=EOF)
                {
                    if (strcmp(buf2, " ") == 0) break;
                    else if (strcmp(buf2, msg_send2.names[0]) == 0)
                    {
                        check = 1;
                        break;
                    }
                    fscanf(file, "%[\n]", buf);
                }
                break;
            }
            fscanf(file, "%[\n]", buf);
        }
        fclose(file);
        if (check == 1)
        {
            char buf3[15];
            char buf4[1];
            FILE * file;
            file = fopen("groups.txt", "r");
            int n;
            while (fscanf(file, "%[^\n]", buf3)!=EOF)
            {
                if (strcmp(buf3, msg_send2.names[1])==0)
                {
                    fscanf(file, "%[\n]", buf4);
                    while (fscanf(file, "%[^\n]", buf3)!=EOF)
                    {
                        if (strcmp(buf3, " ") == 0) break;
                        else
                        {
                            for (int i=0; i<32; i++)
                            {
                                if(strcmp(buf3, names[i])==0)
                                {
                                    msg_go.type = i+10;
                                    strcpy(msg_go.name, msg_send2.names[0]);
                                    strcpy(msg_go.msg, msg_send2.msg);
                                    n=msgsnd(msgmid, &msg_go, sizeof(msg_go.name)+sizeof(msg_go.msg), 0);
                                    if (n==-1) msg_back.back = -1;
                                    break;
                                }

                            }
                            fscanf(file, "%[\n]", buf4);
                        }
                    }
                    break;
                }
                fscanf(file, "%[\n]", buf4);
            }
            fclose(file);
        }
        else
        {
            msg_back.back = 0;
        }
        msgsnd(mid, &msg_back, sizeof(msg_back.back), 0);
    }
}

void view_groups()
{
    struct msgbuf
    {
        long type;
        char name[15];
    } msg_group;
    int value;
    value = msgrcv(mid, &msg_group, 23, 5, IPC_NOWAIT);
    if (value == -1) ;
    else
    {
        int me;
        struct msgbuf2
        {
            long type;
            char user[15];
        } msg_back;
        for (int i=0; i<32; i++)
        {
            if (strcmp(msg_group.name, names[i]) == 0)
            {
                me = i+10;
                break;
            }
        }
        char buf2[15];
        char buf[1];
        FILE *file = fopen("groups.txt", "r");
        fscanf(file, "%[^\n]", buf2);
        msg_back.type = me;
        strcpy(msg_back.user, buf2);
        msgsnd(mid, &msg_back, sizeof(msg_back.user), 0);
        fscanf(file, "%[\n]", buf);
        while (fscanf(file, "%[^\n]", buf2)!=EOF)
        {
            if (strcmp(buf2, " ") == 0)
            {
                fscanf(file, "%[\n]", buf);
                if (fscanf(file, "%[^\n]", buf2) != EOF)
                {
                    msg_back.type = me;
                    strcpy(msg_back.user, buf2);
                    msgsnd(mid, &msg_back, sizeof(msg_back.user), 0);
                }
            }
            fscanf(file, "%[\n]", buf);
        }
        fclose(file);
        msg_back.type = me;
        strcpy(msg_back.user, " ");
        msgsnd(mid, &msg_back, sizeof(msg_back.user), 0);
    }
}

void members()
{
    struct msgbuf
    {
        long type;
        char name[2][15];
    } msg_mlist;
    if (msgrcv(mid, &msg_mlist, 38, 6, IPC_NOWAIT) == -1) ;
    else
    {
        int type;
        for (int i=0; i<32; i++)
        {
            if (strcmp(msg_mlist.name[0], names[i]) == 0)
            {
                type = i+10;
                break;
            }
        }
        struct msgbuf2
        {
            long type;
            char user[15];
        } msg_back2;
        char buf2[15];
        char buf[1];
        FILE * file;
        file = fopen("groups.txt", "r");
        while (fscanf(file, "%[^\n]", buf2)!=EOF)
        {
            if (strcmp(buf2, msg_mlist.name[1])==0)
            {
                fscanf(file, "%[\n]", buf);
                while (fscanf(file, "%[^\n]", buf2)!=EOF)
                {
                    if (strcmp(buf2, " ") == 0) break;
                    else
                    {
                        msg_back2.type = type;
                        strcpy(msg_back2.user, buf2);
                        msgsnd(mid, &msg_back2, sizeof(msg_back2.user), 0);
                        fscanf(file, "%[\n]", buf);
                    }
                }
                break;
            }
            fscanf(file, "%[\n]", buf);
        }
        fclose(file);
        msg_back2.type = type;
        strcpy(msg_back2.user, " ");
        msgsnd(mid, &msg_back2, sizeof(msg_back2.user), 0);
    }
}

void add_to_group()
{
   struct msgbuf
    {
        long type;
        char names[2][15];
    } msg_add;
    int value = msgrcv(mid, &msg_add, 38, 8, IPC_NOWAIT);
    if (value == -1) ;
    else
    {
       struct msg_buf2
        {
            long type;
            int back;
        }msg_back;
        msg_back.back = -1;
        for (int i=0; i<32; i++)
        {
            if (strcmp(names[i], msg_add.names[0]) == 0)
            {
                msg_back.type = i+10;
            }
        }
        char buf2[15];
        char buf[1];
        int check = 0;
        FILE *file = fopen("groups.txt", "r");
        FILE *new = fopen("new.txt", "w");
        while (fscanf(file, "%[^\n]", buf2)!=EOF)
        {
            fprintf(new, "%s\n", buf2);
            if (strcmp(buf2, msg_add.names[1])==0)
            {
                fscanf(file, "%[\n]", buf);
                while (fscanf(file, "%[^\n]", buf2)!=EOF)
                {
                    if (strcmp(buf2, " ") == 0)
                    {
                        if (check == 0)
                        {
                            fprintf(new, "%s\n", msg_add.names[0]);
                            msg_back.back = 1;
                        }
                        fprintf(new, "%s\n", buf2);
                        fscanf(file, "%[\n]", buf);
                        break;
                    }
                    else if (strcmp(buf2, msg_add.names[0]) == 0)
                    {
                        check=1;
                    }
                    fprintf(new, "%s\n", buf2);
                    fscanf(file, "%[\n]", buf);
                }
            }
            fscanf(file, "%[\n]", buf);
        }
        if (check == 1) msg_back.back = 2;
        fclose(file);
        fclose(new);
        remove("groups.txt");
        rename("new.txt", "groups.txt");
        msgsnd(mid, &msg_back, sizeof(msg_back.back), 0);
    }
}

void delete_from_group()
{
    struct msgbuf
    {
        long type;
        char names[2][15];
    } msg_add;
    int value = msgrcv(mid, &msg_add, 38, 9, IPC_NOWAIT);
    if (value == -1) ;
    else
    {
        struct msg_buf2
        {
            long type;
            int back;
        }msg_back;
        for (int i=0; i<32; i++)
        {
            if (strcmp(msg_add.names[0], names[i]) == 0)
            {
                msg_back.type = i+10;
                break;
            }
        }
        msg_back.back = -1;
        char buf2[15];
        char buf[1];
        int check = 0;
        FILE *file = fopen("groups.txt", "r");
        FILE *new = fopen("new.txt", "w");
        while (fscanf(file, "%[^\n]", buf2)!=EOF)
        {
            fprintf(new, "%s\n", buf2);
            if (strcmp(buf2, msg_add.names[1])==0)
            {
                fscanf(file, "%[\n]", buf);
                while (fscanf(file, "%[^\n]", buf2)!=EOF)
                {
                    if (strcmp(buf2, " ") == 0)
                    {
                        fprintf(new, "%s\n", buf2);
                        fscanf(file, "%[\n]", buf);
                        break;
                    }
                    else if (strcmp(buf2, msg_add.names[0]) == 0)
                    {
                        check=1;
                        msg_back.back = 1;
                        fscanf(file, "%[\n]", buf);
                    }
                    else if (strcmp(buf2, msg_add.names[0]) != 0)
                    {
                        fprintf(new, "%s\n", buf2);
                        fscanf(file, "%[\n]", buf);
                    }
                }
            }
            fscanf(file, "%[\n]", buf);
        }
        if (check == 0)
        {
            msg_back.back = 0;
        }
        fclose(file);
        fclose(new);
        remove("groups.txt");
        rename("new.txt", "groups.txt");
        msgsnd(mid, &msg_back, sizeof(msg_back.back), 0);
    }
}

int main()
{
    mid = msgget(0x113, 0777 | IPC_CREAT);
    //msgctl(mid, IPC_RMID, NULL);
    //mid = msgget(0x113, 0777 | IPC_CREAT);
    msgmid = msgget(0x114, 0777 | IPC_CREAT);
    //msgctl(msgmid, IPC_RMID, NULL);
    //msgmid = msgget(0x114, 0777 | IPC_CREAT);
    make_tabs();
    bool work = true;
    while (work)
    {
        logowanie();
        wyloguj();
        send_to_user();
        send_to_group();
        logged_users();
        view_groups();
        members();
        add_to_group();
        delete_from_group();
    }
    return 0;
}
