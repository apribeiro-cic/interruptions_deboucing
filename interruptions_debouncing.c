#include <stdio.h>
#include "pico/stdlib.h"

const uint led_pin_red = 13;

void blink_led() {
    gpio_put(led_pin_red, 1);
    sleep_ms(100);
    gpio_put(led_pin_red, 0);
    sleep_ms(100);
}

int main()
{
    stdio_init_all();
    gpio_init(led_pin_red);
    gpio_set_dir(led_pin_red, GPIO_OUT);

    while (true) {
        blink_led();
    }
}
