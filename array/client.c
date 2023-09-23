#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

void func(int sockfd)
{
	char buf[10];
	buf[0] = 'a';
	buf[1] = '\n';
	buf[2] = 'b';
	buf[3] = '\n';
	buf[4] = 0;

	send(sockfd, buf, 5, 0);
}

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return write(STDERR_FILENO, "Fatal error\n", 13) && 1;

	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	server_addr.sin_port		= htons(1478);

	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		return write(STDERR_FILENO, "Fatal error\n", 13) && 1;

	func(sockfd);
	close(sockfd);
	return 0;
}
