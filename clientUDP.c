#include <errno.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

char*
stringify(
	int mleft,
	int mmiddle,
	int mright,
	int mbytes,
	signed char mx,
	signed char my,
	unsigned char *mdata
){
	if(mbytes > 0)
	{			
		mleft = mdata[0] & 0x1;
		mright = mdata[0] & 0x2;
    	mmiddle = mdata[0] & 0x4;
		mx = mdata[1];
		my = mdata[2];
	}
	char *result = (char*)malloc(14);
	sprintf(result, "%d,%d,%d,%d,%d", mx, my, mleft, mmiddle, mright);
	return result;
}

int
main(int argc, char*argv[])
{
	// check user uinput
	if (argv[1]==0)
	{
		printf("usage: tiom [ipv4] [port](optional)");
		return 1;
	}
	// set all required variables to create connection
	int port = (argv[2] != NULL) ? atoi(argv[2]) : 6968;
	// buffer size is 14 (3*int, 2*signed char)
	int	bufferSize = 14;
	// create socket with UDP comunication
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	// mouse file descriptor
	int	mfd;
	// readed bytes from mdf
	int mbytes;
	// mouseleft, mousemiddle, mouseright
	int mleft, mmiddle, mright;

	// buffer 14 bytes - mx(1), my(1), mleft(4), mmiddle(4), mright(4)
	char *buffer;
	// set ip of server 
	char *ip = argv[1];

	// mouse position per update
	signed char mx, my;
	
	// mouse readed values
	unsigned char mdata[3];
	
	struct sockaddr_in addr;

	socklen_t addr_size;
	
	// print error when cant create socket
	if (sockfd == -1)
	{
		perror("socket() error");
		exit(EXIT_FAILURE);
	}

	// set address family
	addr.sin_family = AF_INET;
 	// set port to operate with
	addr.sin_port = htons(port);
	// convert char*ip and put it in addr.sin_addr
	if (inet_pton(AF_INET, ip, &addr.sin_addr)<=0)
	{
		perror("inet_pton() error");
		exit(EXIT_FAILURE);
	}
	// store size of server address as variable
	addr_size = sizeof(addr);

    const char *pDevice = "/dev/input/mice";
    mfd = open(pDevice, O_RDWR);

	if(mfd == -1)
	{
		printf("ERROR Opening %s\n", pDevice);
		return -1;
	}
	
	while (1)
	{
		// read data from mfd
		mbytes = read(mfd, mdata, sizeof(mdata));
		
		buffer = stringify(mleft, mmiddle, mright, mbytes, mx, my, mdata);

		// send info in bytes 
		sendto(sockfd,buffer,bufferSize,0,(struct sockaddr*)&addr,addr_size);	
		printf("info: %s\n",buffer);

		// free memory
		free(buffer);
	}
	close(sockfd);
	close(mfd);
	return 0;
}
