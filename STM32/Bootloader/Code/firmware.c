#define BOOTLOADER_SIZE (0x8000U)
static void vector_setup(void) {
    // Set the vector table to the start of the application
    SCB->VTOR = BOOTLOADER_SIZE;
}

int main() {
    // Initialize the vector table for the application
    vector_setup();

    // Jump to the application entry point
    void (*app_entry)(void) = (void (*)(void))(*((uint32_t *)(BOOTLOADER_SIZE + 4)));
    app_entry();

    // Should never reach here
    while (1) {
        // Optionally, you can add some error handling or a fallback mechanism here
    }
}