#include"stdafx.h"
#include"mixer.h"

CMixer::CMixer()
{
	m_hMix = NULL;
	CloseDevice();	// init all constants

	num_devices = ::mixerGetNumDevs();

	device_names = (char**)malloc( sizeof(char*) * (num_devices + 32) );
	device_dest_count = (int*)malloc( sizeof(int) * (num_devices + 32) );
	if (NULL == device_names || NULL == device_dest_count)
	{
		num_devices = 0;
		return;
	}

	for (int i=0; i<num_devices; i++)
	{
		char *use_name = "nothing";
		bool listing_error = false;
		::ZeroMemory(&m_hMixCap, sizeof(m_hMixCap));
		m_hMix = NULL;
		if (::mixerOpen(&m_hMix, i, NULL, NULL, MIXER_OBJECTF_MIXER) != MMSYSERR_NOERROR)
		{
			use_name = "cannot open device";
			listing_error = true;
		}
		else if (::mixerGetDevCaps(reinterpret_cast<UINT>(m_hMix), &m_hMixCap, sizeof(m_hMixCap)) != MMSYSERR_NOERROR)
		{
			use_name = "cannot query device";
			listing_error = true;
		}

		if (!listing_error) use_name = m_hMixCap.szPname;
		device_dest_count[i] = (listing_error ? 0 : m_hMixCap.cDestinations);

		device_names[i] = (char*)malloc( sizeof(char) * (strlen(use_name) + 32) );		// can be NULL
		if (NULL != device_names[i]) strcpy(device_names[i], use_name);

		if (NULL != m_hMix) ::mixerClose(m_hMix);
		m_hMix = NULL;
	}
}

CMixer::~CMixer()
{
	CloseDevice();
	if (NULL != device_dest_count) free(device_dest_count);
	if (NULL != device_names)
	{
		for (int i=0; i<num_devices; i++) if (NULL != device_names[i]) free(device_names[i]);
		free(device_names);
	}
}

void CMixer::CloseDevice()
{
	selected_line = -1;

	if (NULL != m_hMix && num_lines && num_devices && NULL != device_lines && selected_device >= 0)
	{
		::mixerClose(m_hMix);

		for (int i=0; i<device_dest_count[selected_device]; i++)
			if (NULL != device_lines[i]) free(device_lines[i]);

		free(device_lines);
		free(line_dst_list);
		free(line_src_list);
	}

	selected_device = -1;
	num_lines = 0;
	device_lines = NULL;
	line_dst_list = NULL;
	line_src_list = NULL;

	m_hMix = NULL;
}

bool CMixer::SelectDevice(int ordno)
{
	CloseDevice();
	if (ordno < 0 || ordno >= num_devices || device_dest_count[ordno] <= 0) return false;

	if (::mixerOpen(&m_hMix, ordno, NULL, NULL, MIXER_OBJECTF_MIXER) != MMSYSERR_NOERROR)
	{
		m_hMix = NULL;
		return false;
	}

	// count all sources

	MIXERLINE mxl, mxls;
	mxl.cbStruct = sizeof(MIXERLINE);
	mxls.cbStruct = sizeof(MIXERLINE);

	num_lines = 0;
	for (int i=0; i<device_dest_count[ordno]; i++)		// for each "Destination" => 
	{
		mxl.dwDestination = i;

		if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_hMix), &mxl,
							   MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_DESTINATION)
					== MMSYSERR_NOERROR)
		{
			num_lines += mxl.cConnections;
		}
	}

	line_dst_list = (int*)malloc( sizeof(int) * (num_lines + 32) );		// we are OK with 
	line_src_list = (int*)malloc( sizeof(int) * (num_lines + 32) );		// minor memory leaks
	device_lines = (char**)malloc( sizeof(char*) * (num_lines + 32) );
	if (!num_lines || NULL == line_dst_list || NULL == line_src_list || NULL == device_lines)
	{
		::mixerClose(m_hMix);
		m_hMix = NULL;
		return false;
	}

	//

	selected_device = ordno;

	int num_cur = 0;
	for (int i=0; i<device_dest_count[ordno]; i++)		// for each "Destination" => 
	{
		mxl.dwDestination = i;
		mxls.dwDestination = i;

		if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_hMix), &mxl,
							   MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_DESTINATION)
					== MMSYSERR_NOERROR)
		{
			for (int j=0; j<(int)(mxl.cConnections); j++)
			{
				mxls.dwSource = j;
				if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_hMix), &mxls,
							   MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_SOURCE)
						== MMSYSERR_NOERROR)
				{
					// record result
					line_dst_list[num_cur] = i;
					line_src_list[num_cur] = j;
					device_lines[num_cur] = (char*)malloc( sizeof(char) *
									(strlen(mxl.szName) + strlen(mxls.szName) + 32) );
					if (NULL != device_lines[num_cur])
					{
						sprintf(device_lines[num_cur], "%s - %s", mxl.szName, mxls.szName);
					}
					num_cur++;
				}
			}
		}
	}

	num_lines = num_cur;	// it can be less

	return true;	// leave mixer open
}

bool CMixer::SelectLine(int ordno)
{
	CloseLine();
	if (selected_device < 0 || ordno < 0 || ordno >= num_lines) return false;
	selected_line = ordno;
	return true;
}

void CMixer::CloseLine()
{
	selected_line = -1;
}

