#include <bits/time.h>
#include <stdint.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_ewmh.h>
#include <unistd.h>
#include "main.h"

xcb_connection_t *c;

int main() {
  xcb_screen_t *screen;
  xcb_window_t win;
  xcb_generic_event_t *e;
  xcb_gcontext_t gc;
  xcb_font_t font;
  uint32_t mask = 0;
  uint32_t values[2];

  c = xcb_connect(NULL, NULL);
  screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;

  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;

  win = create_window(
    screen->root,
    0, 0,
    screen->width_in_pixels,
    screen->height_in_pixels,
    screen->root_visual,
    mask, values
  );
  fullscreen_window(screen->root, win);

  font = open_font("-schumacher-clean-bold-r-normal--50-0-75-75-c-0-iso646.1991-irv");
  gc = create_font_gc(win, font, screen->black_pixel, screen->white_pixel);

  xcb_flush(c);

  while ((e = xcb_wait_for_event(c))) {
    switch (e->response_type & ~0x80) {
      case XCB_EXPOSE: {
          static const char *text = "It's time to take a break!";
          static const int char_offset = 20;
          int16_t x = screen->width_in_pixels / 2 - (char_offset * strlen(text) / 2);
          int16_t y = screen->height_in_pixels / 2 - char_offset;

          xcb_image_text_8(
            c,
            strlen(text),
            win, gc,
            x, y,
            text
          );
          xcb_flush(c);

          break;
      }

      case XCB_KEY_PRESS: {
          xcb_key_press_event_t *ev = (xcb_key_press_event_t *)e;
          // 24 -> q
          static int count = 0;
          if (ev->detail == 24 && ++count >= 3) {
            free(e);
            xcb_close_font(c, font);
            xcb_disconnect(c);
            return 0;
          }

          break;
      }

      default: break;
    }

    free(e);
  } 

  xcb_close_font(c, font);
  xcb_disconnect(c);
  return 0;
}

xcb_font_t open_font(const char *fname) {
  xcb_font_t font = xcb_generate_id(c);
  xcb_open_font(c, font, strlen(fname), fname);

  return font;
}

xcb_window_t create_window(xcb_window_t pid, int16_t x, int16_t y, uint32_t width, uint32_t height, xcb_visualid_t vid, uint32_t mask, uint32_t *values) {
  xcb_window_t win = xcb_generate_id(c);

  xcb_create_window(
    c,
    XCB_COPY_FROM_PARENT,
    win,
    pid,
    x, y,
    width, height,
    0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    vid,
    mask, values
  );

  xcb_map_window(c, win);

  return win;
}

void fullscreen_window(xcb_window_t pid, xcb_window_t wid) {
  const char* state = "_NET_WM_STATE";
  xcb_intern_atom_cookie_t wm_state_cookie =
    xcb_intern_atom(c, 0, strlen(state), state);
  xcb_intern_atom_reply_t *wm_state_reply =
    xcb_intern_atom_reply(c, wm_state_cookie, NULL);
  
  const char* state_fs = "_NET_WM_STATE_FULLSCREEN";
  xcb_intern_atom_cookie_t wm_state_fs_cookie =
    xcb_intern_atom(c, 0, strlen(state_fs), state_fs);
  xcb_intern_atom_reply_t *wm_state_fs_reply =
    xcb_intern_atom_reply(c, wm_state_fs_cookie, NULL);

  xcb_client_message_event_t ev = {
    .response_type = XCB_CLIENT_MESSAGE,
    .format = 32,
    .window = wid,
    .type = wm_state_reply->atom,
    .data.data32 = {
      XCB_EWMH_WM_STATE_ADD,
      wm_state_fs_reply->atom,
      XCB_ATOM_NONE,
      0, 0
    }
  };

  xcb_send_event(
    c, 0, pid,
    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
    (const char*)&ev
  );

  free(wm_state_reply);
  free(wm_state_fs_reply);
}

xcb_gcontext_t create_font_gc(xcb_window_t wid, xcb_font_t fid, xcb_gc_t fg, xcb_gc_t bg) {
  xcb_gcontext_t gc = xcb_generate_id(c);
  uint32_t mask = 0;
  uint32_t values[3];

  mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
  values[0] = fg;
  values[1] = bg;
  values[2] = fid;

  xcb_create_gc(c, gc, wid, mask, values);

  return gc;
};
