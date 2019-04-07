#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
int force = 1;

#define SDL_HWSURFACE 0x00000001
#define SDL_ASYNCBLIT 0x00000004

typedef struct SDLPixelFormat
{
  void *palette;
  uint8_t BitsPerPixel;
  uint8_t BytesPerPixel;
  uint8_t Rloss, Gloss, Bloss, Aloss;
  uint8_t Rshift, Gshift, Bshift, Ashift;
  uint32_t Rmask, Gmask, Bmask, Amask;
  uint32_t colorkey;
  uint8_t alpha;
} SDLPixelFormat;

typedef struct
{
  uint32_t hw_available : 1;
  uint32_t wm_available : 1;
  uint32_t blit_hw : 1;
  uint32_t blit_hw_CC : 1;
  uint32_t blit_hw_A : 1;
  uint32_t blit_sw : 1;
  uint32_t blit_sw_CC : 1;
  uint32_t blit_sw_A : 1;
  uint32_t blit_fill : 1;
  uint32_t video_mem;
  void *vfmt;
  int current_w;
  int current_h;
} SDLVideoInfo;

typedef struct SDLSurface
{
  uint32_t flags;
  SDLPixelFormat *format;
  int w, h;
  int pitch;
  void *pixels;
  void *userdata;
  int locked;
  void *lock_data;

} SDLSurface;

SDLSurface *screen = NULL;
SDLSurface *realScreen = NULL;

/* alekmaul's scaler taken from mame4all */
void bitmap_scale(uint32_t startx, uint32_t starty, uint32_t viswidth, uint32_t visheight, uint32_t newwidth, uint32_t newheight,uint32_t pitchsrc,uint32_t pitchdest, uint16_t* restrict src, uint16_t* restrict dst)
{
    uint32_t W,H,ix,iy,x,y;
    x=startx<<16;
    y=starty<<16;
    W=newwidth;
    H=newheight;
    ix=(viswidth<<16)/W;
    iy=(visheight<<16)/H;

    do 
    {
        uint16_t* restrict buffer_mem=&src[(y>>16)*pitchsrc];
        W=newwidth; x=startx<<16;
        do 
        {
            *dst++=buffer_mem[x>>16];
            x+=ix;
        } while (--W);
        dst+=pitchdest;
        y+=iy;
    } while (--H);
}

void *SDL_GetVideoInfo2()
{
	static void *(*info)() = NULL;

	if (info == NULL)
	{
	info = dlsym(RTLD_NEXT, "SDL_GetVideoInfo");
	assert(info != NULL);
	}

	SDLVideoInfo *inf = (SDLVideoInfo *)info(); //<-- calls SDL_GetVideoInfo();
	int screenWidth = inf->current_w;
	int screenHeight = inf->current_h;

	return inf;
}

void *SDL_SetVideoMode(int width, int height, int bitsperpixel, uint32_t flags)
{
	static void *(*real_func)(int, int, int, uint32_t) = NULL;
	static void *(*create)(uint32_t, int, int, int, uint32_t, uint32_t, uint32_t, uint32_t) = NULL;

	printf("Call to SDL_SetVideoMode intercepted %d %d %d\n", width, height, flags);

	if (real_func == NULL)
	{
		real_func = dlsym(RTLD_NEXT, "SDL_SetVideoMode");
		assert(real_func != NULL);
	}

	if (create == NULL)
	{
		create = dlsym(RTLD_NEXT, "SDL_CreateRGBSurface");
		assert(create != NULL);
	}
	
	screen = (SDLSurface *)create(0, width, height, 16, 0, 0, 0, 0);
	realScreen = (SDLSurface *)real_func(680, 448, 16, SDL_HWSURFACE);
	return screen;
	return realScreen;
}

static void *(*flip)() = NULL;

void *SDL_Flip(void *surface)
{
	if (flip == NULL)
	{
		flip = dlsym(RTLD_NEXT, "SDL_Flip");
		assert(flip != NULL);
	}

	if (surface == realScreen)
	{
		return flip(surface);
	}
	else if (surface == screen)
	{
		bitmap_scale(0, 0, screen->w, screen->h, realScreen->w, realScreen->h, screen->w, 0, (uint16_t* restrict)screen->pixels, (uint16_t* restrict)realScreen->pixels);
		return flip(realScreen);
	}
	else
	{
		return flip(surface);
	}
}
