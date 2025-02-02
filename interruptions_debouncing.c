#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "numeros.h"

#include "interruptions_debouncing.pio.h"

const uint led_pin_red = 13;
const uint button_A = 5;
const uint button_B = 6;

#define NUM_PIXELS 25
#define MATRIX_PIN 7

static volatile uint32_t last_time = 0; //Armazena o tempo do último evento
int num = 0; //Armazena o número atualmente exibido

double* nums[10] = {num_zero, num_um, num_dois, num_tres, num_quatro, num_cinco, num_seis, num_sete, num_oito, num_nove}; //Ponteiros para os desenhos dos números

//Pisca o LED vermelho em ciclos de 200ms, ou seja, 5 vezes por segundo
void blink_led() {
        gpio_put(led_pin_red, 1);
        sleep_ms(100);
        gpio_put(led_pin_red, 0);
        sleep_ms(100);
}

//Rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double r, double g, double b) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

//Rotina para acionar a matrix de leds - ws2812b
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        valor_led = matrix_rgb(r=0.0, desenho[24-i], b=0.0);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

//Rotina de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    //Obtém o tempo atual em ms (milissegundos)
    uint32_t current_time = to_us_since_boot(get_absolute_time()); 
    if (current_time - last_time > 200000) { //200000us = 200ms de debouncing
        last_time = current_time;
        if (gpio == button_A) {
            if (num < 9) {
                num++;
            }
        } else if (gpio == button_B) {
            if (num > 0) {
                num--;
            }
        }
    }
}

//Configuração das GPIOs
void setup_GPIOs() {
    gpio_init(led_pin_red);
    gpio_set_dir(led_pin_red, GPIO_OUT);

    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);

    gpio_init(button_B);
    gpio_set_dir(button_B, GPIO_IN);
    gpio_pull_up(button_B);
}

int main()
{
    stdio_init_all();
    PIO pio = pio0; 
    bool ok;
    uint32_t valor_led;
    double r = 0.0, g = 0.0 , b = 0.0;

    //Coloca a frequência de clock para 128 MHz, facilitando a divisão pelo clock
    ok = set_sys_clock_khz(128000, false);

    //Configurações da PIO
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, MATRIX_PIN);

    setup_GPIOs();

    //Configuração de interrupção para os botões
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        blink_led();
        desenho_pio(nums[num], valor_led, pio, sm, r, g, b);
    }
}
