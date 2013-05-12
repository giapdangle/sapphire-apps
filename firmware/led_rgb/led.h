/* 
 * <license>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * This file is part of the Sapphire Operating System
 *
 * Copyright 2013 Sapphire Open Systems
 *
 * </license>
 */
 
#ifndef _LED_H
#define _LED_H

#include "io.h"

// color references
#define COLOR_HUE_RED         0
#define COLOR_HUE_YELLOW      0.1666666
#define COLOR_HUE_GREEN       0.3333333
#define COLOR_HUE_TEAL        0.5
#define COLOR_HUE_BLUE        0.6666666
#define COLOR_HUE_PURPLE      0.8333333

// LED channel mapping
#define LED_IO_RED              IO_PIN_PWM2
#define LED_IO_GREEN            IO_PIN_PWM0
#define LED_IO_BLUE             IO_PIN_PWM1

#define LED_IO_ENABLE           IO_PIN_GPIO7


void led_v_init( void );

void led_v_enable( void );
void led_v_disable( void );

void led_v_hsv_fade( float h, float s, float v, float fade_rate );
void led_v_start_fade( void );

void led_v_set_hue( float hue );
void led_v_set_saturation( float sat );
void led_v_set_brightness( float val );
void led_v_set_fade( float fade );

float led_f_get_hue( void );
float led_f_get_saturation( void );
float led_f_get_brightness( void );
float led_f_get_fade( void );

bool led_b_fading( void );

void led_v_start_pwm_timer( void );
void led_v_stop_pwm_timer( void );
void led_v_start_fade_timer( void );
void led_v_shutdown( void );







#endif
