// SPRUT
// is a library for reading in tiles and sprites from raster files on disk
// and displaying them on the screen, ala the snes or whatever.
// Sprut uses SDL to do this. However, it does not remove the need to interact
// with SDL--the user must still supply a surface to draw to.

#define SPRUT_STRBUFLEN 256

#define CKEYRED 0
#define CKEYGRN 0
#define CKEYBLU 0

// struct representing a point
typedef struct _sprut_point{
  int x, y;
} sprut_point;

// struct representing a sprite
typedef struct _sprut_sprite{
  int w, h;
  SDL_Rect src_rect; // source rect in source bitmap
  SDL_Surface *src_surface; // source bitmap surface
} sprut_sprite;

sprut_sprite sprut_load_sprite(SDL_Rect src_rect, SDL_Surface *src_surface);
void sprut_draw_sprite(sprut_sprite sprite,
		       int x, int y,
		       SDL_Surface *surface);

// a sprut_spritelib contains all the sprites read from a single tiled raster.
// one is required for each one you wish to draw sprites from.
// also used for tiling backgrounds, in a slightly different fashion
typedef struct _sprut_spritelib{
  sprut_sprite *sprites;
  int nsprites;
  SDL_Surface *src_surface;
} sprut_spritelib;

sprut_spritelib *sprut_read_spritelib(char *raster_filename, unsigned int w, unsigned int h);
sprut_spritelib *sprut_read_spritelib_with_scalefactor(char *raster_filename, unsigned int w, unsigned int h, unsigned int scalefactor);
void sprut_free_spritelib(sprut_spritelib* lib);

// background stuff

// This method returns a surface that's stitched together from a bunch of tiles,
// as specified in the *.smp file. Each layer will have one of these surfaces,
// which can be panned and maybe transformed.
SDL_Surface *sprut_read_bg_map(char *filename);
void sprut_draw_surface(SDL_Surface *surface, SDL_Surface *dest, int x, int y);
void sprut_set_surface_colorkey(SDL_Surface *surface);

/*
  These functions come from SDL documentation on the web.
  They claim that the surface must be locked before you use them. This doesn't
  seem to be the case with the first, however.
*/
Uint32 getpixel(SDL_Surface *surface, int x, int y);
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
