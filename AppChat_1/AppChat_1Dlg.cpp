// AppChat_1Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "AppChat_1.h"
#include "AppChat_1Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define WM_PIPE_MESSAGE (WM_USER + 1)
#define PIPE_NAME_SELF   L"\\\\.\\pipe\\AppChat1Pipe"
#define PIPE_NAME_TARGET L"\\\\.\\pipe\\AppChat2Pipe"

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CAppChat1Dlg::CAppChat1Dlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_APPCHAT_1_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAppChat1Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_user, list_user);
    DDX_Control(pDX, IDC_LIST_mess, list_mess);
    DDX_Control(pDX, IDC_EDIT_text, edit_text);
    DDX_Control(pDX, IDC_BUTTON_send, btn_send);
}

BEGIN_MESSAGE_MAP(CAppChat1Dlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_user, &CAppChat1Dlg::OnLvnItemchangedListuser)
    ON_WM_COPYDATA()
    ON_BN_CLICKED(IDC_BUTTON_send, &CAppChat1Dlg::OnBnClickedButtonsend)
    ON_MESSAGE(WM_PIPE_MESSAGE, &CAppChat1Dlg::OnPipeMessage) 
END_MESSAGE_MAP()


BOOL CAppChat1Dlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetWindowText(L"AppChat_1");
    // Set icon
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);


    list_user.InsertColumn(0, L"Name", LVCFMT_LEFT, 150);
    list_user.InsertItem(0, L"Mơ");
    list_user.InsertItem(1, L"Tiến");
    list_user.InsertItem(2, L"Hùng");

    list_mess.InsertColumn(0, L"Messages", LVCFMT_LEFT, 400);

    
    StartPipeServer();

    return TRUE;
}

void CAppChat1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

void CAppChat1Dlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
        CDialogEx::OnPaint();
    }
}

HCURSOR CAppChat1Dlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CAppChat1Dlg::OnLvnItemchangedListuser(NMHDR* pNMHDR, LRESULT* pResult)
{
    int select = list_user.GetNextItem(-1, LVNI_SELECTED);
    if (select != -1)
    {
        m_selectedUser = list_user.GetItemText(select, 0);

        list_mess.DeleteAllItems();
        auto it = m_chatHistory.find(m_selectedUser);
        if (it != m_chatHistory.end()) {
            const std::vector<CString>& messages = it->second;
            for (size_t i = 0; i < messages.size(); ++i) {
                list_mess.InsertItem((int)i, messages[i]);
            }
        }
    }
    *pResult = 0;
}

void CAppChat1Dlg::SendViaWindowsMessage(const CString& message)
{
    HWND hWnd = ::FindWindow(NULL, L"AppChat_2");
    if (!hWnd) {
        AfxMessageBox(L"Không tìm thấy AppChat_2!");
        return;
    }

    CString msg = L"gửi tới [" + m_selectedUser + L"] " + message;
    COPYDATASTRUCT cds{
        1,  // gán kiểu dữ liệu là 1
        (msg.GetLength() + 1) * sizeof(TCHAR), // chuyển kí tự sang byte; +1 vì getlength() ko tính null
        (PVOID)(LPCTSTR)msg  // con trỏ tới vùng chưa dữ liệu tin nhắn
    };
    ::SendMessage(hWnd,
        WM_COPYDATA,  // mã message chuẩn của wndows để báo app biết đây là dữ liệu copydatastruct
        (WPARAM)m_hWnd, // địa chỉ biết ai gửi - ở đây là appchat2
        (LPARAM)&cds // con trỏ đến copydatastruct chứa dữ liệu cần gửi
    );

    CString display = L"[Tôi → " + m_selectedUser + L"] " + message;
    m_chatHistory[m_selectedUser].push_back(display);
    list_mess.InsertItem(list_mess.GetItemCount(), display);
}

BOOL CAppChat1Dlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
    CString receivedMessage = (LPCTSTR)(pCopyDataStruct->lpData);

    int start = receivedMessage.Find(L"[");
    int end = receivedMessage.Find(L"]");
    CString userName = receivedMessage.Mid(start + 1, end - start - 1);

    CString display = L"[AppChat_2] " + receivedMessage;
    m_chatHistory[userName].push_back(display);

    if (userName == m_selectedUser)
        list_mess.InsertItem(list_mess.GetItemCount(), display);

    return TRUE;
}

UINT CAppChat1Dlg::PipeServerThread(LPVOID pParam)
{
    auto* dlg = (CAppChat1Dlg*)pParam;
    while (true)
    {
        HANDLE hPipe = CreateNamedPipe(
            PIPE_NAME_SELF,
            PIPE_ACCESS_INBOUND,//pipe chỉ nhận dữ liệu 1 chiều 
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // dữ liệu truyền theo từng gói kiểu message
            // pipe readmode message : pipe đọc đúng theo từng message không bị cắt dở
            // pipe wait : chờ đồng bộ;readfile/writefile sẽ chặn thread cho đến khi có dữ liệu sẵn sàng
            PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL); 
        // pipe cho kết nối nhiều client khác ; 4096 là ký tự cho dữ liệu input/output

        if (hPipe == INVALID_HANDLE_VALUE) return 0;

        BOOL ok = FALSE; 
        if (ConnectNamedPipe(hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
            ok = TRUE;
        }

        if (ok)
        {
            wchar_t buf[4096]; DWORD read;
            if (ReadFile(hPipe, buf, sizeof(buf) - sizeof(wchar_t), &read, NULL) && read > 0)
            {
                buf[read / sizeof(wchar_t)] = 0;
                CString* pStr = new CString(L"[AppChat_2] " + CString(buf));
                dlg->PostMessage(WM_PIPE_MESSAGE, (WPARAM)pStr, 0);
            }
        }
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
    return 0;
}
void CAppChat1Dlg::StartPipeServer()
{
    AfxBeginThread(PipeServerThread, this);
}
//LRESULT CAppChat1Dlg::OnPipeMessage(WPARAM wParam, LPARAM)
//{
//    CString* pStr = (CString*)wParam;
//    CString display = *pStr; delete pStr;
//    CString key = m_selectedUser.IsEmpty() ? L"AppChat_2" : m_selectedUser;
//    m_chatHistory[key].push_back(display);
//    if (key == m_selectedUser) list_mess.InsertItem(list_mess.GetItemCount(), display);
//    return 0;
//}

void CAppChat1Dlg::SendViaNamedPipe(const CString& message)
{

    CString fullMsg = L"To[" + m_selectedUser + L"]: " + message;

    HANDLE hPipe = CreateFile(
        PIPE_NAME_TARGET,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        AfxMessageBox(L"Không kết nối được tới pipe của AppChat_2!");
        return;
    }

    DWORD bytesWritten;
    WriteFile(
        hPipe,
        fullMsg.GetString(),
        (fullMsg.GetLength() + 1) * sizeof(wchar_t),
        &bytesWritten,
        NULL
    );
    CloseHandle(hPipe);

    CString display = L"[Tôi → " + m_selectedUser + L"] " + message;
    m_chatHistory[m_selectedUser].push_back(display);
    list_mess.InsertItem(list_mess.GetItemCount(), display);
}

LRESULT CAppChat1Dlg::OnPipeMessage(WPARAM wParam, LPARAM)
{
    CString* pStr = (CString*)wParam;
    CString received = *pStr;
    delete pStr;

    CString sender = L"Unknown", receiver = L"Unknown", content;

    int s1 = received.Find(L"[");
    int d1 = received.Find(L"]");
    if (s1 != -1 && d1 != -1 && d1 > s1)
        sender = received.Mid(s1 + 1, d1 - s1 - 1);

    int s2 = received.Find(L"To[");
    int d2 = received.Find(L"]:", s2);
    if (s2 != -1 && d2 != -1 && d2 > s2)
    {
        receiver = received.Mid(s2 + 3, d2 - s2 - 3);
        content = received.Mid(d2 + 2);
    }
    else
    {
        receiver = L"Unknown";
        content = received;
    }

    CString display = L"[" + sender + L" → " + receiver + L"] " + content;

    m_chatHistory[receiver].push_back(display);
    if (receiver == m_selectedUser)
        list_mess.InsertItem(list_mess.GetItemCount(), display);

    return 0;
   }

void CAppChat1Dlg::OnBnClickedButtonsend()
{
    CString mess;
    edit_text.GetWindowText(mess);
    if (m_selectedUser.IsEmpty()) {
        AfxMessageBox(L"Chưa chọn người nhận !");
        return;
    }
    else if (mess.IsEmpty()) {
        AfxMessageBox(L"chưa nhập nội dung cần gửi !");
    }

    //SendViaWindowsMessage(mess);

    SendViaNamedPipe(mess);

    edit_text.SetWindowText(L"");
   }
