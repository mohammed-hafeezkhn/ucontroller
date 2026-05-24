#include "stm32f4xx.h"
#include <libopencm3/stm32/memorymap.h>

#define BOOTLOADER_SIZE        (0x8000)
#define MAIN_APP_START_ADDRESS (FLASH_BASE + BOOTLOADER_SIZE)//(0x08008000)

static void jump_to_main(void)
{
    typedef void(*func)(void);

    uint32_t* reset_vector_entry = (uint32_t*)(MAIN_APP_START_ADDRESS + 4U);
    uint32_t* reset_vector = (uint32_t*)(*reset_vector_entry);
    
    func jump_fn = (func)reset_vector;

    //Jump to the main application
    jump_fn();

}

int main(void){
    jump_to_main();
    //Never return

    return;
}