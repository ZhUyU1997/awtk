/**
 * File:   font_stb.h
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  stb truetype font loader
 *
 * Copyright (c) 2018 - 2018  Li XianJing <xianjimli@hotmail.com>
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
 * 2018-01-21 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef LFTK_FONT_STB_H
#define LFTK_FONT_STB_H

#include "base/font.h"

BEGIN_C_DECLS

font_t* font_stb_create(const char* name, uint16_t font_size, const uint8_t* buff);

END_C_DECLS

#endif/*LFTK_FONT_STB_H*/
