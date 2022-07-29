#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// data acquizition properties

#define AUDS_CHUNK 256	// elements

// CPlaybackSetup dialog

class CPlaybackSetup : public CDialog
{
	DECLARE_DYNAMIC(CPlaybackSetup)

public:
	CPlaybackSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPlaybackSetup();

	virtual BOOL OnInitDialog();
	void OnCloseDialog();

// Dialog Data
	enum { IDD = IDD_W32AUDIODLL_DIALOG_AUDIO_OUTPUT_SELECT };

	unsigned out_flags;  // possible output modes (caller-assigned)

	int volume_decay;

	int selection_list_size;
	int sel_sample_rate[AUI_DLG_MAX_SAMPLING_MODE_LIST],
		sel_bits[AUI_DLG_MAX_SAMPLING_MODE_LIST];
	bool sel_stereo[AUI_DLG_MAX_SAMPLING_MODE_LIST];

	int selection_list_size_out, selection_list_size_mode_out;
	int sel_sample_rate_out[AUI_DLG_MAX_SAMPLING_MODE_LIST],
		sel_bits_out[AUI_DLG_MAX_SAMPLING_MODE_LIST],
		sel_mode_out[AUI_DLG_MAX_SAMPLING_MODE_LIST];
	bool sel_stereo_out[AUI_DLG_MAX_SAMPLING_MODE_LIST];

	volatile bool local_timer_busy;

	FILE *fdump;
	int64_t tracker;

	int old_wave_in, old_wave_out, old_mode_out;
	int old_sample_rate, old_bits, old_sample_rate_out, old_bits_out;
	bool old_stereo, old_stereo_out;

	Cw32_audioApp *pTheApp;		// reference to self as a program

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnCbnSelchangeW32audiodllComboWavein();
	afx_msg void OnBnClickedW32audiodllCheckShowAll();
	afx_msg void OnCbnSelchangeW32audiodllComboSampling();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedW32audiodllWriteFile();
	afx_msg void OnCbnSelchangeW32audiodllComboWaveout();
	afx_msg void OnBnClickedW32audiodllCheckOutShowAll();
	afx_msg void OnCbnSelchangeW32audiodllComboOutSampling();
	afx_msg void OnCbnSelchangeComboOutModePut();
	afx_msg void OnBnClickedButtonOtst();

	CComboBox box_WaveInList;
	CComboBox box_SamplingModeListIn;
	CProgressCtrl bar_DutyPeak;
	CComboBox box_WaveOutList;
	CComboBox box_SamplingModeListOut;
	CComboBox box_LoadingModeListOut;
	BOOL b_ShowAllOut;
	BOOL b_ShowAllIn;
	CString s_VolumePct;
	BOOL m_bWriteFile;
};
