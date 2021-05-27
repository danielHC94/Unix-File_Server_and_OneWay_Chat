// Client side writen in C

#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/stat.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <poll.h>

#define IP_ADDR 0x7f000001
#define PORT 50000 
#define PORT2 8080
#define MAXLINE 1024

int download(char **fileName, int sock, int length);
int upload(char *fileName, int sock, int length);

int main(int argc, char *argv[]) { 
    int sock = 0, valread,length,i = 1,fd,action=4; 
    int down = 1,up = 2;
    struct sockaddr_in serv_addr; 
    char buffer[1024] = {0}; 

    // message to the server
    if (strcmp(argv[1], "chat") == 0){
	action=3;
	for(;;){
	    int sockfd;
	    char message[MAXLINE];
	    struct sockaddr_in servaddr2;
	  
	    // Creating socket file descriptor
	    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
	    {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	    }
	  
	    memset(&servaddr2, 0, sizeof(servaddr2));
	      
	    // Filling server information
	    servaddr2.sin_family = AF_INET;
	    servaddr2.sin_port = htons(PORT2);
	    servaddr2.sin_addr.s_addr = INADDR_ANY;
	    
	    struct pollfd pfds[1]; // More if you want to monitor more
	    pfds[0].fd = 0;// Standard input
	    pfds[0].events = POLLIN; // Tell me when ready to read    
	    
	  
	    int n, len;
	    printf("Write a message to the server or type exit: \n");
	    scanf("%s",message);
	    if (strcmp(message, "exit") == 0)
	    {
	      int num_events = poll(pfds, 1, 2500); // 2.5 second timeout
              return 0;
	    }
	    sendto(sockfd, (const char *)message, strlen(message),MSG_CONFIRM, (const struct sockaddr *) &servaddr2,sizeof(servaddr2));
	    close(sockfd);    
	 }//End for
	}//End if

//Using argc & argv and Strings//
    if (strcmp(argv[1], "upload") == 0){

//Print file size by using File API//
    FILE *fdd = fopen(argv[2],"r");
    if (fdd == NULL){
        printf("\nError, fail to open the file\n");
        exit(1);
    }
    fseek (fdd, 0, SEEK_END);
    printf("\nFile size: %ld bytes\n\n",ftell(fdd));
    fclose(fdd);

		action = 2;
    }
	else if (strcmp(argv[1], "download") == 0){
		action = 1;
    }
	//Using Stream socket
	    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("\n Socket creation error \n"); 
		return -1; 
	    } 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    
//Convert IPv4 and IPv6 addresses from text to binary form//
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<1) { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        perror("connect");
        return -1;
    } 
    printf("connected\n");
    int filesToSend = argc - 2;

//send amount of files so the server//
	if (send(sock, &filesToSend, sizeof(int), 0) < 0){
		perror("Send");
		return -1;
	}
	printf("action: %d\n",action);

//download/upload action//
    if (send(sock ,&action , sizeof(int) , 0 ) < 0){
        perror("Send");
        return -1;
    }

    if (action == 1){
		if(download(argv, sock, filesToSend) == -1){
			printf("down:%d", down);
			if (send(sock, &down, sizeof(int), 0) < 0){
				perror("download");
			}
		}
	}
	if (action == 2){

		for (i = 2; i < argc; ++i){
			length = strlen(argv[i]);
			if(upload(argv[i], sock, length) == -1){
				printf("up:%d", up);
				if (send(sock, &up, sizeof(int), 0) < 0){
					perror("upload");
				}
			}		
		}
	}
	close(sock);
	if(action=4){
		perror("not download/upload/chat");
		return -1;
	}
	return 0; 
}//End main 

int download(char **fileName, int sock, int length){
	int i = 0,nameLength, check = 0,fd;
	char file[256];
	void *buf;
	void *filesbuf;

	if(length > 1){
		strcpy(file,"MyFiles.zip");
	}
	else {
		strcpy(file,fileName[2]);
	}

//send amount of files//
	if (send(sock, &length, sizeof(int), 0) < 0){
		perror("Send");
		return -1;
	}

//send files to server//
	for(i = 0; i < length; ++i){
		nameLength = strlen(fileName[i+2]);
		if (send(sock, &nameLength, sizeof(int), 0) < 0){
			perror("Send");
			return -1;
		}
		if (send(sock, fileName[i+2], nameLength, 0) < 0){
			perror("Send");
			return -1;
		}
	}

//Authantication//
	if ((recv(sock, &check, sizeof(int), 0)) < 0){
		perror("recv");
		return 0;
	}
	if (check != 1){
		printf("one of the files is not on the server\n");
		return 0;
	}

//get fd stat - size//
	off_t size = 0;
	if ((recv(sock, &size, sizeof(size), 0)) < 0){
		perror("recv");
		return 0;
	}

	if(size == 0){
		printf("no data at the file");
		return 0;
	}

	buf = malloc(size * sizeof(void *));
	if(buf == NULL){
		perror("malloc");
		return 0;
	}
	memset(buf, 0, size); //fill memory with a constant byte

//receive mmap from server - map memory is shared//
	if ((recv(sock, buf, size, 0)) < 0){
		perror("recv");
		return 0;
	}

//check if buf contains any data//
	if(*((int*)buf) == 0){
		perror("recv");
		return 0;
	}

// open zip file//
	if ((fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0){
		perror("open");
		return 0;
	}

	if (lseek(fd, size - 1, SEEK_SET) == -1)//reposition read/write file offset
	{
		perror("lseek");
		return 0;
	}
	if (write(fd, "", 1) != 1){
		perror("write");
		return 0;
	}

	if ((filesbuf = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
		perror("mmap");
		return 0;
	}

// copy the content of buf to filesbuf//
	memcpy(filesbuf, buf, size);
	printf("Files downloaded\n");

	return 0;
}

int upload(char *fileName, int sock, int length){
    int fd = 0;
	struct stat fileInfo;
	void *mybuf = 0;

//open file we want to upload
	if ((fd = open(fileName, O_RDONLY)) < 0){
		perror("open");
		return -1;
	}
//send this file name length to server through socket
	if (send(sock, &length, sizeof(int), 0) < 0){
		perror("send");
		return -1;
	}
//send file name to server
	if (send(sock, fileName, length, 0) < 0){
		perror("send");
		return -1;
	}
//Get file stat 
	if (fstat(fd, &fileInfo) < 0){
		perror("fstat");
		return -1;
	}
//send file size in bytes
	if (send(sock, &fileInfo.st_size, sizeof(off_t), 0) < 0){
		perror("send");
		return -1;
	}

//make a shared memory for processes, mybuf in a pointer for the mapped memory
	if ((mybuf = mmap(0, fileInfo.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED){
		perror("mmap");
		return -1;
	}
//send the pointer of the mapped memory
	if (send(sock, mybuf, fileInfo.st_size, 0) < 0){
		perror("send");
		return -1;
	}
	return 1;
}
