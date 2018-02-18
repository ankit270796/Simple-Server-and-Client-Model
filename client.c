#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 100

int main(int argc, char const *argv[])
{
  int ank_sock,a,n;
  struct hostent *hostname;
  // Creation of a socket 
  ank_sock = socket(AF_INET, SOCK_STREAM, 0);
  
  // Check if the creation was successful
  if(ank_sock<=0)
     printf("Socket Creation was Unsucessful\n");
  else
     printf("Socket Creation Sucessful\n");
  
  if (argc < 3) {
  	printf("Invalid usage. Try ..\n");
  	printf("$ ./echo IP_Addr Port_No\n");
  	return 1;
  }

  // connecting to a remote server
  struct sockaddr_in rem_serv;
  memset(&rem_serv,0,sizeof(rem_serv));
  rem_serv.sin_addr.s_addr = inet_addr(argv[1]);
  rem_serv.sin_family=AF_INET;
  rem_serv.sin_port=htons(atoi(argv[2]));
  
  if((a=connect(ank_sock,(struct sockaddr*)&rem_serv,sizeof(rem_serv)))<0)  {
   perror("couldn't connect");
   exit(1);
  }
  //printf("%d",a);
  char mess[100]="anit";
  char ankit[100]="hello";
    
    
   // while(1){
       printf("server:");
      // reading the reply from the server
      if((n=read(ank_sock,&mess,sizeof(mess)))<0)
        perror("message couldn't be recei");
      else
        printf("%s",mess);
      
      while (1) {

      // receiving the messsage from the user
      printf("me:"); 
      fgets (ankit, MAX_MSG_LEN, stdin);
//      scanf("%s", ankit);
      // When the user enters ^D (EOF), close the connection and exit loop
      if (strlen(ankit) == 0)
        break;
    
      // writing to the server
      if((n=write(ank_sock,&ankit,100)<0))
        perror("message couldn't be sent");
     
      memset(mess,0,strlen(mess));
      memset(ankit,0,strlen(ankit));
  
      printf("server:  ");
      // reading the reply from the server
      if((n=read(ank_sock,&mess,sizeof(mess)))<0)
        perror("message couldn't be recei");
      else
        printf("%s\n",mess);
    // }
    }
  
      close(ank_sock);
      
    
  //printf("port no %d",rem_serv.sin_port);
  //printf("%d",ntohs(rem_serv.sin_port));
}