/*
 * TinyPTC SDL v0.3.2 Main SDL interface file
 * Copyright (C) 2000-2003 Alessandro Gatti <a.gatti@tiscali.it>
 *
 * http://sourceforge.net/projects/tinyptc/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 */

#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_syswm.h"
#include "tinyptc.h"

#ifdef __PTC_ENABLE_ALPHA__
  #define PTC_ALPHA_MASK 0xFF000000
#else
  #define PTC_ALPHA_MASK 0x00000000
#endif /* __PTC_ENABLE_ALPHA__ */

#define PTC_RED_MASK   0x00FF0000
#define PTC_GREEN_MASK 0x0000FF00
#define PTC_BLUE_MASK  0x000000FF

#ifndef __PTC_WINDOWED__
  #undef __PTC_CENTER_WINDOW__
#endif /* !__PTC_WINDOWED__ */

static int ptc_stored_width;
static int ptc_stored_height;
static SDL_Surface *ptc_video_surface;
static SDL_Surface *ptc_blit_surface;
static SDL_Surface *ptc_buffer_surface;
static SDL_Event ptc_sdl_event;
static int32 *ptc_buffer;

#ifndef __PTC_WINDOWED__
static int ptc_pitch;
#endif /* !__PTC_WINDOWED__ */

int
ptc_open (char *title, int width, int height)
{
#ifdef __PTC_CENTER_WINDOW__
  SDL_SysWMinfo ptc_wm_info;
  int ptc_wm_width, ptc_wm_height, ptc_wm_x, ptc_wm_y;
#endif /* __PTC_CENTER_WINDOW__ */
  
	if (SDL_Init (SDL_INIT_VIDEO) < 0) {
		return PTC_FAILURE;
	}
	atexit (SDL_Quit);
#ifdef __PTC_WINDOWED__
	SDL_WM_SetCaption(title, NULL);
#endif /* __PTC_WINDOWED__ */
	ptc_video_surface = SDL_SetVideoMode (width, height,
#ifdef __PTC_ENABLE_CONVERSIONS__				  
	32, 
#else
	0,
#endif /* __PTC_ENABLE_CONVERSIONS__ */
	SDL_HWSURFACE | SDL_DOUBLEBUF
#ifndef __PTC_WINDOWED__
	| SDL_FULLSCREEN
#endif /* !__PTC_WINDOWED__ */
	);
	if (ptc_video_surface == NULL) {
		return PTC_FAILURE;
	}

#ifdef __PTC_CENTER_WINDOW__
    SDL_VERSION(&ptc_wm_info.version);
    if (SDL_GetWMInfo(&ptc_wm_info) > 0) {
      if (ptc_wm_info.subsystem == SDL_SYSWM_X11) {
        ptc_wm_info.info.x11.lock_func();
        ptc_wm_width = DisplayWidth(ptc_wm_info.info.x11.display, DefaultScreen(ptc_wm_info.info.x11.display));
        ptc_wm_height = DisplayHeight(ptc_wm_info.info.x11.display, DefaultScreen(ptc_wm_info.info.x11.display));
        ptc_wm_x = (ptc_wm_width - ptc_video_surface->w) >> 1;
        ptc_wm_y = (ptc_wm_height - ptc_video_surface->h) >> 1;
        XMoveWindow(ptc_wm_info.info.x11.display, ptc_wm_info.info.x11.wmwindow, ptc_wm_x, ptc_wm_y);
        ptc_wm_info.info.x11.unlock_func();
      }
    }
#endif /* __PTC_CENTER_WINDOW__ */

	SDL_ShowCursor(SDL_FALSE);
	ptc_stored_width = width;
	ptc_stored_height = height;
#ifndef __PTC_WINDOWED__
	ptc_pitch = ptc_video_surface->pitch;
#endif /* !__PTC_WINDOWED__ */
	ptc_buffer = (int32 *)calloc(ptc_stored_width * ptc_stored_height, sizeof(int32));
	if (ptc_buffer == NULL) {
		return PTC_FAILURE;
	}
	ptc_buffer_surface = SDL_CreateRGBSurfaceFrom(ptc_buffer, ptc_stored_width, ptc_stored_height, 32, ptc_stored_width*sizeof(int), PTC_RED_MASK, PTC_GREEN_MASK, PTC_BLUE_MASK, PTC_ALPHA_MASK);
#ifndef __PTC_ENABLE_CONVERSIONS__
	ptc_blit_surface = ptc_buffer_surface;
#endif /* !__PTC_ENABLE_CONVERSIONS__ */

	return PTC_SUCCESS;
}

int
ptc_update (void *buffer)
{
	int ptc_return_code;
	
	memcpy(ptc_buffer_surface->pixels, buffer, ptc_stored_width * ptc_stored_height * sizeof(int));
#ifdef __PTC_ENABLE_CONVERSIONS__
	ptc_blit_surface = SDL_DisplayFormat(ptc_buffer_surface);
	if (ptc_blit_surface == NULL) {
		return PTC_FAILURE;
	}
#else
	ptc_blit_surface = ptc_buffer_surface;
#endif /* __PTC_ENABLE_CONVERSIONS__ */
	ptc_return_code = SDL_BlitSurface(ptc_blit_surface, NULL, ptc_video_surface, NULL);
	if (ptc_return_code != 0) {
		return PTC_FAILURE;
	}
#ifdef __PTC_ENABLE_CONVERSIONS__
	SDL_FreeSurface(ptc_blit_surface);
#endif /* __PTC_ENABLE_CONVERSIONS__ */
	ptc_return_code = SDL_Flip(ptc_video_surface);
	if (ptc_return_code != 0) {
		return PTC_FAILURE;
	}
    if (ptc_process_events() == PTC_FAILURE) {
#ifdef __PTC_CLEANUP_CALLBACK__
      ptc_cleanup_callback();
#endif /* __PTC_CLEANUP_CALLBACK__ */
      ptc_close();
      exit(0);
    }
	return PTC_SUCCESS;
}

int
ptc_process_events (void)
{

  

	return PTC_SUCCESS;
}

void
ptc_close (void)
{
	SDL_ShowCursor(SDL_TRUE);
	SDL_FreeSurface(ptc_buffer_surface);
	SDL_FreeSurface(ptc_video_surface);
	free(ptc_buffer);
}
