//*****************************************************************************
// FILE:            WinMTRDialog.cpp
//
//
//*****************************************************************************

#include "WinMTRGlobal.h"
#include "WinMTRDialog.h"
#include "WinMTROptions.h"
#include "WinMTRProperties.h"
#include "WinMTRNet.h"
#include "WinMTRHelp.h"
#include <iostream>
#include <sstream>
#include <vector>
#include "afxlinkctrl.h"

#define TRACE_MSG(msg)										\
	{														\
	std::ostringstream dbg_msg(std::ostringstream::out);	\
	dbg_msg << msg << std::endl;							\
	OutputDebugString(dbg_msg.str().c_str());				\
	}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static	 char THIS_FILE[] = __FILE__;
#endif

void PingThread(void *p);

static CString GetHopDisplayName(WinMTRNet* wmtrnet, int index)
{
	char buf[512];
	wmtrnet->GetDisplayName(index, buf, sizeof(buf));
	if(strcmp(buf, "") == 0) {
		return CString("No response from host");
	}

	return CString(buf);
}

static CString FormatCount(int value)
{
	CString text;
	text.Format("%d", value);
	return text;
}

static CString FormatMetric(double value)
{
	CString text;
	text.Format("%.1f", value);
	return text;
}

static CString FormatLoss(double value)
{
	CString text;
	text.Format("%.1f", value);
	return text;
}

static CString EscapeHtml(const CString& value)
{
	CString escaped(value);
	escaped.Replace("&", "&amp;");
	escaped.Replace("<", "&lt;");
	escaped.Replace(">", "&gt;");
	escaped.Replace("\"", "&quot;");
	escaped.Replace("'", "&#39;");
	return escaped;
}

static CString EscapeCsv(const CString& value)
{
	CString escaped(value);
	escaped.Replace("\"", "\"\"");
	return escaped;
}

static CString GetColumnText(WinMTRNet* wmtrnet, int hopIndex, int column)
{
	switch(column) {
	case MTR_COL_HOSTNAME:
		return GetHopDisplayName(wmtrnet, hopIndex);
	case MTR_COL_NR:
		return FormatCount(hopIndex + 1);
	case MTR_COL_LOSS:
		return FormatLoss(wmtrnet->GetLoss(hopIndex));
	case MTR_COL_DROP:
		return FormatCount(wmtrnet->GetDrop(hopIndex));
	case MTR_COL_RECV:
		return FormatCount(wmtrnet->GetReturned(hopIndex));
	case MTR_COL_SENT:
		return FormatCount(wmtrnet->GetXmit(hopIndex));
	case MTR_COL_LAST:
		return FormatMetric(wmtrnet->GetLast(hopIndex));
	case MTR_COL_BEST:
		return FormatMetric(wmtrnet->GetBest(hopIndex));
	case MTR_COL_AVG:
		return FormatMetric(wmtrnet->GetAvg(hopIndex));
	case MTR_COL_WORST:
		return FormatMetric(wmtrnet->GetWorst(hopIndex));
	case MTR_COL_STDEV:
		return FormatMetric(wmtrnet->GetStDev(hopIndex));
	case MTR_COL_GMEAN:
		return FormatMetric(wmtrnet->GetGmean(hopIndex));
	case MTR_COL_JITTER:
		return FormatMetric(wmtrnet->GetJitter(hopIndex));
	case MTR_COL_JAVG:
		return FormatMetric(wmtrnet->GetJavg(hopIndex));
	case MTR_COL_JMAX:
		return FormatMetric(wmtrnet->GetJworst(hopIndex));
	case MTR_COL_JINT:
		return FormatMetric(wmtrnet->GetJinta(hopIndex));
	default:
		return CString("");
	}
}

static CString BuildTextHeader()
{
	CString header;
	for(int column = 0; column < MTR_NR_COLS; ++column) {
		if(column > 0) {
			header += ", ";
		}
		header += MTR_COLUMNS[column].reportHeader;
	}
	header += "\r\n";
	return header;
}

static CString BuildHtmlHeader()
{
	CString header;
	header += "<tr>";
	for(int column = 0; column < MTR_NR_COLS; ++column) {
		header += "<td>";
		header += EscapeHtml(MTR_COLUMNS[column].reportHeader);
		header += "</td>";
	}
	header += "</tr>\r\n";
	return header;
}

static CString BuildCsvHeader()
{
	CString header;
	for(int column = 0; column < MTR_NR_COLS; ++column) {
		if(column > 0) {
			header += ",";
		}
		header += "\"";
		header += EscapeCsv(MTR_COLUMNS[column].reportHeader);
		header += "\"";
	}
	header += "\r\n";
	return header;
}

static CString BuildTextReport(WinMTRNet* wmtrnet)
{
	CString report;
	report += "WinMTR statistics\r\n";
	report += BuildTextHeader();

	int nh = wmtrnet->GetMax();
	for(int i = 0; i < nh; i++) {
		CString line;
		for(int column = 0; column < MTR_NR_COLS; ++column) {
			if(column > 0) {
				line += ", ";
			}
			line += GetColumnText(wmtrnet, i, column);
		}
		line += "\r\n";
		report += line;
	}

	return report;
}

static CString BuildHtmlReport(WinMTRNet* wmtrnet)
{
	CString report;
	report += "<html><head><title>WinMTR Statistics</title></head><body bgcolor=\"white\">\r\n";
	report += "<center><h2>WinMTR statistics</h2></center>\r\n";
	report += "<p align=\"center\"><table border=\"1\" align=\"center\">\r\n";
	report += BuildHtmlHeader();

	int nh = wmtrnet->GetMax();
	for(int i = 0; i < nh; i++) {
		CString line("<tr>");
		for(int column = 0; column < MTR_NR_COLS; ++column) {
			line += "<td>";
			line += EscapeHtml(GetColumnText(wmtrnet, i, column));
			line += "</td>";
		}
		line += "</tr>\r\n";
		report += line;
	}

	report += "</table></body></html>\r\n";
	return report;
}

static CString BuildCsvReport(WinMTRNet* wmtrnet)
{
	CString report;
	report += BuildCsvHeader();

	int nh = wmtrnet->GetMax();
	for(int i = 0; i < nh; i++) {
		CString line;
		for(int column = 0; column < MTR_NR_COLS; ++column) {
			if(column > 0) {
				line += ",";
			}

			CString value = GetColumnText(wmtrnet, i, column);
			if(column == MTR_COL_HOSTNAME) {
				line += "\"";
				line += EscapeCsv(value);
				line += "\"";
			} else {
				line += value;
			}
		}
		line += "\r\n";
		report += line;
	}

	return report;
}

static BOOL CopyTextToClipboard(CWnd* owner, const CString& source)
{
	if(!::OpenClipboard(owner != NULL ? owner->GetSafeHwnd() : NULL)) {
		return FALSE;
	}

	BOOL success = FALSE;
	HGLOBAL clipbuffer = NULL;
	::EmptyClipboard();

	const size_t bufferSize = source.GetLength() + 1;
	clipbuffer = ::GlobalAlloc(GMEM_MOVEABLE, bufferSize);
	if(clipbuffer != NULL) {
		char* buffer = (char*)::GlobalLock(clipbuffer);
		if(buffer != NULL) {
			memcpy(buffer, (LPCSTR)source, bufferSize);
			::GlobalUnlock(clipbuffer);

			if(::SetClipboardData(CF_TEXT, clipbuffer) != NULL) {
				success = TRUE;
				clipbuffer = NULL;
			}
		}
	}

	if(clipbuffer != NULL) {
		::GlobalFree(clipbuffer);
	}
	::CloseClipboard();
	return success;
}

static BOOL WriteReportFile(const CString& path, const CString& report)
{
	CStdioFile file;
	if(!file.Open(path, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) {
		return FALSE;
	}

	file.WriteString(report);
	file.Close();
	return TRUE;
}

static CString JoinColumnOrder(const std::vector<int>& order)
{
	CString text;
	for(size_t i = 0; i < order.size(); ++i) {
		CString part;
		part.Format("%d", order[i]);
		text += part;
		if(i + 1 < order.size()) {
			text += ",";
		}
	}

	return text;
}

static bool ParseColumnOrder(const CString& text, std::vector<int>& order)
{
	order.clear();
	int start = 0;
	while(start < text.GetLength()) {
		int comma = text.Find(',', start);
		CString token = (comma == -1) ? text.Mid(start) : text.Mid(start, comma - start);
		token.Trim();
		if(token.IsEmpty()) {
			return false;
		}

		order.push_back(_ttoi(token));
		if(comma == -1) {
			break;
		}
		start = comma + 1;
	}

	return !order.empty();
}

//*****************************************************************************
// BEGIN_MESSAGE_MAP
//
// 
//*****************************************************************************
BEGIN_MESSAGE_MAP(WinMTRDialog, CDialog)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_EXITSIZEMOVE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_RESTART, OnRestart)
	ON_BN_CLICKED(ID_OPTIONS, OnOptions)
	ON_BN_CLICKED(ID_HELP_METRICS, OnHelpMetrics)
	ON_BN_CLICKED(ID_CTTC, OnCTTC)
	ON_BN_CLICKED(ID_CHTC, OnCHTC)
	ON_BN_CLICKED(ID_EXPT, OnEXPT)
	ON_BN_CLICKED(ID_EXPC, OnEXPC)
	ON_BN_CLICKED(ID_EXPH, OnEXPH)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MTR, OnDblclkList)
	ON_CBN_SELCHANGE(IDC_COMBO_HOST, &WinMTRDialog::OnCbnSelchangeComboHost)
	ON_CBN_SELENDOK(IDC_COMBO_HOST, &WinMTRDialog::OnCbnSelendokComboHost)
	ON_CBN_CLOSEUP(IDC_COMBO_HOST, &WinMTRDialog::OnCbnCloseupComboHost)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &WinMTRDialog::OnBnClickedCancel)
END_MESSAGE_MAP()


//*****************************************************************************
// WinMTRDialog::WinMTRDialog
//
// 
//*****************************************************************************
WinMTRDialog::WinMTRDialog(CWnd* pParent) 
			: CDialog(WinMTRDialog::IDD, pParent),
			state(IDLE),
			transition(IDLE_TO_IDLE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_autostart = 0;
	useDNS = DEFAULT_DNS;
	interval = DEFAULT_INTERVAL;
	pingsize = DEFAULT_PING_SIZE;
	maxLRU = DEFAULT_MAX_LRU;
	nrLRU = 0;

	hasIntervalFromCmdLine = false;
	hasPingsizeFromCmdLine = false;
	hasMaxLRUFromCmdLine = false;
	hasUseDNSFromCmdLine = false;

	traceThreadMutex = CreateMutex(NULL, FALSE, NULL);
	wmtrnet = new WinMTRNet(this);
}

WinMTRDialog::~WinMTRDialog()
{
	delete wmtrnet;
	CloseHandle(traceThreadMutex);
}

//*****************************************************************************
// WinMTRDialog::DoDataExchange
//
// 
//*****************************************************************************
void WinMTRDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, ID_OPTIONS, m_buttonOptions);
	DDX_Control(pDX, ID_HELP_METRICS, m_buttonHelp);
	DDX_Control(pDX, IDCANCEL, m_buttonExit);
	DDX_Control(pDX, ID_RESTART, m_buttonStart);
	DDX_Control(pDX, IDC_COMBO_HOST, m_comboHost);
	DDX_Control(pDX, IDC_LIST_MTR, m_listMTR);
	DDX_Control(pDX, IDC_STATICS, m_staticS);
	DDX_Control(pDX, IDC_STATICJ, m_staticJ);
	DDX_Control(pDX, ID_EXPH, m_buttonExpH);
	DDX_Control(pDX, ID_EXPT, m_buttonExpT);
	DDX_Control(pDX, ID_EXPC, m_buttonExpC);
}


//*****************************************************************************
// WinMTRDialog::OnInitDialog
//
// 
//*****************************************************************************
BOOL WinMTRDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	#ifndef  _WIN64
	char caption[] = {"WinMTR v0.92 32 bit by Appnor MSP - www.winmtr.net"};
	#else
	char caption[] = {"WinMTR v0.92 64 bit by Appnor MSP - www.winmtr.net"};
	#endif

	SetTimer(1, WINMTR_DIALOG_TIMER, NULL);
	SetWindowText(caption);

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);
	
	if(!statusBar.Create( this ))
		AfxMessageBox("Error creating status bar");
	statusBar.GetStatusBarCtrl().SetMinHeight(23);
		
	UINT sbi[1];
	sbi[0] = IDS_STRING_SB_NAME;	
	statusBar.SetIndicators( sbi,1);
	statusBar.SetPaneInfo(0, statusBar.GetItemID(0),SBPS_STRETCH, NULL );
	{ // Add appnor URL
		CMFCLinkCtrl* m_pWndButton = new CMFCLinkCtrl;
		if (!m_pWndButton->Create(_T("www.appnor.com"), WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0,0,0,0), &statusBar, 1234)) {
			TRACE(_T("Failed to create button control.\n"));
			return FALSE;
		}

		m_pWndButton->SetURL("http://www.appnor.com/?utm_source=winmtr&utm_medium=desktop&utm_campaign=software");
			
		if(!statusBar.AddPane(1234,1)) {
			AfxMessageBox(_T("Pane index out of range\nor pane with same ID already exists in the status bar"), MB_ICONERROR);
			return FALSE;
		}
			
		statusBar.SetPaneWidth(statusBar.CommandToIndex(1234), 100);
		statusBar.AddPaneControl(m_pWndButton, 1234, true);
	}

	for(int i = 0; i< MTR_NR_COLS; i++)
		m_listMTR.InsertColumn(i, MTR_COLUMNS[i].listHeader, LVCFMT_LEFT, MTR_COLUMNS[i].width , -1);

	m_listMTR.SetExtendedStyle(m_listMTR.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
	ApplySavedColumnOrder();

	m_buttonOptions.GetWindowRect(&m_optionsButtonRect);
	ScreenToClient(&m_optionsButtonRect);
	m_buttonHelp.GetWindowRect(&m_helpButtonRect);
	ScreenToClient(&m_helpButtonRect);
	m_buttonExit.GetWindowRect(&m_exitButtonRect);
	ScreenToClient(&m_exitButtonRect);
	m_buttonExpT.GetWindowRect(&m_exportTextButtonRect);
	ScreenToClient(&m_exportTextButtonRect);
	m_buttonExpC.GetWindowRect(&m_exportCsvButtonRect);
	ScreenToClient(&m_exportCsvButtonRect);
	m_buttonExpH.GetWindowRect(&m_exportHtmlButtonRect);
	ScreenToClient(&m_exportHtmlButtonRect);
	m_staticS.GetWindowRect(&m_buttonsPanelRect);
	ScreenToClient(&m_buttonsPanelRect);

	m_comboHost.SetFocus();

	// We need to resize the dialog to make room for control bars.
	// First, figure out how big the control bars are.
	CRect rcClientStart;
	CRect rcClientNow;
	GetClientRect(rcClientStart);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
				   0, reposQuery, rcClientNow);

	// Now move all the controls so they are in the same relative
	// position within the remaining client area as they would be
	// with no control bars.
	CPoint ptOffset(rcClientNow.left - rcClientStart.left,
					rcClientNow.top - rcClientStart.top);

	CRect  rcChild;
	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild)
	{
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}

	// Adjust the dialog window dimensions
	CRect rcWindow;
	GetWindowRect(rcWindow);
	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
	MoveWindow(rcWindow, FALSE);

	// And position the control bars
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	InitRegistry();
	LoadConfigFile();
	ApplySavedColumnOrder();
	SaveConfigFile();

	if (m_autostart) {
		m_comboHost.SetWindowText(msz_defaulthostname);
		OnRestart();
	}

	return FALSE;
}

CString WinMTRDialog::GetConfigFilePath() const
{
	char exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);

	char *slash = strrchr(exePath, '\\');
	if(slash != NULL) {
		*(slash + 1) = '\0';
	}

	CString path(exePath);
	path += "WinMTR.ini";
	return path;
}

BOOL WinMTRDialog::LoadConfigFile()
{
	CString path = GetConfigFilePath();
	if(_access(path, 0) != 0) {
		return FALSE;
	}

	pingsize = GetPrivateProfileInt("Config", "PingSize", pingsize, path);
	maxLRU = GetPrivateProfileInt("Config", "MaxLRU", maxLRU, path);
	useDNS = GetPrivateProfileInt("Config", "UseDNS", useDNS ? 1 : 0, path) ? TRUE : FALSE;
	interval = (double)GetPrivateProfileInt("Config", "IntervalMs", (int)(interval * 1000.0), path) / 1000.0;

	return TRUE;
}

void WinMTRDialog::ApplySavedColumnOrder()
{
	CString path = GetConfigFilePath();
	char buffer[512];
	GetPrivateProfileString("Columns", "Order", "", buffer, sizeof(buffer), path);

	CString orderText(buffer);
	if(orderText.IsEmpty()) {
		return;
	}

	std::vector<int> order;
	if(!ParseColumnOrder(orderText, order)) {
		return;
	}

	if((int)order.size() != MTR_NR_COLS) {
		return;
	}

	std::vector<bool> seen(MTR_NR_COLS, false);
	for(int i = 0; i < MTR_NR_COLS; ++i) {
		if(order[i] < 0 || order[i] >= MTR_NR_COLS || seen[order[i]]) {
			return;
		}
		seen[order[i]] = true;
	}

	m_listMTR.SetColumnOrderArray(MTR_NR_COLS, &order[0]);
}

void WinMTRDialog::SaveConfigFile()
{
	CString path = GetConfigFilePath();
	CString value;

	value.Format("%d", pingsize);
	WritePrivateProfileString("Config", "PingSize", value, path);

	value.Format("%d", maxLRU);
	WritePrivateProfileString("Config", "MaxLRU", value, path);

	value.Format("%d", useDNS ? 1 : 0);
	WritePrivateProfileString("Config", "UseDNS", value, path);

	value.Format("%d", (int)(interval * 1000.0));
	WritePrivateProfileString("Config", "IntervalMs", value, path);

	std::vector<int> order(MTR_NR_COLS, 0);
	if(m_listMTR.GetSafeHwnd() != NULL && m_listMTR.GetHeaderCtrl() != NULL) {
		m_listMTR.GetColumnOrderArray(&order[0], MTR_NR_COLS);
		WritePrivateProfileString("Columns", "Order", JoinColumnOrder(order), path);
	}
}

//*****************************************************************************
// WinMTRDialog::InitRegistry
//
// 
//*****************************************************************************
BOOL WinMTRDialog::InitRegistry()
{
	HKEY hKey, hKey_v;
	DWORD res, tmp_dword, value_size;
	LONG r;

	r = RegCreateKeyEx(	HKEY_CURRENT_USER, 
					"Software", 
					0, 
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS,
					NULL,
					&hKey,
					&res);
	if( r != ERROR_SUCCESS) 
		return FALSE;

	r = RegCreateKeyEx(	hKey, 
					"WinMTR", 
					0, 
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS,
					NULL,
					&hKey,
					&res);
	if( r != ERROR_SUCCESS) 
		return FALSE;

	RegSetValueEx(hKey,"Version", 0, REG_SZ, (const unsigned char *)WINMTR_VERSION, sizeof(WINMTR_VERSION)+1);
	RegSetValueEx(hKey,"License", 0, REG_SZ, (const unsigned char *)WINMTR_LICENSE, sizeof(WINMTR_LICENSE)+1);
	RegSetValueEx(hKey,"HomePage", 0, REG_SZ, (const unsigned char *)WINMTR_HOMEPAGE, sizeof(WINMTR_HOMEPAGE)+1);

	r = RegCreateKeyEx(	hKey, 
					"Config", 
					0, 
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS,
					NULL,
					&hKey_v,
					&res);
	if( r != ERROR_SUCCESS) 
		return FALSE;

	if(RegQueryValueEx(hKey_v, "PingSize", 0, NULL, (unsigned char *)&tmp_dword, &value_size) != ERROR_SUCCESS) {
		tmp_dword = pingsize;
		RegSetValueEx(hKey_v,"PingSize", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
	} else {
		if(!hasPingsizeFromCmdLine) pingsize = tmp_dword;
	}
	
	if(RegQueryValueEx(hKey_v, "MaxLRU", 0, NULL, (unsigned char *)&tmp_dword, &value_size) != ERROR_SUCCESS) {
		tmp_dword = maxLRU;
		RegSetValueEx(hKey_v,"MaxLRU", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
	} else {
		if(!hasMaxLRUFromCmdLine) maxLRU = tmp_dword;
	}
	
	if(RegQueryValueEx(hKey_v, "UseDNS", 0, NULL, (unsigned char *)&tmp_dword, &value_size) != ERROR_SUCCESS) {
		tmp_dword = useDNS ? 1 : 0;
		RegSetValueEx(hKey_v,"UseDNS", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
	} else {
		if(!hasUseDNSFromCmdLine) useDNS = (BOOL)tmp_dword;
	}

	if(RegQueryValueEx(hKey_v, "Interval", 0, NULL, (unsigned char *)&tmp_dword, &value_size) != ERROR_SUCCESS) {
		tmp_dword = (DWORD)(interval * 1000);
		RegSetValueEx(hKey_v,"Interval", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
	} else {
		if(!hasIntervalFromCmdLine) interval = (float)tmp_dword / 1000.0;
	}

	r = RegCreateKeyEx(	hKey, 
					"LRU", 
					0, 
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS,
					NULL,
					&hKey_v,
					&res);
	if( r != ERROR_SUCCESS) 
		return FALSE;
	if(RegQueryValueEx(hKey_v, "NrLRU", 0, NULL, (unsigned char *)&tmp_dword, &value_size) != ERROR_SUCCESS) {
		tmp_dword = nrLRU;
		RegSetValueEx(hKey_v,"NrLRU", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
	} else {
		char key_name[20];
		unsigned char str_host[255];
		nrLRU = tmp_dword;
		for(int i=0;i<maxLRU;i++) {
			sprintf(key_name,"Host%d", i+1);
			if((r = RegQueryValueEx(hKey_v, key_name, 0, NULL, NULL, &value_size)) == ERROR_SUCCESS) {
				RegQueryValueEx(hKey_v, key_name, 0, NULL, str_host, &value_size);
				str_host[value_size]='\0';
				m_comboHost.AddString((CString)str_host);
			}
		}
	}
	m_comboHost.AddString(CString((LPCSTR)IDS_STRING_CLEAR_HISTORY));
	RegCloseKey(hKey_v);
	RegCloseKey(hKey);
	return TRUE;
}


//*****************************************************************************
// WinMTRDialog::OnSizing
//
// 
//*****************************************************************************
void WinMTRDialog::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CDialog::OnSizing(fwSide, pRect);

	int iWidth = (pRect->right)-(pRect->left);
	int iHeight = (pRect->bottom)-(pRect->top);

	if (iWidth < 600)
		pRect->right = pRect->left + 600;
	if (iHeight <250)
		pRect->bottom = pRect->top + 250;
}

void WinMTRDialog::OnExitSizeMove()
{
	CDialog::OnExitSizeMove();

	// Manual resize can leave child controls visually stale until the next full layout pass.
	// Replaying WM_SIZE here preserves the current layout while forcing MFC to repaint children.
	CRect r;
	GetClientRect(&r);
	SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(r.Width(), r.Height()));
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME);
}


//*****************************************************************************
// WinMTRDialog::OnSize
//
// 
//*****************************************************************************
void WinMTRDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	CRect r;
	GetClientRect(&r);
	CRect lb;
	const int outerMargin = 10;
	const int innerPadding = 8;
	const int minGap = 6;
	
	if (::IsWindow(m_staticS.m_hWnd)) {
		CRect startRect;
		m_buttonStart.GetWindowRect(&startRect);
		ScreenToClient(&startRect);

		int preferredPanelWidth = innerPadding * 2 + m_optionsButtonRect.Width() + m_helpButtonRect.Width() + m_exitButtonRect.Width() + (minGap * 2);
		int minPanelLeft = startRect.right + 40;
		int panelLeft = r.Width() - preferredPanelWidth - outerMargin;
		if(panelLeft < minPanelLeft) {
			panelLeft = minPanelLeft;
		}
		int panelWidth = r.Width() - panelLeft - outerMargin;
		if(panelWidth < preferredPanelWidth) {
			panelWidth = preferredPanelWidth;
			panelLeft = r.Width() - panelWidth - outerMargin;
			if(panelLeft < minPanelLeft) {
				panelLeft = minPanelLeft;
				panelWidth = r.Width() - panelLeft - outerMargin;
			}
		}

		m_staticS.GetWindowRect(&lb);
		ScreenToClient(&lb);
		m_staticS.SetWindowPos(NULL, panelLeft, lb.top, panelWidth, lb.Height(), SWP_NOZORDER);
	}

	if (::IsWindow(m_staticJ.m_hWnd)) {
		m_staticJ.GetWindowRect(&lb);
		ScreenToClient(&lb);
		m_staticJ.SetWindowPos(NULL, lb.TopLeft().x, lb.TopLeft().y, r.Width() - 21, lb.Height(), SWP_NOMOVE | SWP_NOZORDER);
	}

	if (::IsWindow(m_buttonOptions.m_hWnd) && ::IsWindow(m_buttonHelp.m_hWnd) && ::IsWindow(m_buttonExit.m_hWnd)) {
		CRect panelRect;
		m_staticS.GetWindowRect(&panelRect);
		ScreenToClient(&panelRect);

		int availableWidth = panelRect.Width() - (innerPadding * 2);
		int totalWidth = m_optionsButtonRect.Width() + m_helpButtonRect.Width() + m_exitButtonRect.Width();
		int gap = minGap;
		if(availableWidth > totalWidth) {
			gap = (availableWidth - totalWidth) / 4;
			if(gap < minGap) {
				gap = minGap;
			}
		}
		int startX = panelRect.right - innerPadding - totalWidth - (gap * 2);
		if(startX < panelRect.left + innerPadding) {
			startX = panelRect.left + innerPadding;
		}
		int topY = panelRect.top + (m_optionsButtonRect.top - m_buttonsPanelRect.top);

		m_buttonOptions.SetWindowPos(NULL, startX, topY, m_optionsButtonRect.Width(), m_optionsButtonRect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
		startX += m_optionsButtonRect.Width() + gap;
		m_buttonHelp.SetWindowPos(NULL, startX, topY, m_helpButtonRect.Width(), m_helpButtonRect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
		startX += m_helpButtonRect.Width() + gap;
		m_buttonExit.SetWindowPos(NULL, startX, topY, m_exitButtonRect.Width(), m_exitButtonRect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
	}
	
	if (::IsWindow(m_buttonExpT.m_hWnd) && ::IsWindow(m_buttonExpC.m_hWnd) && ::IsWindow(m_buttonExpH.m_hWnd)) {
		CRect panelRect;
		m_staticS.GetWindowRect(&panelRect);
		ScreenToClient(&panelRect);

		int availableWidth = panelRect.Width() - (innerPadding * 2);
		int totalWidth = m_exportTextButtonRect.Width() + m_exportCsvButtonRect.Width() + m_exportHtmlButtonRect.Width();
		int gap = minGap;
		if(availableWidth > totalWidth) {
			gap = (availableWidth - totalWidth) / 4;
			if(gap < minGap) {
				gap = minGap;
			}
		}
		int startX = panelRect.right - innerPadding - totalWidth - (gap * 2);
		if(startX < panelRect.left + innerPadding) {
			startX = panelRect.left + innerPadding;
		}
		int topY = panelRect.top + (m_exportTextButtonRect.top - m_buttonsPanelRect.top);

		m_buttonExpT.SetWindowPos(NULL, startX, topY, m_exportTextButtonRect.Width(), m_exportTextButtonRect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
		startX += m_exportTextButtonRect.Width() + gap;
		m_buttonExpC.SetWindowPos(NULL, startX, topY, m_exportCsvButtonRect.Width(), m_exportCsvButtonRect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
		startX += m_exportCsvButtonRect.Width() + gap;
		m_buttonExpH.SetWindowPos(NULL, startX, topY, m_exportHtmlButtonRect.Width(), m_exportHtmlButtonRect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
	}

	if (::IsWindow(m_listMTR.m_hWnd)) {
		m_listMTR.GetWindowRect(&lb);
		ScreenToClient(&lb);
		m_listMTR.SetWindowPos(NULL, lb.TopLeft().x, lb.TopLeft().y, r.Width() - 21, r.Height() - lb.top - 25, SWP_NOMOVE | SWP_NOZORDER);
	}

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
				   0, reposQuery, r);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

}


//*****************************************************************************
// WinMTRDialog::OnPaint
//
// 
//*****************************************************************************
void WinMTRDialog::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


//*****************************************************************************
// WinMTRDialog::OnQueryDragIcon
//
// 
//*****************************************************************************
HCURSOR WinMTRDialog::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


//*****************************************************************************
// WinMTRDialog::OnDblclkList
//
//*****************************************************************************
void WinMTRDialog::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	if(state == TRACING) {
		
		POSITION pos = m_listMTR.GetFirstSelectedItemPosition();
		if(pos!=NULL) {
			int nItem = m_listMTR.GetNextSelectedItem(pos);
			WinMTRProperties wmtrprop;

			if(wmtrnet->GetAddr(nItem)==0) {
				strcpy(wmtrprop.host,"");
				strcpy(wmtrprop.ip,"");
				wmtrnet->GetName(nItem, wmtrprop.comment);

				wmtrprop.pck_sent = 0;
				wmtrprop.pck_recv = 0;
				wmtrprop.pck_loss = 0.0;

				wmtrprop.ping_avrg = wmtrprop.ping_last = 0.0;
				wmtrprop.ping_best = wmtrprop.ping_worst = 0.0;
			} else {
				wmtrnet->GetName(nItem, wmtrprop.host);
				int addr = wmtrnet->GetAddr(nItem);
				sprintf (	wmtrprop.ip , "%d.%d.%d.%d", 
							(addr >> 24) & 0xff, 
							(addr >> 16) & 0xff, 
							(addr >> 8) & 0xff, 
							addr & 0xff
				);
				strcpy(wmtrprop.comment , "Host alive.");

				wmtrprop.ping_avrg = (float)wmtrnet->GetAvg(nItem); 
				wmtrprop.ping_last = (float)wmtrnet->GetLast(nItem); 
				wmtrprop.ping_best = (float)wmtrnet->GetBest(nItem);
				wmtrprop.ping_worst = (float)wmtrnet->GetWorst(nItem); 

				wmtrprop.pck_loss = (float)wmtrnet->GetLoss(nItem);
				wmtrprop.pck_recv = wmtrnet->GetReturned(nItem);
				wmtrprop.pck_sent = wmtrnet->GetXmit(nItem);
			}

			wmtrprop.DoModal();
		}
	}
}


//*****************************************************************************
// WinMTRDialog::SetHostName
//
//*****************************************************************************
void WinMTRDialog::SetHostName(const char *host)
{
	m_autostart = 1;
	strncpy(msz_defaulthostname,host,1000);
}


//*****************************************************************************
// WinMTRDialog::SetPingSize
//
//*****************************************************************************
void WinMTRDialog::SetPingSize(int ps)
{
	pingsize = ps;
}

//*****************************************************************************
// WinMTRDialog::SetMaxLRU
//
//*****************************************************************************
void WinMTRDialog::SetMaxLRU(int mlru)
{
	maxLRU = mlru;
}


//*****************************************************************************
// WinMTRDialog::SetInterval
//
//*****************************************************************************
void WinMTRDialog::SetInterval(float i)
{
	interval = i;
}

//*****************************************************************************
// WinMTRDialog::SetUseDNS
//
//*****************************************************************************
void WinMTRDialog::SetUseDNS(BOOL udns)
{
	useDNS = udns;
}




//*****************************************************************************
// WinMTRDialog::OnRestart
//
// 
//*****************************************************************************
void WinMTRDialog::OnRestart() 
{
	// If clear history is selected, just clear the registry and listbox and return
	if(m_comboHost.GetCurSel() == m_comboHost.GetCount() - 1) {
		ClearHistory();
		return;
	}

	CString sHost;

	if(state == IDLE) {
		m_comboHost.GetWindowText(sHost);
		sHost.TrimLeft();
		sHost.TrimLeft();
      
		if(sHost.IsEmpty()) {
			AfxMessageBox("No host specified!");
			m_comboHost.SetFocus();
			return ;
		}
		m_listMTR.DeleteAllItems();
	}

	if(state == IDLE) {

		if(InitMTRNet()) {
			if(m_comboHost.FindString(-1, sHost) == CB_ERR) {
				m_comboHost.InsertString(m_comboHost.GetCount() - 1,sHost);

				HKEY hKey;
				DWORD tmp_dword;
				LONG r;
				char key_name[20];

				r = RegOpenKeyEx(	HKEY_CURRENT_USER, "Software", 0, KEY_ALL_ACCESS,&hKey);
				r = RegOpenKeyEx(	hKey, "WinMTR", 0, KEY_ALL_ACCESS, &hKey);
				r = RegOpenKeyEx(	hKey, "LRU", 0, KEY_ALL_ACCESS, &hKey);

				if(nrLRU >= maxLRU)
					nrLRU = 0;
				
				nrLRU++;
				sprintf(key_name, "Host%d", nrLRU);
				const char* hostValue = (LPCTSTR)sHost;
				DWORD hostValueBytes = (DWORD)(strlen(hostValue) + 1);
				r = RegSetValueEx(hKey,key_name, 0, REG_SZ, (const unsigned char *)hostValue, hostValueBytes);
				tmp_dword = nrLRU;
				r = RegSetValueEx(hKey,"NrLRU", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
				RegCloseKey(hKey);
			}
			Transit(TRACING);
		}
	} else {
		Transit(STOPPING);
	}
}


//*****************************************************************************
// WinMTRDialog::OnOptions
//
// 
//*****************************************************************************
void WinMTRDialog::OnOptions() 
{
	WinMTROptions optDlg;

	optDlg.SetPingSize(pingsize);
	optDlg.SetInterval(interval);
	optDlg.SetMaxLRU(maxLRU);
	optDlg.SetUseDNS(useDNS);

	if(IDOK == optDlg.DoModal()) {

		pingsize = optDlg.GetPingSize();
		interval = optDlg.GetInterval();
		maxLRU = optDlg.GetMaxLRU();
		useDNS = optDlg.GetUseDNS();

		HKEY hKey;
		DWORD tmp_dword;
		LONG r;
		char key_name[20];

		r = RegOpenKeyEx(	HKEY_CURRENT_USER, "Software", 0, KEY_ALL_ACCESS,&hKey);
		r = RegOpenKeyEx(	hKey, "WinMTR", 0, KEY_ALL_ACCESS, &hKey);
		r = RegOpenKeyEx(	hKey, "Config", 0, KEY_ALL_ACCESS, &hKey);
		tmp_dword = pingsize;
		RegSetValueEx(hKey,"PingSize", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
		tmp_dword = maxLRU;
		RegSetValueEx(hKey,"MaxLRU", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
		tmp_dword = useDNS ? 1 : 0;
		RegSetValueEx(hKey,"UseDNS", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
		tmp_dword = (DWORD)(interval * 1000);
		RegSetValueEx(hKey,"Interval", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
		RegCloseKey(hKey);
		if(maxLRU<nrLRU) {
			r = RegOpenKeyEx(	HKEY_CURRENT_USER, "Software", 0, KEY_ALL_ACCESS,&hKey);
			r = RegOpenKeyEx(	hKey, "WinMTR", 0, KEY_ALL_ACCESS, &hKey);
			r = RegOpenKeyEx(	hKey, "LRU", 0, KEY_ALL_ACCESS, &hKey);

			for(int i = maxLRU; i<=nrLRU; i++) {
					sprintf(key_name, "Host%d", i);
					r = RegDeleteValue(hKey,key_name);
			}
			nrLRU = maxLRU;
			tmp_dword = nrLRU;
			r = RegSetValueEx(hKey,"NrLRU", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
			RegCloseKey(hKey);
		}

		SaveConfigFile();
	}
}

void WinMTRDialog::OnHelpMetrics()
{
	WinMTRHelp helpDlg;
	helpDlg.DoModal();
}


//*****************************************************************************
// WinMTRDialog::OnCTTC
//
// 
//*****************************************************************************
void WinMTRDialog::OnCTTC() 
{	
	CString source = BuildTextReport(wmtrnet);
	if(!CopyTextToClipboard(this, source)) {
		AfxMessageBox("Unable to copy text report to clipboard.");
	}
}


//*****************************************************************************
// WinMTRDialog::OnCHTC
//
// 
//*****************************************************************************
void WinMTRDialog::OnCHTC() 
{
	CString source = BuildHtmlReport(wmtrnet);
	if(!CopyTextToClipboard(this, source)) {
		AfxMessageBox("Unable to copy HTML report to clipboard.");
	}
}


//*****************************************************************************
// WinMTRDialog::OnEXPT
//
// 
//*****************************************************************************
void WinMTRDialog::OnEXPT() 
{	
	TCHAR BASED_CODE szFilter[] = _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||");

	CFileDialog dlg(FALSE,
                   _T("TXT"),
                   NULL,
                   OFN_HIDEREADONLY | OFN_EXPLORER,
                   szFilter,
                   this);
	if(dlg.DoModal() == IDOK) {
		CString report = BuildTextReport(wmtrnet);
		if(!WriteReportFile(dlg.GetPathName(), report)) {
			AfxMessageBox("Unable to save text report.");
		}
	}
}

void WinMTRDialog::OnEXPC()
{
	TCHAR BASED_CODE szFilter[] = _T("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*||");

	CFileDialog dlg(FALSE,
                   _T("CSV"),
                   NULL,
                   OFN_HIDEREADONLY | OFN_EXPLORER,
                   szFilter,
                   this);
	if(dlg.DoModal() == IDOK) {
		CString report = BuildCsvReport(wmtrnet);
		if(!WriteReportFile(dlg.GetPathName(), report)) {
			AfxMessageBox("Unable to save CSV report.");
		}
	}
}


//*****************************************************************************
// WinMTRDialog::OnEXPH
//
// 
//*****************************************************************************
void WinMTRDialog::OnEXPH() 
{
   TCHAR BASED_CODE szFilter[] = _T("HTML Files (*.htm, *.html)|*.htm;*.html|All Files (*.*)|*.*||");

   CFileDialog dlg(FALSE,
                   _T("HTML"),
                   NULL,
                   OFN_HIDEREADONLY | OFN_EXPLORER,
                   szFilter,
                   this);

	if(dlg.DoModal() == IDOK) {
		CString report = BuildHtmlReport(wmtrnet);
		if(!WriteReportFile(dlg.GetPathName(), report)) {
			AfxMessageBox("Unable to save HTML report.");
		}
	}


}


//*****************************************************************************
// WinMTRDialog::WinMTRDialog
//
// 
//*****************************************************************************
void WinMTRDialog::OnCancel() 
{
}


//*****************************************************************************
// WinMTRDialog::DisplayRedraw
//
// 
//*****************************************************************************
int WinMTRDialog::DisplayRedraw()
{
	int nh = wmtrnet->GetMax();
	while( m_listMTR.GetItemCount() > nh ) m_listMTR.DeleteItem(m_listMTR.GetItemCount() - 1);

	for(int i=0;i <nh ; i++) {
		CString host = GetColumnText(wmtrnet, i, MTR_COL_HOSTNAME);
		if(m_listMTR.GetItemCount() <= i )
			m_listMTR.InsertItem(i, host);
		else
			m_listMTR.SetItem(i, MTR_COL_HOSTNAME, LVIF_TEXT, host, 0, 0, 0, 0); 
		
		for(int column = MTR_COL_NR; column < MTR_NR_COLS; ++column) {
			m_listMTR.SetItemText(i, column, GetColumnText(wmtrnet, i, column));
		}
	}

	return 0;
}


//*****************************************************************************
// WinMTRDialog::InitMTRNet
//
// 
//*****************************************************************************
int WinMTRDialog::InitMTRNet()
{
	char strtmp[255];
	char *Hostname = strtmp;
	char buf[255];
	struct hostent *host;
	m_comboHost.GetWindowText(strtmp, 255);
   	
	if (Hostname == NULL) Hostname = "localhost";
   
	int isIP=1;
	char *t = Hostname;
	while(*t) {
		if(!isdigit(*t) && *t!='.') {
			isIP=0;
			break;
		}
		t++;
	}

	if(!isIP) {
		sprintf(buf, "Resolving host %s...", strtmp);
		statusBar.SetPaneText(0,buf);
		host = gethostbyname(Hostname);
		if(host == NULL) {
			statusBar.SetPaneText(0, CString((LPCSTR)IDS_STRING_SB_NAME) );
			AfxMessageBox("Unable to resolve hostname.");
			return 0;
		}
	}

	return 1;
}


//*****************************************************************************
// PingThread
//
// 
//*****************************************************************************
void PingThread(void *p)
{
	WinMTRDialog *wmtrdlg = (WinMTRDialog *)p;
	WaitForSingleObject(wmtrdlg->traceThreadMutex, INFINITE);

	struct hostent *host, *lhost;
	char strtmp[255];
	char *Hostname = strtmp;
	int traddr;
	int localaddr;

	wmtrdlg->m_comboHost.GetWindowText(strtmp, 255);
   	
	if (Hostname == NULL) Hostname = "localhost";
   
	int isIP=1;
	char *t = Hostname;
	while(*t) {
		if(!isdigit(*t) && *t!='.') {
			isIP=0;
			break;
		}
		t++;
	}

	if(!isIP) {
      host = gethostbyname(Hostname);
      traddr = *(int *)host->h_addr;
	} else
      traddr = inet_addr(Hostname);

	lhost = gethostbyname("localhost");
	if(lhost == NULL) {
      AfxMessageBox("Unable to get local IP address.");
      ReleaseMutex(wmtrdlg->traceThreadMutex);
      return;
	}
	localaddr = *(int *)lhost->h_addr;
	
	wmtrdlg->wmtrnet->DoTrace(traddr);

	ReleaseMutex(wmtrdlg->traceThreadMutex);
   _endthread();
}



void WinMTRDialog::OnCbnSelchangeComboHost()
{
}

void WinMTRDialog::ClearHistory()
{
	HKEY hKey;
	DWORD tmp_dword;
	LONG r;
	char key_name[20];

	r = RegOpenKeyEx(	HKEY_CURRENT_USER, "Software", 0, KEY_ALL_ACCESS,&hKey);
	r = RegOpenKeyEx(	hKey, "WinMTR", 0, KEY_ALL_ACCESS, &hKey);
	r = RegOpenKeyEx(	hKey, "LRU", 0, KEY_ALL_ACCESS, &hKey);

	for(int i = 0; i<=nrLRU; i++) {
		sprintf(key_name, "Host%d", i);
		r = RegDeleteValue(hKey,key_name);
	}
	nrLRU = 0;
	tmp_dword = nrLRU;
	r = RegSetValueEx(hKey,"NrLRU", 0, REG_DWORD, (const unsigned char *)&tmp_dword, sizeof(DWORD));
	RegCloseKey(hKey);

	m_comboHost.Clear();
	m_comboHost.ResetContent();
	m_comboHost.AddString(CString((LPCSTR)IDS_STRING_CLEAR_HISTORY));
}

void WinMTRDialog::OnCbnSelendokComboHost()
{
}


void WinMTRDialog::OnCbnCloseupComboHost()
{
	if(m_comboHost.GetCurSel() == m_comboHost.GetCount() - 1) {
		ClearHistory();
	}
}

void WinMTRDialog::Transit(STATES new_state)
{
	switch(new_state) {
		case IDLE:
			switch (state) {
				case STOPPING:
					transition = STOPPING_TO_IDLE;
				break;
				case IDLE:
					transition = IDLE_TO_IDLE;
				break;
				default:
					TRACE_MSG("Received state IDLE after " << state);
					return;
			}
			state = IDLE;
		break;
		case TRACING:
			switch (state) {
				case IDLE:
					transition = IDLE_TO_TRACING;
				break;
				case TRACING:
					transition = TRACING_TO_TRACING;
				break;
				default:
					TRACE_MSG("Received state TRACING after " << state);
					return;
			}
			state = TRACING;
		break;
		case STOPPING:
			switch (state) {
				case STOPPING:
					transition = STOPPING_TO_STOPPING;
				break;
				case TRACING:
					transition = TRACING_TO_STOPPING;
				break;
				default:
					TRACE_MSG("Received state STOPPING after " << state);
					return;
			}
			state = STOPPING;
		break;
		case EXIT:
			switch (state) {
				case IDLE:
					transition = IDLE_TO_EXIT;
				break;
				case STOPPING:
					transition = STOPPING_TO_EXIT;
				break;
				case TRACING:
					transition = TRACING_TO_EXIT;
				break;
				case EXIT:
				break;
				default:
					TRACE_MSG("Received state EXIT after " << state);
					return;
			}
			state = EXIT;
		break;
		default:
			TRACE_MSG("Received state " << state);
	}

	// modify controls according to new state
	switch(transition) {
		case IDLE_TO_TRACING:
			m_buttonStart.EnableWindow(FALSE);
			m_buttonStart.SetWindowText("Stop");
			m_comboHost.EnableWindow(FALSE);
			m_buttonOptions.EnableWindow(FALSE);
			statusBar.SetPaneText(0, "Double click on host name for more information.");
			_beginthread(PingThread, 0 , this);
			m_buttonStart.EnableWindow(TRUE);
		break;
		case IDLE_TO_IDLE:
			// nothing to be done
		break;
		case STOPPING_TO_IDLE:
			m_buttonStart.EnableWindow(TRUE);
			statusBar.SetPaneText(0, CString((LPCSTR)IDS_STRING_SB_NAME) );
			m_buttonStart.SetWindowText("Start");
			m_comboHost.EnableWindow(TRUE);
			m_buttonOptions.EnableWindow(TRUE);
			m_comboHost.SetFocus();
		break;
		case STOPPING_TO_STOPPING:
			DisplayRedraw();
		break;
		case TRACING_TO_TRACING:
			DisplayRedraw();
		break;
		case TRACING_TO_STOPPING:
			m_buttonStart.EnableWindow(FALSE);
			m_comboHost.EnableWindow(FALSE);
			m_buttonOptions.EnableWindow(FALSE);
			wmtrnet->StopTrace();
			statusBar.SetPaneText(0, "Waiting for last packets in order to stop trace ...");
			DisplayRedraw();
		break;
		case IDLE_TO_EXIT:
			m_buttonStart.EnableWindow(FALSE);
			m_comboHost.EnableWindow(FALSE);
			m_buttonOptions.EnableWindow(FALSE);
		break;
		case TRACING_TO_EXIT:
			m_buttonStart.EnableWindow(FALSE);
			m_comboHost.EnableWindow(FALSE);
			m_buttonOptions.EnableWindow(FALSE);
			wmtrnet->StopTrace();
			statusBar.SetPaneText(0, "Waiting for last packets in order to stop trace ...");
		break;
		case STOPPING_TO_EXIT:
			m_buttonStart.EnableWindow(FALSE);
			m_comboHost.EnableWindow(FALSE);
			m_buttonOptions.EnableWindow(FALSE);
		break;
		default:
			TRACE_MSG("Unknown transition " << transition);
	}
}


void WinMTRDialog::OnTimer(UINT_PTR nIDEvent)
{
	static unsigned int call_count = 0;
	call_count += 1;

	if(state == EXIT && WaitForSingleObject(traceThreadMutex, 0) == WAIT_OBJECT_0) {
		ReleaseMutex(traceThreadMutex);
		OnOK();
	}


	if( WaitForSingleObject(traceThreadMutex, 0) == WAIT_OBJECT_0 ) {
		ReleaseMutex(traceThreadMutex);
		Transit(IDLE);
	} else if( (call_count % 10 == 0) && (WaitForSingleObject(traceThreadMutex, 0) == WAIT_TIMEOUT) ) {
		ReleaseMutex(traceThreadMutex);
		if( state == TRACING) Transit(TRACING);
		else if( state == STOPPING) Transit(STOPPING);
	}

	CDialog::OnTimer(nIDEvent);
}


void WinMTRDialog::OnClose()
{
	SaveConfigFile();
	Transit(EXIT);
}


void WinMTRDialog::OnBnClickedCancel()
{
	SaveConfigFile();
	Transit(EXIT);
}

void WinMTRDialog::OnOK()
{
	SaveConfigFile();
	CDialog::OnOK();
}
