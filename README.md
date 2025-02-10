## Tarefa U4C6 - Interfaces de Comunicação Serial com RP2040 UART, SPI e I²C

Este repositório contém o código-fonte de um projeto desenvolvido para a placa BitDogLab utilizando o microcontrolador RP2040. O projeto envolve comunicação UART, controle de LEDs WS2812, manipulação de botões com interrupções e debounce, e exibição de informações em um display SSD1306.

# Link do Vídeo

Link do video: <   > 


📌 # Funcionalidades

- Recebimento de caracteres via UART e exibição no display SSD1306.

- Controle de LEDs RGB WS2812 com base nos números recebidos.

- Interrupções para botões A e B, incluindo debounce por software.

- Atualização dinâmica do display com os caracteres recebidos.

🛠️ # Dependências

Para compilar e rodar este projeto, você precisará de:

- SDK do RP2040 instalado

- Bibliotecas para SSD1306 e WS2812

🔧 # Configuração e Compilação

- Clone este repositório:

`git clone https://github.com/seu-usuario/seu-repositorio.git`
`cd seu-repositorio`

- Crie um diretório para a compilação:

 `mkdir build && cd build`

- Execute o CMake e compile:

`cmake ..`
`make`

Envie o firmware para o RP2040:

`picotool load nome-do-firmware.uf2`

🖥️ # Uso

- Digite um caractere via UART no Serial Monitor do Visual Studio Code e ele será exibido no display.

- Pressione os botões A ou B para ativar funções específicas.

- Envie números via UART para alterar os LEDs WS2812.

📂 # Estrutura do Código

/seu-repositorio
│── inc           
│── │── font.h        
│── │── ssd1306.c
│── │── ssd1306.h 
│──Embarcatech_U4C6.pio
│── CMakeLists.txt  
│── README.md       

# Autor
Desenvolvido por Arilson S. Oliveira, como prática pessoal e requisito de atividade avaliativa da Residência em Sistemas Embarcados Embarcatech, com foco em aprendizado da Raspberry Pi Pico W e placa BitDogLab.
