#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>
#include "../include/sprut.h"
#include "../include/sprutext.h"

#define MAINWINDOWWIDTH 768
#define MAINWINDOWHEIGHT 512

/*
#define ARRWIDTH 80
#define ARRHEIGHT 80
*/

#define WINDOWBUFFERPIXELS LETTERHEIGHT
#define YNPROMPT "(press y or n)"
#define WINDOW_R 0
#define WINDOW_G 100
#define WINDOW_B 255

#define CELLSIDEPX 16

#define WAIT 75

#define LETTERS_SPRITEFILE "graphics/8letters.bmp"

Uint32 black;
Uint32 white;

unsigned int mainwindowwidth = MAINWINDOWWIDTH;
unsigned int mainwindowheight = MAINWINDOWHEIGHT;
unsigned int cellsidepx = CELLSIDEPX;
unsigned int arrwidth = 0;
unsigned int arrheight = 0;
unsigned int wait = WAIT;

SDL_Surface *screen = NULL;
sprut_spritelib *letters;
long int generation = 0;
char strbuf[32];

typedef enum _gol_state{
  gol_state_run,
  gol_state_quit,
  gol_state_pause,
  gol_state_bump
} gol_state;

gol_state state = gol_state_run;

typedef enum _gol_flipflop{
  gol_flip,
  gol_flop
} gol_flipflop;

void sdl_display(unsigned char *arr,
		 int xpos, int ypos,
		 int width, int height);
void ascii_display(unsigned char *arr,
		   int xpos, int ypos,
		   int width, int height);
int gol_prompt(char *text);
void gol_draw_ynprompt_window(char *text,
			      int x, int y);
int gol_max(int one, int two);
int delay();

int main(int argc, char **argv)
{
  int i, j, ni, nj, xpos, ypos, xoff = 0, yoff = 0;
  register int c;
  unsigned char *cur, *evo;
  gol_flipflop ff = gol_flip;

  FILE *file;
  int fx, fy;

  SDL_Thread *waitthread;
  SDL_Event myevent;
  void *voidptr = NULL;

  char *usage = "usage: gol [-t <wait time milliseconds>] [-p <cell side pixels>] [-d <width> <height>] [-w <main window width> <main window height>] [-o <x offset> <y offset>] <filename>\n       \n       commands:\n         q: quit\n         p: pause\n         r: run\n         n: step (while paused)\n";
  char *filename = NULL;

  unsigned int argcount = 1;
  while(argcount < argc){
    if(strcmp(argv[argcount], "-d") == 0){ //dims
      if(argc < argcount + 3){
	puts(usage);
	return 0;
      } else{
	sscanf(argv[argcount + 1], "%i", &arrwidth);
	sscanf(argv[argcount + 2], "%i", &arrheight);
	argcount += 3;
      }
    } else if(strcmp(argv[argcount], "-w") == 0){ //dims
      if(argc < argcount + 3){
	puts(usage);
	return 0;
      } else{
	sscanf(argv[argcount + 1], "%i", &mainwindowwidth);
	sscanf(argv[argcount + 2], "%i", &mainwindowheight);
	argcount += 3;
      }
    } else if(strcmp(argv[argcount], "-o") == 0){ //dims
      if(argc < argcount + 3){
	puts(usage);
	return 0;
      } else{
	sscanf(argv[argcount + 1], "%i", &xoff);
	sscanf(argv[argcount + 2], "%i", &yoff);
	argcount += 3;
      }
    } else if(strcmp(argv[argcount], "-t") == 0){
      if(argc < argcount + 2){
	puts(usage);
	return 0;
      } else{
	sscanf(argv[argcount + 1], "%i", &wait);
	argcount += 2;
      }
    } else if(strcmp(argv[argcount], "-p") == 0){
      if(argc < argcount + 2){
	puts(usage);
	return 0;
      } else{
	sscanf(argv[argcount + 1], "%i", &cellsidepx);
	argcount += 2;
      }
    } else{
      filename = argv[argcount];
      argcount++;
    }
  }

  if(!arrwidth){
    arrwidth = mainwindowwidth / cellsidepx;
  }

  if(!arrheight){
    arrheight = mainwindowheight / cellsidepx;
  }

  printf("%i %i\n", arrwidth, arrheight);
  
  if(filename == NULL){
    puts(usage);
    return 0;
  }

  file = fopen(filename, "r");
  if(file == NULL){
    printf("error -- failed to open file %s", argv[1]);
    return 0;
  }
  
  unsigned char *flip = malloc(sizeof(unsigned char) * arrwidth * arrheight);
  unsigned char *flop = malloc(sizeof(unsigned char) * arrwidth * arrheight);
  unsigned char *count = malloc(sizeof(unsigned char) * arrwidth * arrheight);

  if(!(flip && flop && count)){
    puts("error -- couldn't allocate memory!");
    return 0;
  }

  /*
    Initialize SDL functions... just video for now
  */
  if(SDL_Init(SDL_INIT_VIDEO) < 0){
    fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
    return 0;
  }

  letters = sprut_read_spritelib_with_scalefactor(LETTERS_SPRITEFILE, LETTERWIDTH / LETTERSCALE, LETTERHEIGHT / LETTERSCALE, LETTERSCALE);
  if(!letters){
    printf("Error loading letters from file %s!", LETTERS_SPRITEFILE);
    SDL_Quit();
    return 0;
  }

  /*
    fill array with zeroes
  */
  for(i = 0; i < arrwidth * arrheight; i++)
    flip[i] = 0;

  while(fscanf(file, "%i %i\n", &fx, &fy) == 2){
    fx += xoff;
    fy += yoff;
    if(fx < 0 || fx >= arrwidth || fy < 0 || fy >= arrheight){
      printf("warning -- you specified a point (%i, %i) outside the array, skipping it\n", fx, fy);
      continue;
    }
    flip[fx + fy * arrwidth] = 1;
  }

  fclose(file);

  /*
    Open window
  */
  screen = SDL_SetVideoMode(mainwindowwidth, mainwindowheight, 16, SDL_HWSURFACE);
  if(screen == NULL){
    printf("error -- failed to set SDL video mode: %s\n", SDL_GetError());
    SDL_Quit();
    return 0;
  }
  SDL_WM_SetCaption("life!", NULL);

  black = SDL_MapRGB(screen->format, 0, 0, 0);
  white = SDL_MapRGB(screen->format, 255, 255, 255);

  /*
    main loop
  */
  while(state != gol_state_quit){
    waitthread = SDL_CreateThread(delay, voidptr);
    if(state == gol_state_bump) 
      state = gol_state_pause;
    SDL_PumpEvents();

    while(SDL_PollEvent(&myevent)){
      switch(myevent.type){
      case SDL_KEYDOWN:
	switch(myevent.key.keysym.sym){
	case SDLK_q:
	  if(gol_prompt("quit?"))
	    state = gol_state_quit;
	  break;
	case SDLK_p:
	  state = gol_state_pause;
	  break;
	case SDLK_r:
	  state = gol_state_run;
	  break;
	case SDLK_n:
	  if(state == gol_state_pause)
	    state = gol_state_bump;
	  break;
	default:
	  break;
	}
	break;
      default:
	break;
      }
    }

    if(state == gol_state_pause || state == gol_state_quit){
      sprintf(strbuf, "generation %i (paused)", (int)generation);
      sprut_drawstring(strbuf, letters, screen, 5, mainwindowheight - 5 - LETTERHEIGHT);
      SDL_Flip(screen);
      SDL_WaitThread(waitthread, NULL);
      continue;
    } else
      generation++;

    if(ff == gol_flip){
      cur = flip;
      evo = flop;
      ff = gol_flop;
    } else{
      cur = flop;
      evo = flip;
      ff = gol_flip;
    }
    
    for(c = 0; c < arrwidth * arrheight; c++)
      count[c] = 0;

    //    ascii_display(cur, 0, 0, arrwidth, arrheight);
    sdl_display(cur, 0, 0, arrwidth, arrheight);

    for(i = 0; i < arrwidth; i++){
      for(j = 0; j < arrheight; j++){
	
	if(cur[i + j * arrwidth]){ // add count to neighbors
	  for(ni = 0; ni < 3; ni++){
	    xpos = i - 1 + ni;
	    if(xpos < 0) continue;
	    else if(xpos >= arrwidth) break;
	    for(nj = 0; nj < 3; nj++){
	      ypos = j - 1 + nj;
	      if(ypos < 0 || (ypos == j && xpos == i)) continue;
	      else if(ypos >= arrheight) break;
	      count[xpos + ypos * arrwidth]++;
	    }
	  }
	}
      }
    }
    //	  if(count > 0) printf("%i, %i, %i\n", i, j, count);
    for(i = 0; i < arrwidth * arrheight; i++){
      if(cur[i]){ // live cell
	if(count[i] < 2)
	  evo[i] = 0;
	else if(count[i] <= 3)
	  evo[i] = 1;
	else
	  evo[i] = 0;
      } else{ //dead cell
	if(count[i] == 3)
	  evo[i] = 1;
	else
	  evo[i] = 0;
      }
    }
    SDL_WaitThread(waitthread, NULL);
  }

  free(flip);
  free(flop);
  free(count);
  sprut_free_spritelib(letters);
  SDL_FreeSurface(screen);
  SDL_Quit();
  return 0;
}

void sdl_display(unsigned char *arr,
		 int xpos, int ypos,
		 int width, int height)
{
  int i, j;
  SDL_Rect dst;
  SDL_FillRect(screen, NULL, black);
  for(j = ypos; j < height; j++){
    if(j >= arrheight) break;
    for(i = xpos; i < width; i++){
      if(i >= arrwidth) break;
      else if(arr[i + j * arrwidth]){
	dst.x = i * cellsidepx;
	dst.y = j * cellsidepx;
	dst.w = cellsidepx;
	dst.h = cellsidepx;
	SDL_FillRect(screen, &dst, white);
      }
    }
  }
  sprintf(strbuf, "generation %i", (int)generation);
  sprut_drawstring(strbuf, letters, screen, 5, mainwindowheight - 5 - LETTERHEIGHT);
  SDL_Flip(screen);
}

void ascii_display(unsigned char *arr,
		   int xpos, int ypos,
		   int width, int height)
{
  int i, j;
  for(j = ypos; j < height; j++){
    if(j >= arrheight) break;
    for(i = xpos; i < width; i++){
      if(i >= arrwidth) break;
      switch(arr[i + j * arrheight]){
      case 1:
	printf("%c", (char)35);
      default:
	printf("%c", (char)32);
      }
    }
    printf("\n");
  }
}

int gol_prompt(char *text)
{
  SDL_Event event;
  int boxwidth = gol_max(sprut_gettextwidth(text), sprut_gettextwidth(YNPROMPT)) + 2 * WINDOWBUFFERPIXELS;
  gol_draw_ynprompt_window(text,
			   screen->w / 2 - boxwidth / 2, screen->h / 3);
  SDL_Flip(screen);
  while(SDL_WaitEvent(&event)){
    if(event.type == SDL_KEYDOWN){
      switch(event.key.keysym.sym){
      case SDLK_y:
	return 1;
	break;
      case SDLK_n:
	return 0;
	break;
      default:
	break;
      }
    }
  }
  return 0;
}

void gol_draw_ynprompt_window(char *text,
			      int x, int y)
{
  Uint32 color = SDL_MapRGB(screen->format, WINDOW_R, WINDOW_G, WINDOW_B);
  int width = gol_max(sprut_gettextwidth(text), sprut_gettextwidth(YNPROMPT));
    
  SDL_Rect window;
  window.x = x;
  window.y = y;
  window.h = 3 * WINDOWBUFFERPIXELS + 2 * LETTERHEIGHT;
  window.w = 2 * WINDOWBUFFERPIXELS + width;
  
  SDL_FillRect(screen, &window, color);
  sprut_drawstring(text, letters, screen, x + WINDOWBUFFERPIXELS, y + WINDOWBUFFERPIXELS);
  sprut_drawstring(YNPROMPT, letters, screen, x + WINDOWBUFFERPIXELS, 
		y + 2 * WINDOWBUFFERPIXELS + LETTERHEIGHT);
}

int gol_max(int one, int two)
{
  if(one >= two)
    return one;
  else
    return two;
}

/*
  This function gets run in its own thread every iteration of the main event loop
*/
int delay()
{
  SDL_Delay(wait);
  return 0;
}
