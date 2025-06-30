// w32_audio.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "w32_audio.h"

#include "../lib_common/defines.h"

#include "mixer.h"
#include "audioin.h"

#include "AudioSetup.h"
#include "PlaybackSetup.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// Cw32_audioApp

BEGIN_MESSAGE_MAP(Cw32_audioApp, CWinApp)
END_MESSAGE_MAP()


// Cw32_audioApp construction

Cw32_audioApp::Cw32_audioApp()
{
	local_timer_action = false;

	cur_sample_rate = cur_bits = 0;
	cur_stereo = false;

	cur_sample_rate_out = cur_bits_out = 0;
	cur_stereo_out = false;
	cur_out_mode = CAUDOUT_GREEDY;

//	AfxMessageBox("init");

	MIXER = new CMixer;
	AUDIO = new CAudioIn;
	PLAYBK= new CAudioOut;
}

// destructor
//	delete MX;
//	delete AUI;


// The one and only Cw32_audioApp object // we don't expose that to outside

Cw32_audioApp theApp;


// ----------------------------------------------------------------------
// Section 1
// Cw32_audioApp initialization

BOOL Cw32_audioApp::InitInstance()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CWinApp::InitInstance();
	return TRUE;
}

BOOL _stdcall AudioInSetupDialog(WORD nothing)		// <-- this is for interactive setup by human
{													// (when used, caller program is not required
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	//  to import functions from Section 4)
	CAudioSetup dlg;
	dlg.pTheApp = &theApp;
	return (IDOK == dlg.DoModal() ? TRUE : FALSE);
}

BOOL _stdcall AudioIOSetupDialogNoMixer(WORD flags) // <-- also a menu, no mixer option
{													  // MIXER is left NULL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPlaybackSetup dlg;
	dlg.out_flags = flags & (CAUDOUT_GREEDY | CAUDOUT_QSYNC | CAUDOUT_BLOCKS);
	dlg.pTheApp = &theApp;
	return (IDOK == dlg.DoModal() ? TRUE : FALSE);
}

// ----------------------------------------------------------------------
// Section 2
// online data acquisition - transparent for caller

BOOL _stdcall AudioInGetCurrentRange(DWORD64 *trk_begin, DWORD *trk_avail_size)	// =GetValid()
{
	if (theApp.local_timer_action) return FALSE;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (((class CAudioIn *)(theApp.AUDIO))->GetValid((int64_t*)trk_begin, (int*)trk_avail_size) ? TRUE : FALSE);
}

BYTE * _stdcall AudioInGetCurrentFragment(DWORD64 tracker, DWORD trksize)
{
	if (theApp.local_timer_action) return NULL;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (BYTE*)(((class CAudioIn *)(theApp.AUDIO))->GetFragment((int64_t)tracker, (int)trksize));
}

// ----------------------------------------------------------------------
// Section 3
// optional parameters describing data format of the current active mode

WORD _stdcall AudioInGetCurrentSampleRate(WORD nothing)
{
	if (theApp.local_timer_action || !theApp.cur_sample_rate) return CAUDIN_FREQ_11025;
	return theApp.cur_sample_rate;
}
	
WORD _stdcall AudioInGetCurrentBits(WORD nothing)
{
	if (theApp.local_timer_action || !theApp.cur_bits) return CAUDIN_BITS_8;
	return theApp.cur_bits;
}

WORD _stdcall AudioInGetCurrentChannels(WORD nothing)
{
	if (theApp.local_timer_action) return 1;	// "MONO"
	return (theApp.cur_stereo ? 2 : 1);
}

BYTE * _stdcall AudioInGetCurrentDeviceName(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int n = ((class CAudioIn *)(theApp.AUDIO))->selected_device;
	if (n < 0) return (BYTE*)"none selected";
	if (n >= ((class CAudioIn *)(theApp.AUDIO))->num_devices || ((class CAudioIn *)(theApp.AUDIO))->device_names[n] == NULL) return (BYTE*)"(null name)";
	return (BYTE*)(((class CAudioIn *)(theApp.AUDIO))->device_names[n]);
}

WORD _stdcall AudioOutGetCurrentSampleRate(WORD nothing)
{
	if (theApp.local_timer_action || !theApp.cur_sample_rate_out) return CAUDIN_FREQ_11025;
	return theApp.cur_sample_rate_out;
}
	
WORD _stdcall AudioOutGetCurrentBits(WORD nothing)
{
	if (theApp.local_timer_action || !theApp.cur_bits_out) return CAUDIN_BITS_8;
	return theApp.cur_bits_out;
}

WORD _stdcall AudioOutGetCurrentChannels(WORD nothing)
{
	if (theApp.local_timer_action) return 1;	// "MONO"
	return (theApp.cur_stereo_out ? 2 : 1);
}

BYTE * _stdcall AudioOutGetCurrentDeviceName(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int n = ((class CAudioOut *)(theApp.PLAYBK))->selected_device;
	if (n < 0) return (BYTE*)"none selected";
	if (n >= ((class CAudioOut *)(theApp.PLAYBK))->num_devices || ((class CAudioOut *)(theApp.PLAYBK))->device_names[n] == NULL) return (BYTE*)"(null name)";
	return (BYTE*)(((class CAudioOut *)(theApp.PLAYBK))->device_names[n]);
}

// ----------------------------------------------------------------------------------
// Section 4
// to control parameters of Audio I/O - replaces AudioInSetupDialog() functionality

WORD _stdcall AudioInGetNoOfMixers(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ((class CAudioIn *)(theApp.MIXER))->num_devices;
}

BYTE * _stdcall AudioInGetMixerName(WORD ord_no)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (ord_no < 0 || ord_no >= ((class CAudioIn *)(theApp.MIXER))->num_devices
		|| ((class CAudioIn *)(theApp.MIXER))->device_names[ord_no] == NULL) return (BYTE*)"(null name)";
	return (BYTE*)(((class CAudioIn *)(theApp.MIXER))->device_names[ord_no]);
}

WORD _stdcall AudioInGetCurrentMixerNo(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ((class CAudioIn *)(theApp.MIXER))->selected_device;
}

BOOL _stdcall AudioInSelectMixer(WORD ord_no)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	class CMixer *MX = (class CMixer *)(theApp.MIXER);
	//class CAudioIn *AUI = (class CAudioIn *)(theApp.AUDIO);
	if (ord_no < 0 || ord_no >= MX->num_devices) return FALSE;
	MX->CloseDevice();
	return (MX->SelectDevice(ord_no) ? TRUE : FALSE);
}

// before using functions from Mixer Line group, select Mixer first
WORD _stdcall AudioInGetNoOfMixerLines(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ((class CMixer *)(theApp.MIXER))->num_lines;
}

BYTE * _stdcall AudioInGetMixerLineName(WORD ord_no)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (ord_no < 0 || ord_no >= ((class CMixer *)(theApp.MIXER))->num_lines
		|| ((class CMixer *)(theApp.MIXER))->device_lines == NULL
		|| ((class CMixer *)(theApp.MIXER))->device_lines[ord_no] == NULL) return (BYTE*)"(null name)";
	return (BYTE*)(((class CMixer *)(theApp.MIXER))->device_lines[ord_no]);
}

WORD _stdcall AudioInGetCurrentMixerLineNo(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ((class CMixer *)(theApp.MIXER))->selected_line;
}

BOOL _stdcall AudioInSelectMixerLine(WORD ord_no)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	class CMixer *MX = (class CMixer *)(theApp.MIXER);
	//class CAudioIn *AUI = (class CAudioIn *)(theApp.AUDIO);
	if (ord_no < 0 || ord_no >= MX->num_lines) return FALSE;
	MX->CloseLine();
	return (MX->SelectLine(ord_no) ? TRUE : FALSE);
}

WORD _stdcall AudioInGetNoOfDevices(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ((class CAudioIn *)(theApp.AUDIO))->num_devices;
}

BYTE * _stdcall AudioInGetDeviceName(WORD ord_no)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (ord_no < 0 || ord_no >= ((class CAudioIn *)(theApp.AUDIO))->num_devices
		|| ((class CAudioIn *)(theApp.AUDIO))->device_names[ord_no] == NULL) return (BYTE*)"(null name)";
	return (BYTE*)(((class CAudioIn *)(theApp.AUDIO))->device_names[ord_no]);
}

WORD _stdcall AudioInGetCurrentDeviceNo(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ((class CAudioIn *)(theApp.AUDIO))->selected_device;
}

BOOL _stdcall AudioInSelectDevice(WORD ord_no, WORD sample_rate, WORD bits_no, BOOL stereo,
								  WORD acquisition_granularity_milliseconds,
								  WORD acquisition_total_buffer_length_milliseconds)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//class CMixer *MX = (class CMixer *)(theApp.MIXER);
	class CAudioIn *AUI = (class CAudioIn *)(theApp.AUDIO);
	if (ord_no < 0 || ord_no >= AUI->num_devices) return FALSE;
	AUI->CloseDevice();
	int lmul = (acquisition_total_buffer_length_milliseconds + acquisition_granularity_milliseconds - 1)
				/ acquisition_granularity_milliseconds;
	return (AUI->SelectDevice(ord_no, sample_rate, bits_no, __bool(stereo),
								acquisition_granularity_milliseconds, lmul) ? TRUE : FALSE);
}

// ----------------------------------------------------------------------
// Section 5
// Output functions

BOOL _stdcall AudioOutPutFragment(CONST BYTE *data, WORD sample_count, WORD *get_handle)
{
	if (theApp.local_timer_action) return FALSE;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int int_get_handle=0;
	bool R = ((class CAudioOut *)(theApp.PLAYBK))->PutFragment(data, sample_count, &int_get_handle);
	if (get_handle) *get_handle = (WORD)int_get_handle;
	return (R ? TRUE : FALSE);;
}

BOOL _stdcall AudioOutClearToSend(WORD nothing)
{
	if (theApp.local_timer_action) return FALSE;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (((class CAudioOut *)(theApp.PLAYBK))->ClearToSend() ? TRUE : FALSE);
}

BOOL _stdcall AudioOutIsBufferClear(WORD buf_handle)
{
	if (theApp.local_timer_action) return FALSE;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (((class CAudioOut *)(theApp.PLAYBK))->IsBufferClear(buf_handle) ? TRUE : FALSE);
}

BOOL _stdcall AudioOutIsPlaybackDone(WORD nothing)
{
	if (theApp.local_timer_action) return FALSE;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (((class CAudioOut *)(theApp.PLAYBK))->IsPlaybackDone() ? TRUE : FALSE);
}

BOOL _stdcall AudioOutShutOff(WORD nothing)
{
	if (theApp.local_timer_action) return FALSE;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (((class CAudioOut *)(theApp.PLAYBK))->ShutOff() ? TRUE : FALSE);
}

// ----------------------------------------------------------------------
// Section 6
// Playback device control

WORD _stdcall	AudioOutGetNoOfDevices(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ((class CAudioOut *)(theApp.PLAYBK))->num_devices;
}

BYTE * _stdcall AudioOutGetDeviceName(WORD ord_no)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (ord_no < 0 || ord_no >= ((class CAudioOut *)(theApp.PLAYBK))->num_devices
		|| ((class CAudioOut *)(theApp.PLAYBK))->device_names[ord_no] == NULL) return (BYTE*)"(null name)";
	return (BYTE*)(((class CAudioOut *)(theApp.PLAYBK))->device_names[ord_no]);
}

WORD _stdcall	AudioOutGetCurrentDeviceNo(WORD nothing)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ((class CAudioOut *)(theApp.PLAYBK))->selected_device;
}

BOOL _stdcall	AudioOutSelectDevice(WORD ord_no, WORD sample_rate,
								WORD bits_no, BOOL stereo,
								WORD block_depth, WORD mode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	class CAudioOut *AUO = (class CAudioOut *)(theApp.PLAYBK);
	if (ord_no < 0 || ord_no >= AUO->num_devices) return FALSE;
	AUO->CloseDevice();
	return (AUO->SelectDevice(ord_no, sample_rate, bits_no, __bool(stereo), block_depth, mode) ? TRUE : FALSE);
}

// ----------------------------------------------------------------------


