﻿#include "gtest/gtest.h"
#include "base/button.h"
#include "base/window.h"
#include "designer_support/style_mutable.h"

#include <string>

using std::string;

static ret_t on_style_item(void* ctx, uint32_t widget_state, style_id_t id, const value_t* val) {
  string& log = *(string*)ctx;
  char str[128];
  if (val->type == VALUE_TYPE_STRING) {
    snprintf(str, sizeof(str), "%d,%d,\"%s\";", (int)widget_state, (int)id, value_str(val));
  } else {
    snprintf(str, sizeof(str), "%d,%d,%d;", (int)widget_state, (int)id, value_int(val));
  }
  log += str;

  return RET_OK;
}

TEST(StyleMutable, basic) {
  int32_t i = 0;
  int32_t k = 0;
  int32_t n = 100;
  color_t fg = color_init(0, 0, 0, 0);
  ;
  color_t trans = color_init(0, 0, 0, 0);
  ;
  widget_t* w = window_create(NULL, 10, 20, 30, 40);
  widget_t* b = button_create(w, 0, 0, 100, 100);
  style_mutable_t* s = (style_mutable_t*)style_mutable_create(b);
  style_t* style = (style_t*)s;

  for (k = WIDGET_STATE_NORMAL; k < 3; k++) {
    char name[32];
    snprintf(name, sizeof(name), "name%d", k);
    ASSERT_EQ(style_mutable_set_name(style, name), RET_OK);
    ASSERT_EQ(string(style_mutable_get_name(style)), string(name));
    widget_set_state(b, (widget_state_t)k);
    style_notify_widget_state_changed(style, b);
    widget_state_t state = (widget_state_t)widget_get_prop_int(b, WIDGET_PROP_STATE_FOR_STYLE, 0);
    for (i = 0; i < n; i++) {
      char font_name[32];
      ASSERT_EQ(style_mutable_set_int(style, state, STYLE_ID_FONT_SIZE, i + 1), RET_OK);
      ASSERT_EQ(style_get_int(style, STYLE_ID_FONT_SIZE, 0), i + 1);

      fg.color = 0xffff + 1;
      ASSERT_EQ(style_mutable_set_color(style, state, STYLE_ID_FG_COLOR, fg), RET_OK);
      ASSERT_EQ(style_get_color(style, STYLE_ID_FG_COLOR, trans).color, fg.color);

      snprintf(font_name, sizeof(font_name), "font%d", i);
      ASSERT_EQ(style_mutable_set_str(style, state, STYLE_ID_FONT_NAME, font_name), RET_OK);
      ASSERT_EQ(string(style_get_str(style, STYLE_ID_FONT_NAME, "")), string(font_name));
    }
  }

  string str = string(style_mutable_get_name(style)) + string(":");
  style_mutable_foreach(style, on_style_item, &str);
  ASSERT_EQ(str, "name2:1,4,100;1,2,65536;1,3,\"font99\";2,4,100;2,2,65536;2,3,\"font99\";");

  style_destroy(style);
  widget_destroy(w);
}
