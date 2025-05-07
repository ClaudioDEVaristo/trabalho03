#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <stdio.h>
#include "hardware/pio.h"
#include "trabalho03.pio.h"
#include "config_matriz.h"
#include "hardware/pwm.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define ledVerde 11
#define ledVermelho 13
#define BotaoA 5
#define buzz 10

volatile bool estadoGeral = false; //Flag global
volatile enum estadoCor {COR_VERDE, COR_AMARELA, COR_VERMELHA} estadoCorAtual = COR_VERDE; //Controle das cores para o display

void vBotaoTask() //Tarefa do botão para alterar a flag global
{
    gpio_init(BotaoA);
    gpio_set_dir(BotaoA, GPIO_IN);
    gpio_pull_up(BotaoA);

    bool ultimoEstado = true;

    while (true)
    {
        bool estadoAtual = gpio_get(BotaoA);

        if(ultimoEstado && !estadoAtual)
        {
            estadoGeral = !estadoGeral; //Altera a flag global
        }

        ultimoEstado = estadoAtual;

        vTaskDelay(pdMS_TO_TICKS(50));
    }  
}

void vSemaforoTask() //Tarefa resposável pelo controle dos leds rgb e da variável das cores
{
    gpio_init(ledVerde);
    gpio_set_dir(ledVerde, GPIO_OUT);
    gpio_init(ledVermelho);
    gpio_set_dir(ledVermelho, GPIO_OUT);
    TickType_t xLastWakeTime = xTaskGetTickCount(); //Config para utilizar tempo absoluto 

    while (true)
    {
        if(estadoGeral == false) //Modo Normal
        {
            estadoCorAtual = COR_VERDE; 
            gpio_put(ledVerde, 1);
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1250)); //Usa o timer absoluto para um delay preciso de 1250 ms
            gpio_put(ledVerde, 0);
           
            estadoCorAtual = COR_AMARELA;
            gpio_put(ledVerde, 1);
            gpio_put(ledVermelho, 1);
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1500)); //Usa o timer absoluto para um delay preciso de 1500 ms
            gpio_put(ledVerde, 0);
            gpio_put(ledVermelho, 0);
           
            estadoCorAtual = COR_VERMELHA;
            gpio_put(ledVermelho, 1);
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(6000)); //Usa o timer absoluto para um delay preciso de 6000 ms
            gpio_put(ledVermelho, 0);
        }
        else //Modo Noturno
        {
            estadoCorAtual = COR_AMARELA;
            gpio_put(ledVerde, 1);
            gpio_put(ledVermelho, 1);
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3000));
        }
    }
}

void vMatrizLedTask()
{
    PIO pio = pio_config(); //Config do PIO
    TickType_t xLastWakeTime = xTaskGetTickCount(); //Config para utilizar tempo absoluto

    const uint16_t period = 20000; //  20ms = 20000 ticks
    const float DIVIDER_PWM = 125.0; //Divisor de clock
    uint slice_num;

    gpio_set_function(buzz, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(buzz);
  
    pwm_set_clkdiv(slice_num, 125.0); // 125 MHz / 125 = 1 MHz
    pwm_set_wrap(slice_num, 250);     // 1 MHz / 250 = 4 kHz (frequência audível)
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(buzz), 0); //Inicia o BUZZ desligado
    pwm_set_enabled(slice_num, true);// Inicia o PWM
    uint slice = pwm_gpio_to_slice_num(buzz);
    uint chan = pwm_gpio_to_channel(buzz);

    while (true)
    {
        if(estadoGeral == false) //Modo Normal
        {
        define_numero(0, pio, 0); //Cor Verde da matriz de leds
        pwm_set_chan_level(slice, chan, 150); //Seta o DC para ativação do buzzer
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000)); 
        define_numero(3, pio, 0); //Apaga a matriz de leds
        pwm_set_chan_level(slice, chan, 0); //Desliga o buzzer
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(250)); 

        for(int i = 0; i < 3; i++){
        define_numero(1, pio, 0); //Cor Amarela da matriz de leds
        pwm_set_chan_level(slice, chan, 130); //Seta o DC para ativação do buzzer
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(250));
        define_numero(3, pio, 0);//Apaga a matriz de leds
        pwm_set_chan_level(slice, chan, 0); //Desliga o buzzer
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(250));       
        }
        for (int j = 0; j < 3; j++)
        {
        define_numero(2, pio, 0); //Cor Vermelha da matriz de leds
        pwm_set_chan_level(slice, chan, 160); //Seta o DC para ativação do buzzer
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
        define_numero(3, pio, 0);//Apaga a matriz de leds
        pwm_set_chan_level(slice, chan, 0); //Desliga o buzzer
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1500));
        }
        }else //Modo Noturno
        {
        define_numero(1, pio, 0); //Cor Amarela da matriz de leds
        pwm_set_chan_level(slice, chan, 130); //Seta o DC para ativação do buzzer
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
        define_numero(3, pio, 0);//Apaga a matriz de leds
        pwm_set_chan_level(slice, chan, 0); //Desliga o buzzer
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
        }
    }
}

void vDisplay3Task()
{
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
   
    bool cor = true;
    while (true)
    {
        bool estado = estadoGeral;
        enum estadoCor corAtual = estadoCorAtual;

        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
        ssd1306_draw_string(&ssd, "Modo:  ", 10, 16);  // Desenha uma string
        ssd1306_draw_string(&ssd, "Cor: ", 10, 28); // Desenha uma string
        ssd1306_draw_string(&ssd, "Estado: ", 10, 41);    // Desenha uma string

        if(estado == true){
        ssd1306_draw_string(&ssd, "Noturno", 50, 16);  // Desenha uma string
        ssd1306_draw_string(&ssd, "Amarelo", 45, 28); // Desenha uma string
        ssd1306_draw_string(&ssd, "Atencao!", 65, 41);    // Desenha uma string
        ssd1306_send_data(&ssd);                           // Atualiza o display
        }else{
        ssd1306_draw_string(&ssd, "Normal", 45, 16);  // Desenha uma string

        switch (corAtual)
        {
        case COR_VERDE:
        ssd1306_draw_string(&ssd, "Verde", 45, 28); // Desenha uma string
        ssd1306_draw_string(&ssd, "Livre", 65, 41);    // Desenha uma string
        ssd1306_send_data(&ssd);  
            break;
        case COR_AMARELA:
        ssd1306_draw_string(&ssd, "Amarelo", 45, 28); // Desenha uma string
        ssd1306_draw_string(&ssd, "Atencao!", 65, 41);    // Desenha uma string
        ssd1306_send_data(&ssd);  
            break;
        case COR_VERMELHA:
        ssd1306_draw_string(&ssd, "Vermelho", 45, 28); // Desenha uma string
        ssd1306_draw_string(&ssd, "Pare!", 65, 41);    // Desenha uma string
        ssd1306_send_data(&ssd);  
            break;
        }         
        }
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    stdio_init_all();

    xTaskCreate(vSemaforoTask, "Semaforo Task", configMINIMAL_STACK_SIZE,
         NULL, tskIDLE_PRIORITY + 1, NULL); //Cria a Tarefa para controle do led RGB
    xTaskCreate(vMatrizLedTask, "Matriz Task Led", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY + 1, NULL);//Cria a Tarefa para controle da Matriz de leds e do buzzer
    xTaskCreate(vDisplay3Task, "Cont Task Disp3", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);//Cria a Tarefa para controle displayOled
    xTaskCreate(vBotaoTask, "Bot Task", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY + 3, NULL);//Cria a Tarefa para controle do Botão e definição da flag global
    vTaskStartScheduler();
    panic_unsupported();
}
