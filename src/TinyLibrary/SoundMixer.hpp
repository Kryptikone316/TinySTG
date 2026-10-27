#ifndef TINYSOUNDMIXER_HPP
#define TINYSOUNDMIXER_HPP

#include "Dependent.hpp"

class AdpcmDecorder;

class SoundMixer
{
public:
	enum
	{
		CHANNEL_COUNT = 5,
	};
	SoundMixer(void);
	~SoundMixer(void);
	void Append(int channel, const unsigned char* buffer, int size, bool loop, int type);
	void Append(int channel, const char* path, bool loop, int type);
	void Delete(int channel);
	void Read(unsigned char* buffer, int size);
	void Clear(void);
private:
	void MixBuffer(unsigned char* destination_buffer, const unsigned char* source_buffer, int size);
	// Fills `want` decoded bytes from the channel's ADPCM file stream into
	// `dest`, using any carried-over pending byte first - see Read()'s
	// comment for why this exists instead of simply rounding odd requests
	// down to even.
	void ReadAdpcmFile(int channel, unsigned char* dest, int want);
	TinyFile* file[CHANNEL_COUNT];
	const unsigned char* buffer[CHANNEL_COUNT];
	int size[CHANNEL_COUNT];
	int sample[CHANNEL_COUNT];
	bool loop[CHANNEL_COUNT];
	int type[CHANNEL_COUNT];
	AdpcmDecorder* adpcm_decorder[CHANNEL_COUNT];
	// One decoded byte held over when an ADPCM read produces an extra byte
	// beyond what was immediately requested (see Read()'s comment) - real
	// callers on this port request as little as 1 decoded byte per call
	// (far smaller than the 2-bytes-per-encoded-byte ADPCM granularity), so
	// simply rounding a request down to an even size can round it to zero
	// and stall the channel forever instead of just losing a byte.
	bool pending_valid[CHANNEL_COUNT];
	unsigned char pending_byte[CHANNEL_COUNT];
	SoundMixer(SoundMixer&);
	SoundMixer& operator = (SoundMixer&);
};

#endif
