//*****************************************************************************
// FILE:            WinMTRNet.h
//
//
// DESCRIPTION:
//   
//
// NOTES:
//    
//
//*****************************************************************************

#ifndef WINMTRNET_H_
#define WINMTRNET_H_


class WinMTRDialog;

typedef ip_option_information IPINFO, *PIPINFO, FAR *LPIPINFO;

#ifdef _WIN64
typedef icmp_echo_reply32 ICMPECHO, *PICMPECHO, FAR *LPICMPECHO;
#else
typedef icmp_echo_reply ICMPECHO, *PICMPECHO, FAR *LPICMPECHO;
#endif

#define ECHO_REPLY_TIMEOUT 5000

struct s_nethost {
  __int32 addr;		// IP as a decimal, big endian
  int xmit;			// number of PING packets sent
  int returned;		// number of ICMP echo replies received
  int transit;         // number of probes still in flight
  double total;        // total time
  double last;			// last time
  double best;			// best time
  double worst;		// worst time
  double avg;          // running average
  double gmean;        // geometric mean
  double ssd;          // sum of squares of differences
  double jitter;       // current jitter
  double javg;         // average jitter
  double jworst;       // maximum jitter
  double jinta;        // interarrival jitter
  char name[255];
};

//*****************************************************************************
// CLASS:  WinMTRNet
//
//
//*****************************************************************************

class WinMTRNet {
	typedef HANDLE (WINAPI *LPFNICMPCREATEFILE)(VOID);
	typedef BOOL  (WINAPI *LPFNICMPCLOSEHANDLE)(HANDLE);
	typedef DWORD (WINAPI *LPFNICMPSENDECHO)(HANDLE, u_long, LPVOID, WORD, LPVOID, LPVOID, DWORD, DWORD);

public:

	WinMTRNet(WinMTRDialog *wp);
	~WinMTRNet();
	void	DoTrace(int address);
	void	ResetHops();
	void	StopTrace();

	int		GetAddr(int at);
	int		GetIp(int at, char *n, int size);
	int		GetName(int at, char *n);
	int		GetDisplayName(int at, char *n, int size);
	double	GetBest(int at);
	double	GetWorst(int at);
	double	GetAvg(int at);
	double	GetLoss(int at);
	int		GetDrop(int at);
	double	GetLast(int at);
	double	GetStDev(int at);
	double	GetGmean(int at);
	double	GetJitter(int at);
	double	GetJavg(int at);
	double	GetJworst(int at);
	double	GetJinta(int at);
	int		GetReturned(int at);
	int		GetXmit(int at);
	int		GetMax();

	void	SetAddr(int at, __int32 addr);
	void	SetName(int at, char *n);
	void	BeginProbe(int at);
	void	RecordReply(int at, double current);
	void	EndProbe(int at);

	WinMTRDialog		*wmtrdlg;
	__int32				last_remote_addr;
	bool				tracing;
	bool				initialized;
    HANDLE				hICMP;
	LPFNICMPCREATEFILE	lpfnIcmpCreateFile;
	LPFNICMPCLOSEHANDLE lpfnIcmpCloseHandle;
	LPFNICMPSENDECHO	lpfnIcmpSendEcho;
private:
	HINSTANCE			hICMP_DLL;

    struct s_nethost	host[MaxHost];
	HANDLE				ghMutex; 
};

#endif	// ifndef WINMTRNET_H_
