/*
  h - medidor_h
  Created by Jackson Lago, Dez 21, 2021.
  Not released into the public domain.
*/

#pragma once

#include "signals.h"

using namespace signals;

struct ChannelGains
{
    public:
    static const uint16_t Nch = 3; // N canais
    static const uint16_t Nvs = 3; // número de escalas de tensão (incluindo off)
    static const uint16_t Nis = 5; // número de escalas de corrente (incluindo off)

    uint16_t v_ch_scale_selection[Nch] =     {1 , 1, 1};
    String v_scale_txt_list[Nvs] =           { "off"     , "400Vpk"      , "640Vpk"      };
    float  v_scale_val_list[Nvs] =           { 0.0f      ,  400.0f       ,  640.0f       };
    const float v_ch_scale_gain[Nch][Nvs] = {{ 0.0f      , 0.195f        , 0.3125f       },  // ch0  - manter nahos individuais por escala e canal para calibração
                                             { 0.0f      , 0.195f        , 0.3125f       },  // ch1
                                             { 0.0f      , 0.195f        , 0.3125f       }}; // ch2

    uint16_t i_ch_scale_selection[Nch] =     { 2, 2, 2};
    String i_scale_txt_list[Nis] =           { "off"     , "25Apk"       , "12.5Apk"     , "8.33Apk"     , "6.25Apk"       };
    float  i_scale_val_list[Nis] =           { 0.0f      ,  25.0f        ,  12.5f        ,  8.33f        ,  6.25f          };
    const float i_ch_scale_gain[Nch][Nis] = {{ 0.0f      , .01220703125f , .006103515625f, .004069010417f, .0030517578125f }, // ch0  - manter nahos individuais por escala e canal para calibração
                                             { 0.0f      , .01220703125f , .006103515625f, .004069010417f, .0030517578125f }, // ch1
                                             { 0.0f      , .01220703125f , .006103515625f, .004069010417f, .0030517578125f }}; // ch2


    public:
    float v[Nch];
    float i[Nch];
    float p[Nch];
    float w[Nch];

    ChannelGains()
    {
        calcGains();
    }

    void changeVoltScale(uint16_t ch)
    {
        v_ch_scale_selection[ch]++;
        v_ch_scale_selection[ch] = v_ch_scale_selection[ch] % Nvs;
        calcGains();
    }

    void changeAmpScale(uint16_t ch)
    {
        i_ch_scale_selection[ch]++;
        i_ch_scale_selection[ch] = i_ch_scale_selection[ch] % Nis;
        calcGains();
    }

    String& getVoltScale(const uint16_t ch)
    {
        return v_scale_txt_list[v_ch_scale_selection[ch]];
    }

    String& getAmpScale(const uint16_t ch)
    {
        return i_scale_txt_list[i_ch_scale_selection[ch]];
    }


    private:
    void calcGains()
    {
        for(uint16_t ch=0; ch<Nch; ch++)
        {
            v[ch] = v_ch_scale_gain[ch][v_ch_scale_selection[ch]];
            i[ch] = i_ch_scale_gain[ch][i_ch_scale_selection[ch]];
            p[ch] = v[ch]*i[ch];
            w[ch] = p[ch]*2.7777777777777777777777777777778e-4f;  // J*2.77e-4 = Wh
        }
    }
};

template<typename F, uint16_t BuffSize>
class Medidor
{
    private:
    static const int Nch = 3;
    bool m_run = true;
    bool stop_request = false;

    public:
    int16_t (*data)[BuffSize];

    ChannelGains gains;
    Window<F> window;
    

    uint16_t pll_ch = 2; //7=off
    bool avg_rms_selection[2*Nch] = {true,true,true,true,true,true}; // va vb vc ia ib ic

    F Vrms[Nch];
    F Irms[Nch];
    F Vavg[Nch];
    F Iavg[Nch];
    F P[Nch];
    F S[Nch];
    double W[Nch];

    Medidor(int16_t data[][BuffSize], F fs)
        : data( &data[0] )
    {
        window.setLenght(BuffSize);
        window.setFs(fs);
        reset();
    }

    void run()
    {
        stop_request = false;
        if (!m_run)
        {
            m_run = true;
            reset();
            window.reset();
        }
        
    }
    
    void toggle()
    {
        if(is_running())
            stop();
        else
            run();
    }
    

    void stop()
    {
        stop_request = true; 
    }

    bool is_running()
    {
        if (window.sync & stop_request)
            m_run = false;

        return m_run;
    }

    void reset()
    {
        window.reset();
        for(uint16_t ch = 0; ch < Nch; ch++)
        {
            Vrms[ch] = 0;
            Irms[ch] = 0;
            Vavg[ch] = 0;
            Iavg[ch] = 0;
            P[ch] = 0;
            S[ch] = 0;
            W[ch] = 0;
        }
    }

    void changeFreqChannel()
    {
        pll_ch++;
        pll_ch = pll_ch % 7;
    }

    void changeAvgRms(uint16_t ch)
    {
        avg_rms_selection[ch] = ! avg_rms_selection[ch];
    }

    void calc()
    {
        for(uint16_t ch = 0; ch < Nch; ch++)
        {
            Vrms[ch] = rms(data[ch], window) * gains.v[ch];
            Vavg[ch] = avg(data[ch], window) * gains.v[ch];
            Irms[ch] = rms(data[ch+3], window) * gains.i[ch];
            Iavg[ch] = avg(data[ch+3], window) * gains.i[ch];
            P[ch] = active_power(data[ch], data[ch+3], window) * gains.p[ch];
            if(m_run)
                W[ch] += energy(data[ch], data[ch+3], window) * gains.w[ch];
            S[ch] = Vrms[ch] * Irms[ch];
        }
    }
};