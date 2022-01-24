/*
  h - globals
  Created by Jackson Lago, Dez 15, 2021.
  Not released into the public domain.
*/
#ifndef globals_h
#define globals_h

#include <Arduino.h>
#include <SPI.h>

#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "driver/spi_common.h"

#include "Vec.h"
#include "ad7266.h"
#include "signals.h"
#include "medidor.h"
#include "nextion.h"
#include "scope.h"
#include "fft.h"
#include "log.h"
#include "symmetrical.h"
#include "extendedIO.h"



#define USB_BAULD_RATE 921600
#define HMI_BAULD_RATE 250000

#define LED 2
#define SW 0
#define SEL 34

#define CPU_FREQ 80000000.0
#define TIMER_PRESCALE 88
#define TIMER_PERIOD 37              // 80 MHz / (44*37) -> Fs = 499140.05Hz -> Fs/BUFFER_SIZE = 5.998541 Hz ~ 10 ciclos


#define SAMPLE_PERIOD TIMER_PERIOD*TIMER_PRESCALE/CPU_FREQ
#define SAMPLE_FREQ CPU_FREQ/TIMER_PRESCALE/TIMER_PERIOD

#define sourceCHANNELS 8  //6 
#define BUFFER_SIZE 6800  //8192




volatile bool new_timer_interrupt = false;
volatile bool new_data = false;
volatile bool prerare_next = false; //chama o loop1 um pouco antes de finalizar o preenchimento de buffer[BUFFER_SIZE] para que dê tempo de alocar memória e copiar a posição 0 dos 6 canais antes do loop0 amostrar novamente e atualizar as posiçoes buffer[0..6][0]
portMUX_TYPE timerMux;




hw_timer_t * timer = NULL;
//HardwareSerial *comm = &Serial;


SerialLog logUSB(&Serial);

AD7266::SPI_ADC spi_adc;

int16_t adc_buff[sourceCHANNELS][BUFFER_SIZE];

Medidor<float, BUFFER_SIZE> med(adc_buff, SAMPLE_FREQ);
FFT<float, BUFFER_SIZE> fft(med);
Scope<float, BUFFER_SIZE> scope(adc_buff);
ivSymmetrical<float, BUFFER_SIZE> symm(med);


Page0<float, BUFFER_SIZE> page0(med);
Page1<float, BUFFER_SIZE> page1(med, fft);
Page2<float, BUFFER_SIZE> page2(med, scope);
Page3<float, BUFFER_SIZE> page3(med, symm);
Page4<float, BUFFER_SIZE> page4(med, spi_adc, page2);

Button button(SW, true);
Led led(LED, true);
StateControl trigger_mode;


#endif
