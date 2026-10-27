#if defined(SDL2)

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#if defined(_WIN64)
#pragma comment(lib, "../../lib/sdl2/sdl2/lib/x64/SDL2.lib")
#pragma comment(lib, "../../lib/sdl2/sdl2/lib/x64/SDL2main.lib")
#pragma comment(lib, "../../lib/sdl2/sdl2_image/lib/x64/SDL2_image.lib")
#else
#pragma comment(lib, "../../lib/sdl2/sdl2/lib/x86/SDL2.lib")
#pragma comment(lib, "../../lib/sdl2/sdl2/lib/x86/SDL2main.lib")
#pragma comment(lib, "../../lib/sdl2/sdl2_image/lib/x86/SDL2_image.lib")
#endif
#include "../Mathmatics.hpp"
#include "Screen.hpp"
#include "hwdisp.h"

#if defined(USE_COLOR256)
#include "../Font6x8.h"
#include "../Font4x6.h"
#elif defined(USE_COLOR65536)
#include "../Font6x8_65536.h"
#include "../Font4x6_65536.h"
#endif

Screen* Screen::screen = NULL;

int Screen::WIDTH = CONFIG_WIDTH;
int Screen::HEIGHT = CONFIG_HEIGHT;

const unsigned int Screen::color_table[256] =
{
	0xFF000000, 0xFF550000, 0xFFAA0000, 0xFFFF0000, 0xFF002400, 0xFF552400, 0xFFAA2400, 0xFFFF2400, 0xFF004900, 0xFF554900, 0xFFAA4900, 0xFFFF4900, 0xFF006D00, 0xFF556D00, 0xFFAA6D00, 0xFFFF6D00,
	0xFF009200, 0xFF559200, 0xFFAA9200, 0xFFFF9200, 0xFF00B600, 0xFF55B600, 0xFFAAB600, 0xFFFFB600, 0xFF00DB00, 0xFF55DB00, 0xFFAADB00, 0xFFFFDB00, 0xFF00FF00, 0xFF55FF00, 0xFFAAFF00, 0xFFFFFF00,
	0xFF000024, 0xFF550024, 0xFFAA0024, 0xFFFF0024, 0xFF002424, 0xFF552424, 0xFFAA2424, 0xFFFF2424, 0xFF004924, 0xFF554924, 0xFFAA4924, 0xFFFF4924, 0xFF006D24, 0xFF556D24, 0xFFAA6D24, 0xFFFF6D24,
	0xFF009224, 0xFF559224, 0xFFAA9224, 0xFFFF9224, 0xFF00B624, 0xFF55B624, 0xFFAAB624, 0xFFFFB624, 0xFF00DB24, 0xFF55DB24, 0xFFAADB24, 0xFFFFDB24, 0xFF00FF24, 0xFF55FF24, 0xFFAAFF24, 0xFFFFFF24,
	0xFF000049, 0xFF550049, 0xFFAA0049, 0xFFFF0049, 0xFF002449, 0xFF552449, 0xFFAA2449, 0xFFFF2449, 0xFF004949, 0xFF554949, 0xFFAA4949, 0xFFFF4949, 0xFF006D49, 0xFF556D49, 0xFFAA6D49, 0xFFFF6D49,
	0xFF009249, 0xFF559249, 0xFFAA9249, 0xFFFF9249, 0xFF00B649, 0xFF55B649, 0xFFAAB649, 0xFFFFB649, 0xFF00DB49, 0xFF55DB49, 0xFFAADB49, 0xFFFFDB49, 0xFF00FF49, 0xFF55FF49, 0xFFAAFF49, 0xFFFFFF49,
	0xFF00006D, 0xFF55006D, 0xFFAA006D, 0xFFFF006D, 0xFF00246D, 0xFF55246D, 0xFFAA246D, 0xFFFF246D, 0xFF00496D, 0xFF55496D, 0xFFAA496D, 0xFFFF496D, 0xFF006D6D, 0xFF556D6D, 0xFFAA6D6D, 0xFFFF6D6D,
	0xFF00926D, 0xFF55926D, 0xFFAA926D, 0xFFFF926D, 0xFF00B66D, 0xFF55B66D, 0xFFAAB66D, 0xFFFFB66D, 0xFF00DB6D, 0xFF55DB6D, 0xFFAADB6D, 0xFFFFDB6D, 0xFF00FF6D, 0xFF55FF6D, 0xFFAAFF6D, 0xFFFFFF6D,
	0xFF000092, 0xFF550092, 0xFFAA0092, 0xFFFF0092, 0xFF002492, 0xFF552492, 0xFFAA2492, 0xFFFF2492, 0xFF004992, 0xFF554992, 0xFFAA4992, 0xFFFF4992, 0xFF006D92, 0xFF556D92, 0xFFAA6D92, 0xFFFF6D92,
	0xFF009292, 0xFF559292, 0xFFAA9292, 0xFFFF9292, 0xFF00B692, 0xFF55B692, 0xFFAAB692, 0xFFFFB692, 0xFF00DB92, 0xFF55DB92, 0xFFAADB92, 0xFFFFDB92, 0xFF00FF92, 0xFF55FF92, 0xFFAAFF92, 0xFFFFFF92,
	0xFF0000B6, 0xFF5500B6, 0xFFAA00B6, 0xFFFF00B6, 0xFF0024B6, 0xFF5524B6, 0xFFAA24B6, 0xFFFF24B6, 0xFF0049B6, 0xFF5549B6, 0xFFAA49B6, 0xFFFF49B6, 0xFF006DB6, 0xFF556DB6, 0xFFAA6DB6, 0xFFFF6DB6,
	0xFF0092B6, 0xFF5592B6, 0xFFAA92B6, 0xFFFF92B6, 0xFF00B6B6, 0xFF55B6B6, 0xFFAAB6B6, 0xFFFFB6B6, 0xFF00DBB6, 0xFF55DBB6, 0xFFAADBB6, 0xFFFFDBB6, 0xFF00FFB6, 0xFF55FFB6, 0xFFAAFFB6, 0xFFFFFFB6,
	0xFF0000DB, 0xFF5500DB, 0xFFAA00DB, 0xFFFF00DB, 0xFF0024DB, 0xFF5524DB, 0xFFAA24DB, 0xFFFF24DB, 0xFF0049DB, 0xFF5549DB, 0xFFAA49DB, 0xFFFF49DB, 0xFF006DDB, 0xFF556DDB, 0xFFAA6DDB, 0xFFFF6DDB,
	0xFF0092DB, 0xFF5592DB, 0xFFAA92DB, 0xFFFF92DB, 0xFF00B6DB, 0xFF55B6DB, 0xFFAAB6DB, 0xFFFFB6DB, 0xFF00DBDB, 0xFF55DBDB, 0xFFAADBDB, 0xFFFFDBDB, 0xFF00FFDB, 0xFF55FFDB, 0xFFAAFFDB, 0xFFFFFFDB,
	0xFF0000FF, 0xFF5500FF, 0xFFAA00FF, 0xFFFF00FF, 0xFF0024FF, 0xFF5524FF, 0xFFAA24FF, 0xFFFF24FF, 0xFF0049FF, 0xFF5549FF, 0xFFAA49FF, 0xFFFF49FF, 0xFF006DFF, 0xFF556DFF, 0xFFAA6DFF, 0xFFFF6DFF,
	0xFF0092FF, 0xFF5592FF, 0xFFAA92FF, 0xFFFF92FF, 0xFF00B6FF, 0xFF55B6FF, 0xFFAAB6FF, 0xFFFFB6FF, 0xFF00DBFF, 0xFF55DBFF, 0xFFAADBFF, 0xFFFFDBFF, 0xFF00FFFF, 0xFF55FFFF, 0xFFAAFFFF, 0xFFFFFFFF
};

Screen::Screen(int color_mode)
:color_mode(color_mode)
{
	// screen_buffer������
	this->buffer_size = Screen::WIDTH * Screen::HEIGHT;
	if (color_mode == 16)
	{
		this->buffer_size *= 2;
	}
	this->screen_buffer = new unsigned char[this->buffer_size];
	memset(this->screen_buffer, 0, this->buffer_size);
	// Upscaled frame handed to the driver; size and fit come from video.txt.
	this->hw_frame_buffer = NULL;
	this->hw_col_map = NULL;
	this->hw_row_map = NULL;
	this->hw_src_row = NULL;
	this->hw_col_edge = NULL;
	this->hw_row_edge = NULL;
	this->hw_frames = 0;
	this->hw_compose_ns = 0;
	this->hw_present_ns = 0;
	this->hw_present_max_ns = 0;
	this->LoadVideoConfig();
	if(hwdisp_init() != 0)
	{
		throw "Error: hwdisp_init";
	}
	// The driver's own fixed internal canvas is 640x480 (4:3). A frame that is
	// not 4:3 (for example a video.txt with 320x256) would be stretched to it,
	// so hwdisp pads narrower frames to 4:3 first. This is a no-op for the
	// default 320x240 frame.
	hwdisp_set_target_aspect(4, 3);
}

Screen::~Screen(void)
{
	if(this->screen_buffer != NULL)
	{
		delete [] this->screen_buffer;
		this->screen_buffer = NULL;
	}
	if(this->hw_frame_buffer != NULL)
	{
		delete [] this->hw_frame_buffer;
		this->hw_frame_buffer = NULL;
	}
	delete [] this->hw_col_map;
	delete [] this->hw_row_map;
	delete [] this->hw_src_row;
	delete [] this->hw_col_edge;
	delete [] this->hw_row_edge;
	this->hw_col_edge = NULL;
	this->hw_row_edge = NULL;
	this->hw_col_map = NULL;
	this->hw_row_map = NULL;
	this->hw_src_row = NULL;
	hwdisp_deinit();
}

void Screen::Initialize(int color_mode)
{
	Screen::screen = new Screen(color_mode);
}

void Screen::Finalize(void)
{
	if(Screen::screen != NULL)
	{
		delete Screen::screen;
		Screen::screen = NULL;
	}
}

Screen& Screen::GetInstance()
{
	return *Screen::screen;
}

void Screen::Clear(unsigned int color)
{
	if((this->color_mode == 8) || (color == 0))
	{
		memset(this->screen_buffer, color, this->buffer_size);
	}
	else
	{
		unsigned short* screen_buffer16 = reinterpret_cast<unsigned short*>(this->screen_buffer);
		unsigned short color16 = static_cast<unsigned short>(color);
		for(int i = 0; i < Screen::WIDTH * Screen::HEIGHT; ++ i)
		{
			screen_buffer16[i] = color16;
		}
	}
}

void Screen::DrawBegin(void)
{
}

static long long screen_now_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return static_cast<long long>(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
}

// video.txt (in the port folder), one "key=value" per line:
//   out_w, out_h   size of the frame given to the driver (default 320x240)
//   fit            stretch: fill the whole frame, pixels may be uneven by a
//                           fraction of a pixel (default)
//                  integer: largest whole-number scale, centred, black borders
// Examples: 320x240 stretch (default, cheap), 640x480 integer (7x, crisp).
void Screen::LoadVideoConfig(void)
{
	int out_w = 320;
	int out_h = 240;
	bool integer_fit = false;
	float contrast = 1.15f;
	float saturation = 1.25f;
	float gamma = 1.0f;
	int scanlines = 0;
	int grid = 0;
	FILE* f = fopen("video.txt", "r");
	if(f != NULL)
	{
		char line[128];
		while(fgets(line, sizeof(line), f) != NULL)
		{
			int v = 0;
			float fv = 0.0f;
			char word[32];
			if(sscanf(line, "out_w=%d", &v) == 1) { out_w = v; }
			else if(sscanf(line, "out_h=%d", &v) == 1) { out_h = v; }
			else if(sscanf(line, "contrast=%f", &fv) == 1) { contrast = fv; }
			else if(sscanf(line, "saturation=%f", &fv) == 1) { saturation = fv; }
			else if(sscanf(line, "gamma=%f", &fv) == 1) { gamma = fv; }
			else if(sscanf(line, "scanlines=%d", &v) == 1) { scanlines = v; }
			else if(sscanf(line, "grid=%d", &v) == 1) { grid = v; }
			else if(sscanf(line, "fit=%31s", word) == 1) { integer_fit = (strcmp(word, "integer") == 0); }
		}
		fclose(f);
	}
	if(out_w < Screen::WIDTH || out_w > 1280) { out_w = 320; }
	if(out_h < Screen::HEIGHT || out_h > 720) { out_h = 240; }
	this->hw_out_w = out_w;
	this->hw_out_h = out_h;
	this->hw_frame_buffer = new unsigned short[static_cast<size_t>(out_w) * out_h];
	memset(this->hw_frame_buffer, 0, static_cast<size_t>(out_w) * out_h * sizeof(unsigned short));
	this->hw_col_map = new int[out_w];
	this->hw_row_map = new int[out_h];
	this->hw_src_row = new unsigned short[Screen::WIDTH];
	this->hw_col_edge = new unsigned char[out_w];
	this->hw_row_edge = new unsigned char[out_h];
	int scale = 1;
	int off_x = 0;
	int off_y = 0;
	int used_w = out_w;
	int used_h = out_h;
	if(integer_fit)
	{
		int sx = out_w / Screen::WIDTH;
		int sy = out_h / Screen::HEIGHT;
		scale = (sx < sy) ? sx : sy;
		if(scale < 1) { scale = 1; }
		used_w = Screen::WIDTH * scale;
		used_h = Screen::HEIGHT * scale;
		off_x = (out_w - used_w) / 2;
		off_y = (out_h - used_h) / 2;
	}
	for(int x = 0; x < out_w; ++ x)
	{
		int ux = x - off_x;
		this->hw_col_map[x] = (ux < 0 || ux >= used_w) ? -1 : ux * Screen::WIDTH / used_w;
	}
	for(int y = 0; y < out_h; ++ y)
	{
		int uy = y - off_y;
		this->hw_row_map[y] = (uy < 0 || uy >= used_h) ? -1 : uy * Screen::HEIGHT / used_h;
	}
	if(scanlines < 0 || scanlines > 80) { scanlines = 0; }
	if(grid < 0 || grid > 80) { grid = 0; }
	// Last output column/row of each game pixel: darkened for the grid / scanline effects.
	for(int x = 0; x < out_w; ++ x)
	{
		const bool edge = (this->hw_col_map[x] >= 0) && (x + 1 >= out_w || this->hw_col_map[x + 1] != this->hw_col_map[x]);
		this->hw_col_edge[x] = (grid > 0 && edge) ? 1 : 0;
	}
	for(int y = 0; y < out_h; ++ y)
	{
		const bool edge = (this->hw_row_map[y] >= 0) && (y + 1 >= out_h || this->hw_row_map[y + 1] != this->hw_row_map[y]);
		this->hw_row_edge[y] = (scanlines > 0 && edge) ? 1 : 0;
	}
	const float row_factor = 1.0f - scanlines / 100.0f;
	const float col_factor = 1.0f - grid / 100.0f;
	if(contrast < 0.5f || contrast > 2.0f) { contrast = 1.0f; }
	if(saturation < 0.0f || saturation > 3.0f) { saturation = 1.0f; }
	if(gamma < 0.4f || gamma > 2.5f) { gamma = 1.0f; }
	// Palette tweak, built once: the game's 256-colour palette is dull on this
	// panel, so push saturation and contrast (and optionally gamma) per entry.
	for(int i = 0; i < 256; ++ i)
	{
		unsigned int color32 = Screen::color_table[i];
		float c[3];
		c[0] = ((color32 >> 16) & 0xFF) / 255.0f;
		c[1] = ((color32 >> 8) & 0xFF) / 255.0f;
		c[2] = (color32 & 0xFF) / 255.0f;
		const float luma = 0.299f * c[0] + 0.587f * c[1] + 0.114f * c[2];
		float fin[3];
		for(int k = 0; k < 3; ++ k)
		{
			float v = luma + (c[k] - luma) * saturation;
			v = (v - 0.5f) * contrast + 0.5f;
			if(v < 0.0f) { v = 0.0f; }
			if(v > 1.0f) { v = 1.0f; }
			if(gamma != 1.0f) { v = powf(v, 1.0f / gamma); }
			fin[k] = v;
		}
		for(int vv = 0; vv < 4; ++ vv)
		{
			float fac = 1.0f;
			if(vv & 2) { fac *= row_factor; }
			if(vv & 1) { fac *= col_factor; }
			const unsigned int r8 = static_cast<unsigned int>(fin[0] * fac * 255.0f + 0.5f);
			const unsigned int g8 = static_cast<unsigned int>(fin[1] * fac * 255.0f + 0.5f);
			const unsigned int b8 = static_cast<unsigned int>(fin[2] * fac * 255.0f + 0.5f);
			this->hw_lut4[vv][i] = static_cast<unsigned short>(((r8 >> 3) << 11) | ((g8 >> 2) << 5) | (b8 >> 3));
		}
		this->hw_lut[i] = this->hw_lut4[0][i];
	}
	fprintf(stderr, "tinystg video: game %dx%d -> frame %dx%d, fit=%s, contrast %.2f, saturation %.2f, gamma %.2f, scanlines %d%%, grid %d%%\n", Screen::WIDTH, Screen::HEIGHT, out_w, out_h,
		integer_fit ? "integer" : "stretch", contrast, saturation, gamma, scanlines, grid);
}

void Screen::DrawEnd(void)
{
	const long long t0 = screen_now_ns();
	const int ow = this->hw_out_w;
	const int oh = this->hw_out_h;
	int prev_sy = -1;
	int prev_re = -1;
	for(int y = 0; y < oh; ++ y)
	{
		const int sy = this->hw_row_map[y];
		if(sy < 0)
		{
			prev_sy = -1;
			continue;
		}
		const int re = this->hw_row_edge[y];
		unsigned short* dst = &this->hw_frame_buffer[static_cast<size_t>(y) * ow];
		if(sy == prev_sy && re == prev_re)
		{
			memcpy(dst, dst - ow, static_cast<size_t>(ow) * sizeof(unsigned short));
			continue;
		}
		prev_sy = sy;
		prev_re = re;
		if(this->color_mode == 8)
		{
			const unsigned char* src = &this->screen_buffer[sy * Screen::WIDTH];
			const unsigned short* lut_n = this->hw_lut4[re * 2];
			const unsigned short* lut_e = this->hw_lut4[re * 2 + 1];
			for(int x = 0; x < ow; ++ x)
			{
				const int m = this->hw_col_map[x];
				dst[x] = (m < 0) ? 0 : (this->hw_col_edge[x] ? lut_e[src[m]] : lut_n[src[m]]);
			}
		}
		else
		{
			const unsigned short* src16 = reinterpret_cast<unsigned short*>(this->screen_buffer) + sy * Screen::WIDTH;
			for(int x = 0; x < Screen::WIDTH; ++ x)
			{
				this->hw_src_row[x] = Color16To565(src16[x]);
			}
			for(int x = 0; x < ow; ++ x)
			{
				const int m = this->hw_col_map[x];
				dst[x] = (m < 0) ? 0 : this->hw_src_row[m];
			}
		}
	}
	const long long t1 = screen_now_ns();
	hwdisp_present(this->hw_frame_buffer, ow, oh, ow * 2);
	const long long t2 = screen_now_ns();
	this->hw_compose_ns += t1 - t0;
	this->hw_present_ns += t2 - t1;
	if(t2 - t1 > this->hw_present_max_ns) { this->hw_present_max_ns = t2 - t1; }
	if(++ this->hw_frames >= 120)
	{
		fprintf(stderr, "tinystg video: 120 frames, compose avg %.2f ms, present avg %.2f ms (max %.2f ms)\n",
			this->hw_compose_ns / 120.0 / 1e6, this->hw_present_ns / 120.0 / 1e6, this->hw_present_max_ns / 1e6);
		this->hw_frames = 0;
		this->hw_compose_ns = 0;
		this->hw_present_ns = 0;
		this->hw_present_max_ns = 0;
	}
}

void Screen::DrawSprite(const void* buffer, int x, int y, int width, int height, int color_key)
{
	if(x + width <= 0)
	{
		return;
	}
	if(y + height <= 0)
	{
		return;
	}
	if(x >= Screen::WIDTH)
	{
		return;
	}
	if(y >= Screen::HEIGHT)
	{
		return;
	}
	int clip_width = width;
	int clip_height = height;
	const char* clip_buffer = reinterpret_cast<const char*>(buffer);
	if(x > Screen::WIDTH - width)
	{
		clip_width = width - (x - (Screen::WIDTH - width));
	}
	if (x < 0)
	{
		clip_buffer -= x;
		clip_width += x;
		x = 0;
	}
	if(y > Screen::HEIGHT - height)
	{
		clip_height = height - (y - (Screen::HEIGHT - height));
	}
	if(y < 0)
	{
		clip_buffer -= (static_cast<size_t>(y) * static_cast<size_t>(width));
		clip_height += y;
		y = 0;
	}
	DrawSpriteNoClip(clip_buffer, x, y, clip_width, clip_height, color_key, width);
}

void Screen::DrawSprite(const void* buffer, int x, int y, int width, int height, int source_x, int source_y, int source_width, int color_key)
{
	if(x + width <= 0)
	{
		return;
	}
	if(y + height <= 0)
	{
		return;
	}
	if(x >= Screen::WIDTH)
	{
		return;
	}
	if(y >= Screen::HEIGHT)
	{
		return;
	}
	int clip_width = width;
	int clip_height = height;
	const char* clip_buffer = reinterpret_cast<const char*>(buffer) + static_cast<size_t>(source_y) * static_cast<size_t>(source_width) + static_cast<size_t>(source_x);
	if(x > Screen::WIDTH - width)
	{
		clip_width = width - (x - (Screen::WIDTH - width));
	}
	if (x < 0)
	{
		clip_buffer -= x;
		clip_width += x;
		x = 0;
	}
	if(y > Screen::HEIGHT - height)
	{
		clip_height = height - (y - (Screen::HEIGHT - height));
	}
	if (y < 0)
	{
		clip_buffer -= (static_cast<size_t>(y) * static_cast<size_t>(source_width));
		clip_height += y;
		y = 0;
	}
	DrawSpriteNoClip(clip_buffer, x, y, clip_width, clip_height, color_key, source_width);
}

void Screen::DrawSprite(int destination_x, int destination_y, int destination_left, int destination_top, int destination_right, int destination_bottom, const void* source_buffer, int source_x, int source_y, int source_width, int source_height, int color_key, int rotate, int scale)
{
	rotate = -rotate;
	scale >>= 1;
	if(scale <= 0)
	{
		return;
	}
	int sin = Mathmatics::Sin8(rotate);
	int cos = Mathmatics::Cos8(rotate);
	int x;
	int y;
	int xx;
	int yy;
	int pixel_x;
	int pixel_y;
	if(this->color_mode == 8)
	{
		const unsigned char* image = reinterpret_cast<const unsigned char*>(source_buffer);
		for(y = destination_top; y <= destination_bottom; ++ y)
		{
			yy = y - destination_y;
			for(x = destination_left; x <= destination_right; ++ x)
			{
				unsigned char pixel = 0;
				xx = x - destination_x;
				pixel_x = source_x + (xx * cos - yy * sin + (1 << 6)) / scale;
				pixel_y = source_y + (xx * sin + yy * cos + (1 << 6)) / scale;
				if((pixel_x >= 0) && (pixel_y >= 0) && (pixel_x < source_width) && (pixel_y < source_height))
				{
					pixel = image[pixel_y * source_width + pixel_x];
				}
				if(pixel != color_key)
				{
					this->screen_buffer[Screen::WIDTH * y + x] = pixel;
				}
			}
		}
	}
	else
	{
		const unsigned short* image = reinterpret_cast<const unsigned short*>(source_buffer);
		unsigned short* screen_buffer16 = reinterpret_cast<unsigned short*>(this->screen_buffer);
		for(y = destination_top; y <= destination_bottom; ++ y)
		{
			yy = y - destination_y;
			for(x = destination_left; x <= destination_right; ++ x)
			{
				unsigned short pixel = 0;
				xx = x - destination_x;
				pixel_x = source_x + (xx * cos - yy * sin + (1 << 6)) / scale;
				pixel_y = source_y + (xx * sin + yy * cos + (1 << 6)) / scale;
				if((pixel_x >= 0) && (pixel_y >= 0) && (pixel_x < source_width) && (pixel_y < source_height))
				{
					pixel = image[pixel_y * source_width + pixel_x];
				}
				if(pixel != color_key)
				{
					screen_buffer16[Screen::WIDTH * y + x] = pixel;
				}
			}
		}
	}
}

void Screen::DrawSpriteFrame(const void* buffer, int x, int y, int width, int height, int source_width, int source_height, int color_key)
{
	if((width == source_width) && (height == source_height))
	{
		DrawSprite(buffer, x, y, width, height, color_key);
		return;
	}
	int width_half;
	if(width > source_width)
	{
		width_half = source_width / 2;
	}
	else
	{
		width_half = width / 2;
	}
	int height_half;
	if(height > source_height)
	{
		height_half = source_height / 2;
	}
	else
	{
		height_half = height / 2;
	}
	// back color
	unsigned int color;
	if(this->color_mode == 8)
	{
		const unsigned char* buffer8 = reinterpret_cast<const unsigned char*>(buffer);
		color = static_cast<unsigned int>(buffer8[source_width * (y / 2) + (x / 2)]);
	}
	else
	{
		const unsigned short* buffer16 = reinterpret_cast<const unsigned short*>(buffer);
		color = static_cast<unsigned int>(buffer16[source_width * (y / 2) + (x / 2)]);
	}
	DrawRectangle(x, y, width, height, color);
	// left-top
	int draw_width = width_half;
	int draw_height = height_half;
	int draw_x = x;
	int draw_y = y;
	int source_x = 0;
	int source_y = 0;
	DrawSprite(buffer, draw_x, draw_y, draw_width, draw_height, source_x, source_y, source_width, color_key);
	// right-top
	draw_width = width - width_half;
	draw_height = height_half;
	draw_x = x + width - draw_width;
	draw_y = y;
	source_x = source_width - draw_width;
	source_y = 0;
	DrawSprite(buffer, draw_x, draw_y, draw_width, draw_height, source_x, source_y, source_width, color_key);
	// left-bottom
	draw_width = width_half;
	draw_height = height - height_half;
	draw_x = x;
	draw_y = y + height - draw_height;
	source_x = 0;
	source_y = source_height - draw_height;
	DrawSprite(buffer, draw_x, draw_y, draw_width, draw_height, source_x, source_y, source_width, color_key);
	// right-bottom
	draw_width = width - width_half;
	draw_height = height - height_half;
	draw_x = x + width - draw_width;
	draw_y = y + height - draw_height;
	source_x = source_width - draw_width;
	source_y = source_height - draw_height;
	DrawSprite(buffer, draw_x, draw_y, draw_width, draw_height, source_x, source_y, source_width, color_key);
}

void Screen::DrawFont(int x, int y, const char* text, ...)
{
	char* output_text = reinterpret_cast<char*>(malloc(128));
	va_list arglist;
	va_start(arglist, text);
#if defined(_WIN32)
	vsprintf_s(output_text, 128, text, arglist);
#else
	vsprintf(output_text, text, arglist);
#endif
	va_end(arglist);
	unsigned char* output_text_temp = reinterpret_cast<unsigned char*>(output_text);
	const void* font = NULL;
	if(this->color_mode == 8)
	{
#if defined(USE_COLOR256)
		font = Font6x8;
#endif
	}
	else
	{
#if defined(USE_COLOR65536)
		font = Font6x8_65536;
#endif
	}
	if(font == NULL)
	{
		free(output_text);
		return;
	}
	if(x == CENTER)
	{
		x = ((Screen::WIDTH - 6 * static_cast<int>(strlen(output_text))) >> 1);
	}
	else if(x == LEFT)
	{
		x = 0;
	}
	else if(x == RIGHT)
	{
		x = Screen::WIDTH - 6 * static_cast<int>(strlen(output_text));
	}
	if(y == CENTER)
	{
		y = ((Screen::HEIGHT - 8) >> 1);
	}
	else if(y == TOP)
	{
		y = 0;
	}
	else if(x == BOTTOM)
	{
		y = Screen::HEIGHT - 8;
	}
	while(*output_text_temp != 0)
	{
		int index = static_cast<int>(static_cast<unsigned char>(*output_text_temp)) - 0x20;
		if(index >= 0x80)
		{
			index -= 0x20;
		}
		int source_x = (index % 16) * 6;
		int source_y = (index / 16) * 8;
		DrawSprite(font, x, y, 6, 8, source_x, source_y, 96, 0);
		x += 6;
		++ output_text_temp;
	}
	free(output_text);
}

void Screen::DrawJapaneseFont(int x, int y, const char* text)
{
	char* text_temp = const_cast<char*>(text);
	const void* font = NULL;
	if(this->color_mode == 8)
	{
#if defined(USE_COLOR256)
		font = Font6x8;
#endif
	}
	else
	{
#if defined(USE_COLOR65536)
		font = Font6x8_65536;
#endif
	}
	if(font == NULL)
	{
		return;
	}
	if(x == CENTER)
	{
		x = ((Screen::WIDTH - 6 * static_cast<int>(strlen(text))) >> 1);
	}
	else if(x == LEFT)
	{
		x = 0;
	}
	else if(x == RIGHT)
	{
		x = Screen::WIDTH - 6 * static_cast<int>(strlen(text));
	}
	if(y == CENTER)
	{
		y = ((Screen::HEIGHT - 8) >> 1);
	}
	else if(y == TOP)
	{
		y = 0;
	}
	else if(x == BOTTOM)
	{
		y = Screen::HEIGHT - 8;
	}
	while(*text_temp != 0)
	{
		int index = static_cast<int>(static_cast<unsigned char>(*text_temp)) - 0x20;
		if(index >= 0x80)
		{
			index -= 0x20;
		}
		int source_x = (index % 16) * 6;
		int source_y = (index / 16) * 8;
		DrawSprite(font, x, y, 6, 8, source_x, source_y, 96, 0);
		x += 6;
		++ text_temp;
	}
}

void Screen::DrawNumberFont(int x, int y, const char* text, ...)
{
	char* output_text = reinterpret_cast<char*>(malloc(128));
	va_list arglist;
	va_start(arglist, text);
#if defined(_WIN32)
	vsprintf_s(output_text, 128, text, arglist);
#else
	vsprintf(output_text, text, arglist);
#endif
	va_end(arglist);
	char* output_text_temp = output_text;
	const void* font = NULL;
	if(this->color_mode == 8)
	{
#if defined(USE_COLOR256)
		font = Font4x6;
#endif
	}
	else
	{
#if defined(USE_COLOR65536)
		font = Font4x6_65536;
#endif
	}
	if(font == NULL)
	{
		free(output_text);
		return;
	}
	if(x == CENTER)
	{
		x = ((Screen::WIDTH - 4 * static_cast<int>(strlen(output_text))) >> 1);
	}
	else if(x == LEFT)
	{
		x = 0;
	}
	else if(x == RIGHT)
	{
		x = Screen::WIDTH - 4 * static_cast<int>(strlen(output_text));
	}
	if(y == CENTER)
	{
		y = ((Screen::HEIGHT - 6) >> 1);
	}
	else if(y == TOP)
	{
		y = 0;
	}
	else if(x == BOTTOM)
	{
		y = Screen::HEIGHT - 6;
	}
	while(*output_text_temp != 0)
	{
		if(*output_text_temp != ' ')
		{
			int source_x = (static_cast<int>(*output_text_temp - 0x20) % 8) * 4;
			int source_y = (static_cast<int>(*output_text_temp - 0x20) / 8) * 6;
			DrawSprite(font, x, y, 4, 5, source_x, source_y, 32, 0);
		}
		x += 4;
		++ output_text_temp;
	}
	free(output_text);
}

void Screen::DrawRectangle(int x, int y, int width, int height, unsigned int color)
{
	int destination_address = Screen::WIDTH * y + x;
	if(this->color_mode == 8)
	{
		for(int scan_y = 0; scan_y < height; ++ scan_y)
		{
			for(int scan_x = 0; scan_x < width; ++ scan_x)
			{
				this->screen_buffer[destination_address + scan_x] = static_cast<unsigned char>(color);
			}
			destination_address += Screen::WIDTH;
		}
	}
	else
	{
		unsigned short* screen_buffer16 = reinterpret_cast<unsigned short*>(this->screen_buffer);
		for(int scan_y = 0; scan_y < height; ++ scan_y)
		{
			for(int scan_x = 0; scan_x < width; ++ scan_x)
			{
				screen_buffer16[destination_address + scan_x] = static_cast<unsigned short>(color);
			}
			destination_address += Screen::WIDTH;
		}
	}
}

void Screen::DrawSpriteNoClip(const void* buffer, int x, int y, int width, int height, int color_key, int source_width)
{
	int source_address = 0;
	int destination_address = Screen::WIDTH * y + x;
	if(this->color_mode == 8)
	{
		const unsigned char* buffer8 = reinterpret_cast<const unsigned char*>(buffer);
		for(int scan_y = 0; scan_y < height; ++ scan_y)
		{
			for(int scan_x = 0; scan_x < width; ++ scan_x)
			{
				unsigned char color = buffer8[source_address + scan_x];
				if(static_cast<int>(color) != color_key)
				{
					this->screen_buffer[destination_address + scan_x] = color;
				}
			}
			source_address += source_width;
			destination_address += Screen::WIDTH;
		}
	}
	else
	{
		const unsigned short* buffer16 = reinterpret_cast<const unsigned short*>(buffer);
		unsigned short* screen_buffer16 = reinterpret_cast<unsigned short*>(this->screen_buffer);
		for(int scan_y = 0; scan_y < height; ++ scan_y)
		{
			for(int scan_x = 0; scan_x < width; ++ scan_x)
			{
				unsigned short color = buffer16[source_address + scan_x];
				if(static_cast<int>(color) != color_key)
				{
					screen_buffer16[destination_address + scan_x] = color;
				}
			}
			source_address += source_width;
			destination_address += Screen::WIDTH;
		}
	}
}

// color16 (source, custom packing) GGGR RRRR BBBB BGGG -> standard RGB565.
// Field named "red" in the original Color16To32 is physically blue (and vice
// versa) in this source format - preserved here, only the output packing
// changes (32bpp ARGB -> 16bpp RGB565, since hwdisp wants RGB565 directly).
unsigned short Screen::Color16To565(unsigned short color16)
{
	unsigned short color = (color16 >> 8) | ((color16 & 0xFF00) << 8);
	unsigned int phys_blue = color & 0x1F;
	unsigned int green6 = (color >> 5) & 0x3F;
	unsigned int phys_red = (color >> 11) & 0x1F;
	return static_cast<unsigned short>((phys_red << 11) | (green6 << 5) | phys_blue);
}

#endif
