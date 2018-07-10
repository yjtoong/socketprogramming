#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  // structures for networking
#include <pthread.h> 
#define BUFSIZE 128

void* Receiver(void *arg){
	int sd =*(int*)arg;
	int numbytes;
	char buf[BUFSIZE];
	do {
		memset(buf,0,BUFSIZE); //memset:clear the array, empty the buffer
		numbytes = recv(sd,buf,BUFSIZE,0);
		printf("%s", buf);
		if(strstr(buf,"quit") != NULL){ //receive remote control from netcat, part of portfolio
			printf("..quitting..\n"); //simulation of quiting
			exit(0); //this is to quit the connection
		}
	}while(numbytes>0);

	return 0;
}

void* Sender(void *arg) {
	int sd =*(int*)arg;
	char buf[BUFSIZE];
	while(1) {
		memset(buf,0,BUFSIZE);
		fgets(buf,BUFSIZE,stdin);
		unsigned len = strlen(buf);
		buf[len]='\r';
		send(sd,buf,BUFSIZE,0);
	}
}


int main(){
	pthread_t tidReceiver, tidSender;
	
	char server_message[256] = "You have reached server\n";
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0); // 0 default TCP

	int one = 1;
	// SOL_SOCKET = socket options, one = 1 means enable 
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one));

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002); //9002 is port number
	server_address.sin_addr.s_addr = INADDR_ANY; //binding any address that come to the server

	bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)); //bing server socket to the address

	listen(server_socket,5); //start listening and can accept max 5 connection
	

	int client_socket;
	client_socket = accept(server_socket,NULL,NULL); //accept connection

	if (pthread_create(&tidReceiver, NULL, &Receiver, &client_socket)) { //send receiver function to the "receiver" child //both child use the client_socket
		perror("Fail create Receiver thread");
	}
	if (pthread_create(&tidSender, NULL, &Sender, &client_socket)) { //send sender function to the "sender" child 
		// both the child run parallelly with the parent
		perror("Fail create Sender thread");
	}

	pthread_join(tidReceiver, NULL); //wait for the child to die, if the child havent die, the main parent will not exit
	pthread_join(tidSender, NULL);


	close(client_socket);
	close(server_socket);
	return 0;
}
