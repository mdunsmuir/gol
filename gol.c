#include <stdlib.h>
#include <stdio.h>

#define ARRWIDTH 22
#define ARRHEIGHT 22

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
  unsigned char flip[ARRWIDTH * ARRHEIGHT], flop[ARRWIDTH * ARRHEIGHT], count;
  unsigned char *cur, *evo;
  gol_state state = gol_state_run;
  gol_flipflop ff = gol_flip;

  /*
    fill array with zeroes
  */
  for(i = 0; i < ARRWIDTH * ARRHEIGHT; i++)
    flip[i] = 0;

  flip[113] = 1;
  flip[114] = 1;
  flip[115] = 1;

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
    ascii_display(cur, 0, 0, ARRWIDTH, ARRHEIGHT);
    for(i = 0; i < ARRWIDTH; i++){
      for(j = 0; j < ARRHEIGHT; j++){
	count = 0;
	for(ni = 0; ni < 3; ni++){
	  xpos = i - 1 + ni;
	  if(xpos < 0) continue;
	  if(xpos >= ARRWIDTH) break;
	  for(nj = 0; nj < 3; nj++){
	    ypos = j - 1 + nj;
	    if(ypos < 0 || (ypos == j && xpos == i)) continue;
	    else if(ypos >= ARRHEIGHT) break;
	    count += cur[xpos + ypos * ARRWIDTH];
	  }
	}
	//	  if(count > 0) printf("%i, %i, %i\n", i, j, count);
	if(cur[i + j * ARRWIDTH]){ // live cell
	  if(count < 2)
	    evo[i + j * ARRWIDTH] = 0;
	  else if(count <= 3)
	    evo[i + j * ARRWIDTH] = 1;
	  else
	    evo[i + j * ARRWIDTH] = 0;
	} else{ //dead cell
	  if(count == 3)
	    evo[i + j * ARRWIDTH] = 1;
	  else
	    evo[i + j * ARRWIDTH] = 0;
	}
      }
    }
    sleep(1);
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
