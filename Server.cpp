#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h> 
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/wait.h>
#include <netinet/in.h>
#include <string.h>
#include <string>

/*called when a system call fails*/
void error(char *msg)
{
    perror(msg);
    exit(1);
}

void fireman(int)
{
   while(waitpid(-1, NULL, WNOHANG) > 0)
      std::cout << "";
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen, pid,n;
     struct sockaddr_in serv_addr, cli_addr;
     char buffer[256];

     if (argc < 2) {
         std::cerr << "ERROR, no port provided\n";
         exit(1);
     } 

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        std::cerr << "ERROR opening socket";

     bzero((char *) &serv_addr, sizeof(serv_addr));

     portno = atoi(argv[1]);

     /*defining the address of the server, sockaddr_in struct*/https://i.imgur.com/jQerDEt.gifv
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     /*bind socket to address.bind address of host and port number*/
     if(bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              std::cerr << "ERROR on binding";

     listen(sockfd,100);
     clilen = sizeof(cli_addr);

     signal(SIGCHLD,fireman);
     while (true) {    
         /*causes the process to block until a client connects to the server*/
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
        
         /*fork system call to create the child processes*/
         if (fork() == 0){

             //std::cout << "A child process started" << std::endl;
             /*child process*/

             if(newsockfd < 0)
                std::cerr << "ERROR on accept\n";
            
             /*FIRST MESSAGE*/
             bzero(buffer, 256);
             n = read(newsockfd, buffer, 256);
             if(n < 0) 
                std::cerr << "ERROR reading from the socket\n";
            
            std::cout << "Client: " << buffer << std::endl;

            int size = strlen(buffer);
            char receivedmsg[size];
            for(int i =0; i < size;i++)
                receivedmsg[i] = buffer[i];

            std::cout << "Received message: ";
            for(int i =0; i < size; i++)
                std::cout << receivedmsg[i];
            n = write(newsockfd,"Server:I got your the message",30);
            if(n < 0)
                std::cerr << "ERROR writing into socket\n";
            /*END OF FIRST MESSAGE*/



            /*SECOND MESSAGE -- SYMBOL*/
             bzero(buffer, 256);
             n = read(newsockfd, buffer, 256);
             if(n < 0) 
                std::cerr << "ERROR reading from the socket\n";

            char receivedsymbol = buffer[0];
           
            if(n < 0)
                std::cerr << "ERROR writing into socket\n";

            char binary[size];
            for(int i =0; i < size;i++){
                if(receivedmsg[i] == receivedsymbol)
                    binary[i] = '1';
                else
                    binary[i] = '0'; 
            }

            n = write(newsockfd, binary, strlen(binary));
            if(n < 0)
                std::cerr << "ERROR writing into socket\n";
            /*END OF RECEIVING ANOTHER MESSAGE*/
             
             close(newsockfd);
             _exit(0);
         }
     }
      /* end of while */   
     close(sockfd);
     return 0; /* we never get here */
}



