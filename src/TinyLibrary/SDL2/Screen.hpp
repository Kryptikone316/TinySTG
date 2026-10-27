#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "../../../Config.h"

#define USE_COLOR256
//#define USE_COLOR65536
#define COLOR8(r,g,b)  ((b>>5)<<5|(g>>5)<<2|(r>>6))               // BBBGGGRR
#define COLOR16(r,g,b) ((((g<<3)&0xE)|(r>>3))<<8)+(b&0xF8)|(g>>5) // GGGRRRRRBBBBBGGG

// The frame handed to hwdisp is built in Screen::DrawEnd(): the 80x64 game
// image is upscaled in software (nearest neighbour) to hw_out_w x hw_out_h,
// then the driver scales that to the panel. The size and fit mode come from
// video.txt (see Screen::LoadVideoConfig()).

class Screen
{
public:
	enum
	{
		FPS = CONFIG_FPS,
		SCALE = 1,
		CENTER = -1,
		LEFT = -2,
		RIGHT = -3,
		TOP = -4,
		BOTTOM = -5
	};
	static Screen* screen;
	static int WIDTH;
	static int HEIGHT;
	Screen(int color_mode);
	~Screen(void);
	static void Initialize(int color_mode);
	static void Finalize(void);
	static Screen& GetInstance();
	void Clear(unsigned int color);
	void DrawBegin(void);
	void DrawEnd(void);
	void DrawSprite(const void* buffer, int x, int y, int width, int height, int color_key);
	void DrawSprite(const void* buffer, int x, int y, int width, int height, int source_x, int source_y, int source_width, int color_key);
	void DrawSprite(int destination_x, int destination_y, int destination_left, int destination_top, int destination_right, int destination_bottom, const void* source_buffer, int source_x, int source_y, int source_width, int source_height, int color_key, int rotate, int scale);
	void DrawSpriteFrame(const void* buffer, int x, int y, int width, int height, int source_width, int source_height, int color_key);
	void DrawFont(int x, int y, const char* text, ...);
	void DrawJapaneseFont(int x, int y, const char* text);
	void DrawNumberFont(int x, int y, const char* text, ...);
	void DrawRectangle(int x, int y, int width, int height, unsigned int color);
private:
 	void DrawSpriteNoClip(const void* buffer, int x, int y, int width, int height, int color_key, int source_width);
	unsigned short Color16To565(unsigned short color);
	Screen(Screen&);
	Screen& operator = (Screen&);
	int color_mode;
	unsigned char* screen_buffer;
	int buffer_size;
	static const unsigned int color_table[256];
	unsigned short* hw_frame_buffer;
	int hw_out_w;
	int hw_out_h;
	int* hw_col_map;
	int* hw_row_map;
	unsigned short* hw_src_row;
	unsigned short hw_lut[256];
	unsigned char* hw_col_edge;
	unsigned char* hw_row_edge;
	unsigned short hw_lut4[4][256]; // [row_dark*2 + col_dark][palette index]
	unsigned int hw_frames;
	long long hw_compose_ns;
	long long hw_present_ns;
	long long hw_present_max_ns;
	void LoadVideoConfig(void);
};

#endif
