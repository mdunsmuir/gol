#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>

#define ARRWIDTH 80
#define ARRHEIGHT 80

#define MAINWINDOWWIDTH 768
#define MAINWINDOWHEIGHT 512

#define CELLSIDEPX 16

#define WAIT 75

Uint32 black;
Uint32 white;

unsigned int arrwidth = ARRWIDTH;
unsigned int arrheight = ARRHEIGHT;
unsigned int mainwindowwidth = MAINWINDOWWIDTH;
unsigned int mainwindowheight = MAINWINDOWHEIGHT;
unsigned int cellsidepx = CELLSIDEPX;
unsigned int wait = WAIT;

void sdl_display(unsigned char *arr,
		 SDL_Surface *screen,
		 int xpos, int ypos,
		 int width, int height);

void ascii_display(unsigned char *arr,
		   int xpos, int ypos,
		   int width, int height);

typedef enum _gol_state{
  gol_state_run,
  gol_state_quit
} gol_state;

typedef enum _gol_flipflop{
  gol_flip,
  gol_flop
} gol_flipflop;

int main(int argc, char **argv)
{
  int i, j, ni, nj, xpos, ypos, xoff = 0, yoff = 0;
  register int c;
  unsigned char *cur, *evo;
  gol_state state = gol_state_run;
  gol_flipflop ff = gol_flip;

  FILE *file;
  int fx, fy;

  SDL_Surface *screen = NULL;
  Uint8 *keystate;

  char *usage = "usage: gol [-t <wait time milliseconds>] [-p <cell side pixels>] [-d <width> <height>] [-w <main window width> <main window height>] [-o <x offset> <y offset>] <filename>\n       (press q to quit)";
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

  if(filename == NULL){
    puts(usage);
    return 0;
  }

  file = fopen(filename, "r");
  if(file == NULL){
    printf("error -- failed to open file %s", argv[1]);
    return 0;
  }

  printf("%i, %i", arrwidth, arrheight);
  
  unsigned char *flip = malloc(sizeof(unsigned char) * arrwidth * arrheight);
  unsigned char *flop = malloc(sizeof(unsigned char) * arrwidth * arrheight);
  unsigned char *count = malloc(sizeof(unsigned char) * arrwidth * arrheight);

  if(!(flip && flop && count)){
    puts("error -- couldn't allocate memory!");
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
    SDL_PumpEvents();
    keystate = SDL_GetKeyState(NULL);

    if(keystate[SDLK_q]){
      state = gol_state_quit;
      continue;
    }

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
    sdl_display(cur, screen, 0, 0, arrwidth, arrheight);
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
    SDL_Delay(wait);
  }

  free(flip);
  free(flop);
  free(count);
  SDL_FreeSurface(screen);
  SDL_Quit();
  return 0;
}

void sdl_display(unsigned char *arr,
		 SDL_Surface *screen,
		 int xpos, int ypos,
		 int width, int height)
{
  int i, j;
  SDL_Rect dst;
  SDL_FillRect(screen, NULL, white);
  for(j = ypos; j < height; j++){
    if(j >= arrheight) break;
    for(i = xpos; i < width; i++){
      if(i >= arrwidth) break;
      else if(arr[i + j * arrheight]){
	dst.x = i * cellsidepx;
	dst.y = j * cellsidepx;
	dst.w = cellsidepx;
	dst.h = cellsidepx;
	SDL_FillRect(screen, &dst, black);
      }
    }
  }
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
