// Server side C program to demonstrate Socket programming 
#include <sys/mman.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#define PACKAGE "crlock"
#define PORT 50000 
#define PORT2 8080
#define MAXLINE 1024

int download(int newfd, int amount); 
int upload(int newfd, int amount); 
//get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void) { 
	int server_fd, newfd, valread ,action = 0,size = 0,fd,countFiles = 0,amount = 0, ibuf=0;
    	void * dst;
    	int* fileFD = (int*)malloc(1);
    	fd_set master;    // master file descriptor list
    	fd_set read_fds;  // temp file descriptor list for select()
    	int fdmax;        // maximum file descriptor number 
	struct sockaddr_in address; 
    	struct sockaddr_storage remoteaddr; // client address
	int opt = 1; 
	int addrlen = sizeof(address); 
	char* buf = 0;
	//Log
	FILE *f;
	f = fopen("log", "a+");
	if (f == NULL)
	printf("error - log\n");
	time_t clk = time(NULL);
	fprintf(f, "%s >Server main operation\n", ctime(&clk));
	//fclose(f);
	printf("Enter 1 to enter the server\nEnter 2 to file lock/unlock tool\n");
	int enter=0;
	scanf("%d",&enter);
	if(enter == 1)
	{
	  int ui=0,ans=0;
	  printf("Check if you are robot or human\nEnter a number: ");
	  scanf("%d",&ui);
	  printf("\nWhat the square of your number? \n");
	  scanf("%d",&ans);
	  ui=pow(ui,2);

		if (ui!=ans)
		{
			printf("Try next time... The answer is: %d\n",ui);
			fprintf(f, "%s >Server main operation [user failed to run the server - can be robot]\n", ctime(&clk));
	    		exit(1);
		}
		printf("**********Welcome to the sever**********\n"); 
		printf("Enter 1 to recive/send files\nEnter 2 to recive msg\n"); 
		int xyz=0;
		scanf("%d",&xyz);
		if (xyz==1){
		fprintf(f, "%s >Server main operation [user run the server - pass]\n", ctime(&clk));
	   	char remoteIP[INET6_ADDRSTRLEN]; //IPv6
		// Creating socket file descriptor 
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
			perror("socket failed"); 
			fprintf(f, "%s >Server main operation [socket failed]\n", ctime(&clk));
			exit(EXIT_FAILURE); 
		} 
		//Set Socket options & allow reuse of local addresses and socket tansfter at the same port
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
			perror("setsockopt"); 
			fprintf(f, "%s >Server main operation [socket failed]\n", ctime(&clk));
			exit(EXIT_FAILURE); 
		} 
		address.sin_family = AF_INET; 
		address.sin_addr.s_addr = INADDR_ANY; 
		address.sin_port = htons( PORT ); 
		
		//Bind the name to the socket
		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
			perror("bind failed"); 
			fprintf(f, "%s >Server main operation [bind failed]\n", ctime(&clk));
			exit(EXIT_FAILURE); 
		} 
		//listen for connections on a socket
		if (listen(server_fd, 3) < 0) { 
			perror("listen"); 
			fprintf(f, "%s >Server main operation [listen failed]\n", ctime(&clk));
			exit(EXIT_FAILURE); 
		} 
	    FD_ZERO(&master);// clear the master and temp sets
	    FD_ZERO(&read_fds);

	    //add the listener to the master set
	    FD_SET(server_fd, &master);

	    //keep track of the biggest file descriptor
	    fdmax = server_fd; // so far, it's this one

	    for(;;) {  
		read_fds = master; // copy the fd from master to read_fds
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)// select enable to monitor muliple fd
		 {
		    perror("select");
			fprintf(f, "%s >Server main operation [select failed]\n", ctime(&clk));
		    exit(4);
		}
		for(int i = 0; i <= fdmax; i++) {
		    if (FD_ISSET(i, &read_fds)) { //Tests to see if a fd is part of the set
		        if (i == server_fd) {
		            //handle new connections
		            addrlen = sizeof remoteaddr; //client addr size
					//accept() the connection and add it to the master set.
					if ((newfd = accept(server_fd,(struct sockaddr *)&remoteaddr,&addrlen)) == -1) { 
				       		perror("accept");
						fprintf(f, "%s >Server main operation [accept failed]\n", ctime(&clk));
				     
				}
				else {
		                FD_SET(newfd, &master); // add to master set
		                if (newfd > fdmax) {    // keep track of the max
		                    fdmax = newfd;
		                }
		                printf("selectserver: new connection from %s on socket %d\n",
								inet_ntop(remoteaddr.ss_family,
		                    get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN),newfd);
				fprintf(f, "%s >Server main operation [socket connection from client - pass]\n", ctime(&clk));
		            }
		        } else {
		            //recv argc - this way we'll know how many files we need to download/upload
		            if(recv(i, &amount,sizeof(int),0) < 1){
		                perror("Could not receive amount off files to download/upload");
				fprintf(f, "%s >Server main operation [Could not receive amount off files to download/upload]\n", ctime(&clk));
		                return -1;
		            }           
		            // handle data from a client
		            if(recv(i, &action,sizeof (int),0) < 1){ //'upload' = 2, 'download' = 1
		                perror("recv");
				fprintf(f, "%s >Server main operation [handle data from a client error]\n", ctime(&clk));
		                return -1;
		            }    
		            //choose download/upload
		            switch(action){
		                case 1:
		                if(download(newfd,amount)==0){
		                    perror("download");
				    fprintf(f, "%s >Server main operation [download error]\n", ctime(&clk));
		                    return -1;//dowbload failed
		                }
		                else {
		                    //close(newfd);
		                    FD_CLR(i, &master); // remove from master set
		                }
		                break;
		                case 2:
		                if(upload(newfd,amount)==0){
		                    perror("upload");
				    fprintf(f, "%s >Server main operation [upload error]\n", ctime(&clk));
		                    return -1;//upload failed
		                }
		                else {
		                    close(newfd);
		                    FD_CLR(i, &master); // remove from master set                           
		                }
		                break;
		                default:
		                    close(newfd);
		                    FD_CLR(i, &master); // remove from master set
		                    break;
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
     } // END for(;;)
   }
    if (xyz ==2){
	    // getting message from the client
       	    //Log
	    FILE *f;
	    f = fopen("log", "a+");
	    if (f == NULL)
	    printf("error - log\n");
	    time_t clk = time(NULL);
	    fprintf(f, "%s >Server chat operation\n", ctime(&clk));

	    int sockfd;
	    char buffer[MAXLINE];
	    struct sockaddr_in servaddr, cliaddr;
	      
	    // Creating socket file descriptor
	    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
	    	fprintf(f, "%s >Server chat operation [socket creation failed]\n", ctime(&clk));
		exit(EXIT_FAILURE);
	    }
	      
	    memset(&servaddr, 0, sizeof(servaddr));
	    memset(&cliaddr, 0, sizeof(cliaddr));
	      
	    // Filling server information
	    servaddr.sin_family = AF_INET; // IPv4
	    servaddr.sin_addr.s_addr = INADDR_ANY;
	    servaddr.sin_port = htons(PORT2);
	      
	    // Bind the socket with the server address
	    if ( bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
	    {
		perror("bind failed");
	    	fprintf(f, "%s >Server chat operation [bind failed]\n", ctime(&clk));
		exit(EXIT_FAILURE);
	    }
	  
	    int len, n;
	    len = sizeof(cliaddr);  //len is value/result
	    fclose(f);
	for(;;){
	    f = fopen("log", "a+");
	    if (f == NULL)
	    printf("error - log\n");
	    time_t clk = time(NULL);
	    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr,&len);
	    buffer[n] = '\0';
	    printf("Client : %s\n", buffer);
	    fprintf(f, "%s >Server chat operation [client chat text: %s]\n", ctime(&clk),buffer);
	    fclose(f);
   	}//END for
	    
     }//END if	
  }//END main

	//lock/unlock files tool
	 if (enter == 2){
		 getchar ();
		 //log
		 FILE *f;
		 f = fopen("log", "a+");
		 if (f == NULL)
		 	printf("error - log\n");
		 time_t clk = time(NULL);
	         fprintf(f, "%s >Lock/Unlock tool\n", ctime(&clk));
		 int fd=0;
		 struct flock lock;
		 char file[20];
		 printf ("Enter file name:");
		 scanf ("%s",file);
		 printf ("opening %s\n", file);
		 //Open a file descriptor to the file 
		 printf ("Enter lock time [seconds]:");
		 int locktime=0;
		 scanf("%d",&locktime);
		 getchar ();
		 printf ("locking\n");
		 fprintf(f, "%s >Lock/Unlock tool [Locked file]\n", ctime(&clk));
		 //Initialize the flock structure
		 memset (&lock, 0, sizeof(lock));
		 lock.l_type = F_WRLCK;
		 lock.l_whence=SEEK_SET;
		 lock.l_start=0;
		 lock.l_len=0;
		 lock.l_pid=getpid();
		 //Place a write lock on the file 
		 int i=locktime;
		 fcntl (fd, F_SETLK, &lock);
		 for(; i > 0; i--){
		 	if((fd = open(file,O_RDWR|O_EXCL)) != 0){
		 		fprintf(stderr, "[Pid number:%d][Seconds:%d]: File locked ...\n",lock.l_pid,i);
				fcntl (fd, F_GETLK, &lock);
				sleep(1);
		 	}
			else{
				perror("lock fail");
			 	fprintf(f, "%s >Lock/Unlock tool [Lock fail]\n", ctime(&clk));
			}
		 }
		 printf ("Lock time expired (seconds:%d)\npress Enter to unlock... ",locktime);
		 //Wait for the user to hit Enter 
		 getchar ();

		 printf ("unlocking\n");
		 fprintf(f, "%s >Lock/Unlock tool [Unlocked file]\n", ctime(&clk));
		 //Release the lock 
   		 //unlink(file);
		 lock.l_type = F_UNLCK;
 		 lock.l_whence=SEEK_SET;
		 lock.l_start=0;
		 lock.l_len=0;
		 lock.l_pid=getpid();
		 fcntl (fd, F_SETLK, &lock);
		 fclose(f);
		 close (fd);
	 	 return 0;
	}
 fclose(f);
}
int download(int newfd, int amount){
  	int length = 0,pid;
    	int nameLen = 0;
	int myFiles = 2;
	int fdin = 0;
	struct stat statbuf;
	void *src;
   	char *buff;
	int i = 0;
	//Log
	FILE *f;
	f = fopen("log", "a+");
	if (f == NULL)
	printf("error - log\n");
	time_t clk = time(NULL);
	fprintf(f, "%s >Download operation\n", ctime(&clk));
	//fclose(f);
    //recieving amount of files into length
	if (recv(newfd, &length, sizeof(int), 0) <= 0){
		perror("recv");
		return 0;
		fprintf(f, "%s >Download operation [recv error]\n", ctime(&clk));
	}
	if(length == 0){
		printf("no Files");
		fprintf(f, "%s >Download operation [no files error]\n", ctime(&clk));
		return 0;
	}
    printf("length: %d\n",length);
	//zip - if we download from server more then 1 file its compress to zip and download the zip to the client
	char** files = (char**)malloc((length+3)*sizeof(char*));
	memset(files,0,length+3);
	files[0] = (char*)malloc(256*sizeof(char));
	memset(files[0],0,256);
	strcpy(files[0],"zip");
	files[0][256]='\0';
	files[1] = (char*)malloc(256*sizeof(char));
	memset(files[1],0,256);
	strcpy(files[1],"MyFiles.zip");
	files[1][256]='\0';
	//files[0] = "zip";
	//files[1] = "MyFiles.zip";
	//files[2] = "name of file";
	//    		.
	//		.
	//		.
	//files[n] = "name of file";

    	//get files from client
	for( i=0; i<length;++i){
        	//file length
		if (recv(newfd, &nameLen, sizeof(int), 0) <= 0){
			perror("recv");
		        fprintf(f, "%s >Download operation [file length error]\n", ctime(&clk));
			return 0;
		}
		if(nameLen == 0){
			perror("Didnt receive data from client !");
		        fprintf(f, "%s >Download operation [Didnt receive data from client]\n", ctime(&clk));
			return 0;
		}
        	//Allocating memory from nameLen which is the file length
		buff = (char *)malloc((nameLen + 1) * sizeof(char));
		if( buff == NULL){
			perror("could not allocate memory");
		        fprintf(f, "%s >Download operation [could not allocate memory]\n", ctime(&clk));
			return 0;
		}

		memset(buff, 0, nameLen);
       		//get file name
		if (recv(newfd, buff, nameLen, 0) <= 0){
			perror("recv");
			return 0;
		}

		buff[nameLen] = '\0';

		files[myFiles] = (char*)malloc((nameLen+1)*sizeof(char));

		if(files[myFiles] == NULL){
			perror("malloc");
			return -1;
		}
        	//set into files[myFiles] - file name
		memset(files[myFiles],0,nameLen);
		files[myFiles][nameLen+1] = '\0';
		strcpy(files[myFiles],buff);
		myFiles++; // in order to get another file...
		free(buff);
	}

	files[length+3] = NULL;

	int flag = 1;
	//checks whether the calling process can access the file pathname
	for(i = 0; i < length; ++i){
		if(access(files[i+2],F_OK ) == -1){
			perror("access");
			flag = -1;
			break;
		}
	}
    	//Sending authantication
	if (send(newfd, &flag, sizeof(int), 0) < 0){
		perror("Error could not send data");
		fprintf(f, "%s >Download operation [Error could not send data]\n", ctime(&clk));
		return -1;
	}

	if(flag == -1){
		perror("requested files all do not exist");
		fprintf(f, "%s >Download operation [requested files all do not exist]\n", ctime(&clk));
		return -1;
	}

	//make new process to run excecvp - Using vfork, same as fork but has a separate memory space from the parent process.
	if(pid=vfork() == 0){
		if(length > 1){
			if (execvp("zip",files) == -1){
                perror("execvp");
            }
		}
		else exit(1);
	}
	else {
		//Before wait to child process
		wait(NULL);
		if(length > 1){//if there is more than 1 files
			if ((fdin = open("MyFiles.zip", O_RDONLY)) < 0){
				perror("Could not open FILE with given name: 'MyFiles.zip'\n");
				fprintf(f, "%s >Download operation [Could not open FILE with given name: 'MyFiles.zip]\n", ctime(&clk));
				return -1;
			}
		}
		else{
			if ((fdin = open(files[2], O_RDONLY)) < 0){
				perror("Could not open FILE with given name\n");
				fprintf(f, "%s >Download operation [Could not open FILE with given name]\n", ctime(&clk));
				return -1;
			}
		}
		//get file status
		if (fstat(fdin, &statbuf) < 0){
			perror("fstat");
			return -1;
		}
		//send file size
		if (send(newfd, &statbuf.st_size, sizeof(off_t), 0) < 0){
			perror("send");
			fprintf(f, "%s >Download operation [Send file error]\n", ctime(&clk));
			return -1;
		}
		//get file from memory
		if ((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED){
			perror("mmap");
			return -1;
		}
		//send file to client
		if (send(newfd, src, statbuf.st_size, 0) < 0){
			perror("send");
			return -1;
		}

		printf("Sent to client\n");
		fprintf(f, "%s >Download operation [Download file - pass]\n", ctime(&clk));
		free(files);

		// using vfork to excecute code
		if(vfork() == 0){
			if(length > 1){
				// removing the zip file in server
				execlp("rm","-rf","MyFiles.zip",NULL);
			}
			exit(1);
		}
	}
	fclose(f);
	return 1;
} 
int upload(int newfd, int amount){
    	int i = 0,length = 0,fd = 0;
    	void *mybuf;
	char *buf;
	off_t size = 0;
	//Log
	FILE *f;
	f = fopen("log", "a+");
	if (f == NULL)
	printf("error - log\n");
	time_t clk = time(NULL);
	fprintf(f, "%s >Upload operation\n", ctime(&clk));
	//fclose(f);
	for (i = 0; i < amount; ++i){
        	//recieve file name length
		if (recv(newfd, &length, sizeof(int), 0) < 1){
			perror("recv");
			fprintf(f, "%s >Upload operation [recv error]\n", ctime(&clk));
			return 0;
		}
		if(length == 0){
			printf("no files");
			fprintf(f, "%s >Upload operation [no files]\n", ctime(&clk));
			return -1;
		}
        char *buf = (char *)malloc((length + 1));
        
		if(buf == NULL){
			perror("malloc");
			fprintf(f, "%s >Upload operation [malloc error]\n", ctime(&clk));
			return 0;
		}
		memset(buf, 0, length);
       		//recv file name content
		if (recv(newfd, buf, length, 0) < 1){
			perror("recv");
			fprintf(f, "%s >Upload operation [recv error]\n", ctime(&clk));
			return 0;
		}
        buf[length] = '\0';
        
		if(buf[0] == EOF){
			perror("No data");
			fprintf(f, "%s >Upload operation [no data]\n", ctime(&clk));
			return 0;
		}
		//open buf 
		if ((fd = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0){
			perror("open");
			fprintf(f, "%s >Upload operation [open buf error]\n", ctime(&clk));
			return 0;
		}
		free(buf);

       		//fstat info, recive file size
		if ((recv(newfd, &size, sizeof(size), 0)) < 1){
			perror("receive");
			return 0;
		}
		if(size == 0){
			printf("No data in file");
			fprintf(f, "%s >Upload operation [no data in file error]\n", ctime(&clk));
			return -1;
		}
		//same variable "buf" with differnt purpose
		buf = malloc(size * sizeof(void *));
		if(buf == NULL){
			perror("malloc");
			return 0;
		}
        	//set memory for the file in memory map with 0
		memset(buf, 0, size);
		//recive file from client
		if ((recv(newfd, buf, size, 0)) < 1){
			perror("recv");
			return 0;
		}
       		//check if received data to buf
		if(*((int*)buf) == 0){
			perror("no data");
			fprintf(f, "%s >Upload operation [no data in buf error]\n", ctime(&clk));
			return 0;
		}
		//move to eof
		if (lseek(fd, size - 1, SEEK_SET) == -1){
			perror("lseek");
			fprintf(f, "%s >Upload operation [EOF error]\n", ctime(&clk));
			return 0;
		}
		//write the file until the eof
		if (write(fd, "", 1) != 1){
			perror("write");
			fprintf(f, "%s >Upload operation [write error]\n", ctime(&clk));
			return 0;
		}
		//make a shared memory for processes, mybuf in a pointer for the mapped memory
		if ((mybuf = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
			perror("map");
			fprintf(f, "%s >Upload operation [shared memory error]\n", ctime(&clk));
			return 0;
		}
		fprintf(f, "%s >Upload operation [Uploaded file - pass]\n", ctime(&clk));
		fclose(f);
		memcpy(mybuf, buf, size);
		free(buf);
	
	}
	return 1;
}
