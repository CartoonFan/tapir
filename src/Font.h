#ifndef FONT_H
#define FONT_H

#include <stdbool.h>
#include <ruby.h>
#include <SDL.h>
#include <SDL_ttf.h>

extern VALUE rb_cFont;
extern void Init_Font(void);

struct Font {
  VALUE name;
  int size;
  bool bold, italic;
#if RGSS == 3
  bool outline;
#endif
#if RGSS >= 2
  bool shadow;
#endif
  VALUE color;
#if RGSS == 3
  VALUE out_color;
#endif
  TTF_Font *cache;
};

bool isFont(VALUE obj);
struct Font *convertFont(VALUE obj);
void rb_font_modify(VALUE obj);

VALUE rb_font_new(void);
void rb_font_set(VALUE self, VALUE other);

TTF_Font *rb_font_to_sdl(VALUE self);

#endif /* FONT_H */
