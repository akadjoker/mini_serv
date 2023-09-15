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

typedef struct 
{
	Client **data;
	int count;
	int size;
}Array;



void init_array(Array *array, int size)
{
	array->data = (Client**) malloc(size * sizeof(Client*));
	array->size=size;
	array->count=0;
}

void free_array(Array *array)
{
	for (int i=0;i<array->count;i++)
	{
		if (array->data[i]->msg!=NULL)
		{
			free(array->data[i]->msg);
			array->data[i]->msg=NULL;
		}
		free(array->data[i]);
		
	}
	free(array->data);
}

Client* array_add(Array *a, int id,int socket)
{
	if (a->count==a->size)
	{
		int  newSize = a->size *2;
		a->size = newSize;
		Client **array =(Client**) realloc(a->data, newSize *  sizeof(Client*));
		a->data=array;
	}
	Client* client;
	client=(Client *)malloc(sizeof(Client));
	client->id=id;
	client->socket=socket;
	client->msg = NULL;
	a->data[a->count]=client;
	a->count++;
	return client;
}

int array_get_index(Array* a,int id)
{

	for (int i=0;i<a->count;i++)
	{
		if (a->data[i]->id==id)
			return i;
	}
	return -1;
}

void array_remove(Array *a,int id)
{
	int index = array_get_index(a, id);
	if (index==-1)
	{
		return;
	}
	free(a->data[index]->msg);
	free(a->data[index]);
	for (int i=index;i<a->count-1;i++)
	{
		a->data[i]=a->data[i+1];
	}
	a->count--;
}


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

void send_str(Array *array,const char* msg,int discard)
{
	for (int i=0;i<array->count;i++)
	{
		if (array->data[i]->socket>0)
		{
			if (array->data[i]->socket!=discard)
				send(array->data[i]->socket,msg,strlen(msg),0);
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
		
		panic(); 
	} 
	
	if (listen(server, 10) != 0) 
	{

		panic();
	}

 	fd_set read_fd;
	FD_ZERO(&read_fd);

	Array array;
	init_array(&array,100);

	int max_fd =server;
	int client_id=0;
	char BUFFER[MAX_BUFFER+1];
	int done=0;

	while(!done)
	{
		FD_SET(server, &read_fd);

		for (int i=0;i<array.count;i++)
		{
			if (array.data[i]->socket>0)
			{
					FD_SET(array.data[i]->socket, &read_fd);
					max_fd= (array.data[i]->socket>max_fd)?array.data[i]->socket:max_fd;
			}
		}

		if (select(max_fd + 1 ,&read_fd,NULL,NULL,NULL)==-1)
		{
			continue;
		}

		if (FD_ISSET(server,&read_fd))
		{
						int new_socket = accept(server,NULL,NULL);
						if (new_socket ==-1)
						{
							panic();
						}
						Client* client = array_add(&array,client_id,new_socket);
						if (client==NULL)
							panic();
						client_id++;
						char msg[128];
						sprintf(msg,"server: client %d just arrived\n",client->id);
						send_str(&array,msg,client->socket);
					
		}

			for (int i=0;i<array.count;i++)
			{
					
					if (FD_ISSET(array.data[i]->socket,&read_fd) && array.data[i]->socket>0)
					{
						
						int total = recv(array.data[i]->socket,BUFFER,MAX_BUFFER,0);

						if (total<=0)//remove
						{
							    char msg[128];
								sprintf(msg,"server: client %d just left\n",array.data[i]->id);
								send_str(&array,msg,array.data[i]->socket);
								FD_CLR(array.data[i]->socket,&read_fd);
								close(array.data[i]->socket);
								array_remove(&array,array.data[i]->id);
						} else
						{
							BUFFER[total]='\0';
							char *msg=NULL;
							array.data[i]->msg = str_join(array.data[i]->msg, BUFFER);
							while(extract_message(&array.data[i]->msg, &msg)==1)
							{
								char a[128];
								sprintf(a,"client %d: ",array.data[i]->id);
								send_str(&array,a,array.data[i]->socket);
								send_str(&array,msg,array.data[i]->socket);
								free(msg);
								msg=NULL;
								if (array.data[i]->msg!=NULL)
								{
									free(array.data[i]->msg);
									array.data[i]->msg=NULL;
								}
							}
						}
					}
			}
	}


	free_array(&array);
	close(server);
	return 0;
}
