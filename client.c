#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdbool.h>

char myName[15];
int myType;
int mid, msgmid;

int logowanie()
{
    struct msgbuf
    {
        long type;
        int back;
    } msg_back;
    struct msgbuf2
    {
        long type;
        char data[2][15];
    }msg_info;
    msg_info.type = 1;
    printf("Aby przejsc dalej podaj dane logowania\n");
    printf("Podaj nazwe uzytkownika: ");
    scanf("%s", msg_info.data[0]);
    strcpy(myName, msg_info.data[0]);
    printf("Podaj haslo: ");
    scanf("%s", msg_info.data[1]);
    msgsnd(mid, &msg_info, sizeof(msg_info.data), 0);
    msgrcv(mid, &msg_back, 12, 1035, 0);
    if (msg_back.back == -1)
    {
        printf("Podano nieprawidlowe dane logowania\n\n");
        return 0;
    }
    else
    {
        myType = msg_back.back;
        printf("Logowanie przebieglo pomyslnie\n\n");
        return 1;
    }
}

int wyloguj()
{
    struct msgbuf
    {
        long type;
        int me;
    } msg_logout;
    struct msgbuf2
    {
        long type;
        int back;
    } msg_back;
    msg_logout.type = 2;
    msg_logout.me = myType;
    msgsnd(mid, &msg_logout, sizeof(msg_logout.me), 0);
    msgrcv(mid, &msg_back, 12, myType, 0);
    if (msg_back.back == 1)
    {
        printf("Wylogowano\n");
        return 1;
    }
    else if (msg_back.back == -1)
    {
        printf("Nie udalo sie wylogowac\n");
        return 0;
    }
    return 0;
}

void view_users()
{
    struct msgbuf
    {
        long type;
        int myType;
    } msg_list;
    struct msgbuf2
    {
        long type;
        char name[15];
    } msg_back2;
    msg_list.type = 4;
    msg_list.myType = myType;
    msgsnd(mid, &msg_list, sizeof(msg_list.myType), 0);
    do
    {
        msgrcv(mid, &msg_back2, 23, myType, 0);
        printf("%s\n", msg_back2.name);
    } while (strcmp(msg_back2.name, " ") != 0);
}

void send_to_user()
{
    struct msgbuf
    {
        long type;
        char users[2][15];
        char msg[64];
    } msg_send;
    struct msgbuf2
    {
        long type;
        int back;
    } msg_back;
    msg_send.type = 3;
    strcpy(msg_send.users[0], myName);
    printf("Podaj odbiorce wiadomosci: ");
    scanf("%s", msg_send.users[1]);
    printf("Wpisz tekst wiadomosci: ");
    char bin[32];
    fgets(bin, sizeof(bin), stdin);
    fgets(msg_send.msg, sizeof(msg_send.msg), stdin);
    msgsnd(msgmid, &msg_send, sizeof(msg_send.users) + sizeof(msg_send.msg), 0);
    msgrcv(msgmid, &msg_back, 12, myType, 0);
    if (msg_back.back == 1) printf("\nWiadomosc wyslana poprawnie \n\n");
    else printf("\nWysylanie nie powiodlo sie \n\n");
}

void send_to_group()
{
    struct msgbuf
    {
        long type;
        char names[2][15];
        char msg[64];
    } msg_send2;
    struct msgbuf2
    {
        long type;
        int back;
    } msg_back;
    msg_send2.type = 7;
    strcpy(msg_send2.names[0], myName);
    printf("Podaj nazwe grupy do ktorej wysylasz: ");
    scanf("%s", msg_send2.names[1]);
    printf("Wpisz tekst wiadomosci: ");
    char bin[32];
    fgets(bin, sizeof(bin), stdin);
    fgets(msg_send2.msg, sizeof(msg_send2.msg), stdin);
    msgsnd(msgmid, &msg_send2, sizeof(msg_send2.names) + sizeof(msg_send2.msg), 0);
    msgrcv(msgmid, &msg_back, 12, myType, 0);
    if (msg_back.back == 0)
    {
        printf("Nie nalezysz do tej grupy, nie mozesz wysylac wiadomosci.\n\n");
    }
    else if (msg_back.back == -1)
    {
        printf("Nie udalo sie wyslac wiadomosci.\n\n");
    }
    else
    {
        printf("Wiadomosc wyslana poprawnie.\n\n");
    }
}

void get_message()
{
    struct msgbuf
    {
        long type;
        char user[15];
        char msg[64];
    } msg_receive;
    int value = msgrcv(msgmid, &msg_receive, 87, myType, IPC_NOWAIT);
    while(value != -1)
    {
        printf("\nNadawca: %s \n", msg_receive.user);
        printf("Tresc wiadomosci: %s\n", msg_receive.msg);
        value = msgrcv(msgmid, &msg_receive, 87, myType, IPC_NOWAIT);
    }
    if (value == -1)
    {
        printf("\nBrak nowych wiadomosci.\n\n");
    }
}

void view_groups()
{
    struct msgbuf
    {
        long type;
        char name[15];
    } msg_back2;
    msg_back2.type= 5;
    strcpy(msg_back2.name, myName);
    msgsnd(mid, &msg_back2, sizeof(msg_back2.name), 0);
    do
    {
        msgrcv(mid, &msg_back2, 23, myType, 0);
        printf("%s\n", msg_back2.name);
    } while (strcmp(msg_back2.name, " ") != 0);
}

void members()
{
    struct msgbuf
    {
        long type;
        char name[15];
    } msg_back2;
    struct msgbuf2
    {
        long type;
        char data[2][15];
    }msg_info;
    msg_info.type = 6;
    strcpy(msg_info.data[0], myName);
    printf("Podaj nazwe grupy: \n");
    scanf("%s", msg_info.data[1]);
    msgsnd(mid, &msg_info, sizeof(msg_info.data), 0);
    msgrcv(mid, &msg_back2, 23, myType, 0);
    while (strcmp (msg_back2.name, " ") != 0)
    {
        printf("%s\n", msg_back2.name);
        msgrcv(mid, &msg_back2, 23, myType, 0);
    }
}

void add_to_group()
{
    struct msgbuf
    {
        long type;
        int back;
    } msg_back;
    struct msgbuf2
    {
        long type;
        char data[2][15];
    }msg_info;
    msg_info.type = 8;
    strcpy(msg_info.data[0], myName);
    printf("Do ktorej grupy chcesz dolaczyc?\n");
    scanf("%s", msg_info.data[1]);
    msgsnd(mid, &msg_info, sizeof(msg_info.data), 0);
    msgrcv(mid, &msg_back, 12, myType, 0);
    if (msg_back.back == 2)
    {
        printf("\nNalezysz juz do tej grupy\n\n");
    }
    else if (msg_back.back == 1)
    {
        printf("\nPoprawnie dodano do grupy\n\n");
    }
    else if (msg_back.back == -1)
    {
        printf("\nDodawanie do grupy nie powiodlo sie\n\n");
    }
}

void delete_from_group()
{
    struct msgbuf
    {
        long type;
        int back;
    } msg_back;
    struct msgbuf3
    {
        long type;
        char data[2][15];
    }msg_info;
    msg_info.type = 9;
    strcpy(msg_info.data[0], myName);
    printf("Ktora grupe chcesz opuscic?\n");
    scanf("%s", msg_info.data[1]);
    msgsnd(mid, &msg_info, sizeof(msg_info.data), 0);
    msgrcv(mid, &msg_back, 12, myType, 0);
    if (msg_back.back == 0)
    {
        printf("\nNie nalezales do tej grupy\n\n");
    }
    else if (msg_back.back == 1)
    {
        printf("\nPoprawnie usunieto z grupy\n\n");
    }
    else if (msg_back.back == -1)
    {
        printf("\nUsuwanie z grupy nie powiodlo sie\n\n");
    }
}

int main()
{
    mid = msgget(0x113, 0777 | IPC_CREAT);
    msgmid = msgget(0x114, 0777 | IPC_CREAT);
    int good, choose;
    bool work = true;
    do
    {
        good = logowanie();
    } while (good != 1);
    while (work)
    {
        printf("Podaj co chcesz zrobic:\n");
        printf("[1] Wyloguj sie\n");
        printf("[2] Wyswietl liste zalogowanych uzytkownikow\n");
        printf("[3] Wyslij wiadomosc do uzytkownika\n");
        printf("[4] Wyslij wiadomosc do grupy\n");
        printf("[5] Odbierz wiadomosci\n");
        printf("[6] Wyswietl liste grup\n");
        printf("[7] Wyswietl czlonkow wybranej grupy\n");
        printf("[8] Zapisz sie do grupy\n");
        printf("[9] Usun sie z grupy\n");
        scanf("%d", &choose);
        switch (choose)
        {
            case 1:
            {
                good = wyloguj();
                if (good == 1) work = false;
                break;
            }
            case 2:
            {
                view_users();
                break;
            }
            case 3:
            {
                send_to_user();
                break;
            }
            case 4:
            {
                send_to_group();
                break;
            }
            case 5:
            {
                get_message();
                break;
            }
            case 6:
            {
                view_groups();
                break;
            }
            case 7:
            {
                members();
                break;
            }
            case 8:
            {
                add_to_group();
                break;
            }
            case 9:
            {
                delete_from_group();
                break;
            }
            default:
            {
                printf("Brak takiej opcji, podaj cyfre od 1 do 9\n");
            }
        }
    }
    return 0;
}
