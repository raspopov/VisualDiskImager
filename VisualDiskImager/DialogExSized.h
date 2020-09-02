#pragma once

// CDialogExSized dialog

class CDialogExSized : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogExSized)

public:
	CDialogExSized() = default;
	CDialogExSized(UINT nIDTemplate, CWnd *pParent = nullptr) : CDialogEx( nIDTemplate, pParent ) {}
	CDialogExSized(LPCTSTR lpszTemplateName, CWnd *pParentWnd = nullptr) : CDialogEx( lpszTemplateName, pParentWnd ) {}
	virtual ~CDialogExSized() override = default;

	void ReloadLayout();

	void SaveWindowPlacement();
	void RestoreWindowPlacement();

private:
	CRect m_rcInitial,  m_rcInitialClient;		// Начальный (минимальный) размер окна

protected:
	virtual BOOL OnInitDialog() override;

	afx_msg void OnGetMinMaxInfo( MINMAXINFO* lpMMI );
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
