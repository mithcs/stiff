#include <xcb/xproto.h>

xcb_font_t open_font(const char *fname);
xcb_window_t create_window(xcb_window_t pid, int16_t x, int16_t y, uint32_t width, uint32_t height, xcb_visualid_t vid, uint32_t mask, uint32_t *values);
void fullscreen_window(xcb_window_t pid, xcb_window_t wid);
xcb_gcontext_t create_font_gc(xcb_window_t wid, xcb_font_t fid, xcb_gc_t fg, xcb_gc_t bg);
