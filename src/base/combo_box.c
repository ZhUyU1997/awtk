﻿/**
 * File:   combo_box.h
 * Author: AWTK Develop Team
 * Brief:  combo_box
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-07-25 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "base/mem.h"
#include "base/utils.h"
#include "base/button.h"
#include "base/layout.h"
#include "base/popup.h"
#include "base/window.h"
#include "base/combo_box.h"
#include "base/tokenizer.h"
#include "base/combo_box_item.h"

const char* s_combo_box_properties[] = {WIDGET_PROP_MIN,
                                        WIDGET_PROP_MAX,
                                        WIDGET_PROP_STEP,
                                        WIDGET_PROP_INPUT_TYPE,
                                        WIDGET_PROP_READONLY,
                                        WIDGET_PROP_AUTO_FIX,
                                        WIDGET_PROP_MARGIN,
                                        WIDGET_PROP_LEFT_MARGIN,
                                        WIDGET_PROP_RIGHT_MARGIN,
                                        WIDGET_PROP_TOP_MARGIN,
                                        WIDGET_PROP_BOTTOM_MARGIN,
                                        WIDGET_PROP_TIPS,
                                        WIDGET_PROP_OPEN_WINDOW,
                                        WIDGET_PROP_SELECTED_INDEX,
                                        NULL};

widget_t* combo_box_create_self(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h);

static ret_t combo_box_destroy(widget_t* widget) {
  combo_box_t* combo_box = COMBO_BOX(widget);

  str_reset(&(combo_box->text));
  combo_box_reset_options(widget);
  TKMEM_FREE(combo_box->open_window);

  return RET_OK;
}

static ret_t combo_box_get_prop(widget_t* widget, const char* name, value_t* v) {
  combo_box_t* combo_box = COMBO_BOX(widget);

  if (tk_str_eq(name, WIDGET_PROP_OPEN_WINDOW)) {
    value_set_str(v, combo_box->open_window);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_SELECTED_INDEX)) {
    value_set_int(v, combo_box->selected_index);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_OPTIONS)) {
    value_set_str(v, combo_box->options);
    return RET_OK;
  } else {
    return edit_get_prop(widget, name, v);
  }
}

ret_t combo_box_parse_options(widget_t* widget, const char* str) {
  int32_t i = 0;
  tokenizer_t tokenizer;
  tokenizer_t* t = &tokenizer;
  combo_box_t* combo_box = COMBO_BOX(widget);

  combo_box_reset_options(widget);
  combo_box->options = tk_strdup(str);
  tokenizer_init(t, str, strlen(str), ";");

  while (tokenizer_has_more(t)) {
    int32_t value = 0;
    const char* token = tokenizer_next(t);
    if (*token) {
      const char* text = strchr(token, ':');
      if (text != NULL) {
        text++;
        value = tk_atoi(token);
      } else {
        text = token;
        value = i;
      }

      combo_box_append_option(widget, value, text);
      i++;
    }
  }
  tokenizer_deinit(t);

  return RET_OK;
}

ret_t combo_box_set_options(widget_t* widget, const char* options) {
  return combo_box_parse_options(widget, options);
}

static ret_t combo_box_set_prop(widget_t* widget, const char* name, const value_t* v) {
  if (tk_str_eq(name, WIDGET_PROP_OPEN_WINDOW)) {
    combo_box_set_open_window(widget, value_str(v));
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_SELECTED_INDEX)) {
    combo_box_set_selected_index(widget, value_int(v));
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_OPTIONS)) {
    combo_box_parse_options(widget, value_str(v));
    return RET_OK;
  } else {
    return edit_set_prop(widget, name, v);
  }
}

static const widget_vtable_t s_combo_box_vtable = {.size = sizeof(edit_t),
                                                   .type = WIDGET_TYPE_COMBO_BOX,
                                                   .clone_properties = s_combo_box_properties,
                                                   .persistent_properties = s_combo_box_properties,
                                                   .create = combo_box_create_self,
                                                   .on_paint_self = edit_on_paint_self,
                                                   .set_prop = combo_box_set_prop,
                                                   .get_prop = combo_box_get_prop,
                                                   .destroy = combo_box_destroy,
                                                   .on_event = edit_on_event};

widget_t* combo_box_create_self(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  combo_box_t* combo_box = TKMEM_ZALLOC(combo_box_t);
  edit_t* edit = EDIT(combo_box);
  return_value_if_fail(combo_box != NULL, NULL);

  edit_init(parent, edit, x, y, w, h, &s_combo_box_vtable);

  edit->right_margin = h;
  edit->left_margin = 4;
  str_init(&(combo_box->text), 32);

  return WIDGET(combo_box);
}

static ret_t combo_box_on_item_click(void* ctx, event_t* e) {
  combo_box_t* combo_box = COMBO_BOX(ctx);
  widget_t* widget = WIDGET(combo_box);
  widget_t* item = WIDGET(e->target);
  int32_t index = widget_index_of(item);
  int32_t old_index = combo_box->selected_index;

  combo_box->value = COMBO_BOX_ITEM(item)->value;
  combo_box_set_selected_index(WIDGET(combo_box), index);

  if (item->tr_text != NULL) {
    widget_set_tr_text(widget, item->tr_text);
  } else {
    widget_set_text(widget, item->text.str);
  }

  if (old_index != index) {
    event_t e = event_init(EVT_VALUE_CHANGED, widget);
    widget_dispatch(widget, &e);
  }

  window_close(widget_get_window(item));

  return RET_OK;
}

static ret_t combo_box_visit_item(void* ctx, void* data) {
  widget_t* iter = WIDGET(data);
  combo_box_t* combo_box = COMBO_BOX(ctx);

  if (tk_str_eq(widget_get_type(iter), WIDGET_TYPE_COMBO_BOX_ITEM)) {
    int32_t index = widget_index_of(iter);

    widget_on(iter, EVT_CLICK, combo_box_on_item_click, combo_box);
    if (index == combo_box->selected_index) {
      COMBO_BOX_ITEM(iter)->checked = TRUE;
      widget_update_style(iter);
    }
  }

  return RET_OK;
}

static ret_t combo_box_hook_items(combo_box_t* combo_box, widget_t* popup) {
  return widget_foreach(popup, combo_box_visit_item, combo_box);
}

static ret_t combo_box_create_popup_items(combo_box_t* combo_box, widget_t* parent) {
  combo_box_option_t* iter = NULL;
  return_value_if_fail(combo_box != NULL, RET_BAD_PARAMS);

  iter = combo_box->option_items;
  while (iter != NULL) {
    widget_t* item = combo_box_item_create(parent, 0, 0, 0, 0);

    widget_set_value(item, iter->value);
    widget_set_tr_text(item, iter->text);

    iter = iter->next;
  }

  return RET_OK;
}

static widget_t* combo_box_create_popup(combo_box_t* combo_box) {
  value_t v;
  children_layout_t cl;
  widget_t* widget = WIDGET(combo_box);
  int32_t margin = 2;
  int32_t item_height = 30;
  int32_t nr = combo_box_count_options(widget);
  int32_t w = widget->w;
  int32_t h = nr * item_height + 2 * margin;
  widget_t* win = popup_create(NULL, 0, 0, w, h);

  value_set_bool(&v, TRUE);
  widget_set_prop(win, WIDGET_PROP_CLOSE_WHEN_CLICK_OUTSIDE, &v);

  memset(&cl, 0x00, sizeof(cl));

  cl.inited = TRUE;
  cl.x_margin = margin;
  cl.y_margin = margin;
  cl.rows = nr;
  cl.cols = 1;

  widget_set_children_layout(win, &cl);
  combo_box_create_popup_items(combo_box, win);
  widget_layout(win);

  return win;
}

static ret_t combo_box_on_button_click(void* ctx, event_t* e) {
  point_t p;
  widget_t* wm = NULL;
  widget_t* win = NULL;
  widget_t* widget = WIDGET(ctx);
  combo_box_t* combo_box = COMBO_BOX(ctx);

  if (combo_box->open_window) {
    win = window_open(combo_box->open_window);
    widget_resize(win, widget->w, win->h);
    widget_layout_children(win);
  } else {
    win = combo_box_create_popup(combo_box);
  }

  combo_box_hook_items(combo_box, win);

  p.x = widget->x;
  p.y = widget->y;
  widget_to_screen(win, &p);

  wm = win->parent;
  if ((p.y + widget->h + win->h) < wm->h) {
    p.y += widget->h;
  } else {
    p.y -= win->h;
  }
  widget_move(win, p.x, p.y);

  return RET_OK;
}

widget_t* combo_box_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  char wstr[12];
  widget_t* popup = NULL;
  widget_t* combo_box = combo_box_create_self(parent, x, y, w, h);
  return_value_if_fail(combo_box != NULL, NULL);

  popup = button_create(combo_box, 0, 0, 0, 0);
  popup->auto_created = TRUE;
  widget_set_name(popup, "popup");
  widget_use_style(popup, "combobox_down");

  tk_snprintf(wstr, sizeof(wstr) - 1, "%d", h);
  widget_set_self_layout_params(popup, "right", "0", wstr, "100%");

  widget_on(popup, EVT_CLICK, combo_box_on_button_click, combo_box);

  return combo_box;
}

ret_t combo_box_reset_options(widget_t* widget) {
  combo_box_option_t* iter = NULL;
  combo_box_option_t* next = NULL;
  combo_box_t* combo_box = COMBO_BOX(widget);
  return_value_if_fail(combo_box != NULL, RET_BAD_PARAMS);

  iter = combo_box->option_items;
  while (iter != NULL) {
    next = iter->next;
    TKMEM_FREE(iter->text);
    TKMEM_FREE(iter);
    iter = next;
  }
  combo_box->option_items = NULL;
  TKMEM_FREE(combo_box->options);

  return RET_OK;
}

ret_t combo_box_set_open_window(widget_t* widget, const char* open_window) {
  combo_box_t* combo_box = COMBO_BOX(widget);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  TKMEM_FREE(combo_box->open_window);
  combo_box->open_window = tk_strdup(open_window);

  return RET_OK;
}

ret_t combo_box_append_option(widget_t* widget, int32_t value, const char* text) {
  combo_box_option_t* iter = NULL;
  combo_box_option_t* option = NULL;
  combo_box_t* combo_box = COMBO_BOX(widget);
  return_value_if_fail(combo_box != NULL && text != NULL, RET_BAD_PARAMS);

  option = TKMEM_ZALLOC(combo_box_option_t);
  return_value_if_fail(option != NULL, RET_OOM);

  option->value = value;
  option->text = tk_strdup(text);

  if (option->text == NULL) {
    TKMEM_FREE(option);
    return RET_OOM;
  }

  if (combo_box->option_items != NULL) {
    iter = combo_box->option_items;
    while (iter->next != NULL) iter = iter->next;
    iter->next = option;
  } else {
    combo_box->option_items = option;
  }

  return RET_OK;
}

int32_t combo_box_count_options(widget_t* widget) {
  int32_t nr = 0;
  combo_box_option_t* iter = NULL;
  combo_box_t* combo_box = COMBO_BOX(widget);
  return_value_if_fail(combo_box != NULL, nr);

  iter = combo_box->option_items;
  while (iter != NULL) {
    nr++;
    iter = iter->next;
  }

  return nr;
}

combo_box_option_t* combo_box_get_option(widget_t* widget, uint32_t index) {
  uint32_t i = 0;
  combo_box_option_t* iter = NULL;
  combo_box_t* combo_box = COMBO_BOX(widget);
  return_value_if_fail(combo_box != NULL, NULL);

  iter = combo_box->option_items;
  while (iter != NULL) {
    if (i == index) {
      return iter;
    }

    i++;
    iter = iter->next;
  }

  return NULL;
}

ret_t combo_box_set_selected_index(widget_t* widget, uint32_t index) {
  combo_box_t* combo_box = COMBO_BOX(widget);
  return_value_if_fail(combo_box != NULL, RET_OK);

  combo_box->selected_index = index;
  if (combo_box->option_items != NULL) {
    combo_box_option_t* option = combo_box_get_option(widget, index);

    if (option != NULL) {
      combo_box->value = option->value;
      widget_set_tr_text(widget, option->text);
    }
  }

  return RET_OK;
}

int32_t combo_box_get_value(widget_t* widget) {
  combo_box_t* combo_box = COMBO_BOX(widget);
  return_value_if_fail(combo_box != NULL, 0);

  return combo_box->value;
}

const char* combo_box_get_text(widget_t* widget) {
  combo_box_t* combo_box = COMBO_BOX(widget);
  return_value_if_fail(combo_box != NULL, NULL);

  if (widget->tr_text != NULL && EDIT(widget)->readonly) {
    return widget->tr_text;
  } else {
    str_from_wstr(&(combo_box->text), widget->text.str);
    return combo_box->text.str;
  }
}

widget_t* combo_box_cast(widget_t* widget) {
  return_value_if_fail(widget != NULL && widget->vt == &s_combo_box_vtable, NULL);

  return widget;
}
