#include <stdint.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/uinput.h>

void // from stackoverflow 
emit(int fd, int type, int code, int val)
{
   struct
	input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}

int
main(int argc, char *argv[])
{
	// check user uinput
	if (argc > 1 && strcmp(argv[1], "help") == 0)
	{
        printf("Usage: tiomd [port] (optional)\n");
        return 1;
	}
	// set all required variables to create connection
	int 
	bufferSize = 14;
	int
	port = (argv[1] != NULL) ? atoi(argv[1]) : 6968;
 	int // create socket with UDP communication
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	int // all mouse input variables
	MX=0,
	MY=MX,
	MLEFT=MY,
	MMIDDLE=MLEFT,
	MRIGHT=MRIGHT;

	char // create buffer
	buffer[bufferSize];

	struct sockaddr_in 
	server_addr, client_addr;
	struct uinput_setup
	usetup;

	socklen_t
	addr_size;

	if (sockfd == -1)
	{
		perror("socket() error");
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
 	server_addr.sin_port = htons(port);
 	server_addr.sin_addr.s_addr = INADDR_ANY;

	int // bind socket
	n = bind(
		sockfd,
		(struct sockaddr*)&server_addr,
		sizeof(server_addr));
	
	if(n<0)
	{
		perror("bind() error");
		exit(EXIT_FAILURE);
	}addr_size = sizeof(client_addr);
	
   int // mouse input file descriptor
	mfd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	/* enabled input events */
	ioctl(mfd, UI_SET_EVBIT, EV_KEY);
   ioctl(mfd, UI_SET_KEYBIT, BTN_LEFT);
   ioctl(mfd, UI_SET_EVBIT, EV_REL);
   ioctl(mfd, UI_SET_RELBIT, REL_X);
   ioctl(mfd, UI_SET_RELBIT, REL_Y); 

   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_USB;
   usetup.id.vendor = 0x2007;
   usetup.id.product = 0x2011;
   strcpy(usetup.name, "wayt sync device");
   ioctl(mfd, UI_DEV_SETUP, &usetup);
   ioctl(mfd, UI_DEV_CREATE);
	
	while (1)
	{
		// listen for clients
		recvfrom(sockfd,buffer,bufferSize,0,(struct sockaddr*)&client_addr, &addr_size);
		
		//printf("Data: %s\n",buffer); // ENABLE FOR LOG
		
		// pasting buffer content into variables
		sscanf(buffer, "%d,%d,%d,%d,%d", &MX,&MY,&MLEFT,&MMIDDLE,&MRIGHT);

		// emit all input events
      emit(mfd, EV_REL, REL_X, MX);
      emit(mfd, EV_REL, REL_Y, -MY);
      emit(mfd, EV_SYN, SYN_REPORT, 0);

		// fill buffer with zeros 
		bzero(buffer,bufferSize);
	}

	// close all open file descriptors
	close(sockfd);
	close(mfd);

	return 0;
}
