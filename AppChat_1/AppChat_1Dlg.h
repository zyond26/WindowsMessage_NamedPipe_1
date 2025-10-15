#pragma once

#include "afxcmn.h"
#include "afxwin.h"
#include <map>
#include <vector>

#define WM_PIPE_MESSAGE (WM_USER + 1)
#define PIPE_NAME_SELF   L"\\\\.\\pipe\\AppChat1Pipe"
#define PIPE_NAME_TARGET L"\\\\.\\pipe\\AppChat2Pipe"
class CAppChat1Dlg : public CDialogEx
{
public:
    CAppChat1Dlg(CWnd* pParent = nullptr);    // standard constructor

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_APPCHAT_1_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    // icon
    HICON m_hIcon;

    static UINT PipeServerThread(LPVOID pParam); // Thread server nhận pipe
    void StartPipeServer();                      // Khởi động server

    void SendViaWindowsMessage(const CString& message);
    void SendViaNamedPipe(const CString& message);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
    afx_msg LRESULT OnPipeMessage(WPARAM wParam, LPARAM lParam);

    afx_msg void OnLvnItemchangedListuser(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedButtonsend();

public:
    CListCtrl list_user;
    CListCtrl list_mess;
    CEdit edit_text;
    CButton btn_send;

    CString m_selectedUser;
    std::map<CString, std::vector<CString>> m_chatHistory;
};
