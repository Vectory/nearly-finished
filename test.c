

/*
 * TinyPTC SDL v0.3.2 Example file
 * Copyright (C) 2001-2003 Alessandro Gatti <a.gatti@tiscali.it>
 * Copyright (C) 2000-2001 Glenn Fiedler <gaffer@gaffer.org>
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

#include "tinyptc.h"

#include <stdio.h>
#include "SDL.h"

/*taken over from SDL.c
 *doesnt seem to work under ubuntu
 */
#include "SDL_syswm.h"
static SDL_Event ptc_sdl_event;


#define WIDTH 320//800
#define HEIGHT 240//600
#define SIZE WIDTH*HEIGHT

static int pixel[SIZE];
static int pixel_clearscreen[SIZE];

void
ptc_cleanup_callback (void)
{
  fprintf (stderr, "Callback!\n");
}





/* TODO:
 *	- modules
 *  - unify structs to one struct ent_t
 *  - optimize dead bullt storage
 *	- less globals, without introducing piles of function parameters
 *  - optimize for hi-res
 *  - base actions on timesteps aka realtime
 *  - friction
 */
//////////////////////////////////////////////
#define SAVE_THE_DEAD	/*dont free bullets, instead save them for reuse 
						 *-> less malloc/free... advantages?
						 */
#define DEBUG

#include <stdlib.h>
#include <math.h>
#include <time.h>

//#include <errno.h>

struct point
{
	float x;
	float y;
};
typedef struct point point;

struct box
{
	float x1, y1, x2, y2;
};
typedef struct box box;

struct bullet
{
	float x, y;
	int r;
	float dir;
	float speed;
	int alive;
	void (*bFunc)(struct bullet*, int difficulty, int rank);
	struct bullet *next;
	struct bullet *prev;
};
typedef struct bullet bullet;

struct bullets
{
	bullet* head;
	struct bullets* next;
	struct bullets* prev;
};
typedef struct bullets bullets;

struct enemy
{
	float x1, y1, x2, y2, rot;
	long nextThink;
	struct enemy* next;
	bullets* bList;
};
typedef struct enemy enemy;


const float deg_to_rad_factor=3.141592f/180.0f; // pi/180
float deg_to_rad(float dir)
{
	return dir*deg_to_rad_factor;
}

bullets* bList;
#ifdef SAVE_THE_DEAD
bullet* bListDeadHead;
#endif //SAVE_THE_DEAD
enemy* eList;
enemy *player;

box screen;
int levelTime = 0;

void SetBox(box* b, float x1, float y1, float x2, float y2)
{
	b->x1=x1;
	b->y1=y1;
	b->x2=x2;
	b->y2=y2;
}

void newBullet(enemy *e, float x, float y, int r, float dir, 
				  float speed, 
				  void (*bFunc)(struct bullet*, int difficulty, int rank))
{
	bullet *b;
#ifdef SAVE_THE_DEAD
	if(bListDeadHead)
	{
		b=bListDeadHead;
		bListDeadHead=bListDeadHead->next;

		if(bListDeadHead)
			bListDeadHead->prev = NULL;	//prev ever used???
	}
	else
#endif //SAVE_THE_DEAD
	{
		b=malloc(sizeof(bullet)); //mymalloc...
	}
	b->x=x;
	b->y=y;
	b->r=r;
	b->dir=dir;
	b->speed=speed;
	b->bFunc=bFunc;

	if(!e->bList)
	{
		e->bList=malloc(sizeof(bullets)); //mymalloc
		e->bList->next=NULL;
		e->bList->prev=NULL;
		b->next=NULL;
		bList=e->bList;
	}
	else
	{
		b->next=e->bList->head;
		if(b->next)
			b->next->prev=b;
	}
	e->bList->head=b;
	b->prev=NULL;
}

void b1Func(bullet* b, int difficulty, int rank)
{
	b->x=b->x+b->speed*cosf(b->dir*deg_to_rad_factor);
	b->y=b->y+b->speed*sinf(b->dir*deg_to_rad_factor);
}


void update_bullet(bullet *b)
{
	/*
	int x, y;
	float px, py, r;
	y = (int)(b->y);
	y -= b->r; 
	x = (int)(b->x);
	x -= b->r;
	while(y < (b->y + b->r))
	{
		while(x < (b->x + b->r))
		{
			if(x < 0 || x > WIDTH || y < 0 || y > HEIGHT)
			{
				x++;
				continue;
			}
			px=x - b->x;
			py=y - b->y;
			r=sqrt(px*px+py*py);
			if(r <= b->r)
				pixel[WIDTH*y+x] = ((int)(b->r-r+1)) * 127/(b->r+1);
			x++;
		}
		y++;
	}
	*/
	int x=(int)(b->x);
	int y=(int)(b->y);
	pixel[WIDTH*y+x]=0x44ff00;
}
//inline
void delete_bullet(bullet *b, bullets **bl)
{
	if(b->prev)
	{
		b->prev->next=b->next;
	}
	else if(b == (*bl)->head)
	{
		(*bl)->head=b->next;
	}
	#ifdef DEBUG
	else
	{
		printf("oops; special case (line:%i\n", __LINE__);
		getchar();
	}
	#endif //DEBUG
	if(b->next)
	{
		b->next->prev=b->prev;
	}
	
	if(!(*bl)->head)
	{
		if((*bl)->prev)
			(*bl)->prev->next=(*bl)->next;
		if((*bl)->next)
			(*bl)->next->prev=(*bl)->prev;
		if(*bl==bList)
			bList=NULL;
		free(*bl);
		*bl=NULL;
	}
#ifndef SAVE_THE_DEAD
	free(b);
	//b=NULL; //CLEAN_UP: b is not used after here
#else
		b->next=bListDeadHead;
		bListDeadHead=b;
		b->prev=NULL;	//CLEAN_UP: never used when dead, discard?
#endif //SAVE_THE_DEAD
}

void update_enemy(enemy *e)
{
	int x, y;

	for(y=(int)e->y1; y < e->y1 + e->y2; y++)
	{
		for(x=(int)e->x1; x < e->x1 + e->x2; x++)
		{
			if(x < 0 || x > WIDTH || y < 0 || y > HEIGHT)
				continue;
			pixel[WIDTH*y+x] = 0x00ff0000;
			/*if(x==(int)b->x && y==(int)b->y) pixel[WIDTH*y+x]=0x44ff00;*/
		}
	}
}


void update_player(int color)
{
	int x, y;
	for(y=player->y1; y < player->y1+player->y2; y++)
		for(x=player->x1; x< player->x1 + player->x2; x++)
			pixel[WIDTH*y+x]=color;
}

void update_game()
{
	bullet* b;
	bullet* bNext;
	bullets* blNext;
	bullets* bl=bList;
	enemy* e=eList;
	enemy* eNext;
	float dir;
	int i=0;

	/*TODO:
		atm after a bullet is created it shouldnt move one step, before its drawn
		but enemies shouldnt draw over bullets 
		*/
	while(e)
	{
		int i;
		eNext=e->next;
		if(e->nextThink > levelTime)
		{
			e=eNext;
			continue;
		}
		e->nextThink = levelTime+CLOCKS_PER_SEC/60*2;


		/*nmy->eFunc(...);*/
		for(i=0;i<360;i+=24)
		{
			dir=deg_to_rad((float)i+e->rot);
			newBullet( e, (float)e->x1+1*cosf(dir), (float)e->y1+1*sinf(dir), 5, (float)i+e->rot, 2.0f, b1Func);
		}
		e->rot+=1;//7.5;

		//update_enemy(e);
		e=eNext;
	}
	//printf("b:%i f:%i lf:%i\n", sizeof(bullet), sizeof(float), sizeof(double));

	update_player(0x00FFFF00);

	while(bl)
	{
		blNext=bl->next;
		b=bl->head; 
		while(b)
		{
			i++;
			bNext=b->next;
			pixel[WIDTH*(int)(b->y)+(int)(b->x)]=0x000000;	//undraw bullet at old position
			b->bFunc(b, 0, 0);

/*clean up off screen bullets*/
#define W_FACTOR 0 //100
#define H_FACTOR 0 //65
			if(b->x > WIDTH - W_FACTOR || b->x < 0+W_FACTOR ||
			   b->y > HEIGHT - H_FACTOR || b->y < 0+H_FACTOR)
			{
				delete_bullet(b, &bl);
			}
			else
			{
				update_bullet(b);
			}
			b=bNext;
		}
		bl=blNext;
	}
//	printf("bullets:%i ", i);
}

void run()
{
	clock_t start, end;
	int player_up=0, player_down=0, player_left=0, player_right=0;
	FILE *out=fopen("test.txt", "w");
	bList=NULL;
	eList=NULL;
	SetBox(&screen, 0,0, WIDTH, HEIGHT);

	{ /*init clear screen buffer*/
		int i=0;
		for(;i<SIZE;i++)
			pixel_clearscreen[i]=0;
	}
	levelTime=0;
	bListDeadHead = NULL;
	if(eList==NULL)
	{
		eList=malloc(sizeof(enemy));
		eList->next=NULL;
	}
	eList->x2=2;
	eList->y2=2;
	eList->x1=screen.x2/2 - eList->x2/2;
	eList->y1=screen.y2/2 - eList->y2/2;
	eList->rot=0;
	eList->nextThink=levelTime;
	//
	eList->bList=NULL;
	
	player=malloc(sizeof(enemy));
	player->x2=2;
	player->y2=2;
	player->x1=screen.x2/2 - player->x2/2;
	player->y1=screen.y2 - screen.y2/4 - player->y2/2;

	do
	{
		start=clock();
		//memcpy(pixel, pixel_clearscreen, sizeof(pixel)); //undraw everything
		update_game();
		if(ptc_update(pixel)==PTC_FAILURE)
			return;
		update_player(0x000000); //unupdate player
		{
			unsigned char *ptc_keypressed;
			if (SDL_PollEvent (&ptc_sdl_event)) {
				switch (ptc_sdl_event.type) {
					case SDL_KEYDOWN: {
						ptc_keypressed = SDL_GetKeyState(NULL);
						//printf("\"up_key=%i ", ptc_keypressed[SDLK_UP]);
						if (ptc_keypressed[SDLK_ESCAPE] == SDL_PRESSED) {
						  return;
						}
				    }; break;
					case SDL_QUIT: {
						return;
					}; break;
				}
			}
			ptc_keypressed=SDL_GetKeyState(NULL);
			player_up = (ptc_keypressed[SDLK_UP] == SDL_PRESSED)? -4 : 0;
			player_down = (ptc_keypressed[SDLK_DOWN] == SDL_PRESSED)? 4 : 0;
			player_right = (ptc_keypressed[SDLK_RIGHT] == SDL_PRESSED)? 4 : 0;
			player_left = (ptc_keypressed[SDLK_LEFT] == SDL_PRESSED)? -4 : 0;
			//fprintf("up: %i; down: %i; right: %i; left: %i;\npu: %i; pd: %i; pr: %i; pl: %i;\n\n", ptc_keypressed[SDLK_UP], ptc_keypressed[SDLK_DOWN],ptc_keypressed[SDLK_RIGHT],	ptc_keypressed[SDLK_LEFT], player_up, player_down, player_left,	player_right);
			player->x1+=player_right+player_left;
			player->y1+=player_down+player_up;
			if(player->x1 < 0 || player->x1 > WIDTH) player->x1-=(player_right+player_left);
			if(player->y1 < 0 || player->y1 > HEIGHT) player->y1-=(player_down+player_up);
		}
		do
		{
			end=clock();
		}
		while((end-start) < CLOCKS_PER_SEC/60);
		//printf("FPS: %li\n", CLOCKS_PER_SEC/(end-start));
		levelTime+=end-start;
		//printf("."), fflush(stdout);
	}while(1);//levelTime < CLOCKS_PER_SEC*20);
}

int
main ()
{
  if (!ptc_open ("shmoop", WIDTH, HEIGHT))
    return 1;
  run();
  ptc_close ();
  return 0;
}
