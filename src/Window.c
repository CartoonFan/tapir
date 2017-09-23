#include <SDL.h>
#include "gl_misc.h"
#include "Window.h"
#include "Bitmap.h"
#include "Viewport.h"
#include "Rect.h"
#include "Color.h"
#include "Tone.h"
#include "misc.h"

static GLuint shader1;
#if RGSS >= 2
static GLuint shader2;
#endif
static GLuint shader3;
static GLuint shader4;

static void window_mark(struct Window *ptr);
static void window_free(struct Window *ptr);
static VALUE window_alloc(VALUE klass);

static VALUE rb_window_m_initialize(int argc, VALUE *argv, VALUE self);
static VALUE rb_window_m_initialize_copy(VALUE self, VALUE orig);
static VALUE rb_window_m_dispose(VALUE self);
static VALUE rb_window_m_disposed_p(VALUE self);
#if RGSS == 3
static VALUE rb_window_m_move(
    VALUE self, VALUE x, VALUE y, VALUE width, VALUE height);
static VALUE rb_window_m_open_p(VALUE self);
static VALUE rb_window_m_close_p(VALUE self);
#endif
static VALUE rb_window_m_visible(VALUE self);
static VALUE rb_window_m_set_visible(VALUE self, VALUE newval);
static VALUE rb_window_m_windowskin(VALUE self);
static VALUE rb_window_m_set_windowskin(VALUE self, VALUE newval);
static VALUE rb_window_m_contents(VALUE self);
static VALUE rb_window_m_set_contents(VALUE self, VALUE newval);
static VALUE rb_window_m_x(VALUE self);
static VALUE rb_window_m_set_x(VALUE self, VALUE newval);
static VALUE rb_window_m_y(VALUE self);
static VALUE rb_window_m_set_y(VALUE self, VALUE newval);
static VALUE rb_window_m_width(VALUE self);
static VALUE rb_window_m_set_width(VALUE self, VALUE newval);
static VALUE rb_window_m_height(VALUE self);
static VALUE rb_window_m_set_height(VALUE self, VALUE newval);
static VALUE rb_window_m_z(VALUE self);
static VALUE rb_window_m_set_z(VALUE self, VALUE newval);
static VALUE rb_window_m_ox(VALUE self);
static VALUE rb_window_m_set_ox(VALUE self, VALUE newval);
static VALUE rb_window_m_oy(VALUE self);
static VALUE rb_window_m_set_oy(VALUE self, VALUE newval);
#if RGSS == 3
static VALUE rb_window_m_padding(VALUE self);
static VALUE rb_window_m_set_padding(VALUE self, VALUE newval);
static VALUE rb_window_m_padding_bottom(VALUE self);
static VALUE rb_window_m_set_padding_bottom(VALUE self, VALUE newval);
#endif
#if RGSS >= 2
static VALUE rb_window_m_openness(VALUE self);
static VALUE rb_window_m_set_openness(VALUE self, VALUE newval);
#endif

static void renderWindow(struct Renderable *renderable);

VALUE rb_cWindow;

/*
 * A graphic object containing a bitmap.
 */
void Init_Window(void) {
  rb_cWindow = rb_define_class("Window", rb_cObject);
  rb_define_alloc_func(rb_cWindow, window_alloc);
  rb_define_private_method(rb_cWindow, "initialize",
      rb_window_m_initialize, -1);
  rb_define_private_method(rb_cWindow, "initialize_copy",
      rb_window_m_initialize_copy, 1);
  rb_define_method(rb_cWindow, "dispose", rb_window_m_dispose, 0);
  rb_define_method(rb_cWindow, "disposed?", rb_window_m_disposed_p, 0);
#if RGSS == 3
  rb_define_method(rb_cWindow, "move", rb_window_m_move, 4);
  rb_define_method(rb_cWindow, "open?", rb_window_m_open_p, 0);
  rb_define_method(rb_cWindow, "close?", rb_window_m_close_p, 0);
#endif
  rb_define_method(rb_cWindow, "windowskin", rb_window_m_windowskin, 0);
  rb_define_method(rb_cWindow, "windowskin=", rb_window_m_set_windowskin, 1);
  rb_define_method(rb_cWindow, "contents", rb_window_m_contents, 0);
  rb_define_method(rb_cWindow, "contents=", rb_window_m_set_contents, 1);
  rb_define_method(rb_cWindow, "visible", rb_window_m_visible, 0);
  rb_define_method(rb_cWindow, "visible=", rb_window_m_set_visible, 1);
  rb_define_method(rb_cWindow, "x", rb_window_m_x, 0);
  rb_define_method(rb_cWindow, "x=", rb_window_m_set_x, 1);
  rb_define_method(rb_cWindow, "y", rb_window_m_y, 0);
  rb_define_method(rb_cWindow, "y=", rb_window_m_set_y, 1);
  rb_define_method(rb_cWindow, "width", rb_window_m_width, 0);
  rb_define_method(rb_cWindow, "width=", rb_window_m_set_width, 1);
  rb_define_method(rb_cWindow, "height", rb_window_m_height, 0);
  rb_define_method(rb_cWindow, "height=", rb_window_m_set_height, 1);
  rb_define_method(rb_cWindow, "z", rb_window_m_z, 0);
  rb_define_method(rb_cWindow, "z=", rb_window_m_set_z, 1);
  rb_define_method(rb_cWindow, "ox", rb_window_m_ox, 0);
  rb_define_method(rb_cWindow, "ox=", rb_window_m_set_ox, 1);
  rb_define_method(rb_cWindow, "oy", rb_window_m_oy, 0);
  rb_define_method(rb_cWindow, "oy=", rb_window_m_set_oy, 1);
#if RGSS == 3
  rb_define_method(rb_cWindow, "padding", rb_window_m_padding, 0);
  rb_define_method(rb_cWindow, "padding=", rb_window_m_set_padding, 1);
  rb_define_method(rb_cWindow, "padding_bottom",
      rb_window_m_padding_bottom, 0);
  rb_define_method(rb_cWindow, "padding_bottom=",
      rb_window_m_set_padding_bottom, 1);
#endif
#if RGSS >= 2
  rb_define_method(rb_cWindow, "openness", rb_window_m_openness, 0);
  rb_define_method(rb_cWindow, "openness=", rb_window_m_set_openness, 1);
#endif
  // TODO: implement Window#update
  // TODO: implement Window#contents, Window#contents=
  // TODO: implement Window#cursor_rect
  // TODO: implement Window#viewport
  // TODO: implement Window#active
  // TODO: implement Window#arrows_visible
  // TODO: implement Window#pause
  // TODO: implement Window#ox
  // TODO: implement Window#oy
  // TODO: implement Window#padding
  // TODO: implement Window#padding_bottom
  // TODO: implement Window#opacity
  // TODO: implement Window#back_opacity
  // TODO: implement Window#contents_opacity
  // TODO: implement Window#tone
}

bool rb_window_data_p(VALUE obj) {
  if(TYPE(obj) != T_DATA) return false;
  return RDATA(obj)->dmark == (void(*)(void*))window_mark;
}

const struct Window *rb_window_data(VALUE obj) {
  Check_Type(obj, T_DATA);
  // Note: original RGSS doesn't check types.
  if(RDATA(obj)->dmark != (void(*)(void*))window_mark) {
    rb_raise(rb_eTypeError,
        "can't convert %s into Window",
        rb_class2name(rb_obj_class(obj)));
  }
  struct Window *ret;
  Data_Get_Struct(obj, struct Window, ret);
  return ret;
}

struct Window *rb_window_data_mut(VALUE obj) {
  // Note: original RGSS doesn't check frozen.
  if(OBJ_FROZEN(obj)) rb_error_frozen("Window");
  return (struct Window *)rb_window_data(obj);
}

static void window_mark(struct Window *ptr) {
  rb_gc_mark(ptr->renderable.viewport);
  rb_gc_mark(ptr->windowskin);
  rb_gc_mark(ptr->contents);
}

static void window_free(struct Window *ptr) {
  unregisterRenderable(&ptr->renderable);
  xfree(ptr);
}

static VALUE window_alloc(VALUE klass) {
  struct Window *ptr = ALLOC(struct Window);
  ptr->renderable.render = renderWindow;
#if RGSS == 3
  ptr->renderable.z = 100;
#else
  ptr->renderable.z = 0;
#endif
  ptr->renderable.viewport = Qnil;
  ptr->windowskin = Qnil;
  ptr->contents = rb_bitmap_new(1, 1);
  ptr->disposed = false;
  ptr->visible = true;
  ptr->x = 0;
  ptr->y = 0;
  ptr->width = 0;
  ptr->height = 0;
  ptr->ox = 0;
  ptr->oy = 0;
#if RGSS == 3
  ptr->padding = 12;
  ptr->padding_bottom = 12;
#endif
#if RGSS >= 2
  ptr->openness = 255;
#endif
  registerRenderable(&ptr->renderable);
  VALUE ret = Data_Wrap_Struct(klass, window_mark, window_free, ptr);
  return ret;
}

/*
 * call-seq:
 *   Window.new
 *   Window.new(x)
 *   Window.new(x, y)
 *   Window.new(x, y, width)
 *   Window.new(x, y, width, height)
 *
 * Creates a new window.
 */
static VALUE rb_window_m_initialize(int argc, VALUE *argv, VALUE self) {
  struct Window *ptr = rb_window_data_mut(self);
  if(argc > 4) {
    rb_raise(rb_eArgError,
        "wrong number of arguments (%d for 4)", argc);
  }
  ptr->x = argc > 0 ? NUM2INT(argv[0]) : 0;
  ptr->y = argc > 1 ? NUM2INT(argv[1]) : 0;
  ptr->width = argc > 2 ? NUM2INT(argv[2]) : 0;
  ptr->height = argc > 3 ? NUM2INT(argv[3]) : 0;
  return Qnil;
}

static VALUE rb_window_m_initialize_copy(VALUE self, VALUE orig) {
  struct Window *ptr = rb_window_data_mut(self);
  const struct Window *orig_ptr = rb_window_data(orig);
  ptr->renderable.z = orig_ptr->renderable.z;
  ptr->renderable.viewport = orig_ptr->renderable.viewport;
  ptr->windowskin = orig_ptr->windowskin;
  ptr->contents = orig_ptr->contents;
  ptr->disposed = orig_ptr->disposed;
  ptr->visible = orig_ptr->visible;
  ptr->x = orig_ptr->x;
  ptr->y = orig_ptr->y;
  ptr->width = orig_ptr->width;
  ptr->height = orig_ptr->height;
  ptr->ox = orig_ptr->ox;
  ptr->oy = orig_ptr->oy;
#if RGSS == 3
  ptr->padding = orig_ptr->padding;
  ptr->padding_bottom = orig_ptr->padding_bottom;
#endif
#if RGSS >= 2
  ptr->openness = orig_ptr->openness;
#endif
  return Qnil;
}

static VALUE rb_window_m_dispose(VALUE self) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->disposed = true;
  return Qnil;
}

static VALUE rb_window_m_disposed_p(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->disposed ? Qtrue : Qfalse;
}

#if RGSS == 3
static VALUE rb_window_m_move(
    VALUE self, VALUE x, VALUE y, VALUE width, VALUE height) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->x = NUM2INT(x);
  ptr->y = NUM2INT(y);
  ptr->width = NUM2INT(width);
  ptr->height = NUM2INT(height);
  return Qnil;
}

static VALUE rb_window_m_open_p(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->openness == 255 ? Qtrue : Qfalse;
}

static VALUE rb_window_m_close_p(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->openness == 0 ? Qtrue : Qfalse;
}
#endif

static VALUE rb_window_m_windowskin(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->windowskin;
}

static VALUE rb_window_m_set_windowskin(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  if(newval != Qnil) rb_bitmap_data(newval);
  ptr->windowskin = newval;
  return newval;
}

static VALUE rb_window_m_contents(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->contents;
}

static VALUE rb_window_m_set_contents(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  if(newval != Qnil) rb_bitmap_data(newval);
  ptr->contents = newval;
  return newval;
}

static VALUE rb_window_m_visible(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return ptr->visible ? Qtrue : Qfalse;
}

static VALUE rb_window_m_set_visible(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->visible = RTEST(newval);
  return newval;
}

static VALUE rb_window_m_x(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->x);
}

static VALUE rb_window_m_set_x(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->x = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_y(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->y);
}

static VALUE rb_window_m_set_y(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->y = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_width(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->width);
}

static VALUE rb_window_m_set_width(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->width = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_height(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->height);
}

static VALUE rb_window_m_set_height(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->height = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_z(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->renderable.z);
}

static VALUE rb_window_m_set_z(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->renderable.z = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_ox(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->ox);
}

static VALUE rb_window_m_set_ox(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->ox = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_oy(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->oy);
}

static VALUE rb_window_m_set_oy(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->oy = NUM2INT(newval);
  return newval;
}

#if RGSS == 3
static VALUE rb_window_m_padding(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->padding);
}

static VALUE rb_window_m_set_padding(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->padding_bottom = ptr->padding = NUM2INT(newval);
  return newval;
}

static VALUE rb_window_m_padding_bottom(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->padding_bottom);
}

static VALUE rb_window_m_set_padding_bottom(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->padding_bottom = NUM2INT(newval);
  return newval;
}
#endif

#if RGSS >= 2
static VALUE rb_window_m_openness(VALUE self) {
  const struct Window *ptr = rb_window_data(self);
  return INT2NUM(ptr->openness);
}

static VALUE rb_window_m_set_openness(VALUE self, VALUE newval) {
  struct Window *ptr = rb_window_data_mut(self);
  ptr->openness = saturateInt32(NUM2INT(newval), 0, 255);
  return newval;
}
#endif

static void renderWindow(struct Renderable *renderable) {
  struct Window *ptr = (struct Window *)renderable;
  if(ptr->renderable.viewport != Qnil) WARN_UNIMPLEMENTED("Window#viewport");
  if(ptr->disposed || !ptr->visible) return;
#if RGSS >= 2
  int openness = ptr->openness;
#else
  int openness = 255;
#endif
  if(openness == 0) return;
  if(openness < 255) WARN_UNIMPLEMENTED("Window#openness");

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if(ptr->windowskin != Qnil) {
    const struct Bitmap *skin_bitmap_ptr = rb_bitmap_data(ptr->windowskin);
    SDL_Surface *skin_surface = skin_bitmap_ptr->surface;
    if(!skin_surface) return;

    glActiveTexture(GL_TEXTURE0);
    bitmapBindTexture((struct Bitmap *)skin_bitmap_ptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(shader1);
    glUniform1i(glGetUniformLocation(shader1, "windowskin"), 0);
    glUniform2f(glGetUniformLocation(shader1, "resolution"),
        window_width, window_height);

    gl_draw_rect(
        ptr->x + 2, ptr->y + 2,
        ptr->x + ptr->width - 2, ptr->y + ptr->height - 2,
        0.0, 0.0, 1.0, 1.0);

#if RGSS >= 2
    glUseProgram(shader2);
    glUniform1i(glGetUniformLocation(shader2, "windowskin"), 0);
    glUniform2f(glGetUniformLocation(shader2, "resolution"),
        window_width, window_height);

    gl_draw_rect(
        ptr->x + 2, ptr->y + 2,
        ptr->x + ptr->width - 2, ptr->y + ptr->height - 2,
        0.0, 0.0, (ptr->width - 2) / 64.0, (ptr->height - 2) / 64.0);
#endif

    glUseProgram(shader3);
    glUniform1i(glGetUniformLocation(shader3, "windowskin"), 0);
    glUniform2f(glGetUniformLocation(shader3, "resolution"),
        window_width, window_height);
    glUniform2f(glGetUniformLocation(shader3, "bg_size"),
        ptr->width, ptr->height);

    gl_draw_rect(
        ptr->x, ptr->y, ptr->x + ptr->width, ptr->y + ptr->height,
        0.0, 0.0, ptr->width, ptr->height);
  }

  if(ptr->contents != Qnil && openness == 255) {
    const struct Bitmap *contents_bitmap_ptr = rb_bitmap_data(ptr->contents);
    SDL_Surface *contents_surface = contents_bitmap_ptr->surface;
    if(!contents_surface) return;

#if RGSS == 3
    int padding = ptr->padding;
    int padding_bottom = ptr->padding_bottom;
#else
    // TODO: is the padding correct?
    int padding = 16;
    int padding_bottom = 16;
#endif

    int wcontent_width = ptr->width - padding * 2;
    int wcontent_height = ptr->height - padding - padding_bottom;
    int content_width = contents_surface->w;
    int content_height = contents_surface->h;
    int clip_left = ptr->ox;
    if(clip_left < 0) clip_left = 0;
    int clip_top = ptr->oy;
    if(clip_top < 0) clip_top = 0;
    int clip_right = ptr->ox + wcontent_width;
    if(clip_right > content_width) clip_right = content_width;
    int clip_bottom = ptr->oy + wcontent_height;
    if(clip_bottom > content_height) clip_bottom = content_height;

    glUseProgram(shader4);
    glUniform1i(glGetUniformLocation(shader4, "contents"), 0);
    glUniform2f(glGetUniformLocation(shader4, "resolution"),
        window_width, window_height);

    glActiveTexture(GL_TEXTURE0);
    bitmapBindTexture((struct Bitmap *)contents_bitmap_ptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    gl_draw_rect(
        ptr->x + padding + (clip_left - ptr->ox),
        ptr->y + padding + (clip_top - ptr->oy),
        ptr->x + padding + (clip_right - ptr->ox),
        ptr->y + padding + (clip_bottom - ptr->oy),
        (double)clip_left / content_width,
        (double)clip_top / content_height,
        (double)clip_right / content_width,
        (double)clip_bottom / content_height);
  }

  glUseProgram(0);
}

void initWindowSDL() {
  static const char *vsh1_source =
    "#version 120\n"
    "\n"
    "uniform vec2 resolution;\n"
    "\n"
    "void main(void) {\n"
    "    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "    gl_Position.x = gl_Vertex.x / resolution.x * 2.0 - 1.0;\n"
    "    gl_Position.y = 1.0 - gl_Vertex.y / resolution.y * 2.0;\n"
    "    gl_Position.zw = vec2(0.0, 1.0);\n"
    "}\n";

  static const char *fsh1_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "\n"
    "void main(void) {\n"
#if RGSS >= 2
    "    vec4 color = texture2D(windowskin, vec2(gl_TexCoord[0].x * 0.5, gl_TexCoord[0].y * 0.5));\n"
#else
    "    vec4 color = texture2D(windowskin, vec2(gl_TexCoord[0].x * (2.0 / 3.0), gl_TexCoord[0].y));\n"
#endif
    "    gl_FragColor = color;\n"
    "}\n";

  shader1 = compileShaders(vsh1_source, fsh1_source);

#if RGSS >= 2
  static const char *vsh2_source =
    "#version 120\n"
    "\n"
    "uniform vec2 resolution;\n"
    "\n"
    "void main(void) {\n"
    "    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "    gl_Position.x = gl_Vertex.x / resolution.x * 2.0 - 1.0;\n"
    "    gl_Position.y = 1.0 - gl_Vertex.y / resolution.y * 2.0;\n"
    "    gl_Position.zw = vec2(0.0, 1.0);\n"
    "}\n";

  static const char *fsh2_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "\n"
    "void main(void) {\n"
    "    vec2 coord = mod(gl_TexCoord[0].xy, 1.0);\n"
#if RGSS >= 2
    "    vec4 color = texture2D(windowskin, vec2(coord.x * 0.5, coord.y * 0.5 + 0.5));\n"
#else
    "    vec4 color = texture2D(windowskin, vec2(coord.x * (2.0 / 3.0), coord.y));\n"
#endif
    "    gl_FragColor = color;\n"
    "}\n";

  shader2 = compileShaders(vsh2_source, fsh2_source);
#endif

  static const char *vsh3_source =
    "#version 120\n"
    "\n"
    "uniform vec2 resolution;\n"
    "\n"
    "void main(void) {\n"
    "    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "    gl_Position.x = gl_Vertex.x / resolution.x * 2.0 - 1.0;\n"
    "    gl_Position.y = 1.0 - gl_Vertex.y / resolution.y * 2.0;\n"
    "    gl_Position.zw = vec2(0.0, 1.0);\n"
    "}\n";

  static const char *fsh3_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "uniform vec2 bg_size;\n"
    "\n"
    "void main(void) {\n"
    "    vec2 coord = gl_TexCoord[0].xy;\n"
    "    vec2 reverse_coord = bg_size - coord;\n"
    "    vec2 src_coord;\n"
    "    bool draw = false;\n"
    "    if(coord.x < 16.0) {\n"
    "      src_coord.x = coord.x;\n"
    "      draw = true;\n"
    "    } else if(reverse_coord.x < 16.0) {\n"
    "      src_coord.x = 64.0 - reverse_coord.x;\n"
    "      draw = true;\n"
    "    } else {\n"
    "      src_coord.x = mod(coord.x - 16.0, 32.0) + 16.0;\n"
    "    }\n"
    "    if(coord.y < 16.0) {\n"
    "      src_coord.y = coord.y;\n"
    "      draw = true;\n"
    "    } else if(reverse_coord.y < 16.0) {\n"
    "      src_coord.y = 64.0 - reverse_coord.y;\n"
    "      draw = true;\n"
    "    } else {\n"
    "      src_coord.y = mod(coord.y - 16.0, 32.0) + 16.0;\n"
    "    }\n"
#if RGSS >= 2
    "    src_coord.x = (src_coord.x + 64.0) / 128.0;\n"
    "    src_coord.y = src_coord.y / 128.0;\n"
#else
    "    src_coord.x = (src_coord.x + 128.0) / 192.0;\n"
    "    src_coord.y = src_coord.y / 128.0;\n"
#endif
    "    if(draw) {\n"
    "      gl_FragColor = texture2D(windowskin, src_coord);\n"
    "    } else {\n"
    "      gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
    "    }\n"
    "}\n";

  shader3 = compileShaders(vsh3_source, fsh3_source);

  static const char *vsh4_source =
    "#version 120\n"
    "\n"
    "uniform vec2 resolution;\n"
    "\n"
    "void main(void) {\n"
    "    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "    gl_Position.x = gl_Vertex.x / resolution.x * 2.0 - 1.0;\n"
    "    gl_Position.y = 1.0 - gl_Vertex.y / resolution.y * 2.0;\n"
    "    gl_Position.zw = vec2(0.0, 1.0);\n"
    "}\n";

  static const char *fsh4_source =
    "#version 120\n"
    "#if __VERSION__ >= 130\n"
    "#define texture2D texture\n"
    "#define texture2DProj textureProj\n"
    "#endif\n"
    "\n"
    "uniform sampler2D windowskin;\n"
    "\n"
    "void main(void) {\n"
    "    vec4 color = texture2D(windowskin, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));\n"
    "    gl_FragColor = color;\n"
    "}\n";

  shader4 = compileShaders(vsh4_source, fsh4_source);

}

void deinitWindowSDL() {
  if(shader4) glDeleteProgram(shader4);
  if(shader3) glDeleteProgram(shader3);
#if RGSS >= 2
  if(shader2) glDeleteProgram(shader2);
#endif
  if(shader1) glDeleteProgram(shader1);
}
