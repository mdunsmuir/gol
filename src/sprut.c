#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>
#include "../include/sprut.h"

// SPRITE METHODS --------------------------------------------------

sprut_sprite sprut_load_sprite(SDL_Rect src_rect, SDL_Surface *src_surface)
{
  sprut_sprite new_sprite;
  new_sprite.src_rect = src_rect;
  new_sprite.w = src_rect.w;
  new_sprite.h = src_rect.h;
  new_sprite.src_surface = src_surface;
  return new_sprite;
}

void sprut_draw_sprite(sprut_sprite sprite,
		       int x, int y,
		       SDL_Surface *surface)
{
  SDL_Rect dst_rect;
  dst_rect.x = x;
  dst_rect.y = y;
  dst_rect.w = sprite.src_rect.w;
  dst_rect.h = sprite.src_rect.h;
  if(SDL_BlitSurface(sprite.src_surface,
		     &sprite.src_rect,
		     surface, &dst_rect) < 0)
    fprintf(stderr, "sprite blit error: %s\n", SDL_GetError());
}

// SPRITELIB METHODS --------------------------------------------------

/*
  Given some 16-bit windows .bmp file containing sprites of width w and height h,
  read the file and return a spritelib object containing those sprites.
  
  They can then be drawn to a surface using the sprut_draw_sprite() function.
 */
sprut_spritelib *sprut_read_spritelib(char *raster_filename, unsigned int w, unsigned int h)
{
  int i;
  SDL_Rect rect;
  sprut_spritelib *lib = malloc(sizeof(sprut_spritelib));

  if(lib == NULL){
    fprintf(stderr, "ERROR allocating memory in sprut_read_spritelib!");
    return NULL;
  }

  lib->src_surface = SDL_LoadBMP(raster_filename);
  Uint32 clrkey;
  clrkey = SDL_MapRGB(lib->src_surface->format, CKEYRED, CKEYGRN, CKEYBLU);
  SDL_SetColorKey(lib->src_surface, SDL_SRCCOLORKEY, clrkey);

  if(lib->src_surface == NULL){
    fprintf(stderr, "Error reading sprite lib.src_surface %s: %s", raster_filename,
	    SDL_GetError());
    return NULL;
  }

  /*
    the +1s in this line are because the source sprite image could have extra pixels
    making it a non-integer multiple of the sprite width/length. So, we do this just
    to avoid memory corruption. It's not clean but whatever.
  */
  lib->nsprites = (lib->src_surface->w / w + 1) * (lib->src_surface->h / h + 1);
  lib->sprites = malloc(sizeof(sprut_sprite) * lib->nsprites);
  if(lib->sprites == NULL){
    fprintf(stderr, "ERROR allocating memory");
    return NULL;
  }

  rect.w = w;
  rect.h = h;

  i = 0;

  for(rect.y = 0; rect.y < lib->src_surface->h; rect.y += h){
    for(rect.x = 0; rect.x < lib->src_surface->w; rect.x += w){
      lib->sprites[i] = sprut_load_sprite(rect, lib->src_surface);
      i++;
    }
  }

  return lib;
}

/*
  w and h are original height in pixels
  scalefactor scales the pixels up
  you should probably make sure these things only move around on the interval
  of the scalefactor or it's going to look super dumb
 */
sprut_spritelib *sprut_read_spritelib_with_scalefactor(char *raster_filename, unsigned int w, unsigned int h, unsigned int scalefactor)
{
  int i, j;
  Uint32 clrkey;
  SDL_Rect rect;
  sprut_spritelib *old, *lib = malloc(sizeof(sprut_spritelib));
  old = sprut_read_spritelib(raster_filename, w, h);

  if(lib == NULL){
    fprintf(stderr, "ERROR allocating memory");
    return NULL;
  }

  lib->src_surface = SDL_CreateRGBSurface(SDL_HWSURFACE, old->src_surface->w * scalefactor, old->src_surface->h * scalefactor, old->src_surface->format->BitsPerPixel, 0, 0, 0, 0);

  if(lib->src_surface == NULL){
    fprintf(stderr, "Error creating new surface: %s",
	    SDL_GetError());
    return NULL;
  }

  clrkey = SDL_MapRGB(lib->src_surface->format, CKEYRED, CKEYGRN, CKEYBLU);
  SDL_SetColorKey(lib->src_surface, SDL_SRCCOLORKEY, clrkey);

  // this is how this method is different
  // map the old pixels onto the new pixels
  
  if(SDL_MUSTLOCK(old->src_surface))
     SDL_LockSurface(old->src_surface);

  for(i = 0; i < old->src_surface->w; i++){
    for(j = 0; j < old->src_surface->h; j++){
      rect.x = i * scalefactor;
      rect.y = j * scalefactor;
      rect.w = scalefactor;
      rect.h = scalefactor;
      SDL_FillRect(lib->src_surface, &rect, getpixel(old->src_surface, i, j));
    }
  }

  if(SDL_MUSTLOCK(old->src_surface))
    SDL_UnlockSurface(old->src_surface);

  w *= scalefactor;
  h *= scalefactor;

  lib->nsprites = (lib->src_surface->w / w + 1) * (lib->src_surface->h / h + 1);
  lib->sprites = malloc(sizeof(sprut_sprite) * lib->nsprites);
  if(lib->sprites == NULL){
    fprintf(stderr, "ERROR allocating memory");
    return NULL;
  }

  rect.w = w;
  rect.h = h;
  i = 0;
  for(rect.y = 0; rect.y < lib->src_surface->h; rect.y += h){
    for(rect.x = 0; rect.x < lib->src_surface->w; rect.x += w){
      lib->sprites[i] = sprut_load_sprite(rect, lib->src_surface);
      i++;
    }
  }

  sprut_free_spritelib(old);

  return lib;
}

void sprut_free_spritelib(sprut_spritelib *lib)
{
  free(lib->sprites);
  SDL_FreeSurface(lib->src_surface);
  free(lib);
}

// BACKGROUND STUFF --------------------------------------------

/*
  What this does is obvious, I think... it blits tiles to a surface
  according to what I'm calling a .smp file, then returns the surface.
 */
SDL_Surface *sprut_read_bg_map(char *filename)
{
  unsigned int i, j, w, h, sw, sh, *map, scale;
  char cbuf[SPRUT_STRBUFLEN]; /* string buffer */
  sprut_spritelib *slib;
  SDL_Surface *surfmap;
  Uint32 clrkey;

  FILE *file = fopen(filename, "r");

  // read header with map dimensions

  if(fscanf(file, "%i %i %i %i %i %s\n", &w, &h, &sw, &sh, &scale, cbuf) != 6){
    printf("ERROR reading header of file %s: expected format <width> <height> <sprite width> <sprite height> <scale factor> <sprite file name>\n", filename);
    return NULL;
  }

  // allocate map array
  map = malloc(sizeof(unsigned int) * w * h);
  if(map == NULL){
    printf("couldn't allocate memory for map (file %s)!\n", filename);
    return NULL;
  }

  // read map array
  for(i = 0; i < w * h; i++){
    if(fscanf(file, "%i", &map[i]) != 1){
      printf("ERROR reading map records in file %s: fewer than expected (%i)!\n", filename, i);      return NULL;
    }
  }

  // get spritelib
  slib = sprut_read_spritelib_with_scalefactor(cbuf, sw, sh, scale);
  if(slib == NULL)
    return NULL;

  // now set up a new surface and blit tiles to it
  surfmap = SDL_CreateRGBSurface(SDL_HWSURFACE, w * sw * scale, h * sh * scale, slib->src_surface->format->BitsPerPixel, 0, 0, 0, 0);

  clrkey = SDL_MapRGB(surfmap->format, CKEYRED, CKEYGRN, CKEYBLU);
  SDL_SetColorKey(surfmap, SDL_SRCCOLORKEY, clrkey);

  for(j = 0; j < h; j++){
    for(i = 0; i < w; i++){
      sprut_draw_sprite(slib->sprites[map[(j * w) + i]], i * sw * scale, j * sh * scale, surfmap);
    }
  }

  sprut_free_spritelib(slib);
  fclose(file);
  free(map);
  return surfmap;
}

/*
  x and y are the co-ordinates of the background that should coincide with the top-left
  corner of the destination surface when drawn to it.

  They can be negative, and they can be greater than the size of the background. It loops.
  That's the point of this function.
*/
void sprut_draw_surface(SDL_Surface *surface, SDL_Surface *dest, int x, int y)
{
  SDL_Rect src_rect, dst_rect;
  unsigned int xcount, ycount;
  int ytemp;

  // normalize x and y to within surface
  while(x < 0)
    x += surface->w;
  while(x >= surface->w)
    x -= surface->w;

  while(y < 0)
    y += surface->h;
  while(y >= surface->h)
    y -= surface->h;

  ytemp = y;

  xcount = 0;
  ycount = 0;
  while(xcount < dest->w){
    src_rect.x = x;
    src_rect.w = (surface->w - 1) - x;
    dst_rect.x = xcount;
    dst_rect.w = src_rect.w;
    if(x != 0)
      x = 0;
    xcount += src_rect.w;
    while(ycount < dest->h){
      src_rect.y = y;
      src_rect.h = (surface->h - 1) - y;
      dst_rect.y = ycount;
      dst_rect.h = src_rect.h;
      if(y != 0)
	y = 0;
      ycount += src_rect.h;
      SDL_BlitSurface(surface, &src_rect, dest, &dst_rect);
    }
    ycount = 0;
    y = ytemp;
  }
}

void sprut_set_surface_colorkey(SDL_Surface *surface)
{
  Uint32 clrkey;
  clrkey = SDL_MapRGB(surface->format, CKEYRED, CKEYGRN, CKEYBLU);
  SDL_SetColorKey(surface, SDL_SRCCOLORKEY, clrkey);
}

/*
  Example function from SDL documentation... I'm going to use it, because I'm lazy
 */
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
  case 1:
    return *p;

  case 2:
    return *(Uint16 *)p;

  case 3:
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
      return p[0] << 16 | p[1] << 8 | p[2];
    else
      return p[0] | p[1] << 8 | p[2] << 16;

  case 4:
    return *(Uint32 *)p;

  default:
    return 0;       /* shouldn't happen, but avoids warnings */
  }
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
  case 1:
    *p = pixel;
    break;

  case 2:
    *(Uint16 *)p = pixel;
    break;

  case 3:
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
      p[0] = (pixel >> 16) & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = pixel & 0xff;
    } else {
      p[0] = pixel & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = (pixel >> 16) & 0xff;
    }
    break;

  case 4:
    *(Uint32 *)p = pixel;
    break;
  }
}
