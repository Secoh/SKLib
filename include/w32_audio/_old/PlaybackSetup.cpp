// PlaybackSetup.cpp : implementation file
//

#include "stdafx.h"
#include "w32_audio.h"
#include "PlaybackSetup.h"

//#include "mixer.h"
#include "audioin.h"


// CPlaybackSetup dialog

IMPLEMENT_DYNAMIC(CPlaybackSetup, CDialog)

CPlaybackSetup::CPlaybackSetup(CWnd* pParent /*=NULL*/)
	: CDialog(CPlaybackSetup::IDD, pParent)
	, b_ShowAllOut(FALSE)
	, b_ShowAllIn(FALSE)
	, s_VolumePct(_T(""))
	, m_bWriteFile(FALSE)
{
	fdump = NULL;
	tracker = -1;
}

CPlaybackSetup::~CPlaybackSetup()
{
	if (NULL != fdump) fclose(fdump);	// done with output!
}

void CPlaybackSetup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_W32AUDIODLL_COMBO_WAVEIN, box_WaveInList);
	DDX_Control(pDX, IDC_W32AUDIODLL_COMBO_SAMPLING, box_SamplingModeListIn);
	DDX_Control(pDX, IDC_W32AUDIODLL_PROGRESS_DUTY_CYCLE, bar_DutyPeak);
	DDX_Control(pDX, IDC_W32AUDIODLL_COMBO_WAVEOUT, box_WaveOutList);
	DDX_Control(pDX, IDC_W32AUDIODLL_COMBO_OUT_SAMPLING, box_SamplingModeListOut);
	DDX_Control(pDX, IDC_COMBO_OUT_MODE_PUT, box_LoadingModeListOut);
	DDX_Check(pDX, IDC_W32AUDIODLL_CHECK_OUT_SHOW_ALL, b_ShowAllOut);
	DDX_Check(pDX, IDC_W32AUDIODLL_CHECK_SHOW_ALL, b_ShowAllIn);
	DDX_Text(pDX, IDC_W32AUDIODLL_STATIC_VOLUME_PCT, s_VolumePct);
	DDX_Check(pDX, IDC_W32AUDIODLL_WRITE_FILE, m_bWriteFile);
}


BEGIN_MESSAGE_MAP(CPlaybackSetup, CDialog)
ON_CBN_SELCHANGE(IDC_W32AUDIODLL_COMBO_WAVEIN, &CPlaybackSetup::OnCbnSelchangeW32audiodllComboWavein)
ON_BN_CLICKED(IDC_W32AUDIODLL_CHECK_SHOW_ALL, &CPlaybackSetup::OnBnClickedW32audiodllCheckShowAll)
ON_CBN_SELCHANGE(IDC_W32AUDIODLL_COMBO_SAMPLING, &CPlaybackSetup::OnCbnSelchangeW32audiodllComboSampling)
ON_BN_CLICKED(IDOK, &CPlaybackSetup::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CPlaybackSetup::OnBnClickedCancel)
ON_WM_TIMER()
ON_BN_CLICKED(IDC_W32AUDIODLL_WRITE_FILE, &CPlaybackSetup::OnBnClickedW32audiodllWriteFile)
ON_CBN_SELCHANGE(IDC_W32AUDIODLL_COMBO_WAVEOUT, &CPlaybackSetup::OnCbnSelchangeW32audiodllComboWaveout)
ON_BN_CLICKED(IDC_W32AUDIODLL_CHECK_OUT_SHOW_ALL, &CPlaybackSetup::OnBnClickedW32audiodllCheckOutShowAll)
ON_CBN_SELCHANGE(IDC_W32AUDIODLL_COMBO_OUT_SAMPLING, &CPlaybackSetup::OnCbnSelchangeW32audiodllComboOutSampling)
ON_CBN_SELCHANGE(IDC_COMBO_OUT_MODE_PUT, &CPlaybackSetup::OnCbnSelchangeComboOutModePut)
ON_BN_CLICKED(IDC_BUTTON_OTST, &CPlaybackSetup::OnBnClickedButtonOtst)
END_MESSAGE_MAP()

//

static const struct
{
	DWORD dwFormat;		// 0-0-0 is for end of list
	int SampleRate;
	int Bits;
	bool Stereo;
	bool Native;
	const char *name;
}
	Sample_Modes[] = {
		{	WAVE_FORMAT_1M08,	CAUDIN_FREQ_11025,	CAUDIN_BITS_8,	false,	false,	"11.025 kHz, mono, 8-bit"	 },
		{	WAVE_FORMAT_1M16,	CAUDIN_FREQ_11025,	CAUDIN_BITS_16,	false,	true,	"11.025 kHz, mono, 16-bit"	 },
		{	WAVE_FORMAT_1S08,	CAUDIN_FREQ_11025,	CAUDIN_BITS_8,	true,	false,	"11.025 kHz, stereo, 8-bit"	 },
		{	WAVE_FORMAT_1S16,	CAUDIN_FREQ_11025,	CAUDIN_BITS_16,	true,	false,	"11.025 kHz, stereo, 16-bit" },
		{	WAVE_FORMAT_2M08,	CAUDIN_FREQ_22050,	CAUDIN_BITS_8,	false,	false,	"22.050 kHz, mono, 8-bit"	 },
		{	WAVE_FORMAT_2M16,	CAUDIN_FREQ_22050,	CAUDIN_BITS_16,	false,	true,	"22.050 kHz, mono, 16-bit"	 },
		{	WAVE_FORMAT_2S08,	CAUDIN_FREQ_22050,	CAUDIN_BITS_8,	true,	false,	"22.050 kHz, stereo, 8-bit"	 },
		{	WAVE_FORMAT_2S16,	CAUDIN_FREQ_22050,	CAUDIN_BITS_16,	true,	false,	"22.050 kHz, stereo, 16-bit" },
		{	WAVE_FORMAT_1M08,	CAUDIN_FREQ_44100,	CAUDIN_BITS_8,	false,	false,	"44.100 kHz, mono, 8-bit"	 },
		{	WAVE_FORMAT_4M16,	CAUDIN_FREQ_44100,	CAUDIN_BITS_16,	false,	true,	"44.100 kHz, mono, 16-bit"	 },
		{	WAVE_FORMAT_4S08,	CAUDIN_FREQ_44100,	CAUDIN_BITS_8,	true,	false,	"44.100 kHz, stereo, 8-bit"	 },
		{	WAVE_FORMAT_4S16,	CAUDIN_FREQ_44100,	CAUDIN_BITS_16,	true,	false,	"44.100 kHz, stereo, 16-bit" },
		{	0, 0, 0	  }
	};

static const struct
{
	int mode;
	const char *name;
}
	Load_Modes[] = {
		{	CAUDOUT_GREEDY,		"Allocate Buffers On-The-Fly"	},
		{	CAUDOUT_QSYNC,		"Buffers Managed By Caller"		},
		{	CAUDOUT_BLOCKS,		"Always Same Size Blocks"		},
		{	0	}
	};

//

BOOL CPlaybackSetup::OnInitDialog()		// copy-paste from AudioSetup.cpp (!)
{
	CDialog::OnInitDialog();

	// init specific dialog items

//	AfxMessageBox("catch");

	//

	class CMixer *MX = (class CMixer *)(pTheApp->MIXER);
	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);
	class CAudioOut *AUO = (class CAudioOut *)(pTheApp->PLAYBK);

	//

	volume_decay = 0;

	sel_sample_rate[0] = sel_bits[0] = sel_sample_rate_out[0] = sel_bits_out[0] = 0;
	sel_stereo[0] = sel_stereo_out[0] = false;
	selection_list_size = selection_list_size_out = selection_list_size_mode_out = 0;

	out_flags = 0;

	local_timer_busy = false;

	old_wave_in = AUI->selected_device;
	old_wave_out = AUO->selected_device;

	old_sample_rate = pTheApp->cur_sample_rate;
	old_bits = pTheApp->cur_bits;
	old_stereo = pTheApp->cur_stereo;

	old_sample_rate_out = pTheApp->cur_sample_rate_out;
	old_bits_out = pTheApp->cur_bits_out;
	old_stereo_out = pTheApp->cur_stereo_out;
	old_mode_out = pTheApp->cur_out_mode;

	//

	box_WaveInList.SetCurSel(-1);
	box_WaveInList.ResetContent();
	for (int i=0; i<AUI->num_devices; i++) box_WaveInList.AddString( (AUI->device_names[i] == NULL) ? "(null)" : AUI->device_names[i] );
	if (old_wave_in >= 0) box_WaveInList.SetCurSel(old_wave_in);
//	box_SamplingModeListIn.SetCurSel(-1);
//	box_SamplingModeListIn.ResetContent();

	bar_DutyPeak.SetRange(0, AUI_DLG_DUTY_PEAK_DISPLAY);
	bar_DutyPeak.SetPos(0);

	box_WaveOutList.SetCurSel(-1);
	box_WaveOutList.ResetContent();
	for (int i=0; i<AUO->num_devices; i++) box_WaveOutList.AddString( (AUO->device_names[i] == NULL) ? "(null)" : AUO->device_names[i] );
	if (old_wave_out >= 0) box_WaveOutList.SetCurSel(old_wave_out);
//	box_SamplingModeListOut.SetCurSel(-1);
//	box_SamplingModeListOut.ResetContent();

	//

	pTheApp->local_timer_action = true;
	SetTimer(1, 100, NULL);
	UpdateData(false);

	OnCbnSelchangeW32audiodllComboWavein();  // update with  real droplist handler
	OnCbnSelchangeW32audiodllComboWaveout();

	return TRUE;
}

void CPlaybackSetup::OnCbnSelchangeW32audiodllComboWavein()
{
	UpdateData();

	while (local_timer_busy) Sleep(0);

	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);

	int wave_in_source_no = box_WaveInList.GetCurSel();
	if (wave_in_source_no >= AUI->num_devices) wave_in_source_no = (-1);

	box_SamplingModeListIn.SetCurSel(-1);
	box_SamplingModeListIn.ResetContent();

	AUI->CloseDevice();

	if (wave_in_source_no >= 0)  // then fill Sampling Mode List and try to position
	{
		int selection = (-1);
		int i, j;
		for (i=j=0; Sample_Modes[i].SampleRate; i++)
			if ((AUI->device_caps[wave_in_source_no] & Sample_Modes[i].dwFormat) && (b_ShowAllIn ? true : Sample_Modes[i].Native))
			{
				box_SamplingModeListIn.AddString(Sample_Modes[i].name);
				sel_sample_rate[j] = Sample_Modes[i].SampleRate;
				sel_bits[j] = Sample_Modes[i].Bits;
				sel_stereo[j] = Sample_Modes[i].Stereo;
				if (pTheApp->cur_sample_rate == sel_sample_rate[j] &&
					pTheApp->cur_bits == sel_bits[j] &&
					pTheApp->cur_stereo == sel_stereo[j]) selection = j;
				j++;
			}

		selection_list_size = j;

		if (selection >= 0)
		{
			box_SamplingModeListIn.SetCurSel(selection);
			AUI->SelectDevice(wave_in_source_no, pTheApp->cur_sample_rate, pTheApp->cur_bits, pTheApp->cur_stereo, AUI_DLG_TIME_CONSTANT_MS);
		}
	}

	UpdateData(false);
}

void CPlaybackSetup::OnBnClickedW32audiodllCheckShowAll()
{
	OnCbnSelchangeW32audiodllComboWavein();
}

void CPlaybackSetup::OnCbnSelchangeW32audiodllComboSampling()
{
	UpdateData();

	while (local_timer_busy) Sleep(0);

	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);

	int wave_in_source_no = box_WaveInList.GetCurSel();
	int selection = box_SamplingModeListIn.GetCurSel();
	if (wave_in_source_no >= 0 && selection >= 0 && selection < selection_list_size)
	{
		pTheApp->cur_sample_rate = sel_sample_rate[selection];
		pTheApp->cur_bits = sel_bits[selection];
		pTheApp->cur_stereo = sel_stereo[selection];
		AUI->CloseDevice();
		AUI->SelectDevice(wave_in_source_no, pTheApp->cur_sample_rate, pTheApp->cur_bits, pTheApp->cur_stereo, AUI_DLG_TIME_CONSTANT_MS);
	}
}

void CPlaybackSetup::OnCloseDialog()
{
	KillTimer(1);
	pTheApp->local_timer_action = false;
}

void CPlaybackSetup::OnBnClickedOk()
{
	OnCloseDialog();
	OnOK();
}

void CPlaybackSetup::OnBnClickedCancel()
{
	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);
	class CAudioOut *AUO = (class CAudioOut *)(pTheApp->PLAYBK);

	pTheApp->cur_sample_rate = old_sample_rate;
	pTheApp->cur_bits = old_bits;
	pTheApp->cur_stereo = old_stereo;

	AUI->CloseDevice();
	AUI->SelectDevice(old_wave_in, old_sample_rate, old_bits, old_stereo, AUI_DLG_TIME_CONSTANT_MS);

	pTheApp->cur_sample_rate_out = old_sample_rate_out;
	pTheApp->cur_bits_out = old_bits_out;
	pTheApp->cur_stereo_out = old_stereo_out;
	pTheApp->cur_out_mode = old_mode_out;

	AUO->CloseDevice();
	AUO->SelectDevice(old_wave_out, old_sample_rate_out, old_bits_out, old_stereo_out, AUO_DLG_PLAYBK_QUEUE, old_mode_out);

	OnCloseDialog();
	OnCancel();
}

void CPlaybackSetup::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
}

void CPlaybackSetup::OnBnClickedW32audiodllWriteFile()
{
	UpdateData();

	if (NULL != fdump) { while (local_timer_busy) Sleep(1); fclose(fdump); }
	fdump = NULL;
	tracker = -1;

	if (m_bWriteFile) 
	{
		CFileDialog dlg(false, "csv", "selection.csv");
		if (dlg.DoModal()==IDOK) fdump = fopen(dlg.GetPathName(), "wt");
	}

	if (NULL == fdump) m_bWriteFile = false;
//	else fwrite(write_Selection, "Header\n");

	UpdateData(false);
}

void CPlaybackSetup::OnCbnSelchangeW32audiodllComboWaveout()
{
	UpdateData();

	while (local_timer_busy) Sleep(0);

	// lets update loading mode first... 
	if (!(out_flags & (CAUDOUT_GREEDY|CAUDOUT_QSYNC|CAUDOUT_BLOCKS))) out_flags = (CAUDOUT_GREEDY|CAUDOUT_QSYNC|CAUDOUT_BLOCKS);

	box_LoadingModeListOut.SetCurSel(-1);
	box_LoadingModeListOut.ResetContent();

	int selection_mode = -1;
	int i, j;
	for (i=j=0; Load_Modes[i].mode; i++)
		if (Load_Modes[i].mode & (out_flags | (b_ShowAllOut ? CAUDOUT_GREEDY : 0)))
		{
			box_LoadingModeListOut.AddString(Load_Modes[i].name);
			sel_mode_out[j] = Load_Modes[i].mode;
			if (pTheApp->cur_out_mode == sel_mode_out[j]) selection_mode = j;
			j++;
		}

	selection_list_size_mode_out = j;

	// continue

	class CAudioOut *AUO = (class CAudioOut *)(pTheApp->PLAYBK);

	int wave_out_source_no = box_WaveOutList.GetCurSel();
	if (wave_out_source_no >= AUO->num_devices) wave_out_source_no = (-1);

	box_SamplingModeListOut.SetCurSel(-1);
	box_SamplingModeListOut.ResetContent();

	AUO->CloseDevice();

	if (wave_out_source_no >= 0)  // then fill Sampling Mode List and try to position
	{
		int selection = (-1);
		int i, j;
		for (i=j=0; Sample_Modes[i].SampleRate; i++)
			if ((AUO->device_caps[wave_out_source_no] & Sample_Modes[i].dwFormat) && (b_ShowAllOut ? true : Sample_Modes[i].Native))
			{
				box_SamplingModeListOut.AddString(Sample_Modes[i].name);
				sel_sample_rate_out[j] = Sample_Modes[i].SampleRate;
				sel_bits_out[j] = Sample_Modes[i].Bits;
				sel_stereo_out[j] = Sample_Modes[i].Stereo;
				if (pTheApp->cur_sample_rate_out == sel_sample_rate_out[j] &&
					pTheApp->cur_bits_out == sel_bits_out[j] &&
					pTheApp->cur_stereo_out == sel_stereo_out[j]) selection = j;
				j++;
			}

		selection_list_size_out = j;

		if (selection >= 0 && selection_mode >= 0)
		{
			box_SamplingModeListOut.SetCurSel(selection);
			box_LoadingModeListOut.SetCurSel(selection_mode);
			AUO->SelectDevice(wave_out_source_no, pTheApp->cur_sample_rate_out, pTheApp->cur_bits_out, pTheApp->cur_stereo_out, AUO_DLG_PLAYBK_QUEUE, pTheApp->cur_out_mode);
		}
	}

	UpdateData(false);
}

void CPlaybackSetup::OnBnClickedW32audiodllCheckOutShowAll()
{
	OnCbnSelchangeW32audiodllComboWaveout();
}

void CPlaybackSetup::OnCbnSelchangeW32audiodllComboOutSampling()
{
	UpdateData();

	while (local_timer_busy) Sleep(0);

	class CAudioOut *AUO = (class CAudioOut *)(pTheApp->PLAYBK);

//	AfxMessageBox("check");

	int wave_out_source_no = box_WaveOutList.GetCurSel();
	int selection = box_SamplingModeListOut.GetCurSel();
	int selection_mode = box_LoadingModeListOut.GetCurSel();

//	CString S1;
//	S1.Format("WO=%d; SN=%d; MN=%d", wave_out_source_no, selection, selection_mode);
//	AfxMessageBox(S1);

	if (wave_out_source_no >= 0 && selection >= 0 && selection < selection_list_size_out
		&& selection_mode >= 0 && selection_mode < selection_list_size_mode_out)
	{
		pTheApp->cur_sample_rate_out = sel_sample_rate_out[selection];
		pTheApp->cur_bits_out = sel_bits_out[selection];
		pTheApp->cur_stereo_out = sel_stereo_out[selection];
		pTheApp->cur_out_mode = sel_mode_out[selection_mode];
		AUO->CloseDevice();

//		CString S;
//		S.Format("WO=%d; SR=%d; BT=%d; ST=%c; MD=%d", wave_out_source_no, pTheApp->cur_sample_rate_out,
//			pTheApp->cur_bits_out, (pTheApp->cur_stereo_out?'T':'F'), pTheApp->cur_out_mode);
//		AfxMessageBox(S);

//		if (!
			AUO->SelectDevice(wave_out_source_no, pTheApp->cur_sample_rate_out, pTheApp->cur_bits_out, pTheApp->cur_stereo_out, AUO_DLG_PLAYBK_QUEUE, pTheApp->cur_out_mode);
//			)
//			AfxMessageBox("false");
	}
}

void CPlaybackSetup::OnCbnSelchangeComboOutModePut()
{
	OnCbnSelchangeW32audiodllComboOutSampling();
}

#define FPLAY 1000	// Hz
#define TPLAY 1000	// ms

void CPlaybackSetup::OnBnClickedButtonOtst()
{
	UpdateData();

	// prepare wave 1 kHz 1 sec

	bool sixteen = (pTheApp->cur_bits_out > 10);
	bool stereo = pTheApp->cur_stereo_out;
	int elem_size = (sixteen ? 2 : 1) * (stereo ? 2 : 1);
	int buf_count = (pTheApp->cur_sample_rate_out * TPLAY) / 1000;
	int buf_size = elem_size * buf_count;
	uint8_t *BUF = (uint8_t*)malloc(sizeof(uint8_t) * (buf_size+32));
	if (!BUF) return;

//	CString SM;
//	SM.Format("alloc %d ptr %p", sizeof(uint8_t) * (buf_size+32), BUF);
//	AfxMessageBox(SM);

	for (int i=0; i<buf_count; i++)
	{
		int p = i*elem_size;
		double vform = sin(2*3.1416*FPLAY*(double)i/pTheApp->cur_sample_rate_out);
		if (sixteen)
		{
			*(int16_t*)(BUF+p) = (int16_t)(vform * 0x7FFE);
			if (stereo) *(int16_t*)(BUF+p+2) = *(int16_t*)(BUF+p);
		}
		else
		{
			BUF[p] = (uint8_t)((vform+1)*0x7E);
			if (stereo) BUF[p+1] = BUF[p];
		}
	}
//	srand(time(NULL));
//	for (int i=0; i<buf_size; i++) BUF[i]=(rand()%0x7E);

	// launch!

	if (! ((class CAudioOut *)(pTheApp->PLAYBK))->PutFragment(BUF, buf_count) ) AfxMessageBox("false");

	// wait for completion and reset

	while ( !((class CAudioOut *)(pTheApp->PLAYBK))->IsPlaybackDone() ) Sleep(1);

//	AfxMessageBox("pre-shutoff");

	((class CAudioOut *)(pTheApp->PLAYBK))->ShutOff();

	free(BUF);

	// done!
}
