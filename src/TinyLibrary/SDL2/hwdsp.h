/* Lightweight in-process audio DSP stage - gain + simple shelf EQ applied to
 * the already-resampled 48kHz stereo S16 buffer, right before hwaudio_write().
 * Deliberately NOT a separate process: this device is single-core, and we
 * already found (audiotest, 2026-10-09) that extra scheduled entities
 * contending for the same core around the audio path cause real, measured
 * glitches. This runs inline on the same producer thread/call that already
 * builds the PCM buffer - zero extra threads, zero IPC, cheap per-sample
 * math only (two one-pole filters + a few multiplies).
 *
 * Tunable via a plain text file instead of a rebuild - see hwdsp_init(). */
#ifndef SF3000_HWDSP_H
#define SF3000_HWDSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Reads config_path (format below) if present; any missing/unparsable key
 * keeps its unity default, so a missing file is the same as all-unity (no
 * audible change) - safe to call even if the file was never pushed.
 *
 *   master_gain=1.0   # overall linear gain, 1.0 = unity
 *   bass_gain=1.0      # low-shelf gain below ~400Hz, 1.0 = unity
 *   treble_gain=1.0    # high-shelf gain above ~400Hz, 1.0 = unity
 *
 * Lines starting with '#' or that don't parse are ignored. */
void hwdsp_init(const char *config_path);

/* In-place process of an interleaved stereo S16 buffer, `frames` frames
 * (2 * frames shorts). Call once per batch, right before hwaudio_write(). */
void hwdsp_process(short *pcm, int frames);

#ifdef __cplusplus
}
#endif

#endif
