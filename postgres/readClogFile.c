/*
* usage: ./readClog clogFileName gxid
*
/

#include <stdio.h>
#include <errno.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#define CLOG_BITS_PER_XACT      2
#define CLOG_XACTS_PER_BYTE     4
#define CLOG_XACT_BITMASK       ((1 << CLOG_BITS_PER_XACT) - 1)

#define TRANSACTION_STATUS_IN_PROGRESS          0x00
#define TRANSACTION_STATUS_COMMITTED            0x01
#define TRANSACTION_STATUS_ABORTED              0x02
#define TRANSACTION_STATUS_SUB_COMMITTED        0x03

static void func_print(int gxid, int status);
static void func_deal_clog(char cr, int gxid);
static void usage(char *argv[]);

static void func_deal_clog(char cr, int gxid)
{
	assert(gxid >= 0);
	int i = 0;
	int n;
	int nBitPostion = gxid%4;
	int status = (cr>>nBitPostion*2) & CLOG_XACT_BITMASK;
	func_print(gxid, status);
}

static void func_print(int gxid, int status)
{
	const char *statusstr = NULL;

	switch (status)
	{
		case TRANSACTION_STATUS_IN_PROGRESS:
				statusstr = "IN PROGRESS";
				break;
		case TRANSACTION_STATUS_COMMITTED:
				statusstr = "COMMITTED";
				break;
		case TRANSACTION_STATUS_ABORTED:
				statusstr = "ABORTED";
				break;
		case TRANSACTION_STATUS_SUB_COMMITTED:
				statusstr = "SUB COMMITTED";
				break;
		default:
				statusstr = "UNKNOWN";
				break;
	}
	fprintf(stdout, "TransactionId: %u, status: %s\n", gxid, statusstr); 
}

static void usage(char *argv[])
{
	fprintf(stdout, "usage:");
	fprintf(stdout, "\t%s clogFileName gxid\n", argv[0]);
}


int main(int argc, char **argv)
{
	int fd;
	int res = 0;
	int gxid = 0;
	char *clogFileName;
	struct stat statBuf;

	if (argc < 3)
	{
		usage(argv);
		return 1;
	}
	
	clogFileName = argv[1];
	/* check this file exist */
	res = stat(clogFileName, &statBuf);
	if (res !=0)
	{
		perror(clogFileName);
		return 1;
	}
	if (statBuf.st_mode & S_IFMT != S_IFREG)
	{
		fprintf(stderr, "the file \"%s\" is not regule file\n", clogFileName);
		return 1;
	}

	/* check the input gxid */
	gxid = atoi(argv[2]);
	if (gxid == 0)
	{
		fprintf(stderr, "gxid should be larger then 0\n");
		return 1;
	}

	fd = open(clogFileName, O_RDONLY);
	if (fd<0)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		return 1;
	}

	char cr;
	int num;
	int nBytePostion = gxid/4;
	if (lseek(fd, nBytePostion, SEEK_SET)==-1)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		return 1;
	}

	num = read(fd, &cr, 1);
	func_deal_clog(cr, gxid);

	return 0;
}
