/* This is a sample demonstration application that showcases usage of remoteproc
and rpmsg APIs on the remote core. This application is meant to run on the remote CPU 
running baremetal code. This applicationr receives two matrices from the master, 
multiplies them and returns the result to the master core. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include "matrix_multiply.h"
#include "platform_info.h"

#define	MAX_SIZE      6
#define NUM_MATRIX    2
#define APP_EPT_ADDR  0

#define raw_printf(format, ...) printf(format, ##__VA_ARGS__)
#define LPRINTF(format, ...) raw_printf("CLIENT> " format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

typedef struct _matrix {
	unsigned int size;
	unsigned int elements[MAX_SIZE][MAX_SIZE];
} matrix;

/* Globals */
static struct rpmsg_endpoint lept;
static struct _matrix i_matrix[2];
static struct _matrix e_matrix;
static unsigned int result_returned = 0;
static int err_cnt = 0;
static int ept_deleted = 0;

/**
 * _gettimeofday() is called from time() which is used by srand() to generate
 * random number. It is defined here in case this function is not defined in
 * library.
 */
int __attribute__((weak)) _gettimeofday(struct timeval *tv, void *tz)
{
	(void)tv;
	(void)tz;
	return 0;
}

static void matrix_print(struct _matrix *m)
{
	unsigned int i, j;

	/* Generate two random matrices */
	LPRINTF("Printing matrix... \n");

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < m->size; ++j)
			raw_printf(" %u ", m->elements[i][j]);
		raw_printf("\n");
	}
}

static void generate_matrices(int num_matrices,
			      unsigned int matrix_size, void *p_data)
{
	unsigned int i, j, k;
	struct _matrix *p_matrix = p_data;
	unsigned long value;


	for (i = 0; i < (unsigned int)num_matrices; i++) {
		/* Initialize workload */
		p_matrix[i].size = matrix_size;

		LPRINTF("Input matrix %d \n", i);
		for (j = 0; j < matrix_size; j++) {
			raw_printf("\n");
			for (k = 0; k < matrix_size; k++) {

				value = (rand() & 0x7F);
				value = value % 10;
				p_matrix[i].elements[j][k] = value;
				raw_printf(" %u ", p_matrix[i].elements[j][k]);
			}
		}
		raw_printf("\n");
	}

}

static void matrix_multiply(const matrix * m, const matrix * n, matrix * r)
{
	unsigned int i, j, k;

	memset(r, 0x0, sizeof(matrix));
	r->size = m->size;

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < n->size; ++j) {
			for (k = 0; k < r->size; ++k) {
				r->elements[i][j] +=
				    m->elements[i][k] * n->elements[k][j];
			}
		}
	}
}

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data,
			     size_t len, uint32_t src, void *priv)
{
	struct _matrix *r_matrix = (struct _matrix *)data;
	int i, j;

	(void)ept;
	(void)priv;
	(void)src;
	if (len != sizeof(struct _matrix)) {
		LPERROR("Received matrix is of invalid len: %d:%lu\n",
			(int)sizeof(struct _matrix), (unsigned long)len);
		err_cnt++;
		return RPMSG_SUCCESS;
	}
	for (i = 0; i < MAX_SIZE; i++) {
		for (j = 0; j < MAX_SIZE; j++) {
			if (r_matrix->elements[i][j] !=
				e_matrix.elements[i][j]) {
				err_cnt++;
				break;
			}
		}
	}
	if (err_cnt) {
		LPERROR("Result mismatched...\n");
		LPERROR("Expected matrix:\n");
		matrix_print(&e_matrix);
		LPERROR("Actual matrix:\n");
		matrix_print(r_matrix);
	} else {
		result_returned = 1;
	}
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	rpmsg_destroy_ept(&lept);
	LPRINTF("echo test: service is destroyed\n");
	ept_deleted = 1;
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
				       const char *name, uint32_t dest)
{
	if (strcmp(name, RPMSG_SERVICE_NAME))
		LPERROR("Unexpected name service %s.\n", name);
	else
		(void)rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
				       APP_EPT_ADDR, dest,
				       rpmsg_endpoint_cb,
				       rpmsg_service_unbind);

}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app (struct rpmsg_device *rdev, void *priv)
{
	int c;
	int ret;

	LPRINTF("Compute thread unblocked ..\n");
	LPRINTF("It will generate two random matrices.\n");
	LPRINTF("Send to the remote and get the computation result back.\n");
	LPRINTF("It will then check if the result is expected.\n");

	/* Create RPMsg endpoint */
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME, APP_EPT_ADDR,
			       RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);
	if (ret) {
		LPERROR("Failed to create RPMsg endpoint.\n");
		return ret;
	}

	while (!is_rpmsg_ept_ready(&lept))
		platform_poll(priv);

	LPRINTF("RPMSG endpoint is binded with remote.\n");
	err_cnt = 0;
	srand(time(NULL));
	for (c = 0; c < 200; c++) {
		generate_matrices(2, MAX_SIZE, i_matrix);
		matrix_multiply(&i_matrix[0], &i_matrix[1],
				&e_matrix);
		result_returned = 0;
		ret = rpmsg_send(&lept, i_matrix, sizeof(i_matrix));

		if (ret < 0) {
			LPRINTF("Error sending data...\n");
			break;
		}
		LPRINTF("Matrix multiply: sent : %lu\n",
			(unsigned long)sizeof(i_matrix));
		do {
			platform_poll(priv);
		} while (!result_returned && !err_cnt && !ept_deleted);

		if (err_cnt)
			break;
	}

	LPRINTF("**********************************\n");
	LPRINTF(" Test Results: Error count = %d \n", err_cnt);
	LPRINTF("**********************************\n");

	/* Detroy RPMsg endpoint */
	rpmsg_destroy_ept(&lept);
	LPRINTF("Quitting application .. Matrix multiplication end\n");

	return 0;
}

int main(int argc, char *argv[])
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret;

	/* Initialize platform */
	ret = platform_init(argc, argv, &platform);
	if (ret) {
		LPERROR("Failed to initialize platform.\n");
		ret = -1;
	} else {
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						  VIRTIO_DEV_MASTER,
						  NULL,
						  rpmsg_name_service_bind_cb);
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

