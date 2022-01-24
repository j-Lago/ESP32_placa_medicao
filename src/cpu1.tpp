
#include <Arduino.h>
#include "Vec.h"
#include "CosSin.h"
#include "globals.h"
#include "ad7266.h"
#include "medidor.h"




void IRAM_ATTR cpu1()
{

    //spi_adc.setRange(digitalRead(SW));  //para teste, talvez implementar ajuste automatico em caso de sinais de baixa amplitude (aplicada simultaneamente em v e i de uma mesa fase)
    
    // trata botão físico
    //    - um click -> toogle run/stop
    //    // função passada pra hmi- dois clicks -> calibra offset e grava valor na flash
    //    - hold -> toggle modo trigger
    button.update();
    //if(button.double_press_flag)
    //    spi_adc.updateOffsets();

    if(button.hold_flag)
        trigger_mode.toggle();

    led.mode = (trigger_mode()) ? on_off : blink_fast;
        

    if(button.release_flag)
        med.toggle();

    led(med.is_running());
    led.update();
    button.clear_flags();


    using namespace signals;

    // alocado dinamicamente memoria para cópia do buffer a cada iteração do loop1 pois a memória é reutilizada para FFT
    // 96kB (BUFFER_SIZE=8192)  /* 6*2*BUFFER_SIZE  */
    auto buff = new int16_t[sourceCHANNELS][BUFFER_SIZE];  
    med.data = buff;
    scope.data = buff;



    // loop1 é chamado um pouco antes (prerare_next=1) de finalizar o preenchimento de buffer[BUFFER_SIZE] (new_data=1) para que dê tempo
    // de alocar memória e copiar a posição 0 dos 6 canais antes do loop0 amostrar novamente e atualizar as posiçoes buffer[0..6][0]
    // Assim, a próxima janela de amostragem inicia já no próximo sampple, sem período morto. Isso é essencial para medição de energia! 
    while(prerare_next & !new_data); // aguarda final de preenchimento do buffer pelo loop0 (final da janela de amostragem)

    // copia buffer(loop0) para buff(loop1) para que o loop0 inicie a amostragem da próxima janela em paralelo com o processameto e exibição 
    // dos dados amostrados na janela anterior
    for(uint32_t k=0; k<BUFFER_SIZE; k++)
        for(uint32_t ch=0; ch<sourceCHANNELS; ch++)
            buff[ch][k] = adc_buff[ch][k];

    
    


    // processa dados e autualiza janelas da HMI
    page0.update();
    page2.update();
    page3.update();
    page4.update();
    page1.update();  // página do FFT deve ser chamada por último pois libera e reutiliza a a memoria ocupada por data




    


    logUSB(vec2str(med.window.pll.dq));


    if (page0.activePage() != 1) 
        delete[] buff;   //se não, deletado dentro de fft para liberar espaço
    
    logUSB.println();
}