
class CMixer	// note: user must allow for unexpected NULLs is any place (!)
{
public:

	int num_devices;			// global; total # of mixers, list of mixer names
	char **device_names;		// list of destination counts per mixer device
	int *device_dest_count;		// <== filled in class constructor

	int selected_device, num_lines,			// # of active (open) mixer
		*line_dst_list, *line_src_list;		// translation ord no => dest no + source no
	char **device_lines;					// num_lines entries of descriptions for dest+src pairs
											// <== filled when SelectDevice() is called

	int selected_line;					// selected destination and source for an open mixer (Mixer Line)
										// <== filled when SelectLine() is called // device must be selected first
	bool SelectDevice(int ordno);
	void CloseDevice();					// note: 1) CMixer is only functional if num_devices > 0
	bool SelectLine(int ordno);			//		 2) selection is only valid if selected_device >= 0
	void CloseLine();

// constructor & deconstructor
	CMixer();
	~CMixer();

private:
	HMIXER m_hMix;
	MIXERCAPS m_hMixCap;
};


