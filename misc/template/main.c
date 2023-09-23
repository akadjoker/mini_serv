#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h> 
#include <stdio.h>
#include <stdlib.h>

int server=0;

void clean_all()
{
    if (server>0)
    {
        close(server);
    }

}

void erro_msg(char *msg)
{
  write(2, msg, strlen(msg));
  clean_all();
  exit(1);
}

int main(int argc, char **argv) 
{

 if (argc != 2)
 {
  erro_msg("Wrong number of arguments\n");
 }




 struct sockaddr_in servaddr;
 
//  socklen_t len;
 
//  , cli; 

// socket create and verification 
server = socket(AF_INET, SOCK_STREAM, 0); 
if (server == -1)
    erro_msg("Fatal error\n");

bzero(&servaddr, sizeof(servaddr)); 

servaddr.sin_family = AF_INET; 
servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
servaddr.sin_port = htons(atoi(argv[1])); 


if ((bind(server, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    erro_msg("Fatal error\n");
if (listen(server, 10) != 0)
    erro_msg("Fatal error\n");


clean_all();

return 0;
}