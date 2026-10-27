#ifndef TINYSOUNDSTREAM_HPP
#define TINYSOUNDSTREAM_HPP

class SoundStream
{
public:
	enum
	{
		MAX_VOLUME = 8
	};
	SoundStream(int frequency);
	~SoundStream(void);
	static SoundStream* sound_stream;
	static SoundStream& GetInstance();
	static void Initialize(int frequency);
	static void Finalize(void);
	void Update(void);
	int GetWritableSize(void) const;
	bool Write(const unsigned char* buffer);
	void SetVolume(int volume);
	int GetVolume(void);
private:
	int frequency;
	int writableSize;
	int volume;
	long long start_time_ns;
	unsigned long long frames_produced;
	unsigned long long out_frames_produced;
	int prev_sample;
	int hold_count;
	SoundStream(SoundStream&);
	SoundStream& operator = (SoundStream&);
};

#endif
