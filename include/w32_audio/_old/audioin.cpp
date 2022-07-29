#include"stdafx.h"
#include"audioin.h"

// internal

static void CALLBACK CAUDIN_CALLBACK( HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR P1, DWORD_PTR P2 );
static void CALLBACK CAUDOUT_CALLBACK( HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR P1, DWORD_PTR P2 );

//

CAudioIn::CAudioIn()
{
	m_hWaveIn = NULL;
	CloseDevice();

	num_devices = waveInGetNumDevs();

	device_names = (char**)malloc( sizeof(char*) * (num_devices + 32) );
	device_caps = (DWORD*)malloc( sizeof(DWORD) * (num_devices + 32) );
	device_stereo = (BOOL*)malloc( sizeof(BOOL) * (num_devices + 32) );
	if (NULL == device_names || NULL == device_caps || NULL == device_stereo)
	{
		num_devices = 0;
		return;
	}

	for (int i=0; i<num_devices; i++)
	{
		char *use_name = "nothing";
		bool listing_error = false;

		WAVEINCAPS wincaps;
		memset(&wincaps, 0, sizeof(wincaps));
		if (waveInGetDevCaps(i, &wincaps, sizeof(wincaps)) != MMSYSERR_NOERROR)
		{
			use_name = "cannot query device";
			listing_error = true;
		}

		if (listing_error)
		{
			device_stereo[i] = false;
			device_caps[i] = 0;
		}
		else
		{
			use_name = wincaps.szPname;
			device_stereo[i] = (wincaps.wChannels == 2);
			device_caps[i] = wincaps.dwFormats;
		}

		device_names[i] = (char*)malloc( sizeof(char) * (strlen(use_name) + 32) );		// can be NULL
		if (NULL != device_names[i]) strcpy(device_names[i], use_name);
	}
}

CAudioIn::~CAudioIn()
{
	CloseDevice();
	if (NULL != device_caps) free(device_caps);
	if (NULL != device_stereo) free(device_stereo);
	if (NULL != device_names)
	{
		for (int i=0; i<num_devices; i++) if (NULL != device_names[i]) free(device_names[i]);
		free(device_names);
	}
}

void CAudioIn::CloseDevice()
{
	update_refuse = true;	// do it before file closing

	if (NULL != m_hWaveIn && num_devices && selected_device >= 0)
	{
		waveInReset(m_hWaveIn);
		for (int i=0; i<units_total; i++)
			waveInUnprepareHeader(m_hWaveIn, m_hWaveInHdrArr+i, sizeof(WAVEHDR));

		waveInClose(m_hWaveIn);

		free(m_hWaveInHdrArr);
		free(roll_array);
	}

	selected_device = -1;
	m_hWaveIn = NULL;
	m_hWaveInHdrArr = NULL;

	update_busy = false;	// shutdown roll array ops
	unit_elem_count = 16;
	units_total = 16;
	elem_size = 1;
	unit_begin = unit_active = 0;
	roll_array = roll_array_aligned = NULL;
	track_begin = 0;

//	heartbeat = 0;
}

bool CAudioIn::SelectDevice(int ordno, int sample_rate, int bits, bool stereo,
							int time_constant_ms, int length_multiplier)
{
	CloseDevice();
	if (ordno < 0 || ordno >= num_devices ||
		sample_rate <= 0 || time_constant_ms <= 0 || length_multiplier <= 0) return false;

	elem_size = 0;
	if (bits == CAUDIN_BITS_8) elem_size = 1;
	if (bits == CAUDIN_BITS_16) elem_size = 2;
	if (stereo) elem_size *= 2;
	if (!elem_size) return false;

	// unit length!

	unit_elem_count = (int)((sample_rate * time_constant_ms) / 1000.0 + 0.5);
	units_total = length_multiplier;

	// get memory
	roll_array = malloc( elem_size * unit_elem_count *
						 (units_total + CAUDIN_EXTRA_BUF) + 2*CAUDIN_ALIGN );
	m_hWaveInHdrArr = (WAVEHDR*)malloc( sizeof(WAVEHDR) * units_total );
	if (NULL == roll_array || NULL == m_hWaveInHdrArr)
	{
		CloseDevice();	// safe to call it from here
		return false;
	}

	// align

	int64_t ra_fwd = ((int64_t)roll_array) + CAUDIN_ALIGN - 1;	// make things aligned
	ra_fwd -= (ra_fwd % CAUDIN_ALIGN);
	roll_array_aligned = (int8_t*)ra_fwd;

	// open

	pFmt.cbSize = 0;
	pFmt.wFormatTag = WAVE_FORMAT_PCM;
	pFmt.nChannels = (stereo ? 2 : 1);
	pFmt.nSamplesPerSec = sample_rate;
	pFmt.wBitsPerSample = bits;
	pFmt.nBlockAlign = pFmt.nChannels*bits/8;		// always whole
	pFmt.nAvgBytesPerSec = pFmt.nChannels * pFmt.nSamplesPerSec * pFmt.wBitsPerSample / 8;
	if (waveInOpen(&m_hWaveIn, ordno, &pFmt, (DWORD_PTR)CAUDIN_CALLBACK, (DWORD_PTR)this,
				   CALLBACK_FUNCTION+WAVE_FORMAT_DIRECT) != MMSYSERR_NOERROR)
	{
		m_hWaveIn = NULL;
		CloseDevice();
		return false;	// Keep It Simple
	}

	// buffer

	for (int i=0; i<length_multiplier; i++)
	{
		m_hWaveInHdrArr[i].lpData = (LPSTR)(roll_array_aligned + i * unit_elem_count * elem_size);
		m_hWaveInHdrArr[i].dwBufferLength = unit_elem_count * elem_size;
		m_hWaveInHdrArr[i].dwBytesRecorded = 0;
		m_hWaveInHdrArr[i].dwUser = i;
		m_hWaveInHdrArr[i].dwFlags = 0;
		m_hWaveInHdrArr[i].dwLoops = 0;

		if (waveInPrepareHeader(m_hWaveIn, &(m_hWaveInHdrArr[i]), sizeof(WAVEHDR)) != MMSYSERR_NOERROR ||
			waveInAddBuffer(m_hWaveIn, &(m_hWaveInHdrArr[i]), sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			CloseDevice();
			return false;	// can close now
		}
	}

	// params... only update those not set by default

	update_refuse = false;
	selected_device = ordno;

	// fire up!

	if (waveInStart(m_hWaveIn) != MMSYSERR_NOERROR)
	{
		CloseDevice();
		return false;
	}

	// this is it!

	return true;
}

void *CAudioIn::GetFragment(int64_t tracker, int trksize)
{
	if (NULL == m_hWaveIn || !num_devices || selected_device < 0) return NULL;
	if (update_refuse) { CloseDevice(); return NULL; }
	if (trksize > CAUDIN_MAX_REQUEST*unit_elem_count) return NULL;
	while (update_busy) Sleep(0);	// wait for semaphore

	// what is available interval?

	int avsize;
	if (!GetValid(NULL, &avsize)) return NULL;

	// is it not completely inside? (case 0)

	if (tracker < track_begin || tracker+trksize > track_begin+avsize) return NULL; // nothing

	// is it available without break?

	int elems_total = units_total * unit_elem_count;
	int elem_get = (int)(tracker - track_begin) + unit_begin * unit_elem_count;

	// case A: continuous, rolled (get >= total)
	if (elem_get >= elems_total)
	{
		return roll_array_aligned + elem_size * (elem_get-elems_total);
	}

	// case B: continuous, not rolled
	if (elem_get + trksize <= elems_total)
	{
		return roll_array_aligned + elem_size * elem_get;
	}

	// not continuous, copy what is the rest (case C)

	memcpy( roll_array_aligned + elem_size * elems_total,
			roll_array_aligned,
			(elem_get + trksize - elems_total) * elem_size );

	return roll_array_aligned + elem_size * elem_get;
}

int CAudioIn::GetMaxFragment()
{ return ((NULL == m_hWaveIn || !num_devices || selected_device < 0) ? 0 : CAUDIN_MAX_REQUEST*unit_elem_count); }

bool CAudioIn::GetValid(int64_t *trk_begin, int *trk_avail_size)
{
	if (NULL == m_hWaveIn || !num_devices || selected_device < 0) return false;
	if (update_refuse) { CloseDevice(); return false; }

	if (NULL != trk_begin) *trk_begin = track_begin;

	if (NULL != trk_avail_size) *trk_avail_size = unit_elem_count *
		(unit_active + ((unit_active >= unit_begin) ? 0 : units_total) - unit_begin);

	return true;
}

static void CALLBACK CAUDIN_CALLBACK( HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR P1, DWORD_PTR P2 )
{
	class CAudioIn *RCD = (class CAudioIn *)dwInstance;
	if (NULL == RCD || RCD->update_refuse) return;

	if (uMsg == WIM_DATA)
	{
//		RCD->heartbeat++;

		RCD->update_busy = true;

		// are we about to overflow?

		bool need_to_move_start = false;		// 1 unit goes out guaranteed
		if (RCD->unit_active >= RCD->unit_begin)
		{
			if (RCD->unit_begin + RCD->units_total <= RCD->unit_active + CAUDIN_MAINTAIN_GAP)
				need_to_move_start = true;
		}
		else
		{
			if (RCD->unit_active + CAUDIN_MAINTAIN_GAP >= RCD->unit_begin)
				need_to_move_start = true;
		}

		// verify that we received correct buffer; re-submit buffer

		WAVEHDR *wcur = (WAVEHDR*)P1;
		waveInUnprepareHeader(RCD->m_hWaveIn, wcur, sizeof(WAVEHDR));	// do it, otherwise handle leak

		if (wcur->dwUser != RCD->unit_active || wcur->dwBytesRecorded != RCD->unit_elem_count * RCD->elem_size)
		{
			RCD->update_refuse = true;	// this is "fatal error, immediately shutdown" flag in this context
		}
		else
		{
			wcur->dwBytesRecorded = 0;	// reset 1 buffer, append to queue
			wcur->dwFlags = 0;			// all other params must be already set
			wcur->dwLoops = 0;
			if (waveInPrepareHeader(RCD->m_hWaveIn, wcur, sizeof(WAVEHDR)) != MMSYSERR_NOERROR ||
				waveInAddBuffer(RCD->m_hWaveIn, wcur, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				RCD->update_refuse = true;
			}
		}

		// update positions

		RCD->unit_active = ((RCD->unit_active + 1) % RCD->units_total);
		if (need_to_move_start)
		{
			RCD->unit_begin = ((RCD->unit_begin + 1) % RCD->units_total);
			RCD->track_begin += RCD->unit_elem_count;
		}

		RCD->update_busy = false;
	}
}

// ----------- playback -----

CAudioOut::CAudioOut()
{
	m_hWaveOut = NULL;
	CloseDevice();

	num_devices = waveOutGetNumDevs();

	device_names  = (char**)malloc( sizeof(char*) * (num_devices + 32) );
	device_caps   = (DWORD*)malloc( sizeof(DWORD) * (num_devices + 32) );
	device_options= (DWORD*)malloc( sizeof(DWORD) * (num_devices + 32) );
	device_stereo = (BOOL*)malloc( sizeof(BOOL) * (num_devices + 32) );
	if (NULL == device_names || NULL == device_caps || NULL == device_options || NULL == device_stereo)
	{
		num_devices = 0;
		return;
	}

	for (int i=0; i<num_devices; i++)
	{
		char *use_name = "nothing";
		bool listing_error = false;

		WAVEOUTCAPS woutcaps;
		memset(&woutcaps, 0, sizeof(woutcaps));
		if (waveOutGetDevCaps(i, &woutcaps, sizeof(woutcaps)) != MMSYSERR_NOERROR)
		{
			use_name = "cannot query device";
			listing_error = true;
		}

		if (listing_error)
		{
			device_stereo[i] = false;
			device_caps[i] = 0;
			device_options[i] = 0;
		}
		else
		{
			use_name = woutcaps.szPname;
			device_stereo[i] = (woutcaps.wChannels == 2);
			device_caps[i] = woutcaps.dwFormats;
			device_options[i] = woutcaps.dwSupport;
		}

		device_names[i] = (char*)malloc( sizeof(char) * (strlen(use_name) + 32) );		// can be NULL
		if (NULL != device_names[i]) strcpy(device_names[i], use_name);
	}
}

CAudioOut::~CAudioOut()
{
	CloseDevice();
	if (NULL != device_caps) free(device_caps);
	if (NULL != device_options) free(device_options);
	if (NULL != device_stereo) free(device_stereo);
	if (NULL != device_names)
	{
		for (int i=0; i<num_devices; i++) if (NULL != device_names[i]) free(device_names[i]);
		free(device_names);
	}
}

void CAudioOut::CloseDevice()
{
	update_refuse = true;	// do it before file closing

	if (NULL != m_hWaveOut && num_devices && selected_device >= 0)
	{
		waveOutReset(m_hWaveOut);
		while (update_busy) Sleep(0);		// unlike CAudioIn, we need to make sure any interrupt activity is clear
		for (int i=0; i<blocks_count; i++)
		{
			waveOutUnprepareHeader(m_hWaveOut, m_hWaveOutHdrArr+i, sizeof(WAVEHDR));
			if ((playmode == CAUDOUT_GREEDY || playmode == CAUDOUT_BLOCKS) && m_hWaveOutHdrArr[i].lpData)
				free(m_hWaveOutHdrArr[i].lpData);
		}

		waveOutClose(m_hWaveOut);
		free(m_hWaveOutHdrArr);
	}

	selected_device = -1;
	m_hWaveOut = NULL;
	m_hWaveOutHdrArr = NULL;

	update_busy = false;

	playmode = CAUDOUT_GREEDY;
	elem_size = 1;
	blocks_count = CAUDOUT_MIN_BLOCK_COUNT;
	block_size = 0;
	block_vacant = block_active = 0;
}

bool CAudioOut::SelectDevice(int ordno, int sample_rate, int bits, bool stereo, int blkdepth, int mode)
{
	CloseDevice();
	if (ordno < 0 || ordno >= num_devices ||
		sample_rate <= 0 || blkdepth < CAUDOUT_MIN_BLOCK_COUNT) return false;
	if (mode != CAUDOUT_GREEDY && mode != CAUDOUT_QSYNC && mode != CAUDOUT_BLOCKS) return false;

	elem_size = 0;
	if (bits == CAUDIN_BITS_8) elem_size = 1;
	if (bits == CAUDIN_BITS_16) elem_size = 2;
	if (stereo) elem_size *= 2;
	if (!elem_size) return false;

	// get memory
	m_hWaveOutHdrArr = (WAVEHDR*)malloc(sizeof(WAVEHDR) * blkdepth);
	if (NULL == m_hWaveOutHdrArr)
	{
		CloseDevice();	// safe to call it from here
		return false;
	}

	// open

	pFmt.cbSize = 0;
	pFmt.wFormatTag = WAVE_FORMAT_PCM;
	pFmt.nChannels = (stereo ? 2 : 1);
	pFmt.nSamplesPerSec = sample_rate;
	pFmt.wBitsPerSample = bits;
	pFmt.nBlockAlign = pFmt.nChannels*bits/8;		// always whole
	pFmt.nAvgBytesPerSec = pFmt.nChannels * pFmt.nSamplesPerSec * pFmt.wBitsPerSample / 8;
	if (waveOutOpen(&m_hWaveOut, ordno, &pFmt, (DWORD_PTR)CAUDOUT_CALLBACK, (DWORD_PTR)this,
				   CALLBACK_FUNCTION+WAVE_FORMAT_DIRECT) != MMSYSERR_NOERROR)
	{
		m_hWaveOut = NULL;
		CloseDevice();
		return false;	// Keep It Simple
	}

	// just in case...
	waveOutSetVolume(m_hWaveOut, 0xFFFFFFFFUL);		// all that is, play "as-is"
	waveOutSetPlaybackRate(m_hWaveOut, 0x00010000);
	waveOutSetPitch(m_hWaveOut, 0x00010000);

	// buffer

	for (int i=0; i<blkdepth; i++)
	{
		m_hWaveOutHdrArr[i].lpData = NULL;		// these two are populated by PutFragment()
		m_hWaveOutHdrArr[i].dwBufferLength = 0;

		m_hWaveOutHdrArr[i].dwBytesRecorded = 0;
		m_hWaveOutHdrArr[i].dwUser = i;
		m_hWaveOutHdrArr[i].dwFlags = 0;
		m_hWaveOutHdrArr[i].dwLoops = 1;
	}

	// params... only update those not set by default

	playmode = mode;
	blocks_count = blkdepth;

	selected_device = ordno;
	update_refuse = false;

	// this is it!

//	CString S;
//	S.Format("ba=%d; bv=%d; bc=%d; pm=%d; es=%d; seld=%d", (int)block_active, (int)block_vacant, blocks_count, playmode, elem_size, selected_device);
//	AfxMessageBox(S);

	return true;
}

bool CAudioOut::IsBufferClear(int handle)
{
	if (NULL == m_hWaveOut || !num_devices || selected_device < 0 || update_refuse ||
		handle < 0 || handle >= blocks_count || block_vacant - block_active >= blocks_count) return false;
	while (update_busy) Sleep(0);

	int bA = (int)(block_active % blocks_count);
	int bL = (int)(block_vacant - block_active);
	return !(bA >= handle && handle < bA+bL || bA >= handle + blocks_count && handle + blocks_count < bA+bL);
}

bool CAudioOut::IsPlaybackDone()
{
	return (NULL == m_hWaveOut || !num_devices || selected_device < 0 || update_refuse || block_active == block_vacant);
}

bool CAudioOut::ClearToSend()
{
	return (NULL != m_hWaveOut && num_devices && selected_device >= 0 && !update_refuse && block_vacant - block_active < blocks_count);
}

bool CAudioOut::ShutOff()
{
	if (NULL == m_hWaveOut || !num_devices || selected_device < 0) return false;

	if (waveOutReset(m_hWaveOut) != MMSYSERR_NOERROR) { CloseDevice(); return false; }
	while (block_vacant > block_active) Sleep(0);

	for (int i=0; i<blocks_count; i++)
	{
		if (playmode == CAUDOUT_BLOCKS && block_size > 0) free(m_hWaveOutHdrArr[i].lpData);
		m_hWaveOutHdrArr[i].lpData = NULL;
	}

	block_active = block_vacant = 0;
	block_size = 0;						// reset running params; queue length stays the same
	return true;
}

bool CAudioOut::PutFragment(const void *data, int sample_count, int *get_handle)
{
	if (NULL == m_hWaveOut || !num_devices || selected_device < 0 || NULL == data || sample_count <= 0) return false;
	if (update_refuse) { CloseDevice(); return false; }
	while (update_busy) Sleep(0);	// wait for semaphore
	if (block_vacant - block_active >= blocks_count) return false;

	int NX = (int)(block_vacant % blocks_count);	// next block to use

//	blksize -= (blksize % elem_size);	// actually, must be multiplies of elem_size
//	int wbl = blksize * elem_size;		// size in bytes
	int wbl = sample_count * elem_size;

//	CString S;
//	S.Format("nx=%d; ba=%d; bv=%d; bc=%d; pm=%d; es=%d; sc=%d; wbl=%d",
//		NX, (int)block_active, (int)block_vacant, blocks_count, playmode, elem_size, sample_count, wbl);
//	AfxMessageBox(S);


	if (playmode == CAUDOUT_GREEDY)		// allocate memory for each new block; free memory when playback of this block is done
	{
		if (NULL != m_hWaveOutHdrArr[NX].lpData) { CloseDevice(); return false; }	// this cannot happen

		m_hWaveOutHdrArr[NX].lpData = (LPSTR)malloc(wbl);
		if (NULL == m_hWaveOutHdrArr[NX].lpData) { CloseDevice(); return false; }

//		memset(m_hWaveOutHdrArr[NX].lpData, 0, wbl);

//		CString S1;
//		S1.Format("prep-m;  lpd=%p; dat=%p; wbl=%d", m_hWaveOutHdrArr[NX].lpData, data, wbl);
//		AfxMessageBox(S1);	//"prep-m");

		memcpy(m_hWaveOutHdrArr[NX].lpData, data, wbl);
	}
	else if (playmode == CAUDOUT_QSYNC)		// caller must manage buffers itself - use get_handle && IsBufferClear() to check if specific buffer no longer in use
	{
		m_hWaveOutHdrArr[NX].lpData = (LPSTR)data;
	}
	else if (playmode == CAUDOUT_BLOCKS)	// blocks are always the same size (like after FFT transform)
	{
		if (block_size)
		{
			if (block_size != sample_count) { CloseDevice(); return false; }		// this is error
		}
		else
		{
			block_size = sample_count;
			for (int i=0; i<blocks_count; i++)		// do allocations, but only once before playback starts (!)
			{
				m_hWaveOutHdrArr[i].lpData = (LPSTR)malloc(wbl);
				if (NULL == m_hWaveOutHdrArr[i].lpData) { CloseDevice(); return false; }
			}
		}

		memcpy(m_hWaveOutHdrArr[NX].lpData, data, wbl);
	}
	else { CloseDevice(); return false; }	// invalid mode

//	AfxMessageBox("prepd");

	m_hWaveOutHdrArr[NX].dwBufferLength = wbl;
	if (waveOutPrepareHeader(m_hWaveOut, m_hWaveOutHdrArr+NX, sizeof(WAVEHDR)) != MMSYSERR_NOERROR ||
		waveOutWrite(m_hWaveOut, m_hWaveOutHdrArr+NX, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) { CloseDevice(); return false; }

	block_vacant++;
	if (get_handle) *get_handle = NX;
	return true;
}

static void CALLBACK CAUDOUT_CALLBACK( HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR P1, DWORD_PTR P2 )
{
	class CAudioOut *PLAY = (class CAudioOut *)dwInstance;
	if (NULL == PLAY || PLAY->update_refuse) return;

	if (uMsg == WOM_DONE)
	{
//		AfxMessageBox("entry_done");

		PLAY->update_busy = true;

		WAVEHDR *wcur = (WAVEHDR*)P1;
		waveOutUnprepareHeader(PLAY->m_hWaveOut, wcur, sizeof(WAVEHDR));	// do it, otherwise handle leak

		int A = PLAY->block_active;
		if (wcur->dwUser != (A % PLAY->blocks_count) || A == PLAY->block_vacant)	// check on order of playback
		{
			PLAY->update_refuse = true;	// this is "fatal error, immediately shutdown" flag in this context
		}
		else
		{
			// unlike listening, header and buffer preps are done at PutFragment()
			// for Greedy mode, deallocate memory

			if (PLAY->playmode == CAUDOUT_GREEDY)
			{
				free(wcur->lpData);
				wcur->lpData = (LPSTR)NULL;		// more reliable indication of occupancy / vacancy
			}

			PLAY->block_active++;	// this command must be the last one (before process release)
		}

		PLAY->update_busy = false;
	}
}

