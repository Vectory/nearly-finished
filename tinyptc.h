/*
 * TinyPTC SDL v0.3.2 Main header file
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

#ifndef __TINYPTC_SDL
#define __TINYPTC_SDL

#define PTC_FAILURE 0
#define PTC_SUCCESS 1

/* This directive enables windowed output */

//#define __PTC_WINDOWED__

/* This directive enables window centering */

/* #define __PTC_CENTER_WINDOW__ */

/* This directive enables pixelformat conversions */
   
#define __PTC_ENABLE_CONVERSIONS__

/* This directive enables automatic processing of the alpha channel */

/* #define __PTC_ENABLE_ALPHA__ */

/* This directive enables the use of an user-supplied callback that will be
 * triggered upon exit */

#define __PTC_CLEANUP_CALLBACK__

typedef unsigned int int32;
typedef unsigned char char8;
typedef unsigned short int16;
typedef unsigned short short16;

#ifdef __cplusplus
        extern "C" {
#endif /* __cplusplus */
          
extern int ptc_open(char *title, int width, int height);
extern int ptc_update(void *buffer);
extern void ptc_close(void);
int ptc_process_events(void);

#ifdef __PTC_CLEANUP_CALLBACK__
extern void ptc_cleanup_callback (void);
#endif /* __PTC_CLEANUP_CALLBACK__ */

#ifdef __cplusplus
        }
#endif /* __cplusplus */

#endif /* __TINYPTC_SDL */
