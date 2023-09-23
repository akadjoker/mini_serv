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
#include <signal.h>



int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


typedef struct Client
{
		int id;
		int fd;
		char *msg;
}Client;


int server;
int clients_count=0;
int clients_ids=0;
Client clients[FD_SETSIZE+1];
int clientsfds[FD_SETSIZE+1];


int create_client(int fd)
{
	int id = clients_ids;
	clients[fd].fd=fd;	
	clients[fd].id=id;	
	if (clients[fd].msg!=NULL)
		free(clients[fd].msg);
	clients[fd].msg=NULL;
	clientsfds[clients_count] = fd;
	clients_count++;
	clients_ids++;
	return id;
}

int remove_client(int fd)
{
	int id=clients[fd].id;
	clients[fd].fd=-1;	
	clients[fd].id=-1;	
	if (clients[fd].msg!=NULL)
		free(clients[fd].msg);
	clients[fd].msg=NULL;
	for (int i=0;i<clients_count;i++)
	{
		if (clientsfds[i]==fd)
		{
			for (int j=i;j<clients_count;j++)
			{
				clientsfds[j]=clientsfds[j+1];
			}
			break;
		}
	}
	clients_count--;
	return id;
}


void remove_clients()
{
	for (int i=0;i<FD_SETSIZE;i++)
	{
		if (clients[i].fd>0)
			close(clients[i].fd);
		if (clients[i].msg!=NULL)
			free(clients[i].msg);
	}
}

void send_msg(const char* msg, int ignore)
{
	for (int i=0;i<=clients_count;i++)
	{
		if (clientsfds[i]>0 && clientsfds[i]!=ignore)
		{
			send(clientsfds[i],msg,strlen(msg),0);
		}
	}
}


void erro_msg(char *msg)
{
	remove_clients();
    write(2, msg, strlen(msg));
   exit(1);
}



void handlesginal(int sig)
{
    (void)sig;
	erro_msg("\nAborting...\n");
    
}


int main(int argc, char **argv) 
{

 if (argc != 2)
 {
  erro_msg("Wrong number of arguments\n");
 }



signal(SIGINT, handlesginal);
signal(SIGQUIT, handlesginal);
signal(SIGTERM, handlesginal);



 struct sockaddr_in servaddr, cli; 
  socklen_t len;
 
server = socket(AF_INET, SOCK_STREAM, 0); 
if (server == -1)
    erro_msg("Fatal error\n");

bzero(&servaddr, sizeof(servaddr)); 

servaddr.sin_family = AF_INET; 
servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
servaddr.sin_port = htons(atoi(argv[1])); 
//servaddr.sin_port = htons(1478); 


if ((bind(server, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    erro_msg("Fatal error\n");
if (listen(server, 10) != 0)
    erro_msg("Fatal error\n");


for (int i=0;i<FD_SETSIZE;i++)
{
	clients[i].id=-1;
	clients[i].fd=-1;
	clients[i].msg=NULL;
	clientsfds[i]=-1;
}


fd_set master,fd_list;
FD_ZERO(&fd_list);
FD_SET(server,&fd_list);

while(1)
{


	int max_fd=server;
	for (int i=0;i<=clients_count;i++)
	{
		if (clientsfds[i]>max_fd)
			max_fd = clientsfds[i];

	}

    printf("process Max fd: %d - total: %d  Mids: %d \n",max_fd,clients_count,clients_ids);

	master = fd_list;
	if (select(max_fd+1,&master,NULL,NULL,NULL)<=0)
	{
		erro_msg("select Fatal error\n");
	}

	for (int fd=0;fd<=max_fd;fd++)
	{
		if (FD_ISSET(fd,&master))
		{
			if (fd==server)
			{	
				   		len = sizeof(cli);
                        int socket = accept(server, (struct sockaddr *)&cli, &len);
                        if (socket<=0)
                        {
                            erro_msg("accept : Fatal error\n");
                        }

						int id = create_client(socket);
						FD_SET(socket,&fd_list);
						char status[80]={'\0'};
						sprintf(status,"server: client %d just arrived\n",id);
						send_msg(status,socket);
                        

			} else
			{
				char buffer[4096]={'\0'};
				int bytes = recv(fd,buffer,sizeof(buffer)-1,MSG_DONTWAIT);
				if (bytes<=0)
				{
					int id =remove_client(fd);
					FD_CLR(fd,&fd_list);
					close(fd);
					char status[80]={'\0'};
					sprintf(status,"server: client %d just left\n",id);
					send_msg(status,fd);

				} else
				{
					buffer[bytes]='\0';
					char *text	=NULL;
					clients[fd].msg = str_join(clients[fd].msg,buffer);
					while(extract_message(&clients[fd].msg,&text))
					{
						char status[80]={'\0'};
					sprintf(status,"client %d: ",clients[fd].id);
					send_msg(status,fd);
					send_msg(text,fd);
					}
				}
			}
		}
	}


}


remove_clients();

}

