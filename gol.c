//#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>

#define ARRWIDTH 30
#define ARRHEIGHT 30

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
  int i, j, ni, nj, xpos, ypos, ystart;
  register int c;
  unsigned char flip[ARRWIDTH * ARRHEIGHT], flop[ARRWIDTH * ARRHEIGHT],
    count[ARRWIDTH * ARRHEIGHT];
  unsigned char *cur, *evo;
  gol_state state = gol_state_run;
  gol_flipflop ff = gol_flip;

  FILE *file;
  int fx, fy;

  //  SDL_Surface *screen = NULL;

  struct timespec wait;
  wait.tv_sec = 0;
  wait.tv_nsec = 200000000;

  if(argc != 2){
    puts("usage -- gol <board file>");
    return 0;
  }

  file = fopen(argv[1], "r");
  if(file == NULL){
    printf("error -- failed to open file %s", argv[1]);
    return 0;
  }

  /*
    fill array with zeroes
  */
  for(i = 0; i < ARRWIDTH * ARRHEIGHT; i++)
    flip[i] = 0;

  while(fscanf(file, "%i %i\n", &fx, &fy) == 2){
    if(fx < 0 || fx >= ARRWIDTH || fy < 0 || fy >= ARRHEIGHT){
      puts("warning -- you specified a point (%i, %i) outside the array, skipping it");
      continue;
    }
    flip[fx + fy * ARRWIDTH] = 1;
  }

  fclose(file);

  /*
    main loop
  */
  while(state != gol_state_quit){
    if(ff == gol_flip){
      cur = flip;
      evo = flop;
      ff = gol_flop;
    } else{
      cur = flop;
      evo = flip;
      ff = gol_flip;
    }
    
    for(c = 0; c < ARRWIDTH * ARRHEIGHT; c++)
      count[c] = 0;

    ascii_display(cur, 0, 0, ARRWIDTH, ARRHEIGHT);
    for(i = 0; i < ARRWIDTH; i++){
      for(j = 0; j < ARRHEIGHT; j++){
	
	if(cur[i + j * ARRWIDTH]){ // add count to neighbors
	  for(ni = 0; ni < 3; ni++){
	    xpos = i - 1 + ni;
	    if(xpos < 0) continue;
	    else if(xpos >= ARRWIDTH) break;
	    for(nj = 0; nj < 3; nj++){
	      ypos = j - 1 + nj;
	      if(ypos < 0 || (ypos == j && xpos == i)) continue;
	      else if(ypos >= ARRHEIGHT) break;
	      count[xpos + ypos * ARRWIDTH]++;
	    }
	  }
	}
      }
    }
    //	  if(count > 0) printf("%i, %i, %i\n", i, j, count);
    for(i = 0; i < ARRWIDTH * ARRHEIGHT; i++){
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
    nanosleep(&wait, NULL);
  }

  return 0;
}

void ascii_display(unsigned char *arr,
		   int xpos, int ypos,
		   int width, int height)
{
  int i, j;
  for(j = ypos; j < height; j++){
    if(j >= ARRHEIGHT) break;
    for(i = xpos; i < width; i++){
      if(i >= ARRWIDTH) break;
      switch(arr[i + j * ARRHEIGHT]){
      case 1:
	printf("%c", (char)35);
      default:
	printf("%c", (char)32);
      }
    }
    printf("\n");
  }
}
