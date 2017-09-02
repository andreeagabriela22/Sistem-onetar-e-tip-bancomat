#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>

#define BUFLEN 1000

int sockfd;
char buffer[1000];
int closed;
int udpfd;


void initUdp(int port) {
    struct sockaddr_in dest;
    int len = sizeof(struct sockaddr_in);

    udpfd = socket(AF_INET, SOCK_DGRAM, 0);

    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(udpfd, (struct sockaddr*)&dest, len);
}

void udpSend(char* message, int len, char* address, int port) {
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = htonl(address);

	inet_aton(address, &dest.sin_addr);

	sendto(udpfd, message, BUFLEN, 0, (struct sockaddr *)&dest, sizeof(dest));
}

void udpRecv(char* message, char* address, int port) {
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	inet_aton(address, &server.sin_addr);

	int len = sizeof(struct sockaddr_in);
	recvfrom(udpfd, message, BUFLEN, 0, (struct sockaddr *)&server, &len);
}

void initTcp(char* address, int port) {
if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    		perror("socket");
  			exit(1);
  		}
    
    	// completare informatii despre adresa serverului
    	struct sockaddr_in serv_addr;
  		serv_addr.sin_family = PF_INET;
    	serv_addr.sin_port = htons(port);
  		serv_addr.sin_addr.s_addr = inet_addr(address);
    	bzero(&(serv_addr.sin_zero), 8);
 
    	// conectare la server
    	int len = sizeof(serv_addr);

    	if(connect(sockfd, (struct sockaddr*)&serv_addr, len) < 0){
    	  perror("Fail");
    	  exit(-1);
    	}

        //fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

int main(int argc, char* argv[]) {
	char* address = argv[1];
	int port = atoi(argv[2]);

	initTcp(address, port);
	initUdp(port);

	while(1) {
		char command[BUFLEN];
		char response[BUFLEN];
		
		fgets(command, 100, stdin);
		char firstThingy[30];
        sscanf(command, "%s", firstThingy);
        if (strcmp(firstThingy, "unlock") == 0) {
			struct sockaddr_in si_other;
			int len = sizeof(struct sockaddr_in);

			udpSend("unlock\0", 7, address, port);
			recvfrom(udpfd, response, BUFLEN, 0, (struct sockaddr*)&si_other, &len);
			printf("UNLOCK> %s\n", response);

			char command2[100];
			fgets(command2, 100, stdin);
			udpSend(command2, strlen(command2), address, port);
			recvfrom(udpfd, response, BUFLEN, 0, (struct sockaddr*)&si_other, &len);
			printf("UNLOCK> %s\n", response);
		}
		else if (strcmp(firstThingy, "quit") == 0) {
			break;
		} else {
			send(sockfd, command, BUFLEN, 0);
			recv(sockfd, response, BUFLEN, 0);
			printf("ATM> %s\n", response);
		}
	}


	//close tcp
	//hasta la vista baby
	return 0;
}