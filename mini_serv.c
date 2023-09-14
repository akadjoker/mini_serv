#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

const int MAX_CLIENT = 10;
const int MAX_BUFFER = 4096;

typedef struct
{
		int socket;
		int id;
		char *msg;
}Client;

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


void ft_print(const char* str)
{
	write(2,str,strlen(str));
}

void panic()
{
	ft_print("Fatal error\n");
	exit(1);
}

void send_str(Client *clients,const char* msg,int discard)
{
	for (int i=0;i<MAX_CLIENT;i++)
	{
		if (clients[i].socket>0)
		{
			if (clients[i].socket!=discard)
				send(clients[i].socket,msg,strlen(msg),0);
		}
	}
}


int main() 
{
	int server;
	struct sockaddr_in servaddr; 

	// socket create and verification 
	server = socket(AF_INET, SOCK_STREAM, 0); 
	if (server == -1) 
	{ 
		panic();
	} 
	

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(1479); 
  
	
	if ((bind(server, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) 
	{ 
		printf("socket bind failed...\n"); 
		panic(); 
	} 
	
	if (listen(server, 10) != 0) 
	{
		printf("cannot listen\n"); 
		panic();
	}

 	fd_set read_fd;
	FD_ZERO(&read_fd);

	Client clients[MAX_CLIENT+1];
	bzero(&clients,MAX_CLIENT * sizeof(Client));
	for (int i=0;i<MAX_CLIENT;i++)
	{
		clients[i].id=-1;
		clients[i].socket=0;
		clients[i].msg=NULL;
	}

	int max_fd =server;
	int client_id=0;
	char BUFFER[MAX_BUFFER+1];
	int done=0;

	while(!done)
	{
		FD_SET(server, &read_fd);

		for (int i=0;i<MAX_CLIENT;i++)
		{
			if (clients[i].socket>0)
			{
					FD_SET(clients[i].socket, &read_fd);
					max_fd= (clients[i].socket>max_fd)?clients[i].socket:max_fd;
			}
		}

		if (select(max_fd + 1 ,&read_fd,NULL,NULL,NULL)==-1)
		{
			continue;
		}

		if (FD_ISSET(server,&read_fd))
		{
			for (int i=0;i<MAX_CLIENT;i++)
			{
				if (clients[i].socket==0)
				{
						clients[i].socket = accept(server,NULL,NULL);
						if (clients[i].socket ==-1)
						{
							panic();
						}
						clients[i].id=client_id;
						clients[i].msg=NULL;
						client_id++;
						char msg[128];
						sprintf(msg,"server: client %d just arrived\n",clients[i].id);
						send_str(clients,msg,clients[i].socket);
						break;
				}
			}

		}

			for (int i=0;i<MAX_CLIENT;i++)
			{
					
					if (FD_ISSET(clients[i].socket,&read_fd) && clients[i].socket>0)
					{
						
						int total = recv(clients[i].socket,BUFFER,MAX_BUFFER,0);

						if (total<=0)//remove
						{
							    char msg[128];
								sprintf(msg,"server: client %d just left\n",clients[i].id);
								send_str(clients,msg,clients[i].socket);
							
								FD_CLR(clients[i].socket,&read_fd);
								close(clients[i].socket);
								clients[i].socket=0;
								clients[i].id=-1;
						} else
						{
							BUFFER[total]='\0';
							char *msg=NULL;
							clients[i].msg = str_join(clients[i].msg, BUFFER);
							while(extract_message(&clients[i].msg, &msg)==1)
							{
								char a[128];
								sprintf(a,"client %d: ",clients[i].id);
								send_str(clients,a,clients[i].socket);
								send_str(clients,msg,clients[i].socket);
								free(msg);
								msg=NULL;
								if (clients[i].msg!=NULL)
								{
									free(clients[i].msg);
									clients[i].msg=NULL;
								}
							}
						}
					}
			}
	}



	close(server);
	printf("end\n");



	return 0;
}
