/*
  h - board_setup
  Created by Jackson Lago, Dez 15, 2021.
  Not released into the public domain.
*/


/*   ADC(slave)                                  HSPI(master)   
 *   +---------+                                 +---------+
 *   |    SCLK |<---------------------SCLK-IO14-<| SCLK    |
 *   |     CS\ |<----------------------CS\-IO15-<| SS      |
 *   |   DOUTA |>--------+-----R200--------IO12->| MISO    |
 *   |         |         |-----    --------IO13-<| MOSI    |
 *   |         |                                 +---------+ 
 *   |         |
 *   |         |                                 VSPI(slave)
 *   |         |                                 +---------+
 *   |         |    SCLK--------R36--------IO18->| SCLK    |
 *   |         |    CS\---------R37--------IO05->| SS      |
 *   |   DOUTB |>---------+-----    -------IO19-<| MISO    |
 *   |         |          |-----R203-------IO23->| MOSI    |
 *   +---------+                                 +---------+
 *   
 */  


//controle ADc
//#define A0 25
//#define A1 26
//#define A2 27
//#define RANGE 4

// SPI H
//#define SCLK_A  14
//#define MISO_A  12
//#define MOSI_A  13
//#define SS_A    15  

// SPI V
//#define SCLK_B  18
//#define MISO_B  19
//#define MOSI_B  23
//#define SS_B    05



