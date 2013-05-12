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

#include "init.h"

void main( void ) __attribute__ ((noreturn));

__attribute__ ((section (".fwinfo"))) fw_info_t fw_info;


void main( void ){      
        
    if( sapphire_i8_init() == 0 ){
        
        app_v_init();
    }
    
	sapphire_run();
	
	// should never get here:
	for(;;);
}   

