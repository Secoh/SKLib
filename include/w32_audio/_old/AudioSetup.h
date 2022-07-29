#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// data acquizition properties

#define AUDS_CHUNK 256	// elements

// CAudioSetup dialog

class CAudioSetup : public CDialog
{
	DECLARE_DYNAMIC(CAudioSetup)

public:
	CAudioSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAudioSetup();

	virtual BOOL OnInitDialog();
	void OnCloseDialog();

// Dialog Data
	enum { IDD = IDD_W32AUDIODLL_DIALOG_AUDIO_INPUT_SELECT };

	int volume_decay;

	int selection_list_size;
	int sel_sample_rate[AUI_DLG_MAX_SAMPLING_MODE_LIST],
		sel_bits[AUI_DLG_MAX_SAMPLING_MODE_LIST];
	bool sel_stereo[AUI_DLG_MAX_SAMPLING_MODE_LIST];
	volatile bool local_timer_busy;

	FILE *fdump;
	int64_t tracker;

	int old_wave_in;
	int old_sample_rate, old_bits;
	bool old_stereo;

	int old_mx, old_mx_line;

	Cw32_audioApp *pTheApp;		// reference to self as a program

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboMixer();
	afx_msg void OnCbnSelchangeComboMxline();
	afx_msg void OnCbnSelchangeComboWavein();
	afx_msg void OnBnClickedCheckShowAll();
	afx_msg void OnCbnSelchangeComboSampling();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CComboBox box_MixerList;
	CComboBox box_ChannelList;
	CComboBox box_WaveInList;
	CComboBox box_SamplingModeList;
	CProgressCtrl bar_DutyPeak;
	BOOL b_ShowAll;
	CString s_VolumePct;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedW32audiodllWriteFile();
	BOOL m_bWriteFile;
};
