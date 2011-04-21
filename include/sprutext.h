/*
  Header for sprut text functions
*/

#define LETTERSCALE 2
#define LETTERWIDTH (LETTERSCALE * 9)
#define LETTERHEIGHT (LETTERSCALE * 8)
#define EXTRAWIDES ""

int sprut_gettextwidth(char *text);
void sprut_drawstring(char *string,
		      sprut_spritelib *letters, 
		      SDL_Surface *dest, int x, int y);

/*
  don't see why you would use this externally
*/
sprut_sprite sprut_getlettersprite(sprut_spritelib *letters,
				   char letter);
