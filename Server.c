#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <string.h>

#define BUFLEN 1000

int PORT;

struct sockaddr_in serv_addr;
int serverfd;
int clientfd[1000], cfdpos;
int contlegat[1000];
int udpfd;

typedef struct Cont {
    char nume[20];
    char prenume[20];
    char parola[20];

    double suma;

    int nrcont;
    int pin;
    int incercari;
} Cont;

Cont accounts[1000]; int accnum;

void initTcp() {
    unsigned int len = sizeof(struct sockaddr);
    serverfd = socket(PF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bzero(&(serv_addr.sin_zero), 8);

    int bind_val = bind(serverfd, (struct sockaddr*)&serv_addr, len);
    
    int optval = 1;
    int set_val = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &optval, 4);

    listen(serverfd, 1);

    fcntl(serverfd, F_SETFL, O_NONBLOCK);
}


void initUdp() {
    struct sockaddr_in si_me;
    int len = sizeof(struct sockaddr_in);

    udpfd = socket(AF_INET, SOCK_DGRAM, 0);

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(udpfd, (struct sockaddr*)&si_me, len);
    fcntl(udpfd, F_SETFL, O_NONBLOCK);
}

void loadAccounts(char* filename) {
    FILE* f = fopen(filename, "r");
    fscanf(f, "%d", &accnum);

    int i;
    for (i = 1; i <= accnum; i++) {
        fscanf(f, "%s %s %d %d %s %lf", 
        accounts[i].nume, accounts[i].prenume, 
        &accounts[i].nrcont, &accounts[i].pin, 
        accounts[i].parola, &accounts[i].suma);
        accounts[i].incercari = 3;

        printf("parola -> %s\n", accounts[i].parola);
    }

    fclose(f);
}

void checkUdp() {
    char command[BUFLEN];
    struct sockaddr_in si_other;
    int len = sizeof(struct sockaddr_in);

    int recvlen = recvfrom(udpfd, command, BUFLEN, 0, (struct sockaddr*)&si_other, &len);

    if (recvlen > 0) {
        char firstThingy[30];
        sscanf(command, "%s", firstThingy);
        if (strcmp(firstThingy, "unlock") == 0) {
            sendto(udpfd, "Trimite parola secreta\0", 23, 0, (struct sockaddr*)&si_other, len);
        } else {
            int cardnr;
            char parola[30];

            sscanf(command, "%d %s", &cardnr, parola);

            for (int i = 1; i <= accnum; i++) {
                if (accounts[i].nrcont == cardnr) {
                    if (strcmp(accounts[i].parola, parola) == 0) {
                        accounts[i].incercari = 3;
                        sendto(udpfd, "Client deblocat\0", 16, 0, (struct sockaddr*)&si_other, len);
                    } else {
                        sendto(udpfd, "-7 : Eroare la deblocare\0", 25, 0, (struct sockaddr*)&si_other, len);
                    }
                    return;
                }
            }
            sendto(udpfd, "-7 : Eroare la deblocare\0", 25, 0, (struct sockaddr*)&si_other, len);
        }
    }
}

void login(int index, char* command, char* response) {
    char aux[30];
    int nrcard, pin;

    if (contlegat[index] != 0) {
        strcpy(response, "Client deja logat");
        return;
    }

    sscanf(command, "%s %d %d", aux, &nrcard, &pin);

    for (int i = 1; i <= accnum; i++) {
        if (accounts[i].nrcont == nrcard) {
            if (accounts[i].incercari <= 0) {
                strcpy(response, "-5: Card blocat");
                return;
            }
            if (accounts[i].pin == pin) {
                strcpy(response, "Client logat");
                contlegat[index] = i;
                return;
            } else {
                accounts[i].incercari--;
                if (accounts[i].incercari  <= 0) {
                    strcpy(response, "-5: Card blocat");
                    return;
                }
                strcpy(response, "-3: Pin gresit");
                return;
            }
        }
    }
}

void logout(int index, char* command, char* response) {
    if (contlegat[index] == 0) {
        strcpy(response, "-1: Clientul nu este autentificat");
        return;
    }
    strcpy(response, "Client delogat");
    contlegat[index] = 0;
}

void sold(int index, char* command, char* response) {
    if (contlegat[index] == 0) {
        strcpy(response, "-1: Clientul nu este autentificat");
        return;
    }
    int cont = contlegat[index];
    sprintf(response, "%.2lf ", accounts[cont].suma);
}

void getmoney(int index, char* command, char* response) {
    char aux[30];
    int amount;
    int cont;

    if (contlegat[index] == 0) {
        strcpy(response, "-1: Clientul nu este autentificat");
        return;
    }

    cont = contlegat[index];
    sscanf(command,  "%s %d", aux, &amount);

    if (amount % 10 != 0) {
        strcpy(response, "-9: Suma nu e multiplu de 10");
        return;
    }

    if (accounts[cont].suma < amount) {
        strcpy(response, "-8: Fonduri insuficiente");
        return;
    }

    accounts[cont].suma -= amount;
    strcpy(response, "Suma retrasa cu succes");
}

void putmoney(int index, char* command, char* response) {
    char aux[30];
    double amount;
    int cont;

    if (contlegat[index] == 0) {
        strcpy(response, "-1: Clientul nu este autentificat");
        return;
    }

    cont = contlegat[index];
    sscanf(command,  "%s %lf", aux, &amount);

    accounts[cont].suma += amount;
    strcpy(response, "Suma depusa cu succes");
}

int available() {
	struct pollfd pollfds;
	pollfds.fd = STDIN_FILENO;
	pollfds.events = POLLIN;

	if (poll(&pollfds, 1, 0))
		return 1;
	else return 0;
}


int main(int argc, char* argv[]) {
    PORT = atoi(argv[1]);
    loadAccounts(argv[2]);
   
    initTcp();
    initUdp();

    while(1) {
        char command[BUFLEN];
        char response[BUFLEN];
        int len = sizeof(struct sockaddr);
        int connfd = accept(serverfd, (struct sockaddr*)&serv_addr, &len);

        if (connfd > 0) {
            clientfd[cfdpos++] = connfd;
            fcntl(connfd, F_SETFL, O_NONBLOCK);
            fcntl(0, F_SETFL, 0);
            printf("Hotline: new client\n");
        }

        int i;
        for (i = 0; i < cfdpos; i++) {
            if (clientfd[i] > 0) {
                int recvsize = recv(clientfd[i], command, BUFLEN, 0);
                if (recvsize > 0) {
                    char firstThingy[30];
                    sscanf(command, "%s", firstThingy);
                    if (strcmp(firstThingy, "login") == 0) {
                        login(i, command, response);
                    }
                    if (strcmp(firstThingy, "logout") == 0) {
                        logout(i, command, response);
                    }
                    if (strcmp(firstThingy, "listsold") == 0) {
                        sold(i, command, response);
                    }
                    if (strcmp(firstThingy, "putmoney") == 0) {
                        putmoney(i, command, response);
                    }
                    if (strcmp(firstThingy, "getmoney") == 0) {
                        getmoney(i, command, response);
                    }
                    
                    send(clientfd[i], response, BUFLEN, 0);
                }
            }
        }

        if (available()) {
            char string[30];
            scanf("%s", string);
            if (strcmp(string, "quit") == 0) {
                break;
            }
        }

        checkUdp();
    }
}