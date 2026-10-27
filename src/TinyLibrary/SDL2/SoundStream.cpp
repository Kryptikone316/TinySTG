#if defined(SDL2)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "SoundStream.hpp"
#include "hwaudio.h"
#include "hwdsp.h"

// Relative to the working directory: run.sh cd's into the port folder before
// launching, so this follows the port wherever it is installed.
#define DSP_CONFIG_PATH "hwdsp.txt"

static long long SoundStream_NowNs(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return static_cast<long long>(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
}

// Update()/Write() are driven by MainLoop_NoWaitLoop()'s busy-spin, so they
// can fire very often. A per-call heap alloc there (the original new[]/
// delete[] below) is real churn on an embedded allocator - intermittently
// slow enough to let hwaudio's ring buffer run dry and audibly glitch.
// Fixed-size static scratch buffers avoid that; MAX_IN_FRAMES is generous
// for any normal call (steady operation needs ~11025/call-rate, far below
// this), so hitting the cap only drops backlog after an actual stall -
// same "drop the oldest, don't grow unbounded" spirit as hwaudio's own ring.
static const int MAX_IN_FRAMES = 2048;
// audiotest (real-hardware run) found 216087 hwaudio_write() calls in 20
// seconds (avg 4.44 frames/call) - that call/mutex-lock rate measurably
// starved the SCHED_FIFO consumer thread's wake cadence (same single core,
// our busy-spin producer stealing cycles from it), and the ring filled to
// its (then-4096-frame) cap within ~8 seconds and stayed saturated - a
// real, confirmed overflow, not a guess. Batching input before resampling/
// pushing cuts the call rate by ~HOLD_THRESHOLD x (11025/128 =~ 86 calls/sec
// instead of ~10800/sec) without touching the drift-free accounting above,
// which only cares about cumulative totals, not how calls are chunked.
static const int HOLD_THRESHOLD = 128;
// g_scaled must hold a full MAX_IN_FRAMES catch-up batch ON TOP OF whatever
// was already sitting in the hold buffer just under HOLD_THRESHOLD - not
// just MAX_IN_FRAMES alone, or a post-stall call overflows it.
static unsigned char g_scaled[MAX_IN_FRAMES + HOLD_THRESHOLD];
// MAX_OUT_BATCH is BOTH the g_pcm[] allocation size AND how far
// out_frames_produced is allowed to fall behind out_target before the
// backlog is simply dropped (see the comment at its use below). These two
// numbers MUST be the same value - the five rounds of real-hardware testing
// that led here were exactly the failure of having them be separate,
// independently-guessed constants (a g_pcm sized from the input side's
// MAX_IN_FRAMES, a backlog cap sized from an arbitrary multiple of
// HWAUDIO_CHUNK) that were never actually related to what hwaudio.c's own
// ring could absorb. Deriving straight from HWAUDIO_RING_SOFT_CAP - the
// single shared constant in hwaudio.h - means this file can never again
// drift out of sync with the ring's real capacity.
static const int MAX_OUT_BATCH = HWAUDIO_RING_SOFT_CAP;
static short g_pcm[MAX_OUT_BATCH * 2];

SoundStream* SoundStream::sound_stream;

SoundStream::SoundStream(int frequency)
:frequency(frequency)
,writableSize(0)
,volume(0)
,start_time_ns(SoundStream_NowNs())
,frames_produced(0)
,out_frames_produced(0)
,prev_sample(0)
,hold_count(0)
{
	if(hwaudio_init() != 0)
	{
		throw "Error: hwaudio_init";
	}
	hwdsp_init(DSP_CONFIG_PATH);
}

SoundStream::~SoundStream(void)
{
	hwaudio_deinit();
}

void SoundStream::Initialize(int frequency)
{
	SoundStream::sound_stream = new SoundStream(frequency);
}

void SoundStream::Finalize(void)
{
	if(SoundStream::sound_stream != NULL)
	{
		delete SoundStream::sound_stream;
		SoundStream::sound_stream = NULL;
	}
}

SoundStream& SoundStream::GetInstance()
{
	return *SoundStream::sound_stream;
}

// MainLoop_NoWaitLoop() calls Sound::Update() -> here in a tight busy-spin
// (no sleep, unbounded call rate) - it's what paced the real SDL audio
// device via its own callback thread in the original backend. Without that,
// pacing has to come from real elapsed wall-clock time, not call count:
// exact integer rate conversion (no drift) against CLOCK_MONOTONIC.
void SoundStream::Update(void)
{
	long long elapsed_ns = SoundStream_NowNs() - this->start_time_ns;
	unsigned long long target = static_cast<unsigned long long>(elapsed_ns) * static_cast<unsigned long long>(this->frequency) / 1000000000ULL;
	unsigned long long owed = target - this->frames_produced;
	if(owed > static_cast<unsigned long long>(MAX_IN_FRAMES))
	{
		owed = static_cast<unsigned long long>(MAX_IN_FRAMES);
	}
	this->writableSize = static_cast<int>(owed);
	this->frames_produced += owed;
}

int SoundStream::GetWritableSize(void) const
{
	return this->writableSize;
}

bool SoundStream::Write(const unsigned char* buffer)
{
	int new_frames = this->writableSize;
	this->writableSize = 0;
	if(new_frames <= 0)
	{
		return true;
	}
	// volume (upstream's own shift table: 8 = unity, <8 attenuates, >8 boosts),
	// appended into the hold buffer rather than processed immediately - see
	// HOLD_THRESHOLD above for why.
	unsigned char* scaled = g_scaled;
	for(int i = 0; i < new_frames; ++ i)
	{
		int raw = static_cast<int>(buffer[i]);
		if(this->volume < 8)
		{
			raw >>= (8 - this->volume);
		}
		else if(this->volume > 8)
		{
			raw <<= (this->volume - 8);
			if(raw > 255) raw = 255;
		}
		scaled[this->hold_count + i] = static_cast<unsigned char>(raw);
	}
	this->hold_count += new_frames;
	if(this->hold_count < HOLD_THRESHOLD)
	{
		return true;
	}
	int frames = this->hold_count;
	this->hold_count = 0;

	// Linear resample mono U8 @ this->frequency -> stereo S16 @ HWAUDIO_RATE.
	// out_frames MUST be derived from the cumulative input/output totals, not
	// (frames * HWAUDIO_RATE) / frequency per call: that truncates every
	// single time (e.g. 1 input frame -> 4.35 ideal output frames, truncated
	// to 4, every call), and since Update()/Write() fire far more often than
	// the source rate (most calls see frames==1), this systematically lost
	// ~8% of output throughput forever - heard as constant, regularly-spaced
	// underrun clicks ("boom boom boom") once hwaudio's ring ran chronically
	// dry. Same drift-free trick as frames_produced above, one level further.
	unsigned long long out_target = this->frames_produced * static_cast<unsigned long long>(HWAUDIO_RATE) / static_cast<unsigned long long>(this->frequency);
	unsigned long long owed_out = out_target - this->out_frames_produced;
	if(owed_out > static_cast<unsigned long long>(MAX_OUT_BATCH))
	{
		// hwaudio_write() only advances out_frames_produced by what it
		// ACTUALLY accepted, so if the ring stays backed up across several
		// calls, this gap can grow without bound - out_target keeps climbing
		// off real wall-clock time regardless of whether output is reaching
		// the hardware. Snap forward and accept the loss, same "don't chase
		// an unpayable debt" rule as hwaudio_write()'s own soft cap. Now that
		// MAX_OUT_BATCH is sized directly from hwaudio.h's shared
		// HWAUDIO_RING_SOFT_CAP (not an independently-guessed constant),
		// this should only trigger on a genuine, large stall - not as
		// routine behavior every few calls.
		this->out_frames_produced = out_target - static_cast<unsigned long long>(MAX_OUT_BATCH);
		owed_out = static_cast<unsigned long long>(MAX_OUT_BATCH);
	}
	int out_frames = static_cast<int>(owed_out);
	if(out_frames > 0)
	{
		// Interpolate across [-1 .. frames-1], where index -1 is prev_sample
		// (the last sample emitted by the PREVIOUS call) - not [0 ..
		// frames-1] in isolation. Call batches are almost always tiny (often
		// 1 input frame), so resampling each call in isolation started every
		// batch flat at its own first sample with no ramp-in from where the
		// last call left off: a real amplitude step at every call boundary,
		// at roughly the 11025Hz source rate - audible as a fast, constantly
		// repeating tick ("baseball card on a bike spoke"), independent of
		// the separate throughput bug fixed above (that one caused slow,
		// periodic underrun clicks; this one is the resampler's own output
		// having no continuity between calls).
		short* pcm = g_pcm;
		int prev = this->prev_sample;
		for(int i = 0; i < out_frames; ++ i)
		{
			long long src_pos_fixed = static_cast<long long>(i) * frames;
			int idx0 = static_cast<int>(src_pos_fixed / out_frames) - 1;
			int frac = static_cast<int>(src_pos_fixed % out_frames);
			int s0 = (idx0 < 0) ? prev : (static_cast<int>(scaled[idx0]) - 128);
			int idx1 = idx0 + 1;
			int s1;
			if(idx1 < 0)
			{
				s1 = prev;
			}
			else if(idx1 < frames)
			{
				s1 = static_cast<int>(scaled[idx1]) - 128;
			}
			else
			{
				s1 = static_cast<int>(scaled[frames - 1]) - 128;
			}
			int blended = s0 + ((s1 - s0) * frac) / out_frames;
			short pcm16 = static_cast<short>(blended * 256);
			pcm[i * 2] = pcm16;
			pcm[i * 2 + 1] = pcm16;
		}
		hwdsp_process(pcm, out_frames);
		// hwaudio_write() may silently accept fewer than requested (its own
		// soft-cap backpressure) - advance by what it ACTUALLY took, not
		// the requested count, or dropped frames permanently corrupt this
		// drift-free counter (found 2026-10-09: the old void-returning
		// version let this silently drift, reintroducing a shortfall).
		this->out_frames_produced += static_cast<unsigned long long>(hwaudio_write(pcm, out_frames));
	}
	this->prev_sample = static_cast<int>(scaled[frames - 1]) - 128;
	return true;
}

// volume = 0-10
void SoundStream::SetVolume(int volume)
{
	// Global.cpp's default-constructor value for this build's #else branch
	// (neither ADAFRUIT nor _WIN32) is 10 - two steps above MAX_VOLUME (8,
	// the documented "unity" point the in-game volume UI itself already
	// clamps to). That 2-step excess drives Write()'s own gain math
	// (raw <<= (volume-8)) into a hard 4x pre-resample amplification,
	// clamped at 255 - real, severe clipping from the very first frame of
	// every session, confirmed on real hardware (2026-10-09): every single
	// write in a full gameplay capture peaked at or past full scale (max
	// peak 32768/32767, the digital clipping ceiling). Clamping here
	// enforces the same MAX_VOLUME ceiling the UI already assumes is
	// absolute everywhere else in this codebase, for the one code path
	// (the untouched default) that could still exceed it.
	if(volume > SoundStream::MAX_VOLUME)
	{
		volume = SoundStream::MAX_VOLUME;
	}
	else if(volume < 0)
	{
		volume = 0;
	}
	this->volume = volume;
}

int SoundStream::GetVolume(void)
{
	return this->volume;
}

#endif
