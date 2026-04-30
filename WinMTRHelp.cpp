// WinMTRHelp.cpp : implementation file
//

#include "WinMTRGlobal.h"
#include "WinMTRHelp.h"
#include "afxdialogex.h"


// WinMTRHelp dialog

IMPLEMENT_DYNAMIC(WinMTRHelp, CDialog)

WinMTRHelp::WinMTRHelp(CWnd* pParent /*=NULL*/)
	: CDialog(WinMTRHelp::IDD, pParent)
{

}

WinMTRHelp::~WinMTRHelp()
{
}

void WinMTRHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_HELPTEXT, m_helpText);
}


BEGIN_MESSAGE_MAP(WinMTRHelp, CDialog)
	ON_BN_CLICKED(IDOK, &WinMTRHelp::OnBnClickedOk)
END_MESSAGE_MAP()


// WinMTRHelp message handlers

static void AppendMetricHelp(CString& helpText, int column, const char* orderKey, const char* fullName, const char* description, const char* interpretation)
{
	helpText += orderKey;
	helpText += " (";
	helpText += MTR_COLUMNS[column].reportHeader;
	helpText += ") - ";
	helpText += fullName;
	helpText += "\r\n";
	helpText += description;
	helpText += "\r\n";
	helpText += interpretation;
	helpText += "\r\n\r\n";
}

void WinMTRHelp::OnBnClickedOk()
{
	CDialog::OnOK();
}

BOOL WinMTRHelp::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString helpText = "WinMTR - Guide to the displayed metrics\r\n\r\n";

	AppendMetricHelp(helpText, MTR_COL_LOSS, "L", "Loss Percentage",
		"Shows the percentage of probes that did not receive a reply from this hop.",
		"Interpretation: 0.0% is ideal. Persistent loss on a middle hop is only meaningful if it continues on later hops.");
	AppendMetricHelp(helpText, MTR_COL_DROP, "D", "Dropped Packets",
		"Absolute number of probes sent to the hop that did not produce a reply.",
		"Interpretation: useful to confirm whether Loss% comes from a small sample or a sustained issue.");
	AppendMetricHelp(helpText, MTR_COL_RECV, "R", "Received Packets",
		"Number of replies successfully received from the hop.",
		"Interpretation: low values with many sent probes may indicate packet loss or rate-limited ICMP responses.");
	AppendMetricHelp(helpText, MTR_COL_SENT, "S", "Sent Packets",
		"Total number of probes transmitted toward the hop.",
		"Interpretation: compare with Recv and Drop to judge stability over time.");
	AppendMetricHelp(helpText, MTR_COL_LAST, "N", "Latest RTT",
		"Most recent round-trip time in milliseconds.",
		"Interpretation: good for spotting sudden spikes, but do not use it alone to judge overall quality.");
	AppendMetricHelp(helpText, MTR_COL_BEST, "B", "Best RTT",
		"Lowest round-trip time observed for the hop.",
		"Interpretation: approximates the path's best-case latency under the current test conditions.");
	AppendMetricHelp(helpText, MTR_COL_AVG, "A", "Average RTT",
		"Arithmetic mean of all successful round-trip times for the hop.",
		"Interpretation: use as the main latency baseline. Compare against Best and Worst to understand stability.");
	AppendMetricHelp(helpText, MTR_COL_WORST, "W", "Worst RTT",
		"Highest round-trip time observed for the hop.",
		"Interpretation: isolated peaks are normal; repeated high values can indicate congestion or queuing.");
	AppendMetricHelp(helpText, MTR_COL_STDEV, "V", "Standard Deviation",
		"Measures how spread out the RTT samples are around the average.",
		"Interpretation: low StDev means stable latency. High StDev means inconsistent latency, often seen with congestion.");
	AppendMetricHelp(helpText, MTR_COL_GMEAN, "G", "Geometric Mean",
		"Geometric mean of the RTT samples.",
		"Interpretation: less sensitive to extreme spikes than Avg, useful when occasional outliers would distort the arithmetic mean.");
	AppendMetricHelp(helpText, MTR_COL_JITTER, "J", "Current Jitter",
		"Absolute difference between the latest RTT and the previous RTT sample.",
		"Interpretation: shows the most recent latency variation. High current jitter often means unstable delivery.");
	AppendMetricHelp(helpText, MTR_COL_JAVG, "M", "Jitter Average",
		"Average jitter observed over the session.",
		"Interpretation: a sustained indicator of latency instability. Lower is better, especially for voice, gaming and streaming.");
	AppendMetricHelp(helpText, MTR_COL_JMAX, "X", "Maximum Jitter",
		"Highest jitter value observed during the test.",
		"Interpretation: reveals the worst latency jump seen so far. Useful for identifying bursty problems.");
	AppendMetricHelp(helpText, MTR_COL_JINT, "I", "Interarrival Jitter",
		"Smoothed jitter estimate inspired by RTP interarrival jitter calculations.",
		"Interpretation: useful as a less noisy long-term jitter signal. Rising Jint suggests persistent timing variation, not just isolated spikes.");

	helpText += "General reading tips\r\n";
	helpText += "1. Always compare a hop with the hops after it. If a middle hop looks bad but later hops are healthy, that hop may only be rate-limiting ICMP.\r\n";
	helpText += "2. End-to-end problems matter most on the destination row.\r\n";
	helpText += "3. Loss plus rising jitter and higher Avg/Worst together usually indicate a real path quality issue.\r\n";

	m_helpText.SetWindowText(helpText);
	return TRUE;
}
