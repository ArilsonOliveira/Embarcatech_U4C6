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

// Definição dos pinos
#define I2C_SDA 14
#define I2C_SCL 15
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define LED_VERDE 11
#define LED_AZUL 12
#define LED_VERMELHO 13
#define BOTAO_A 5
#define BOTAO_B 6
#define WS2812 7
#define MATRIZ_LEDS 25  // Número de pixels da matriz de LEDs 5x5


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
char ultimo_caractere = ' '; // Variável global para armazenar o último caractere recebido

// Protótipos das funções
static void gpio_irq_handler(uint gpio, uint32_t events); // Lida com interrupções através do botão A e B
void WS2812_LEDs(uint sm);
uint32_t matrix_rgb(double r, double b, double g);
void iniciar_pinos();

        

void atualizar_display_completo() {
    ssd1306_fill(&ssd, false); // Limpa o display antes de redesenhar tudo

    // Exibe estado dos LEDs
    ssd1306_draw_string(&ssd, "LED Verde:", 0, 10);
    if (gpio_get(LED_VERDE))
        ssd1306_draw_string(&ssd, "ON", 80, 10);
    else
        ssd1306_draw_string(&ssd, "OFF", 80, 10);

    ssd1306_draw_string(&ssd, "LED Azul:", 0, 30);
    if (gpio_get(LED_AZUL))
        ssd1306_draw_string(&ssd, "ON", 80, 30);
    else
        ssd1306_draw_string(&ssd, "OFF", 80, 30);

    // Exibe último caractere recebido
    ssd1306_draw_string(&ssd, "Caractere:", 0, 50);
    char str[2] = {ultimo_caractere, '\0'};
    ssd1306_draw_string(&ssd, str, 90, 50);

    ssd1306_send_data(&ssd); // Envia as informações ao display
    // Se for de 0 a 9, é também exibido na matriz de LEDs ws2812
    if (ultimo_caractere >= '0' && ultimo_caractere <= '9')
    num = ultimo_caractere - '0'; // Converte o caractere para número
else
    num = -1; // Se não for um número, define num como -1 (apaga a matriz)
     
}

void gpio_irq_handler(uint gpio, uint32_t events){

    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Obtém o tempo atual em microssegundos
    

    // Verifica se passou tempo suficiente desde o último evento (debouncing)
    if(current_time - last_time > 200000){ // 200 ms de debouncing

        if(gpio == BOTAO_A){
            gpio_put(LED_VERDE, !(gpio_get(LED_VERDE))); // Alterna o estado do LED verde
            printf("Estado do LED verde mudado\n");
        }
        else if(gpio == BOTAO_B){
            gpio_put(LED_AZUL, !(gpio_get(LED_AZUL))); // Alterna o estado do LED azul
            printf("Estado do LED azul mudado\n");
        }

        atualizar_display_completo(); // Atualiza display com tudo
        last_time = current_time;
    }
}
  

void iniciar_pinos(){

    // Inicializa LEDs
    gpio_init(LED_VERDE); // Inicializa o pino do LED verde
    gpio_set_dir(LED_VERDE, GPIO_OUT); // Define o pino como saída
    gpio_put(LED_VERDE, 0); // Desliga o LED verde

    gpio_init(LED_AZUL); // Inicializa o pino do LED azul
    gpio_set_dir(LED_AZUL, GPIO_OUT); // Define o pino como saída
    gpio_put(LED_AZUL, 0); // Desliga o LED azul

    gpio_init(LED_VERMELHO); // Inicializa o pino do LED vermelho
    gpio_set_dir(LED_VERMELHO, GPIO_OUT); // Define o pino como saída
    gpio_put(LED_VERMELHO, 0); // Desliga o LED vermelho

    // Inicializa botões A e B
    gpio_init(BOTAO_A); // Inicializa o pino do botão A
    gpio_set_dir(BOTAO_A, GPIO_IN); // Define o pino como entrada
    gpio_pull_up(BOTAO_A); // Habilita o pull-up interno
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita interrupção no botão A

    gpio_init(BOTAO_B); // Inicializa o pino do botão B
    gpio_set_dir(BOTAO_B, GPIO_IN); // Define o pino como entrada
    gpio_pull_up(BOTAO_B); // Habilita o pull-up interno
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita interrupção no botão B

    // Inicializa e configura UART
    uart_init(UART_ID, BAUD_RATE); // Inicializa a UART com a taxa de transmissão especificada
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // Configura o pino TX para função UART
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // Configura o pino RX para função UART
    uart_set_fifo_enabled(UART_ID, true); // Habilita o FIFO para evitar sobrecarga no buffer
    uart_set_hw_flow(UART_ID, false, false); // Desativa controle de fluxo de hardware
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE); // Configura a UART para 8 bits, 1 stop bit, sem paridade

    // Inicializa o I2C
    i2c_init(I2C_ID, I2C_FREQ); // Inicializa o I2C com a frequência especificada
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Configura o pino SDA para função I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura o pino SCL para função I2C

    // Inicializa o SSD1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, I2C_ADDR, I2C_ID); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

}

void WS2812_LEDs(uint sm){
    
    uint32_t led;
    double numero[11][MATRIZ_LEDS] = {

    // ---------------  Número 0
    {0.0, 0.5, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0, // numeros em porcentagem para definir intensidade dos leds
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // --------------- Número 1
    {0.0, 0.0, 0.1, 0.0, 0.0,
     0.0, 0.0, 0.1, 0.1, 0.0,
     0.0, 0.0, 0.1, 0.0, 0.0,
     0.0, 0.0, 0.1, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // --------------- Número 2
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // --------------- Número 3
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // --------------- Número 4
    {0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0},

    // --------------- Número 5
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    // --------------- Número 6
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    //--------------- Número 7
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.1, 0.1, 0.1,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.1, 0.0},
    
    // --------------- Número 8
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0},

    //---------------  Número 9
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0}};


    for (int16_t i = 0; i < MATRIZ_LEDS; i++){   // Iteração de pixels

        led = matrix_rgb(0, numero[num][24 - i], 0); // Converte o valor do pixel para a cor correspondente
        if (num == -1)
            led = matrix_rgb(0, 0, 0);  // Apaga a matriz se num não está entre 0 e 9
        pio_sm_put_blocking(pio0, sm, led); // Envia o valor do pixel para a matriz de LEDs
         
    }
}


uint32_t matrix_rgb(double r, double b, double g){
    unsigned char R, G, B;
    R = r * 255; // Converte o valor de vermelho para 8 bits
    G = g * 255; // Converte o valor de verde para 8 bits
    B = b * 255; // Converte o valor de azul para 8 bits
    return (G << 24) | (R << 16) | (B << 8); // Retorna o valor no formato GRB
}

int main()
{
    stdio_init_all(); // Inicializa todas as funcionalidades padrão do Pico
    iniciar_pinos(); // Inicializa os periféricos (LEDs, botões, UART, I2C, display)

    printf("Sistema inicializado...\n");

    // Configuração da matriz de LEDs
    uint offset = pio_add_program(pio0, &ws2812_program); // Adiciona o programa PIO para controlar os LEDs WS2812
    uint sm = pio_claim_unused_sm(pio0, true); // Obtém uma máquina de estado (SM) disponível no PIO
    ws2812_program_init(pio0, sm, offset, WS2812); // Inicializa o programa PIO na SM



    while (true) {

        atualizar_display_completo(); // Mantém a exibição atualizada

        //char c = getchar(); // Lê um caractere da entrada padrão (UART)

        char c = getchar(); // Captura um caractere
        if (c != EOF) { // Se recebeu um caractere válido
            ultimo_caractere = c; // Armazena o último caractere
            atualizar_display_completo(); // Atualiza o display com o novo caractere
            WS2812_LEDs(sm); // Atualiza a matriz de LEDs
        }

        sleep_ms(100);
    }
}
