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
 
#include "sapphire.h" 

#include "led.h"

/*

Pin connections on Sapphire multiboard:
PWM0 = Green
PWM1 = Blue
PWM2 = Red

*/


static float target_h;
static float target_s;
static float target_v;
static float target_fade;

static float current_h; 
static float current_s;
static float current_v;

static float step_h;
static float step_s;
static float step_v;

static bool fading;



// hue (h) is 0 to 359 degrees mapped onto 0.0 to 1.0
// saturation (s) is 0.0 to 1.0
// value (v) is 0.0 to 1.0
void hsv_to_rgb( float h, float s, float v, float *r, float *g, float *b ){
    
    // hue indicates which sextant of the color cylinder we're in
    h *= 6;
    
    // this gets the integer sextant
    uint8_t i = floor( h );

    float f = h - i;
    
    float p = v * ( 1.0 - s );
    
    float q = v * ( 1.0 - f * s );
    
    float t = v * ( 1.0 - ( 1.0 - f ) * s );
    
    switch( i ){
        
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;
        
        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;
            
        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;
        
        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;
        
        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;
        
        case 5:
            *r = v;
            *g = p;
            *b = q;
            break;
    }
}

static float calc_hue_step( float current, float target, float rate ){
    
    rate /= 100;
    
    float distance = target - current;
        
    // adjust to shortest distance and allow the fade to wrap around
    // the hue circle
    if( f_abs( distance ) > 0.5 ){
        
        if( distance > 0.0 ){
            
            distance -= 1.0;
        }
        else{
            
            distance += 1.0;
        }
    }

    // invert rate if distance is negative
    if( distance < 0 ){
        
        rate *= -1;
    }
    
    return rate;
}

static float calc_fader_step( float current, float target, float rate ){
    
    rate /= 100;
    
    float distance = target - current;
    
    if( distance < 0 ){
        
        rate *= -1;
    }
    
    return rate;
}


// Timer 4 compare match A interrupt:
ISR(TIMER4_COMPA_vect){
	// runs at 10 ms intervals
    
    // hue channel
    if( step_h != 0 ){ // check if channel needs to change
        
        // check if the current value has reached it's target, or gone past it,
        // in either direction
        
        if( f_abs( current_h - target_h ) < f_abs( step_h ) ){
            
            current_h = target_h;

            step_h = 0;
        }
        else{
        
            // add step change to current value
            current_h += step_h;

            // check if we went negative
            if( current_h < 0.0 ){
                
                current_h += 1.0;
            }
            // or, if we went greater than 1.0
            else if( current_h > 1.0 ){
                
                current_h -= 1.0;
            }
        }
    }
    
    // saturation channel
    if( step_s != 0 ){
        
        current_s += step_s;
        
        if( ( ( step_s > 0 ) && ( current_s > target_s ) ) ||
            ( ( step_s < 0 ) && ( current_s < target_s ) ) ){
            
            step_s = 0;
            
            current_s = target_s;
        }
    }
    
    // value channel
    if( step_v != 0 ){
        
        current_v += step_v;
        
        if( ( ( step_v > 0 ) && ( current_v > target_v ) ) ||
            ( ( step_v < 0 ) && ( current_v < target_v ) ) ){
            
            step_v = 0;
            
            current_v = target_v;
        }
    }
    
    float r;
    float g;
    float b;
    
    hsv_to_rgb( current_h, current_s, current_v, &r, &g, &b );

    // map float range 0.0 to 1.0 onto PWM range 0 to 1023
    uint16_t r_pwm = r * 1023;
    uint16_t g_pwm = g * 1023;
    uint16_t b_pwm = b * 1023;
    
    OCR3C = r_pwm;
    OCR3A = g_pwm;
    OCR3B = b_pwm;
    
    /*
    // for 10 bit PWM:
    OCR3C = r >> 6;
    OCR3A = g >> 6;
    OCR3B = b >> 6;
    */

    /* 
    // for 9 bit PWM:
    OCR3C = r >> 7;
    OCR3A = g >> 7;
    OCR3B = b >> 7;
    */

    if( ( step_h == 0 ) && ( step_s == 0 ) && ( step_v == 0 ) ){
        
        fading = FALSE;
    }

    // check value
    if( current_v == 0.0 ){
        
        led_v_disable();
    }
    else{
        
        led_v_enable();
    }
}

void led_v_enable( void ){

    led_v_start_pwm_timer();
    io_v_digital_write( LED_IO_ENABLE, HIGH );
}

void led_v_disable( void ){

    led_v_stop_pwm_timer();
    io_v_digital_write( LED_IO_ENABLE, LOW );
}

void led_v_hsv_fade( float h, float s, float v, float fade_rate ){
   
    led_v_set_hue( h );
    led_v_set_saturation( s );
    led_v_set_brightness( v );
    led_v_set_fade( fade_rate );
    
    led_v_start_fade();
}

void led_v_start_fade( void ){
    
    step_h = calc_hue_step( current_h, target_h, target_fade );
    step_s = calc_fader_step( current_s, target_s, target_fade );
    step_v = calc_fader_step( current_v, target_v, target_fade );
    
    fading = TRUE;
}

void led_v_set_hue( float hue ){
    
    // bounds check
    if( hue < 0.0 ){
        
        hue = 0.0;
    }
    else if( hue >= 1.0 ){

        hue = 0.999999;
    }

    target_h = hue;
}

void led_v_set_saturation( float sat ){
   
    // bounds check
    if( sat < 0.0 ){
        
        sat = 0.0;
    }
    else if( sat >= 1.0 ){

        sat = 0.999999;
    }

    target_s = sat;
}

void led_v_set_brightness( float val ){
   
    // bounds check
    if( val < 0.0 ){
        
        val = 0.0;
    }
    else if( val >= 1.0 ){

        val = 0.999999;
    }

    target_v = val;
}

void led_v_set_fade( float fade ){
   
    // bounds check
    if( fade < 0.0 ){
        
        fade = 0.0;
    }

    target_fade = fade;
}

float led_f_get_hue( void ){
    
    return current_h;
}

float led_f_get_saturation( void ){
    
    return current_s;
}

float led_f_get_brightness( void ){
    
    return current_v;
}

float led_f_get_fade( void ){
    
    return target_fade;
}

bool led_b_fading( void ){
    
    return fading;
}

void led_v_start_pwm_timer( void ){
	
    // pwm frequency:
    // 16,000,000 / 1024 = 15.6 KHz
    // 2,000,000 / 1024 = 1.95 KHz
    // 250,000 / 1024 = 244 Hz
    	
    TCCR3A = 0b10101011; // 10 bit pwm (fast pwm)
    //TCCR3B = 0b00001010; // prescaler / 8 (enable fast pwm)
    TCCR3B = 0b00001011; // prescaler / 64 (enable fast pwm)
    TCCR3C = 0;
}

void led_v_stop_pwm_timer( void ){

    TCCR3A = 0;
    TCCR3B = 0;
    TCCR3C = 0;
}

void led_v_start_fade_timer( void ){
    
    // compare match timing:
	// 16,000,000 / 8 = 2,000,000
	// 2,000,000 / 20000 = 100 hz.
	OCR4A = ( TICKS_PER_MS * 10 ) - 1;
	
	TCCR4A = 0b00000000;
	TCCR4B = 0b00001010; // Clear timer on compare match, prescaler / 8
	TCCR4C = 0;
	
	TIMSK4 = 0b00000010; // compare match A interrupt enabled
}

// shutdown timers
void led_v_shutdown( void ){
 
    TCCR3A = 0;
    TCCR3B = 0;
    TCCR3C = 0;

    TCCR4A = 0;
    TCCR4B = 0;
    TCCR4C = 0;
}

void led_v_init( void ){

	// init io
    io_v_set_mode( LED_IO_RED, IO_MODE_OUTPUT );
    io_v_set_mode( LED_IO_GREEN, IO_MODE_OUTPUT );
    io_v_set_mode( LED_IO_BLUE, IO_MODE_OUTPUT );
	
    // initialize timers
	led_v_start_pwm_timer();
    led_v_start_fade_timer();
	
    // set all channels to off
    OCR3A = 0;
    OCR3B = 0;
    OCR3B = 0;
    
    // enable PWM outputs
    io_v_set_mode( LED_IO_ENABLE, IO_MODE_OUTPUT );
}


