#if defined(SDL2)

#include <time.h>
#include <stdio.h>
#include "../../TinyLibrary.hpp"
#include "../../MainLoop.hpp"
#include "../../Global.hpp"

static long long now_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return static_cast<long long>(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
}

int main(int argc, char* argv[])
{
	MainLoop_Setup();
	long long beforeTime = now_ns();
	long long intervalTime = 1000000000LL / Screen::FPS;
	bool exit = false;
	long long loop_ns = 0, loop_max_ns = 0, gap_ns = 0, last_start = 0;
	int loop_count = 0;
	while(exit == false)
	{
		long long nowTime = now_ns();
		long long elapsed = nowTime - beforeTime;
		if(elapsed > intervalTime)
		{
			const long long loop_start = now_ns();
			exit = MainLoop_Loop();
			const long long loop_end = now_ns();
			loop_ns += loop_end - loop_start;
			if(loop_end - loop_start > loop_max_ns) { loop_max_ns = loop_end - loop_start; }
			if(last_start != 0) { gap_ns += loop_start - last_start; }
			last_start = loop_start;
			if(++ loop_count >= 120)
			{
				fprintf(stderr, "tinystg loop: 120 frames, tick+draw avg %.2f ms (max %.2f ms), frame interval avg %.2f ms (%.1f fps)\n",
					loop_ns / 120.0 / 1e6, loop_max_ns / 1e6, gap_ns / 119.0 / 1e6, 119.0 * 1e9 / gap_ns);
				loop_count = 0; loop_ns = 0; loop_max_ns = 0; gap_ns = 0; last_start = 0;
			}
			Controller& controller = Controller::GetInstance();
			if((controller.GetButton() & Controller::BUTTON_QUIT) != 0)
			{
				exit = true;
			}
			beforeTime = nowTime;
		}
		MainLoop_NoWaitLoop();
	}
	MainLoop_Finalize();
	return 0;
}

#endif
