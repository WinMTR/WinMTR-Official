//*****************************************************************************
// FILE:            WinMTRGlobal.h
//
//
// DESCRIPTION:
//   
//
// NOTES:
//    
//
//*****************************************************************************

#ifndef GLOBAL_H_
#define GLOBAL_H_

#ifndef  _WIN64
#define  _USE_32BIT_TIME_T
#endif

#define VC_EXTRALEAN

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#include <afxdtctl.h>

#ifndef _AFX_NO_AFXCMN_SUPPORT
  #include <afxcmn.h>
#endif 
#include <afxsock.h>

#include <process.h>
#include <stdio.h>
#include <io.h> 
#include <time.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <math.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/stat.h>

#include "resource.h"

#define WINMTR_VERSION	"0.9"
#define WINMTR_LICENSE	"GPL - GNU Public License"
#define WINMTR_COPYRIGHT "WinMTR 0.9 (c) 2010-2011 Appnor MSP - Fully Managed Hosting & Cloud Provider www.appnor.com"
#define WINMTR_HOMEPAGE	"http://WinMTR.sourceforge.net"

#define DEFAULT_PING_SIZE	64
#define DEFAULT_INTERVAL	1.0
#define DEFAULT_MAX_LRU		128
#define DEFAULT_DNS			TRUE

#define SAVED_PINGS 100
#define MaxHost 256
//#define MaxSequence 65536
#define MaxSequence 32767
//#define MaxSequence 5

#define MAXPACKET 4096
#define MINPACKET 64

#define MaxTransit 4

 
#define ICMP_ECHO		8
#define ICMP_ECHOREPLY		0

#define ICMP_TSTAMP		13
#define ICMP_TSTAMPREPLY	14

#define ICMP_TIME_EXCEEDED	11

#define ICMP_HOST_UNREACHABLE 3

#define MAX_UNKNOWN_HOSTS 10

#define IP_HEADER_LENGTH   20


#define MTR_NR_COLS 16

enum MTR_COLUMN_INDEX {
		MTR_COL_HOSTNAME = 0,
		MTR_COL_NR,
		MTR_COL_LOSS,
		MTR_COL_DROP,
		MTR_COL_RECV,
		MTR_COL_SENT,
		MTR_COL_LAST,
		MTR_COL_BEST,
		MTR_COL_AVG,
		MTR_COL_WORST,
		MTR_COL_STDEV,
		MTR_COL_GMEAN,
		MTR_COL_JITTER,
		MTR_COL_JAVG,
		MTR_COL_JMAX,
		MTR_COL_JINT
};

struct MTR_COLUMN_DEFINITION {
	const char* listHeader;
	const char* reportHeader;
	int width;
};

const MTR_COLUMN_DEFINITION MTR_COLUMNS[ MTR_NR_COLS ] = {
		{ "Hostname", "Host", 300 },
		{ "Nr", "Nr", 30 },
		{ "Loss %", "Loss %", 55 },
		{ "Drop", "Drop", 50 },
		{ "Recv", "Recv", 50 },
		{ "Sent", "Sent", 50 },
		{ "Last", "Last", 55 },
		{ "Best", "Best", 55 },
		{ "Avg", "Avg", 55 },
		{ "Worst", "Worst", 60 },
		{ "StDev", "StDev", 60 },
		{ "Gmean", "Gmean", 55 },
		{ "Jttr", "Jttr", 55 },
		{ "Javg", "Javg", 55 },
		{ "Jmax", "Jmax", 55 },
		{ "Jint", "Jint", 55 }
};

int gettimeofday(struct timeval* tv, struct timezone *tz);

#endif // ifndef GLOBAL_H_
