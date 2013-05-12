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

#include "app.h"
#include "led.h"


#define KV_GROUP_LED     KV_GROUP_APP_BASE

#define KV_ID_HUE               1
#define KV_ID_SAT               2
#define KV_ID_VAL               3
#define KV_ID_FADE              4


static float hue;
static float sat;
static float val;
static float fade;


static bool trigger;

int8_t app_i8_kv_handler( 
    kv_op_t8 op,
    kv_grp_t8 group,
    kv_id_t8 id,
    void *data,
    uint16_t len )
{
    if( op == KV_OP_SET ){
        
        if( ( id == KV_ID_HUE ) ||
            ( id == KV_ID_SAT ) ||
            ( id == KV_ID_VAL ) ||
            ( id == KV_ID_FADE ) ){

            trigger = TRUE;
        }
    }

    return 0;
}


KV_SECTION_META kv_meta_t app_kv[] = {
    { KV_GROUP_LED, KV_ID_HUE,         SAPPHIRE_TYPE_FLOAT, 0, &hue,       app_i8_kv_handler,  "hue" },
    { KV_GROUP_LED, KV_ID_SAT,         SAPPHIRE_TYPE_FLOAT, 0, &sat,       app_i8_kv_handler,  "sat" },
    { KV_GROUP_LED, KV_ID_VAL,         SAPPHIRE_TYPE_FLOAT, 0, &val,       app_i8_kv_handler,  "val" },
    { KV_GROUP_LED, KV_ID_FADE,        SAPPHIRE_TYPE_FLOAT, 0, &fade,      app_i8_kv_handler,  "fade" },
};


PT_THREAD( kv_trigger_thread( pt_t *pt, void *state ) )
{
PT_BEGIN( pt );  
    
    while(1){
        
        // wait for trigger signal
        THREAD_WAIT_WHILE( pt, trigger == FALSE );
        
        trigger = FALSE;

        led_v_hsv_fade( hue, sat, val, fade );

        // prevent runaway thread
        THREAD_YIELD( pt );
    }

PT_END( pt );
}


PT_THREAD( init_thread( pt_t *pt, void *state ) )
{
PT_BEGIN( pt );  
    
    static uint32_t timer;
    
    // self test sequence
    // Red, green, blue, then white, 1 second delay between each
    led_v_enable();

    led_v_set_saturation( 1.0 );
    led_v_set_brightness( 1.0 );
    led_v_set_fade( 1000.0 );
    led_v_start_fade();
    
    timer = 500;
    TMR_WAIT( pt, timer );

    led_v_set_fade( 2.0 );
    led_v_set_hue( COLOR_HUE_RED );
    
    led_v_start_fade();
    timer = 1000;
    TMR_WAIT( pt, timer );

    led_v_set_hue( COLOR_HUE_GREEN );

    led_v_start_fade();
    timer = 1000;
    TMR_WAIT( pt, timer );

    led_v_set_hue( COLOR_HUE_BLUE );

    led_v_start_fade();
    timer = 1000;
    TMR_WAIT( pt, timer );

    led_v_set_brightness( 0.05 );
    led_v_set_brightness( 1.0 );
    led_v_set_saturation( 0.0 );

    led_v_start_fade();
    timer = 1000;
    TMR_WAIT( pt, timer );

    led_v_set_brightness( 0.0 );
    led_v_set_saturation( 1.0 );

    led_v_start_fade();
    timer = 1000;
    TMR_WAIT( pt, timer );

    // set to start up defaults
    hue = 0.0;
    sat = 1.0;
    val = 0.0;
    fade = 1.0; 

    trigger = TRUE;

    kv_i8_notify( KV_GROUP_LED, KV_ID_HUE );
    kv_i8_notify( KV_GROUP_LED, KV_ID_SAT );
    kv_i8_notify( KV_GROUP_LED, KV_ID_VAL );
    kv_i8_notify( KV_GROUP_LED, KV_ID_FADE );
   	
PT_END( pt );
}	


void app_v_init( void ){

	// initialize LED driver
    led_v_init();
 
    thread_t_create( init_thread,
                     PSTR("init"),
                     0,
                     0 );

    thread_t_create( kv_trigger_thread,
                     PSTR("trigger"),
                     0,
                     0 );
}


