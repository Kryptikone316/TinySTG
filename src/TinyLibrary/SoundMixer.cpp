#include <stdio.h>
#include <string.h>
#include "Dependent.hpp"
#include "SoundMixer.hpp"
#include "AdpcmDecorder.hpp"

SoundMixer::SoundMixer(void)
{
	for(int i = 0; i < CHANNEL_COUNT; ++ i)
	{
		this->file[i] = NULL;
		this->buffer[i] = NULL;
		this->size[i] = 0;
		this->sample[i] = 0;
		this->loop[i] = false;
		this->type[i] = 0;
		this->adpcm_decorder[i] = NULL;
		this->pending_valid[i] = false;
		this->pending_byte[i] = 0;
	}
}

SoundMixer::~SoundMixer(void)
{
	Clear();
}

void SoundMixer::Append(int channel, const unsigned char* buffer, int size, bool loop, int type)
{
	this->file[channel] = NULL;
	this->buffer[channel] = buffer;
	this->size[channel] = size;
	this->sample[channel] = 0;
	this->loop[channel] = loop;
	this->type[channel] = type;
	this->pending_valid[channel] = false;
	if(type == 1)
	{
		this->adpcm_decorder[channel] = new AdpcmDecorder();
	}
	else
	{
		this->adpcm_decorder[channel] = NULL;
	}
}

void SoundMixer::Append(int channel, const char* path, bool loop, int type)
{
	TinyFile* file = new TinyFile;
	if(file->Open(path, TinyFile::READ))
	{
		this->file[channel] = file;
	}
	else
	{
		this->file[channel] = NULL;
	}
	this->buffer[channel] = NULL;
	this->size[channel] = 0;
	this->sample[channel] = 0;
	this->loop[channel] = loop;
	this->type[channel] = type;
	this->pending_valid[channel] = false;
	if (type == 1)
	{
		this->adpcm_decorder[channel] = new AdpcmDecorder();
	}
	else
	{
		this->adpcm_decorder[channel] = NULL;
	}
}

void SoundMixer::Delete(int channel)
{
	if(this->file[channel] != NULL)
	{
		this->file[channel]->Close();
		delete this->file[channel];
		this->file[channel] = NULL;
	}
	this->buffer[channel] = NULL;
	this->size[channel] = 0;
	this->sample[channel] = 0;
	this->loop[channel] = false;
	this->pending_valid[channel] = false;
}

// Fills `want` decoded bytes into `dest` from channel's ADPCM file stream.
// Real callers on this port can request as few as 1 decoded byte per call
// (SoundMixer::Read() gets driven straight from MainLoop_NoWaitLoop()'s
// busy-spin at the same tiny per-iteration granularity SoundStream's own
// writableSize uses, not any larger batched amount) - far below the 2
// decoded bytes per 1 encoded byte this 4-bit codec actually reads at.
// An earlier fix here simply rounded an odd `want` down to even and
// deferred the remainder to "next call" - correct in isolation, but with
// `want` frequently being exactly 1, rounding 1 down to 0 skips the whole
// read, and since the NEXT call is just as likely to also be 1, the
// channel could stall forever making zero progress (confirmed on real
// hardware, 2026-10-09: SFX got cleaner - no more garbage noise injected
// via MixBuffer - but BGM went completely silent instead of merely
// corrupted). This version never stalls: it decodes in real 2-byte pairs
// (rounding the READ up, not the request down) and banks any one extra
// decoded byte in pending_byte[channel] for the very next call to drain
// first - every call that still has file data left makes real progress,
// and the output is byte-identical to a correct single-shot decode
// regardless of how the caller chops up its requests.
void SoundMixer::ReadAdpcmFile(int channel, unsigned char* dest, int want)
{
	int pos = 0;
	if(this->pending_valid[channel] && (want > 0))
	{
		dest[0] = this->pending_byte[channel];
		this->pending_valid[channel] = false;
		pos = 1;
	}
	int remaining = want - pos;
	if(remaining > 0)
	{
		size_t read_size = (static_cast<size_t>(remaining) + 1) / 2;
		unsigned char* read_buffer = new unsigned char[read_size];
		this->file[channel]->Read(read_buffer, read_size);
		unsigned char* decode_buffer = new unsigned char[read_size * 2];
		this->adpcm_decorder[channel]->Decord(decode_buffer, read_buffer, static_cast<int>(read_size));
		delete [] read_buffer;
		memcpy(dest + pos, decode_buffer, remaining);
		if(static_cast<int>(read_size * 2) > remaining)
		{
			this->pending_byte[channel] = decode_buffer[remaining];
			this->pending_valid[channel] = true;
		}
		delete [] decode_buffer;
	}
}

void SoundMixer::Read(unsigned char* buffer, int size)
{
	memset(buffer, 0x80, size);
	for(int channel = 0; channel < CHANNEL_COUNT; ++ channel)
	{
		if(this->buffer[channel] == NULL)
		{
			if(this->file[channel] == NULL)
			{
				// this channel is free
				continue;
			}
			int file_size = static_cast<int>(this->file[channel]->GetSize());
			if(file_size == 0)
			{
				continue;
			}
			if(this->type[channel] == 1)
			{
				file_size *= 2;
			}
			int mix_size = size;
			int left_size = file_size - this->sample[channel];
			if(left_size < size)
			{
				mix_size = left_size;
			}
			if(mix_size > 0)
			{
				unsigned char* mix_buffer = new unsigned char[mix_size];
				if(this->type[channel] == 1)
				{
					ReadAdpcmFile(channel, mix_buffer, mix_size);
				}
				else
				{
					this->file[channel]->Read(mix_buffer, mix_size);
				}
				MixBuffer(buffer, mix_buffer, mix_size);
				delete[] mix_buffer;
				this->sample[channel] += mix_size;
				if(this->loop[channel] == true)
				{
					if(size > left_size)
					{
						int left_mix_size = size - left_size;
						if(left_mix_size > 0)
						{
							unsigned char* mix_buffer = new unsigned char[left_mix_size];
							this->file[channel]->Seek(0, TinyFile::TOP);
							this->sample[channel] = 0;
							if(this->type[channel] == 1)
							{
								// Any carried-over pending byte belonged to
								// the position BEFORE this seek - discard it,
								// the decoder is resetting to a fresh stream.
								this->pending_valid[channel] = false;
								this->adpcm_decorder[channel]->Clear();
								ReadAdpcmFile(channel, mix_buffer, left_mix_size);
							}
							else
							{
								this->file[channel]->Read(mix_buffer, left_mix_size);
							}
							MixBuffer(&buffer[mix_size], mix_buffer, left_mix_size);
							delete[] mix_buffer;
							this->sample[channel] += left_mix_size;
						}
					}
				}
			}
			if(this->sample[channel] >= file_size)
			{
				// Second, separate EOF-reset path from the one inside the
				// loop[channel] block above - this one fires when a call
				// happens to land EXACTLY on the file's last byte with no
				// overflow needed THIS call (the common case here, since
				// real requests are tiny - 0/1/2 bytes - so exact-boundary
				// landings are the likely path, not the rare one). It reset
				// the file seek and sample[channel] but never the ADPCM
				// decoder's own differential state or the pending-byte
				// cache - confirmed on real hardware (2026-10-09): BGM
				// played correctly for the whole first loop (~82 seconds,
				// matching this track's real length) then degraded into
				// sustained near-full-scale static right at the loop
				// boundary and never recovered. Same "stale state corrupts
				// everything after" mechanism as the original odd-byte bug,
				// just hiding behind the OTHER reset path.
				this->file[channel]->Seek(0, TinyFile::TOP);
				this->sample[channel] = 0;
				if(this->type[channel] == 1)
				{
					this->pending_valid[channel] = false;
					this->adpcm_decorder[channel]->Clear();
				}
				if(this->loop[channel] == false)
				{
					Delete(channel);
				}
			}
		}
		else
		{
			// Memory data
			int mix_size = size;
			int left_size = this->size[channel] - this->sample[channel];
			if(left_size < size)
			{
				mix_size = left_size;
			}
			if(mix_size > 0)
			{
				MixBuffer(buffer, &this->buffer[channel][this->sample[channel]], mix_size);
				this->sample[channel] += mix_size;
				if(this->loop[channel] == true)
				{
					if(size > left_size)
					{
						int left_mix_size = size - left_size;
						if(left_mix_size > 0)
						{
							this->sample[channel] = 0;
							if(this->type[channel] == 1)
							{
								size_t read_size = left_mix_size / 2;
								unsigned char* mix_buffer = new unsigned char[left_mix_size];
								this->adpcm_decorder[channel]->Decord(mix_buffer, &this->buffer[channel][this->sample[channel] / 2], static_cast<int>(read_size));
								MixBuffer(&buffer[mix_size], mix_buffer, left_mix_size);
								delete [] mix_buffer;
							}
							else
							{
								MixBuffer(&buffer[mix_size], &this->buffer[channel][this->sample[channel]], left_mix_size);
							}
							this->sample[channel] += left_mix_size;
						}
					}
				}
			}
			if(this->sample[channel] >= this->size[channel])
			{
				this->sample[channel] = 0;
				if(this->loop[channel] == false)
				{
					Delete(channel);
				}
			}
		}
	}
}

void SoundMixer::Clear(void)
{
	for(int channel = 0; channel < CHANNEL_COUNT; ++ channel)
	{
		Delete(channel);
	}
}

void SoundMixer::MixBuffer(unsigned char* destination_buffer, const unsigned char* source_buffer, int size)
{
	for(int sample = 0; sample < size; ++ sample)
	{
		short data = static_cast<short>(source_buffer[sample]) + static_cast<short>(destination_buffer[sample]) - 128;
		data = ((data > 0) ? ((data < 255) ? data : 255) : 0);
		destination_buffer[sample] = static_cast<unsigned char>(data);
	}
}
