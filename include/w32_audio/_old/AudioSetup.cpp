// AudioSetup.cpp : implementation file
//

#include "stdafx.h"
#include "w32_audio.h"
#include "AudioSetup.h"

#include "mixer.h"
#include "audioin.h"


// CAudioSetup dialog

IMPLEMENT_DYNAMIC(CAudioSetup, CDialog)

CAudioSetup::CAudioSetup(CWnd* pParent /*=NULL*/)
	: CDialog(CAudioSetup::IDD, pParent)
	, b_ShowAll(FALSE)
	, s_VolumePct(_T(""))
	, m_bWriteFile(FALSE)
{
	fdump = NULL;
	tracker = -1;
}

CAudioSetup::~CAudioSetup()
{
	if (NULL != fdump) fclose(fdump);	// done with output!
}

void CAudioSetup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_W32AUDIODLL_COMBO_MIXER, box_MixerList);
	DDX_Control(pDX, IDC_W32AUDIODLL_COMBO_MXLINE, box_ChannelList);
	DDX_Control(pDX, IDC_W32AUDIODLL_COMBO_WAVEIN, box_WaveInList);
	DDX_Control(pDX, IDC_W32AUDIODLL_COMBO_SAMPLING, box_SamplingModeList);
	DDX_Control(pDX, IDC_W32AUDIODLL_PROGRESS_DUTY_CYCLE, bar_DutyPeak);
	DDX_Check(pDX, IDC_W32AUDIODLL_CHECK_SHOW_ALL, b_ShowAll);
	DDX_Text(pDX, IDC_W32AUDIODLL_STATIC_VOLUME_PCT, s_VolumePct);
	DDX_Check(pDX, IDC_W32AUDIODLL_WRITE_FILE, m_bWriteFile);
}


BEGIN_MESSAGE_MAP(CAudioSetup, CDialog)
	ON_CBN_SELCHANGE(IDC_W32AUDIODLL_COMBO_MIXER, &CAudioSetup::OnCbnSelchangeComboMixer)
	ON_CBN_SELCHANGE(IDC_W32AUDIODLL_COMBO_MXLINE, &CAudioSetup::OnCbnSelchangeComboMxline)
	ON_CBN_SELCHANGE(IDC_W32AUDIODLL_COMBO_WAVEIN, &CAudioSetup::OnCbnSelchangeComboWavein)
	ON_BN_CLICKED(IDC_W32AUDIODLL_CHECK_SHOW_ALL, &CAudioSetup::OnBnClickedCheckShowAll)
	ON_CBN_SELCHANGE(IDC_W32AUDIODLL_COMBO_SAMPLING, &CAudioSetup::OnCbnSelchangeComboSampling)
	ON_BN_CLICKED(IDOK, &CAudioSetup::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CAudioSetup::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_W32AUDIODLL_WRITE_FILE, &CAudioSetup::OnBnClickedW32audiodllWriteFile)
END_MESSAGE_MAP()


//

BOOL CAudioSetup::OnInitDialog()
{
	CDialog::OnInitDialog();

//	// Set the icon for this dialog.  The framework does this automatically
//	//  when the application's main window is not a dialog
//	SetIcon(m_hIcon, TRUE);			// Set big icon
//	SetIcon(m_hIcon, FALSE);		// Set small icon

	// init specific dialog items

//	AfxMessageBox("catch");

	//

	class CMixer *MX = (class CMixer *)(pTheApp->MIXER);
	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);

	//

	volume_decay = 0;

	sel_sample_rate[0] = sel_bits[0] = 0;
	sel_stereo[0] = false;
	selection_list_size = 0;

	local_timer_busy = false;

	old_wave_in = AUI->selected_device;

	old_sample_rate = pTheApp->cur_sample_rate;
	old_bits = pTheApp->cur_bits;
	old_stereo = pTheApp->cur_stereo;

	old_mx = MX->selected_device;
	old_mx_line = MX->selected_line;

	//

	box_MixerList.SetCurSel(-1);
	box_MixerList.ResetContent();
	for (int i=0; i<MX->num_devices; i++) box_MixerList.AddString( (MX->device_names[i] == NULL) ? "(null)" : MX->device_names[i] );
	if (old_mx >= 0) box_MixerList.SetCurSel(old_mx);

	box_WaveInList.SetCurSel(-1);
	box_WaveInList.ResetContent();
	for (int i=0; i<AUI->num_devices; i++) box_WaveInList.AddString( (AUI->device_names[i] == NULL) ? "(null)" : AUI->device_names[i] );
	if (old_wave_in >= 0) box_WaveInList.SetCurSel(old_wave_in);

//	box_ChannelList.SetCurSel(-1);
//	box_ChannelList.ResetContent();

//	box_SamplingModeList.SetCurSel(-1);
//	box_SamplingModeList.ResetContent();

	bar_DutyPeak.SetRange(0, AUI_DLG_DUTY_PEAK_DISPLAY);
	bar_DutyPeak.SetPos(0);

	//

	pTheApp->local_timer_action = true;
	SetTimer(1, 100, NULL);
	UpdateData(false);

	OnCbnSelchangeComboWavein();	// update with  real droplist handler

	OnCbnSelchangeComboMixer();		// fill mixer info (not actually used though)
	if (old_mx_line >= 0 && old_mx_line < MX->num_lines)
	{
		box_ChannelList.SetCurSel(old_mx_line);
		OnCbnSelchangeComboMxline();
	}

	return TRUE;
}

// CAudioSetup message handlers

void CAudioSetup::OnCbnSelchangeComboMixer()
{
	UpdateData();

	class CMixer *MX = (class CMixer *)(pTheApp->MIXER);
	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);

	MX->CloseDevice();
	MX->SelectDevice( box_MixerList.GetCurSel() );

	box_ChannelList.SetCurSel(-1);
	box_ChannelList.ResetContent();

	for (int i=0; i<MX->num_lines; i++) box_ChannelList.AddString(MX->device_lines[i]);

	UpdateData(false);
}

void CAudioSetup::OnCbnSelchangeComboMxline()
{
	UpdateData();

	class CMixer *MX = (class CMixer *)(pTheApp->MIXER);
	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);

	if (MX->selected_device < 0) return;	// no selection, do nothing

	MX->CloseLine();
	MX->SelectLine( box_ChannelList.GetCurSel() );

	UpdateData(false);
}

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

void CAudioSetup::OnCbnSelchangeComboWavein()
{
	UpdateData();

	while (local_timer_busy) Sleep(0);

	class CMixer *MX = (class CMixer *)(pTheApp->MIXER);
	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);

	int wave_in_source_no = box_WaveInList.GetCurSel();
	if (wave_in_source_no >= AUI->num_devices) wave_in_source_no = (-1);

	box_SamplingModeList.SetCurSel(-1);
	box_SamplingModeList.ResetContent();

	AUI->CloseDevice();

	if (wave_in_source_no >= 0)  // then fill Sampling Mode List and try to position
	{
		int selection = (-1);
		int i, j;
		for (i=j=0; Sample_Modes[i].SampleRate; i++)
			if ((AUI->device_caps[wave_in_source_no] & Sample_Modes[i].dwFormat) && (b_ShowAll ? true : Sample_Modes[i].Native))
			{
				box_SamplingModeList.AddString(Sample_Modes[i].name);
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
			box_SamplingModeList.SetCurSel(selection);
			AUI->SelectDevice(wave_in_source_no, pTheApp->cur_sample_rate, pTheApp->cur_bits, pTheApp->cur_stereo, AUI_DLG_TIME_CONSTANT_MS);
		}
	}

	UpdateData(false);
}

void CAudioSetup::OnBnClickedCheckShowAll()
{
	OnCbnSelchangeComboWavein();	// just re-list
}

void CAudioSetup::OnCbnSelchangeComboSampling()
{
	UpdateData();

	while (local_timer_busy) Sleep(0);

	class CMixer *MX = (class CMixer *)(pTheApp->MIXER);
	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);

	int wave_in_source_no = box_WaveInList.GetCurSel();
	int selection = box_SamplingModeList.GetCurSel();
	if (wave_in_source_no >= 0 && selection >= 0 && selection < selection_list_size)
	{
		pTheApp->cur_sample_rate = sel_sample_rate[selection];
		pTheApp->cur_bits = sel_bits[selection];
		pTheApp->cur_stereo = sel_stereo[selection];
		AUI->CloseDevice();
		AUI->SelectDevice(wave_in_source_no, pTheApp->cur_sample_rate, pTheApp->cur_bits, pTheApp->cur_stereo, AUI_DLG_TIME_CONSTANT_MS);
	}
}

//

void CAudioSetup::OnCloseDialog()
{
	KillTimer(1);
	pTheApp->local_timer_action = false;
}

void CAudioSetup::OnBnClickedOk()
{
	OnCloseDialog();
	OnOK();
}

void CAudioSetup::OnBnClickedCancel()
{
	class CMixer *MX = (class CMixer *)(pTheApp->MIXER);
	class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);

	MX->SelectDevice(old_mx);
	MX->SelectLine(old_mx_line);

	pTheApp->cur_sample_rate = old_sample_rate;
	pTheApp->cur_bits = old_bits;
	pTheApp->cur_stereo = old_stereo;

	AUI->CloseDevice();
	AUI->SelectDevice(old_wave_in, old_sample_rate, old_bits, old_stereo, AUI_DLG_TIME_CONSTANT_MS);

	OnCloseDialog();
	OnCancel();
}

void CAudioSetup::OnTimer(UINT_PTR nIDEvent)
{
	if (pTheApp->local_timer_action)
	{
		class CMixer *MX = (class CMixer *)(pTheApp->MIXER);
		class CAudioIn *AUI = (class CAudioIn *)(pTheApp->AUDIO);

		local_timer_busy = true;
		int Display = 0;
		if (AUI->selected_device >= 0)
		{
			int64_t begin;
			int size;
			if (AUI->GetValid(&begin, &size))
			{
				int64_t end = begin + size - 10;

				if (fdump != NULL)		// insert
				{
					bool eight = (AUI->pFmt.wBitsPerSample == 8);
					bool stereo = (AUI->pFmt.nChannels > 1);

					if (tracker < 0) tracker = end;
					if (tracker < begin) tracker = begin;
					while (tracker < end)
					{
						void *fetch = AUI->GetFragment(tracker++, 1);
						if (NULL == fetch) break;
						// if (?binary?) fwrite(fetch, (stereo?(eight?2:4):(eight?1:2)), 1, fdump);
						fprintf(fdump,"%d\n",
							(eight ? (int)(stereo ?
										(((uint8_t*)fetch)[0]+((uint8_t*)fetch)[1])
										: *(uint8_t*)fetch )
								   : (stereo ?
										( ( (int)(((int16_t*)fetch)[0]) + (int)(((int16_t*)fetch)[1]) +1) /2 )
										: (int)*(int16_t*)fetch )
							 ) );
					}
				}						// ---

				int64_t beg2 = end - AUI->pFmt.nSamplesPerSec / 5;
				if (beg2 < begin) beg2 = begin;
				if (beg2 < end)
				{
					size = (int)(end - beg2);
					uint8_t *input = (uint8_t*)(AUI->GetFragment(beg2, size));
					if (input != NULL)
					{
						int AVG = 0;
						if (AUI->pFmt.nChannels > 1) size *= 2;
						for (int i=0; i<size; i++)
							if (AUI->pFmt.wBitsPerSample == 8) AVG += input[i];
							else AVG += ((int16_t*)input)[i];
						AVG /= size;

						int Swing = 0;
						for (int i=0; i<size; i++)
						{
							int SMP;
							if (AUI->pFmt.wBitsPerSample == 8) SMP = input[i];
							else SMP = ((int16_t*)input)[i];
							SMP = abs(AVG - SMP);
							if (Swing < SMP) Swing = SMP;
						}
						if (AUI->pFmt.wBitsPerSample == 8) Swing *= 0x100;	// bring to 16-bit equivalent

						Display = (int)(Swing * (double)AUI_DLG_DUTY_PEAK_DISPLAY / (unsigned)0x8000L);
						if (Display < 0) Display = 0;
						if (Display > AUI_DLG_DUTY_PEAK_DISPLAY) Display = AUI_DLG_DUTY_PEAK_DISPLAY;
					}
				}
			}
		}

		UpdateData();
		bar_DutyPeak.SetPos(Display);
		volume_decay = 9*volume_decay/10;
		if (volume_decay < Display) volume_decay = Display;
		s_VolumePct.Format(" %.0lf%%", 100*volume_decay/(double)AUI_DLG_DUTY_PEAK_DISPLAY );
		UpdateData(false);
		local_timer_busy = false;
	}

	CDialog::OnTimer(nIDEvent);
}

void CAudioSetup::OnBnClickedW32audiodllWriteFile()
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
