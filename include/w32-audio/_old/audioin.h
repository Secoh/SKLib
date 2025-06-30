//

#define CAUDIN_MAINTAIN_GAP	2	// minimum gap between read and write positions
#define CAUDIN_EXTRA_BUF	4	// additional data blocks in circular buffer
#define CAUDIN_MAX_REQUEST	2	// maximum requestable buffer size

#ifndef AUD_DEF_STANDARD
#define AUD_DEF_STANDARD
#define CAUDIN_FREQ_11025	11025	// for SelectDevice()
#define CAUDIN_FREQ_22050	22050
#define CAUDIN_FREQ_44100	44100
#define CAUDIN_BITS_8		8
#define CAUDIN_BITS_16		16
#endif

class CAudioIn	// note: user must allow for unexpected NULLs is any place (!)
{
public:

	int num_devices;		// global; total # of input lines, list of names
	char **device_names;
	DWORD *device_caps;		// flags from WAVEINCAPS/dwFormats; speed, bits and mono/stereo
	BOOL *device_stereo;
	int selected_device;	// device is opened for reading when selected

	// note: CAudioIn is only functional if num_devices > 0

	// audio input uses buffer with total size
	// (time_constant_ms/1000 * sample_rate * length_multiplier) elements - not bytes
	// time_constant_ms defines acquisition granularity: data is read from hardware
	// by chunks of size (time_constant_ms/1000 * sample_rate) elements; program waits
	// and does nothing until the portion of data is received (1st, at least)
	// smaller time_constant_ms also affects readount by GetFragment() -
	// caller may not request fragment larger than
	// (CAUDIN_MAX_REQUEST * time_constant_ms/1000 * sample_rate) elements
	// warning: requests for large fragments also slow down performance

	bool SelectDevice(int ordno, int sample_rate, int bits, bool stereo,
					  int time_constant_ms, int length_multiplier = 32);
	void CloseDevice();

	int GetMaxFragment();	// just for reference, maximum size of unit that can be requested, in elements

	// when device is open, here is function to listen

	bool GetValid(int64_t *trk_begin, int *trk_avail_size);
	void *GetFragment(int64_t tracker, int trksize);// returns pointer with requested data or NULL
													// unsigned int8 - 8 bits; signed int16 - 16 bits
// constructor & deconstructor						// 1 elem - mono; 2 elems - stereo (LR)
	CAudioIn();										// returns NULL if dislikes addressing
	~CAudioIn();									// -or- when requested too big size
													// size is in elements (rather than bytes)
// internal state									// tracker is continuous address, not rolling back (i.e., "time")

	HWAVEIN m_hWaveIn;
	WAVEHDR *m_hWaveInHdrArr;
	WAVEFORMATEX pFmt;

	bool update_busy, update_refuse;						// "refuse" may also indicate fatal error
	int elem_size, unit_elem_count, units_total, unit_begin, unit_active;
	void *roll_array;
	int8_t *roll_array_aligned;
	int64_t track_begin;

//	int heartbeat;
};

#define CAUDIN_ALIGN 0x100

// Busy is set by callback; regular funcs must wait until Busy is cleared
// Refuse is set by class funcs; when set, the callback drops any input


// --------------- For completeness... playback ----

// output modes (bits)
#define CAUDOUT_GREEDY	0x01	// allocates memory for each new block, frees block when it is not needed (potentially slow)
#define CAUDOUT_QSYNC	0x02	// quasi-syncronous output, user supplies buffers (never freed) and must keep buffers alive for all time of playback
#define CAUDOUT_BLOCKS	0x04	// quasi-syncronous output, user must always send blocks of same size, must verify if can send next, can clear buffer after call

#define CAUDOUT_MIN_BLOCK_COUNT  2

// parameters and functions are similar to CAudioIn, differences are specifically described
class CAudioOut
{
public:

	int num_devices;
	char **device_names;
	DWORD *device_caps, *device_options;
	BOOL *device_stereo;
	int selected_device;

	CAudioOut();
	~CAudioOut();

	bool SelectDevice(int ordno, int sample_rate, int bits, bool stereo, int blkdepth = CAUDOUT_MIN_BLOCK_COUNT+1, int mode = CAUDOUT_GREEDY);
	void CloseDevice();

	bool ClearToSend();
	bool IsBufferClear(int handle);		// <-- for QSYNC only, verify if specific buffer# is no longer used
	bool IsPlaybackDone();				// playback is in "done" state after init, reset, or if all buffers are emptied
	bool ShutOff();						// stop playback and reset stream

	// Greedy: send data and size if clear to send - audio plays until queue is exhausted
	// QSync: send data and size if clear to send, receive handle for buffer being played/queued (get_handle must not be NULL)
	//		  Caller must not destroy or reuse buffer until IsBufferClear() returns true
	// Blocks: if clear to send, send data. Size must be sent only once for the 1-st block.
	//		   sample_count must be equal to the first block size of the first call or 0; sample_count of the first call must not be 0
	//		   caller can reuse buffer after the call
	bool PutFragment(const void *data, int sample_count = 0, int *get_handle = NULL);

	// Block handle (QSYNC) is number from 0 to (blkdepth-1); which was set in SelectDevice()

	// Exausting stream will pause playback and mark all buffers vacant (for qsync), but it will NOT reset block size (for blocks mode)
	// To reset block size, use ShutOff()
	// To change mode, close device and reopen with new mode


	HWAVEOUT m_hWaveOut;
	WAVEHDR *m_hWaveOutHdrArr;
	WAVEFORMATEX pFmt;

	int playmode, elem_size, blocks_count, block_size;
	int64_t block_active, block_vacant;	// active and vacant are through counters; "write" moves vacant, "read" moves active
	bool update_busy, update_refuse;
};



