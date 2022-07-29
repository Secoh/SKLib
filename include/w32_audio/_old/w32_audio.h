// w32_audio.h : main header file for the w32_audio DLL
//

#pragma once

#ifndef AUD_DEF_STANDARD

#define AUD_DEF_STANDARD
#define CAUDIN_FREQ_11025	11025
#define CAUDIN_FREQ_22050	22050
#define CAUDIN_FREQ_44100	44100
#define CAUDIN_BITS_8		8
#define CAUDIN_BITS_16		16

#define CAUDOUT_GREEDY	0x01	// allocates memory for each new block, frees block when it is not needed (potentially slow)
#define CAUDOUT_QSYNC	0x02	// quasi-syncronous output, user supplies buffers (never freed) and must keep buffers alive for all time of playback
#define CAUDOUT_BLOCKS	0x04	// quasi-syncronous output, user must always send blocks of same size, must verify if can send next, can clear buffer after call

#endif


#define AUI_DLG_DUTY_PEAK_DISPLAY		0x3FFF
#define AUI_DLG_MAX_SAMPLING_MODE_LIST	64		// <-- far more than real

#define AUI_DLG_TIME_CONSTANT_MS	100		// buffer acquisition granularity time
#define AUO_DLG_PLAYBK_QUEUE		4		// number of frames for playback buffering (default is small int >=2)

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cw32_audioApp
// See w32_audio.cpp for the implementation of this class
//

class Cw32_audioApp : public CWinApp	// <-- (!!!) Physical instance of this object shall not be exposed
{
public:
	Cw32_audioApp();

	// working space

	void *MIXER;		// class CMixer;	#include "mixer.h"
	void *AUDIO;		// class CAudioIn;	#include "audioin.h"
	void *PLAYBK;		// class CAudioOut;	--//--

	// current wave-in selection	// for current wave device, refer to 'selected_device' in AUDIO

	int cur_sample_rate, cur_bits;
	bool cur_stereo;
	bool local_timer_action;

	int cur_sample_rate_out, cur_bits_out, cur_out_mode;
	bool cur_stereo_out;

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

// export functions - sections 1, 2, 3 (see source code)

// analogs for AUI functions -- active when dialog is closed
// get available range of input audio data in buffer
// trk_begin is absolute samples counter since start of acquisition
BOOL _stdcall AudioInGetCurrentRange(DWORD64 *trk_begin, DWORD *trk_avail_size);	// =GetValid()

// return a buffer that has in it all the requested time interval
// NULL if not available (too late, too early, requested too large, etc)
BYTE * _stdcall AudioInGetCurrentFragment(DWORD64 tracker, DWORD trksize);

// call interactive setup dialog
BOOL _stdcall AudioInSetupDialog(WORD nothing);
BOOL _stdcall AudioIOSetupDialogNoMixer(WORD flags);	// I/O setup without mixer (use ext prog if need)

// Remark: flags for AudioIOSetupDialogNoMixer() are possible CAUDOUT_ modes, OR-ed; 0 - all

// for output...
BOOL _stdcall AudioOutPutFragment(CONST BYTE *data, WORD sample_count, WORD *get_handle);
BOOL _stdcall AudioOutClearToSend(WORD nothing);
BOOL _stdcall AudioOutIsBufferClear(WORD buf_handle);
BOOL _stdcall AudioOutIsPlaybackDone(WORD nothing);
BOOL _stdcall AudioOutShutOff(WORD nothing);

// basic query functions for current audio setting
BYTE * _stdcall AudioInGetCurrentDeviceName(WORD nothing);
WORD _stdcall AudioInGetCurrentSampleRate(WORD nothing);
WORD _stdcall AudioInGetCurrentBits(WORD nothing);
WORD _stdcall AudioInGetCurrentChannels(WORD nothing);

// basic query functions for current *output* audio setting
BYTE * _stdcall AudioOutGetCurrentDeviceName(WORD nothing);
WORD _stdcall AudioOutGetCurrentSampleRate(WORD nothing);
WORD _stdcall AudioOutGetCurrentBits(WORD nothing);
WORD _stdcall AudioOutGetCurrentChannels(WORD nothing);


// names and types

#define AUD_DLL_NAME "w32_audio.dll"

#define AUD_RANGE_CALL		"AudioInGetCurrentRange"
#define AUD_FRAGMENT_CALL	"AudioInGetCurrentFragment"
typedef BOOL _stdcall	type_AudioInGetCurrentRange(DWORD64 *trk_begin, DWORD *trk_avail_size);
typedef BYTE * _stdcall type_AudioInGetCurrentFragment(DWORD64 tracker, DWORD trksize);

#define AUD_DIALOG_CALL			"AudioInSetupDialog"
#define AUD_IONM_DIALOG_CALL	"AudioIOSetupDialogNoMixer"
typedef BOOL _stdcall			type_AudioInSetupDialog(WORD nothing);
typedef BOOL _stdcall			type_AudioIOSetupDialogNoMixer(WORD flags);

#define AUD_OUT_PUT_FRAGMENT	"AudioOutPutFragment"
typedef BOOL _stdcall		type_AudioOutPutFragment(CONST BYTE *data, WORD sample_count, WORD *get_handle);

#define AUD_OUT_CLEAR_TO_SEND		"AudioOutClearToSend"
#define AUD_OUT_IS_BUFFER_CLEAR		"AudioOutIsBufferClear"
#define AUD_OUT_IS_PLAYBACK_DONE	"AudioOutIsPlaybackDone"
#define AUD_OUT_SHUT_OFF			"AudioOutShutOff"
typedef BOOL _stdcall type_AudioOutClearToSend(WORD nothing);
typedef BOOL _stdcall type_AudioOutIsBufferClear(WORD buf_handle);
typedef BOOL _stdcall type_AudioOutIsPlaybackDone(WORD nothing);
typedef BOOL _stdcall type_AudioOutShutOff(WORD nothing);

#define AUD_QUERY_NAME		 "AudioInGetCurrentDeviceName"
#define AUD_QUERY_SAMPLERATE "AudioInGetCurrentSampleRate"
#define AUD_QUERY_BITS		 "AudioInGetCurrentBits"
#define AUD_QUERY_CHANNELS	 "AudioInGetCurrentChannels"
typedef BYTE * _stdcall type_AudioInGetCurrentDeviceName(WORD nothing);
typedef WORD _stdcall	type_AudioInGetCurrentSampleRate(WORD nothing);
typedef WORD _stdcall	type_AudioInGetCurrentBits(WORD nothing);
typedef WORD _stdcall	type_AudioInGetCurrentChannels(WORD nothing);

#define AUD_OUT_QUERY_NAME		 "AudioOutGetCurrentDeviceName"
#define AUD_OUT_QUERY_SAMPLERATE "AudioOutGetCurrentSampleRate"
#define AUD_OUT_QUERY_BITS		 "AudioOutGetCurrentBits"
#define AUD_OUT_QUERY_CHANNELS	 "AudioOutGetCurrentChannels"
typedef BYTE * _stdcall type_AudioOutGetCurrentDeviceName(WORD nothing);
typedef WORD _stdcall	type_AudioOutGetCurrentSampleRate(WORD nothing);
typedef WORD _stdcall	type_AudioOutGetCurrentBits(WORD nothing);
typedef WORD _stdcall	type_AudioOutGetCurrentChannels(WORD nothing);

// additional function to contol data acquisition from command line - section 4

WORD _stdcall AudioInGetNoOfMixers(WORD nothing);
BYTE * _stdcall AudioInGetMixerName(WORD ord_no);
WORD _stdcall AudioInGetCurrentMixerNo(WORD nothing);
BOOL _stdcall AudioInSelectMixer(WORD ord_no);			// all "select" functions return TRUE if OK or FALSE if error

WORD _stdcall AudioInGetNoOfMixerLines(WORD nothing);	// before using functions from this group, select Mixer first
BYTE * _stdcall AudioInGetMixerLineName(WORD ord_no);
WORD _stdcall AudioInGetCurrentMixerLineNo(WORD nothing);
BOOL _stdcall AudioInSelectMixerLine(WORD ord_no);

WORD _stdcall AudioInGetNoOfDevices(WORD nothing);
BYTE * _stdcall AudioInGetDeviceName(WORD ord_no);
WORD _stdcall AudioInGetCurrentDeviceNo(WORD nothing);
BOOL _stdcall AudioInSelectDevice(WORD ord_no, WORD sample_rate, WORD bits_no, BOOL stereo,
								  WORD acquisition_granularity_milliseconds,
								  WORD acquisition_total_buffer_length_milliseconds);

#define AUD_CTRL_MX_COUNT			"AudioInGetNoOfMixers"
#define AUD_CTRL_MX_NAME			"AudioInGetMixerName"
#define AUD_CTRL_MX_CURRENT			"AudioInGetCurrentMixerNo"
#define AUD_CTRL_MX_SELECT			"AudioInSelectMixer"

#define AUD_CTRL_MX_LINE_COUNT		"AudioInGetNoOfMixerLines"
#define AUD_CTRL_MX_LINE_NAME		"AudioInGetMixerLineName"
#define AUD_CTRL_MX_LINE_CURRENT	"AudioInGetCurrentMixerLineNo"
#define AUD_CTRL_MX_LINE_SELECT		"AudioInSelectMixerLine"

#define AUD_CTRL_DEVICE_COUNT		"AudioInGetNoOfDevices"
#define AUD_CTRL_DEVICE_NAME		"AudioInGetDeviceName"
#define AUD_CTRL_DEVICE_CURRENT		"AudioInGetCurrentDeviceNo"
#define AUD_CTRL_DEVICE_SELECT		"AudioInSelectDevice"

typedef WORD _stdcall	type_AudioInGetNoOfMixers(WORD nothing);
typedef BYTE * _stdcall type_AudioInGetMixerName(WORD ord_no);
typedef WORD _stdcall	type_AudioInGetCurrentMixerNo(WORD nothing);
typedef BOOL _stdcall	type_AudioInSelectMixer(WORD ord_no);

typedef WORD _stdcall	type_AudioInGetNoOfMixerLines(WORD nothing);
typedef BYTE * _stdcall type_AudioInGetMixerLineName(WORD ord_no);
typedef WORD _stdcall	type_AudioInGetCurrentMixerLineNo(WORD nothing);
typedef BOOL _stdcall	type_AudioInSelectMixerLine(WORD ord_no);

typedef WORD _stdcall	type_AudioInGetNoOfDevices(WORD nothing);
typedef BYTE * _stdcall type_AudioInGetDeviceName(WORD ord_no);
typedef WORD _stdcall	type_AudioInGetCurrentDeviceNo(WORD nothing);
typedef BOOL _stdcall	type_AudioInSelectDevice(WORD ord_no, WORD sample_rate,
								WORD bits_no, BOOL stereo,
								WORD acquisition_granularity_milliseconds,
								WORD acquisition_total_buffer_length_milliseconds);


WORD _stdcall	AudioOutGetNoOfDevices(WORD nothing);
BYTE * _stdcall AudioOutGetDeviceName(WORD ord_no);
WORD _stdcall	AudioOutGetCurrentDeviceNo(WORD nothing);
BOOL _stdcall	AudioOutSelectDevice(WORD ord_no, WORD sample_rate,
								WORD bits_no, BOOL stereo,
								WORD block_depth, WORD mode);

#define AUD_CTRL_OUT_DEVICE_COUNT		"AudioOutGetNoOfDevices"
#define AUD_CTRL_OUT_DEVICE_NAME		"AudioOutGetDeviceName"
#define AUD_CTRL_OUT_DEVICE_CURRENT		"AudioOutGetCurrentDeviceNo"
#define AUD_CTRL_OUT_DEVICE_SELECT		"AudioOutSelectDevice"

typedef WORD _stdcall	type_AudioOutGetNoOfDevices(WORD nothing);
typedef BYTE * _stdcall type_AudioOutGetDeviceName(WORD ord_no);
typedef WORD _stdcall	type_AudioOutGetCurrentDeviceNo(WORD nothing);
typedef BOOL _stdcall	type_AudioOutSelectDevice(WORD ord_no, WORD sample_rate,
								WORD bits_no, BOOL stereo,
								WORD block_depth, WORD mode);


//

