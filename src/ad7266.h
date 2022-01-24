/*
  h - ad7266_h
  Created by Jackson Lago, Dez 18, 2021.
  Not released into the public domain.
*/
#ifndef ad7266_h
#define ad7266_h

#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include "globals.h"

#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "driver/spi_common.h"



#include "soc/spi_struct.h"

struct spi_struct_t {
    spi_dev_t * dev;
#if !CONFIG_DISABLE_HAL_LOCKS
    xSemaphoreHandle lock;
#endif
    uint8_t num;
};


#define MSB_32_SET(var, val) { uint8_t * d = (uint8_t *)&(val); (var) = d[3] | (d[2] << 8) | (d[1] << 16) | (d[0] << 24); }


namespace AD7266 
{
    #define SCLK_A  14
    #define MISO_A  12
    #define MOSI_A  13
    #define SS_A    15
    
    #define MISO_B  19
    #define MOSI_B  23
    #define SCLK_B  18
    #define SS_B    05

    #define A0      25
    #define A1      26
    #define A2      27
    #define RANGE   4

    

    #define MASCARA12bits 0b0000111111111111
    #define MASCARAsinal  0b0000100000000000
    #define MASCARAneg    0b1111000000000000

    struct pair_int16_t
    {
        union
        {   
            int32_t data32;
            int16_t data16[2];
            struct
            {
                int16_t v;
                int16_t i;
            };
        };
    };

    


    class SPI_ADC
    {
        private:
        static const uint16_t EEPROM_SIZE = 6*sizeof(uint16_t);
        uint32_t conv_result32_raw[3];
        pair_int16_t ch_raw[3];

        spi_struct_t * bus_master;
        SPIClass* spi_master = NULL;

        const uint32_t spiClk = 24000000;

        public:
        pair_int16_t ch[3];
        pair_int16_t ch_offset[3];

        SPI_ADC()
        {
            defaultOffsets();
        }

        void defaultOffsets()
        {
            // todo: ler valores inicias da flash
            ch_offset[0].v = -6;
            ch_offset[0].i = -8;
            ch_offset[1].v = 8;
            ch_offset[1].i = -5;
            ch_offset[2].v = 2;
            ch_offset[2].i = -3;
        }

        void loadOffsets() 
        {
            int16_t new_offsets[6];

            // load ADC offsets
            EEPROM.begin(EEPROM_SIZE);
            for(uint16_t k=0; k<6; k++)
            {
                new_offsets[k] = ((short) EEPROM.read(k*2)) | ((short) EEPROM.read(k*2+1)<<8);
            }

            ch_offset[0].v = new_offsets[0];
            ch_offset[0].i = new_offsets[3];
            ch_offset[1].v = new_offsets[1];
            ch_offset[1].i = new_offsets[4];
            ch_offset[2].v = new_offsets[2];
            ch_offset[2].i = new_offsets[5];
        }

        void updateOffsets()
        {
            //todo: calcular a média da última janela ao invés da última amostra apenas

            int16_t new_offsets[6];
            new_offsets[0] = ch_raw[0].v;
            new_offsets[3] = ch_raw[0].i;
            new_offsets[1] = ch_raw[1].v;
            new_offsets[4] = ch_raw[1].i;
            new_offsets[2] = ch_raw[2].v;
            new_offsets[5] = ch_raw[2].i;

            ch_offset[0].v = ch_raw[0].v;
            ch_offset[0].i = ch_raw[0].i;
            ch_offset[1].v = ch_raw[1].v;
            ch_offset[1].i = ch_raw[1].i;
            ch_offset[2].v = ch_raw[2].v;
            ch_offset[2].i = ch_raw[2].i;

            for (uint16_t k=0; k<6; k++)
            {
                char a = (char)  (new_offsets[k] & 0x00ff);
                char b = (char) ((new_offsets[k] & 0xff00)>>8);
                EEPROM.write(k*2, a);
                EEPROM.write(k*2+1, b);
            }

            EEPROM.commit();
        }


        ~SPI_ADC()
        {
            delete spi_master;
        }

        void setRange(uint8_t range)
        {
            digitalWrite(RANGE, range);
        }


        void setup()
        {
            pinMode(A0, OUTPUT);
            pinMode(A1, OUTPUT);
            pinMode(A2, OUTPUT);
            pinMode(RANGE, OUTPUT);

            pinMode(SCLK_A, OUTPUT);
            pinMode(SS_A,   OUTPUT);
            pinMode(MISO_A, INPUT);
            pinMode(MOSI_A, OUTPUT);

            digitalWrite(SS_A, HIGH);
            digitalWrite(A2, HIGH);
            digitalWrite(A1, HIGH);
            digitalWrite(A0, LOW);
            digitalWrite(RANGE, HIGH);

            //inicailiza SPI para canal A
            spi_master = new SPIClass(HSPI);
            spi_master->begin(SCLK_A, MISO_A, MOSI_A, SS_A); //SCLK, MISO, MOSI, SS
            spi_master->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));

            bus_master = spi_master->bus();
            bus_master->dev->mosi_dlen.usr_mosi_dbitlen = 29;   //29 recebe 30 bits (os últimos 2 enviados pelo adc, no caso de 32 bits, são sempre zero)
            bus_master->dev->miso_dlen.usr_miso_dbitlen = 29;


        };

        inline void sample()
        {

            // Lê AD7266 pela SPI
            // [A2 A1] = 00
            GPIO.out_w1tc = 1<<A2 | 1<<A1;
            GPIO.out_w1tc = 1<<SS_A;
            bus_master->dev->cmd.usr = 1;
            while(bus_master->dev->cmd.usr);
            conv_result32_raw[0] = bus_master->dev->data_buf[0];
            GPIO.out_w1ts = 1<<SS_A | 1<<A2 | 1<<A1;

            // [A2 A1] = 01
            GPIO.out_w1tc = 1<<A2;
            GPIO.out_w1tc = 1<<SS_A;
            bus_master->dev->cmd.usr = 1;
            trata_dado_ad7266(0, conv_result32_raw); //aproveita o tempo da proxima conversão pra tratar os dados da conv anterior
            while(bus_master->dev->cmd.usr);
            conv_result32_raw[1] = bus_master->dev->data_buf[0];
            GPIO.out_w1ts = 1<<SS_A | 1<<A2 | 1<<A1;

            // [A2 A1] = 10
            GPIO.out_w1tc = 1<<A1; 
            GPIO.out_w1tc = 1<<SS_A;
            bus_master->dev->cmd.usr = 1;
            trata_dado_ad7266(1, conv_result32_raw);  //aproveita o tempo da proxima conversão pra tratar os dados da conv anterior
            while(bus_master->dev->cmd.usr);
            conv_result32_raw[2] = bus_master->dev->data_buf[0];
            GPIO.out_w1ts = 1<<SS_A | 1<<A2 | 1<<A1;


            trata_dado_ad7266(2, conv_result32_raw);

        }

        inline void trata_dado_ad7266(uint16_t id, uint32_t* raw_buffer)
        {
            uint32_t raw = raw_buffer[id];
            MSB_32_SET(raw, raw);
            ch_raw[id].v = (raw>>2) & MASCARA12bits;
                if(ch_raw[id].v & MASCARAsinal)
                    ch_raw[id].v |= MASCARAneg;

            ch_raw[id].i = (raw>>18) & MASCARA12bits;
                if(ch_raw[id].i & MASCARAsinal)
                    ch_raw[id].i |= MASCARAneg;

            ch[id].v = ch_raw[id].v - ch_offset[id].v;
            ch[id].i = ch_raw[id].i - ch_offset[id].i;
        }


    };


    



}

#endif