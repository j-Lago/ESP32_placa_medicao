/*
  h - scope_h
  Created by Jackson Lago, Dez 22, 2021.
  Not released into the public domain.
*/

#pragma once
#include "Arduino.h"



template<typename F, uint16_t BuffSize>
class Scope
{
    
    public:
    int16_t (*data)[BuffSize];

    const int16_t MAX_Yscale = 4;
    const int16_t MAX_Xscale = 11;
    bool x_lim_p = false;
    bool x_lim_n = false;


    public:
    static const uint16_t scopeLength = 515;
    static const uint16_t dataNch = 7;
    static const uint16_t scopeNch = 3;

    int16_t x_pos = 0;


    String source_txt[dataNch+1] = {"vA", "vB", "vC", "iA", "iB", "iC", "pll", "off"};
    int16_t scope_ch_selection[scopeNch] = {2, 5, 6}; //  (scope_ch_selection[][] >= dataNch) -> off
    
    int16_t scope_y_scale[scopeNch] = {1, 1, 1};
    int16_t scope_x_scale = 2;

    float scope_time_base = 2.09605f * 2; // em us para uma divisão na menor escala de tempo do scope 


    Scope(int16_t data[][BuffSize])
        : data( &data[0] ){}

    void incYScale(uint16_t ch)
    {
        scope_y_scale[ch]++;
        if (scope_y_scale[ch] >  MAX_Yscale)
            scope_y_scale[ch] = MAX_Yscale;
    }

    String& getSource(const uint16_t ch)
    {
        return source_txt[scope_ch_selection[ch]];
    }

    String getScale(const uint16_t ch, Medidor<F, BuffSize>* med)
    {
        uint16_t source = scope_ch_selection[ch];
        if(source < 3)
            return String(med->gains.v_scale_val_list[med->gains.v_ch_scale_selection[source]] * 0.25 / scope_y_scale[ch], 3).substring(0, 4) + "V";
        else if(source < 6)
            return String(med->gains.i_scale_val_list[med->gains.i_ch_scale_selection[source-3]] * 0.25 / scope_y_scale[ch], 3).substring(0, 4) + "A";
        else if(source == 6)
            return String( 180.0f / scope_y_scale[ch], 3).substring(0, 4) + "d";
        else
            return String("--");

    }

    void decXpos()
    {
        if(!x_lim_n)
        {
            x_lim_p = false;
            x_pos--;
        }
    }

    void incXpos()
    {
        if(!x_lim_p)
        {
            x_lim_n = false;
            x_pos++;
        }
    }

    void decYScale(uint16_t ch)
    {
        scope_y_scale[ch]--;
        if (scope_y_scale[ch] <  1)
            scope_y_scale[ch] = 1;
    }

    void incXScale()
    {
        scope_x_scale++;
        if (scope_x_scale >  MAX_Xscale)
            scope_x_scale = MAX_Xscale;
    }

    void decXScale()
    {
        scope_x_scale--;
        if (scope_x_scale <  1)
            scope_x_scale = 1;
    }

    void changeSource(uint16_t ch)
    {
        scope_ch_selection[ch]++;
        if(scope_ch_selection[ch] > dataNch)
            scope_ch_selection[ch] = 0;
    }

};