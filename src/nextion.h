/*
  h - nextion_h
  Created by Jackson Lago, Dez 22, 2021.
  Not released into the public domain.
*/

#pragma once
#include "medidor.h"
#include "scope.h"
#include "fft.h"
#include "symmetrical.h"
#include "ad7266.h"

//global por problema com static member
uint16_t active_page = 0;


class Nextion
{
    public:
    HardwareSerial* serial = &Serial2;
    uint16_t page_id;
    //static uint16_t active_page;

    Nextion(){}

    void setPort(HardwareSerial* port){ serial = port; }

    void update()
    {
        if (activePage() == page_id)
            __commRead();

        refresh();

    }

    void send(String str)
    {
        serial->print(str); 
        serial->write(0xff); 
        serial->write(0xff); 
        serial->write(0xff);
    }

    void changePage(uint16_t id)
    {
        active_page = id;
        send("page " + String(id));
    }

    uint16_t activePage()
    {
        return active_page;
    }
    
    virtual void refresh(void) = 0;
    virtual void handleCommand(String str) = 0;

    void __commRead()
    {
        char last_char;
        String command="";

        while (serial->available() > 0) {
          last_char = serial->read();
              
            if (last_char == 255) 
            {
                if (command.length() != 0)
                    handleCommand(command);
                
                command = ""; 
            }
            else
                command += last_char;
        }
    }

};


template<typename F, uint16_t BuffSize>
class Page0 : public Nextion
{
    public:
    Medidor<F, BuffSize>* med;

    Page0(Medidor<F, BuffSize>& med)
        : med(&med)
        {
            page_id = 0;
            changePage(0);
        }

    void handleCommand(String str) override
    {
        if       (str.equals("f1"))  { med->changeFreqChannel(); }
        //----------------------------------------------------------------
        else if  (str.equals("m0"))  { med->changeAvgRms(0); }
        else if  (str.equals("m1"))  { med->changeAvgRms(1); }
        else if  (str.equals("m2"))  { med->changeAvgRms(2); }
        else if  (str.equals("m3"))  { med->changeAvgRms(3); }
        else if  (str.equals("m4"))  { med->changeAvgRms(4); }
        else if  (str.equals("m5"))  { med->changeAvgRms(5); }
        //----------------------------------------------------------------
        else if  (str.equals("s0"))  { if(med->is_running()) med->gains.changeVoltScale(0); }
        else if  (str.equals("s1"))  { if(med->is_running()) med->gains.changeVoltScale(1); }
        else if  (str.equals("s2"))  { if(med->is_running()) med->gains.changeVoltScale(2); }
        else if  (str.equals("s3"))  { if(med->is_running()) med->gains.changeAmpScale(0); }
        else if  (str.equals("s4"))  { if(med->is_running()) med->gains.changeAmpScale(1); }
        else if  (str.equals("s5"))  { if(med->is_running()) med->gains.changeAmpScale(2); }
        //----------------------------------------------------------------
        else if  (str.equals("b1"))  { changePage(0); }
        else if  (str.equals("b0"))  { changePage(1); }
        else if  (str.equals("sr"))  { changePage(2); }
        else if  (str.equals("pt"))  { changePage(3); }
        else if  (str.equals("cf"))  { changePage(4); }

    }

    void refresh() override
    {
        med->calc();

        send("state.val=" + String(med->is_running()));
        if (activePage() != 4) 
        {
            send("p3.pic=" + String(med->pll_ch+10));
            if(med->window.valid)
            {
                send("flock.val=1");
                send("x15.val=" + String(int(med->window.pll.w * 1.5915494309189533576888376337251f)));
            }
            else
            {
                send("flock.val=0");
                send("x15.val=0");
            }
        }

        if (activePage() == page_id)
        {

            for(uint16_t ch=0; ch<3; ch++)
            {
                if (med->avg_rms_selection[ch])
                {
                    send("x"+ String(ch) + ".val=" + String((int)( med->Vrms[ch] *10)));
                    send("t"+ String(ch) + ".txt=\"rms\"");
                }
                else
                {
                    send("x"+ String(ch) + ".val=" + String((int)( med->Vavg[ch] *10)));
                    send("t"+ String(ch) + ".txt=\"avg\"");
                }
                

                if (med->avg_rms_selection[ch+3])
                {
                    send("x"+ String(ch+3) + ".val=" + String((int)( med->Irms[ch] *100)));
                    send("t"+ String(ch+3) + ".txt=\"rms\"");
                }
                else
                {
                    send("x"+ String(ch+3) + ".val=" + String((int)( med->Iavg[ch] *100)));
                    send("t"+ String(ch+3) + ".txt=\"avg\"");
                }

                send("s"+ String(ch) + ".txt=\"" + med->gains.getVoltScale(ch) + "\"");
                send("s"+ String(ch+3) + ".txt=\"" + med->gains.getAmpScale(ch) + "\"");
            }

            send("x6.val=" + String((int)(med->P[0])));
            send("x7.val=" + String((int)(med->P[1])));
            send("x8.val=" + String((int)(med->P[2])));
            send("x9.val=" + String((int)(med->S[0])));
            send("x10.val=" + String((int)(med->S[1])));
            send("x11.val=" + String((int)(med->S[2])));
            send("x12.val=" + String((int)(med->W[0])));
            send("x13.val=" + String((int)(med->W[1])));
            send("x14.val=" + String((int)(med->W[2])));
        }
    }

};


template<typename F, uint16_t BuffSize>
class Page2 : public Nextion
{
    private:
        uint16_t refresh_count = 0;
        uint16_t send_new_data_every_N_refresh = 2;

    public:
    Medidor<F, BuffSize>* med;
    Scope<F, BuffSize>* scope;


    Page2(Medidor<F, BuffSize>& med, Scope<F, BuffSize>& scope)
        : med(&med), scope(&scope)
        {
            page_id = 2;
        }

    void handleCommand(String str) override
    {
        if       (str.equals("f1")) { if(med->is_running()) med->changeFreqChannel(); }
        //----------------------------------------------------------------
        else if  (str.equals("a+"))  { scope->incYScale(0); }
        else if  (str.equals("b+"))  { scope->incYScale(1); }
        else if  (str.equals("c+"))  { scope->incYScale(2); }
        else if  (str.equals("a-"))  { scope->decYScale(0); }
        else if  (str.equals("b-"))  { scope->decYScale(1); }
        else if  (str.equals("c-"))  { scope->decYScale(2); }
        else if  (str.equals("t+"))  { scope->incXScale(); }
        else if  (str.equals("t-"))  { scope->decXScale(); }
        //----------------------------------------------------------------
        else if  (str.equals("as"))  { scope->changeSource(0); }
        else if  (str.equals("bs"))  { scope->changeSource(1); }
        else if  (str.equals("cs"))  { scope->changeSource(2); }
        //----------------------------------------------------------------
        else if  (str.equals("b1"))  { changePage(0); }
        else if  (str.equals("b0"))  { changePage(1); }
        else if  (str.equals("sr"))  { changePage(2); }
        else if  (str.equals("pt"))  { changePage(3); }
        else if  (str.equals("cf"))  { changePage(4); }
        //----------------------------------------------------------------
        else if  (str.equals("h-"))  { scope->decXpos(); }
        else if  (str.equals("h+"))  { scope->incXpos(); }

    }


    void refresh() override
    {
        refresh_count++;
        if (refresh_count >= send_new_data_every_N_refresh)
            refresh_count = 0;

        if ( (activePage() == page_id) & (refresh_count == 0))
        {

            float tempo = scope->scope_x_scale*scope->scope_time_base;
            send("t9.txt=\""+ String(tempo, 4).substring(0,4) + "\"" );
            send("t10.txt=\"ms/d\"");


            send("t0.txt=\"" + scope->getSource(0) + "\"");
            send("t1.txt=\"" + scope->getSource(1) + "\"");
            send("t2.txt=\"" + scope->getSource(2) + "\"");    
            send("t3.txt=\"" + scope->getScale(0, med) + "\"");
            send("t4.txt=\"" + scope->getScale(1, med) + "\"");
            send("t5.txt=\"" + scope->getScale(2, med) + "\""); 

            char c;
            int32_t ic;
            send("ref_stop");
            delay(1);

            int16_t x_start = med->window.start + (scope->x_pos * 40 * scope->scope_x_scale);
            if(x_start<med->window.start)
            {
                x_start = med->window.start;
                scope->x_lim_n = true;
            }
            else if((x_start + scope->scopeLength*scope->scope_x_scale) > BuffSize)
            {
                x_start = BuffSize - scope->scopeLength*scope->scope_x_scale;
                scope->x_lim_p = true;
            }
            for(uint16_t ch=0; ch<scope->scopeNch; ch++)
            {
                send("addt 2," + String(ch) + "," + String(scope->scopeLength));   // pula o chanal 0 do nextion pq ele ncoloca lixo na memoria
                delay(5);
                //uint16_t data_ch = (window_ch + ch*3) % 6;
                for(uint16_t k=0; k<scope->scopeLength; k++)
                {
                    if(scope->scope_ch_selection[ch] < (scope->dataNch-1) )
                        ic = (int32_t) ( (float)((float)scope->data[scope->scope_ch_selection[ch]][x_start + k*scope->scope_x_scale] * (int32_t)scope->scope_y_scale[ch])*.0625f + 128.0f);
                    else if (scope->scope_ch_selection[ch] == (scope->dataNch-1))
                        ic = (int32_t) ( (float)((float)scope->data[scope->scope_ch_selection[ch]][x_start + k*scope->scope_x_scale] * (int32_t)scope->scope_y_scale[ch])*.0009765625f + 128.0f);
                    else
                        ic= (int32_t) 255; // ch off
                    
                    if(ic>=255)
                        c = 255;
                    else if(ic<=0)
                        c = 0;
                    else
                        c = (char) (ic);

                    serial->write(c);
                }
                serial->write(0xff); serial->write(0xff); serial->write(0xff);
                delay(5);
            }
            send("ref_star");
        }
    }

};



template<typename F, uint16_t BuffSize>
class Page1 : public Nextion
{
    private:
        uint16_t refresh_count = 0;
        uint16_t send_new_data_every_N_refresh = 2;

    public:
    Medidor<F, BuffSize> *med;
    FFT<F, BuffSize> *fft;


    Page1(Medidor<F, BuffSize>& med, FFT<F, BuffSize>& fft)
        : med(&med), fft(&fft)
        {
            page_id = 1;
        }

    void handleCommand(String str) override
    {
        if       (str.equals("f1"))  { if(med->is_running()) med->changeFreqChannel(); }
        //----------------------------------------------------------------
        else if  (str.equals("b1"))  { changePage(0); }
        else if  (str.equals("b0"))  { changePage(1); }
        else if  (str.equals("sr"))  { changePage(2); }
        else if  (str.equals("pt"))  { changePage(3); }
        //----------------------------------------------------------------

    }

    void refresh() override
    {
        if (activePage() == page_id)
        {
            fft->calc();
            
            send(String("x16.val=" + String((int)(fft->v1*10.0f))));
            send(String("x19.val=" + String((int)(fft->f1))));
            send(String("x20.val=" + String((int)(fft->thd*100.0f))));

            for(uint16_t k=0; k<fft->Nbars; k++)
            {
                send(String("j" + String(k) +".val=" + String(fft->j[k])));
                if(fft->saturated[k])
                    send(String("j" + String(k) +".ppic=20" ));
                else
                    send(String("j" + String(k) +".ppic=19" ));
            }
        }
        
    }

};



template<typename F, uint16_t BuffSize>
class Page3 : public Nextion
{
    private:
        
    public:
    Medidor<F, BuffSize>* med;
    ivSymmetrical<F, BuffSize>* symm;


    Page3(Medidor<F, BuffSize>& med, ivSymmetrical<F, BuffSize>& symm)
        : med(&med), symm(&symm)
        {
            page_id = 3;
        }

    void handleCommand(String str) override
    {
        if       (str.equals("f1"))  { if(med->is_running()) med->changeFreqChannel(); }
        //----------------------------------------------------------------
        else if  (str.equals("b1"))  { changePage(0); }
        else if  (str.equals("b0"))  { changePage(1); }
        else if  (str.equals("sr"))  { changePage(2); }
        else if  (str.equals("pt"))  { changePage(3); }
        else if  (str.equals("cf"))  { changePage(4); }
        //----------------------------------------------------------------

    }


    void refresh() override
    {
        
        if (activePage() == page_id)
        {

            symm->batch();

            send(String("x0.val=" + String((int)(symm->symm_v.dq0_p[0]*10.0f))));
            send(String("x1.val=" + String((int)(symm->symm_v.dq0_p[1]*10.0f))));
            send(String("x3.val=" + String((int)(symm->symm_i.dq0_p[0]*100.0f))));
            send(String("x4.val=" + String((int)(symm->symm_i.dq0_p[1]*100.0f))));

            send(String("x6.val=" + String((int)(symm->symm_v.dq0_n[0]*10.0f))));
            send(String("x7.val=" + String((int)(symm->symm_v.dq0_n[1]*10.0f))));
            send(String("x9.val=" + String((int)(symm->symm_i.dq0_n[0]*100.0f))));
            send(String("x10.val=" + String((int)(symm->symm_i.dq0_n[1]*100.0f))));

            //send(String("x10.val=" + String((int)(symm->symm_i.dq0_n[1]*100.0f))));

        }
    }

};





template<typename F, uint16_t BuffSize>
class Page4 : public Nextion
{
    private:
        
    public:
    Medidor<F, BuffSize> *med;
    AD7266::SPI_ADC *adc;
    Nextion *page_scope;

    Page4(Medidor<F, BuffSize>& med, AD7266::SPI_ADC& adc, Nextion& page_scope)
        : med(&med), adc(&adc), page_scope(&page_scope)
    {
        page_id = 4;
    }

    void handleCommand(String str) override
    {
        //----------------------------------------------------------------
        if       (str.equals("b1"))  { changePage(0); }
        else if  (str.equals("b0"))  { changePage(1); }
        else if  (str.equals("sr"))  { changePage(2); }
        else if  (str.equals("pt"))  { changePage(3); }
        else if  (str.equals("cf"))  { changePage(4); }
        else if  (str.equals("bg"))  { changePage(2); send("page 5");}
        //----------------------------------------------------------------
        else if  (str.equals("of"))  { adc->updateOffsets(); }

    }


    void refresh() override
    {
        
        if (activePage() == page_id)
        {

            //send(String("x0.val=" + String((int)(symm->symm_v.dq0_p[0]*10.0f))));

        }
    }

};