#include "signals.h"

void IRAM_ATTR cpu0()
{
    
//GPIO.out_w1ts = 1<<22;


    if(med.is_running())
    {
        spi_adc.sample();

        uint16_t k = med.window.id();
        adc_buff[0][k] = spi_adc.ch[0].v;
        adc_buff[1][k] = spi_adc.ch[1].v;
        adc_buff[2][k] = spi_adc.ch[2].v;

        adc_buff[3][k] = spi_adc.ch[0].i;
        adc_buff[4][k] = spi_adc.ch[1].i;
        adc_buff[5][k] = spi_adc.ch[2].i;


        int16_t pll_source = (med.pll_ch<6) ? adc_buff[med.pll_ch][k]: 0;
        med.window.step(pll_source);

        if(trigger_mode() & med.window.pll.locked)
        {
            trigger_mode(false);
            med.stop();
        }


        //adc_buff[6][k] = (med.window.pll.locked) ? med.window.pll.th.rad() * 10430.378350470452724949566316381f : 0.0f;  // pi -> (2^15-1)
        adc_buff[6][k] = med.window.pll.th.rad() * 10430.378350470452724949566316381f;  // pi -> (2^15-1)
        adc_buff[7][k] = med.window.pll.w * 43.459909793626886353956526318255f; // MAXw = 2*pi*fmax -> (2^15-1)
    }
    else
        med.window.dummy_step();

    






}