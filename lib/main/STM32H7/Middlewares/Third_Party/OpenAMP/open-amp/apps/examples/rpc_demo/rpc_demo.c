/* This is a sample demonstration application that showcases usage of proxy from the remote core. 
 This application is meant to run on the remote CPU running baremetal.
 This applicationr can print to to master console and perform file I/O using proxy mechanism. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <openamp/rpmsg_retarget.h>
#include "rsc_table.h"
#include "platform_info.h"
#include "rpmsg-rpc-demo.h"

#define REDEF_O_CREAT   0000100
#define REDEF_O_EXCL    0000200
#define REDEF_O_RDONLY  0000000
#define REDEF_O_WRONLY  0000001
#define REDEF_O_RDWR    0000002
#define REDEF_O_APPEND  0002000
#define REDEF_O_ACCMODE 0000003

#define LPRINTF(format, ...) xil_printf(format, ##__VA_ARGS__)
//#define LPRINTF(format, ...)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

static void rpmsg_rpc_shutdown(struct rpmsg_rpc_data *rpc)
{
	(void)rpc;
	LPRINTF("RPMSG RPC is shutting down.\n");
}

/*-----------------------------------------------------------------------------*
 *  Application specific
 *-----------------------------------------------------------------------------*/
int app(struct rpmsg_device *rdev, void *priv)
{
	struct rpmsg_rpc_data rpc;
	struct rpmsg_rpc_syscall rpccall;
	int fd, bytes_written, bytes_read;
	char fname[] = "remote.file";
	char wbuff[50];
	char rbuff[1024];
	char ubuff[50];
	float fdata;
	int idata;
	int ret;

	/* redirect I/Os */
	LPRINTF("Initializating I/Os redirection...\n");
	ret = rpmsg_rpc_init(&rpc, rdev, RPMSG_SERVICE_NAME,
			     RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			     priv, platform_poll, rpmsg_rpc_shutdown);
	rpmsg_set_default_rpc(&rpc);
	if (ret) {
		LPRINTF("Failed to intialize rpmsg rpc\n");
		return -1;
	}

	printf("\r\nRemote>Baremetal Remote Procedure Call (RPC) Demonstration\r\n");
	printf("\r\nRemote>***************************************************\r\n");

	printf("\r\nRemote>Rpmsg based retargetting to proxy initialized..\r\n");

	/* Remote performing file IO on Master */
	printf("\r\nRemote>FileIO demo ..\r\n");

	printf("\r\nRemote>Creating a file on master and writing to it..\r\n");
	fd = open(fname, REDEF_O_CREAT | REDEF_O_WRONLY | REDEF_O_APPEND,
		  S_IRUSR | S_IWUSR);
	printf("\r\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);

	sprintf(wbuff, "This is a test string being written to file..");
	bytes_written = write(fd, wbuff, strlen(wbuff));
	printf("\r\nRemote>Wrote to fd = %d, size = %d, content = %s\r\n", fd,
	       bytes_written, wbuff);
	close(fd);
	printf("\r\nRemote>Closed fd = %d\r\n", fd);

	/* Remote performing file IO on Master */
	printf("\r\nRemote>Reading a file on master and displaying its contents..\r\n");
	fd = open(fname, REDEF_O_RDONLY, S_IRUSR | S_IWUSR);
	printf("\r\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);
	bytes_read = read(fd, rbuff, 1024);
	*(char *)(&rbuff[0] + bytes_read + 1) = 0;
	printf("\r\nRemote>Read from fd = %d, size = %d, printing contents below .. %s\r\n",
		fd, bytes_read, rbuff);
	close(fd);
	printf("\r\nRemote>Closed fd = %d\r\n", fd);

	while (1) {
		/* Remote performing STDIO on Master */
		printf("\r\nRemote>Remote firmware using scanf and printf ..\r\n");
		printf("\r\nRemote>Scanning user input from master..\r\n");
		printf("\r\nRemote>Enter name\r\n");
		ret = scanf("%s", ubuff);
		if (ret) {
			printf("\r\nRemote>Enter age\r\n");
			ret = scanf("%d", &idata);
			if (ret) {
				printf("\r\nRemote>Enter value for pi\r\n");
				ret = scanf("%f", &fdata);
				if (ret) {
					printf("\r\nRemote>User name = '%s'\r\n", ubuff);
					printf("\r\nRemote>User age = '%d'\r\n", idata);
					printf("\r\nRemote>User entered value of pi = '%f'\r\n", fdata);
				}
			}
		}
		if (!ret) {
			scanf("%s", ubuff);
			printf("Remote> Invalid value. Starting again....");
		} else {
			printf("\r\nRemote>Repeat demo ? (enter yes or no) \r\n");
			scanf("%s", ubuff);
			if ((strcmp(ubuff, "no")) && (strcmp(ubuff, "yes"))) {
				printf("\r\nRemote>Invalid option. Starting again....\r\n");
			} else if ((!strcmp(ubuff, "no"))) {
				printf("\r\nRemote>RPC retargetting quitting ...\r\n");
				break;
			}
		}
	}

	printf("\r\nRemote> Firmware's rpmsg-rpc-channel going down! \r\n");
	rpccall.id = TERM_SYSCALL_ID;
	(void)rpmsg_rpc_send(&rpc, &rpccall, sizeof(rpccall), NULL, 0);

	LPRINTF("Release remoteproc procedure call\n");
	rpmsg_rpc_release(&rpc);
	return 0;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret;

	LPRINTF("Starting application...\n");

	/* Initialize platform */
	ret = platform_init(argc, argv, &platform);
	if (ret) {
		LPERROR("Failed to initialize platform.\n");
		ret = -1;
	} else {
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						   VIRTIO_DEV_SLAVE,
						   NULL, NULL);
		if (!rpdev) {
			LPERROR("Failed to create rpmsg virtio device.\n");
			ret = -1;
		} else {
			app(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev);
			ret = 0;
		}
	}

	LPRINTF("Stopping application...\n");
	platform_cleanup(platform);

	return ret;
}
