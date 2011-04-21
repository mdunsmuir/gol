#include "SDL.h"
#include <stdlib.h>
#include <string.h>
#include "../include/sprut.h"
#include "../include/sprutext.h"

int sprut_gettextwidth(char *text)
{
  size_t len = strlen(text);
  int width = 0, i;
  for(i = 0; i < len; i++){
    if(strchr(EXTRAWIDES, text[i]) == NULL)
      width += LETTERWIDTH;
    else
      width += LETTERWIDTH + LETTERSCALE;
  }
  return width;
}

void sprut_drawstring(char *string,
		      sprut_spritelib *letters, 
		      SDL_Surface *dest, int x, int y)
{
  int i, place = x;
  size_t len = strlen(string);
  
  for(i = 0; i < len; i++){
    if(string[i] != ' ')
      sprut_draw_sprite(sprut_getlettersprite(letters, string[i]), place, y, dest);
    if(strchr(EXTRAWIDES, string[i]) == NULL)
      place += LETTERWIDTH;
    else
      place += LETTERWIDTH + LETTERSCALE;
  }
}

sprut_sprite sprut_getlettersprite(sprut_spritelib *letters,
				   char letter)
{
  // uppercase
  if(letter >= 'A' && letter <= 'Z')
    return letters->sprites[letter - 65];
  else if(letter >= 'a' && letter <= 'z')
    return letters->sprites[letter - 65 - 32];
    //    return letters->sprites[26 + letter - 97];
  else if(letter >= '0' && letter <= '9')
    return letters->sprites[52 + letter - 48];
  else if(letter == '.')
    return letters->sprites[62];
  else if(letter == '!')
    return letters->sprites[63];
  else if(letter == '?')
    return letters->sprites[64];
  else if(letter == '(')
    return letters->sprites[65];
  else if(letter == ')')
    return letters->sprites[66];
  else if(letter == ',')
    return letters->sprites[67];
  else
    return letters->sprites[0];
}
