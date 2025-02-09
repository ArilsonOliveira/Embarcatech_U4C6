#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "Embarcatech_U4C6.pio.h"

#define NUM_PIXELS 25  // Número de pixels da matriz de LEDs 5x5

// Definição dos pinos
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define BUTTON_A 5
#define BUTTON_B 6
#define WS2812 7
#define LED_G 11
#define LED_B 12
#define LED_R 13
#define I2C_SDA 14
#define I2C_SCL 15

// UART
#define UART_ID uart0
#define BAUD_RATE 115200

// I2C
#define I2C_ID i2c1
#define I2C_ADDR 0x3C  // Endereço do dispositivo i2c
#define I2C_FREQ 100000 // 100kHz

// Variáveis globais. Não devem ser alteradas manualmente!
static volatile int num = 0; // Altera o número exibido na matriz de LEDS
static volatile uint32_t last_time = 0; // Armazena o tempo em microssegundos
ssd1306_t ssd; // Inicializa a estrutura do display ssd1306

// Protótipos das funções
static void gpio_irq_handler(uint gpio, uint32_t events); // Lida com interrupções através do botão A e B
void WS2812_LEDs(uint sm);
uint32_t matrix_rgb(double r, double b, double g);
void inicializar_perifericos();


int main()
{
    stdio_init_all();
    inicializar_perifericos();

    // Configuração da matriz de LEDs
    uint offset = pio_add_program(pio0, &ws2812_program);
    uint sm = pio_claim_unused_sm(pio0, true);
    ws2812_program_init(pio0, sm, offset, WS2812);

    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    printf("Sistema inicializado. Aguardando interações...");

    while (true) {
            char c = getchar();

            // Todo caractere digitado é exibido no display SSD1306
            ssd1306_fill(&ssd, false);
            ssd1306_draw_char(&ssd, c, 64, 32);
            ssd1306_send_data(&ssd);

            // Se for de 0 a 9, é também exibido na matriz de LEDs ws2812
            if (c >= '0' && c <= '9')
                num = c - '0';
            else
                num = -1;
            WS2812_LEDs(sm);
        sleep_ms(100);
    }
}


void gpio_irq_handler(uint gpio, uint32_t events){

    uint32_t current_time = to_us_since_boot(get_absolute_time());
    ssd1306_fill(&ssd, false);
    
    // Verifica se passou tempo suficiente desde o último evento
    if(current_time - last_time > 300000){ // 300 ms de debouncing

        if(gpio == BUTTON_A){
            gpio_put(LED_G, !(gpio_get(LED_G)));
            printf("Estado do LED verde foi alterado através de uma interrupção pelo Botão A!\n");
        }
        else if(gpio == BUTTON_B){
            gpio_put(LED_B, !(gpio_get(LED_B)));
            printf("Estado do LED azul foi alterado através de uma interrupção pelo Botão B!\n");
        }

        if (!gpio_get(LED_G) && !gpio_get(LED_B))
            ssd1306_draw_string(&ssd, "LEDs apagados!", 0, 32);
        else if (gpio_get(LED_G) && !gpio_get(LED_B))
            ssd1306_draw_string(&ssd, "LED Verde aceso!", 0, 32);
        else if (!gpio_get(LED_G) && gpio_get(LED_B))
            ssd1306_draw_string(&ssd, "LED Azul aceso!", 0, 32);
        else
            ssd1306_draw_string(&ssd, "LEDs Verde e Azul Acesos!", 0, 31);

        ssd1306_send_data(&ssd);
        last_time = current_time; // Atualiza o tempo do último evento
    }
}


void WS2812_LEDs(uint sm){
    
    uint32_t led;
    double numero[11][NUM_PIXELS] = {

    // Número 0
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // Número 1
    {0.0, 0.0, 0.1, 0.0, 0.0,
     0.0, 0.0, 0.1, 0.0, 0.0,
     0.0, 0.0, 0.1, 0.0, 0.0,
     0.0, 0.0, 0.1, 0.0, 0.0,
     0.0, 0.0, 0.1, 0.0, 0.0},

    // Número 2
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // Número 3
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // Número 4
    {0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0},

    // Número 5
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // Número 6
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // Número 7
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0},
    
    // Número 8
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // Número 9
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0}};


    for (int16_t i = 0; i < NUM_PIXELS; i++){   // Iteração de pixels

        led = matrix_rgb(numero[num][24 - i], 0, 0);
        if (num == -1)
            led = matrix_rgb(0, 0, 0);  // Apaga a matriz se num não está entre 0 e 9
        pio_sm_put_blocking(pio0, sm, led);
         
    }
}


uint32_t matrix_rgb(double r, double b, double g){
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}


void inicializar_perifericos(){

    // Inicializa LEDs
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, 0);

    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_put(LED_B, 0);

    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_put(LED_R, 0);

    // Inicializa botões A e B
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);          // Habilita o pull-up interno

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);          // Habilita o pull-up interno

    // Inicializa e configura UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_fifo_enabled(UART_ID, true); // Evita sobrecarga no buffer
    uart_set_hw_flow(UART_ID, false, false);  // Desativa controle de fluxo de hardware
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);  // 8 bits, 1 stop bit, sem paridade

    // Inicializa o I2C
    i2c_init(I2C_ID, I2C_FREQ);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    // Inicializa o SSD1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, I2C_ADDR, I2C_ID); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

}