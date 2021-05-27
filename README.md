# Unix-File_Server_and_OneWay_Chat
A file server with Stream Socket and one-way chat from client to the server with Datagram socket, this project using much more APIs, ran and tested on Unix environment (C code)

To use the project, send an email to the code writer,
Use is permitted for private use only.

APIs list:

1.	argc, argv (used at client see below)
2.  null terminated strings - strcat(3), strstr(3), strchar(3) (used at server and client)
3.	File API of C (fread, fwrite, fopen etc.) (used at server and client)
4.  read and write (used at server and client)
5.	mmap (used at server and client)
6.	IPv4 (used at server and client)
7.	IPv6 (used at server and client)
8.	Stream sockets (used at server and client -> files)
9.	datagram sockets (used at server and client -> chat)
10.	IP multiplexing (used at server and client)
11.	poll (used at client -> press exit to quit the chat)
12. wait and vfork (used at server -> to download more then one file, the server compress to ZIP the files and transfer to the client)
13. fcntl (used at server -> file PID lock)
14. math lib (-lm) (used at server -> enter the server with human question)
15. time.h lib for log.txt file at the server (used at server -> create log with timestamp)

Run: (the project in UNIX environment)
./server
  Enter 1 to enter the server
    Check if you are robot or human
    Enter a number
      ************Welcome to the server************
      Enter 1 to recive/send files
      Enter 2 to recive msg
    What the square of your number?  
  Enter 2 to file lock/unlock tool
    Enter file name
      Enter lock time [seconds]
      
./client download testfile2 testfile3 testfile4
./client upload testfile2 testfile3 testfile4
./client chat
  (chat msg)
  exit (wait 2.5 and exit)
compile:
gcc -g client.c -o client
gcc -g server-math-lock-log.c -lm -o server


