/*
  h - gpio_funcionati

  funcionalidades botões

  Created by Jackson Lago, Dez 22, 2021.
  Not released into the public domain.
*/

#pragma once
#include "Arduino.h"

enum led_mode {on_off, blink, blink_fast, pulse, pulse_fast};


class StateControl
{
    public:
    bool state = false;


    StateControl(){}

    void set()
    { 
        state = true; 
    }

    void clear()
    { 
        state = false; 
    
    }
    bool toggle()
    { 
        state = !state; 
        return state; 
    
    }
    void operator()(bool new_state) 
    {
        state = new_state; 
    }

    bool operator()() const 
    { 
        return state; 
    }
};

class Led : public StateControl
{
    /*
     *  funcionalidades para leds
     */
    private:
    uint16_t pin;
    bool active_low = false;
    uint32_t count = 0;
    uint32_t slow_period = 6;
    uint32_t fast_period = 0;
    uint32_t period = slow_period;
    bool blink_out = false;
    bool pulse_out = false;
    


    public:
    led_mode mode = on_off;

    Led(uint16_t pin)
        :pin(pin)
    {
        pinMode(pin, OUTPUT);
        update();
    }

    Led(uint16_t pin, bool active_low)
        :pin(pin), active_low(active_low)
    {
        pinMode(pin, OUTPUT);
        update();
    }

    Led(uint16_t pin, bool active_low, uint32_t slow_period, uint32_t fast_period)
        :pin(pin)
    {
        pinMode(pin, OUTPUT);
        config(active_low, slow_period, fast_period);
        update();
    }

    void config(bool active_low, uint32_t slow_period, uint32_t fast_period)
    {
        this->active_low = active_low;
        this->slow_period = slow_period;
        this->fast_period = fast_period;

    }

    //void set(){state = true;}
    //void clear(){state = false;}
    //void toggle() {state = !state;}


    void update()
    {
        period = (mode==blink_fast)? fast_period : slow_period;

        if (count >= period)
        {
            count = 0;
            blink_out = !blink_out;
            pulse_out = false;
            if( (mode == pulse) | (mode == pulse_fast) )
                state = false;
        }
        else
        {
            count++;
            pulse_out = true;
        }



        bool out;
        switch (mode)
        {
            break; case on_off:     out = state;
            break; case blink:      out = blink_out & state;
            break; case blink_fast: out = blink_out & state;
            break; case pulse:      out = pulse_out & state;
            break; case pulse_fast: out = pulse_out & state;
            break; default:         out = false;
        }



        digitalWrite(pin, out ^ active_low);
    }



};

class Button
{
    /*
     *  funcionalidades para botões físicos
     */
    private:
    uint16_t pin;
    bool last_state;
    bool active_low = false;
    bool hold_cleared = false;
    bool prevent_next_release = false;  // naão identificar como release  a saída de um hold
 
    uint16_t hold_window = 4;               // número de updates para deterctar hold
    uint16_t double_press_window = 5;       // número de updates para detectar double_press   
    int16_t hold_timer = 0;
    int16_t double_press_timer = 0;



    public:
    bool state;
    bool press = false;
    bool release = false;

    bool press_flag = false;
    bool release_flag = false;
    bool double_press_flag = false;
    bool hold_flag = false;

    Button(uint16_t pin)
        : pin(pin)
    {
        pinMode(pin, INPUT);
        update();
    }

    Button(uint16_t pin, bool active_low, uint16_t double_press_window, uint16_t hold_window)
        : pin(pin)
    {
        config(active_low, double_press_window, hold_window);
        pinMode(pin, INPUT);
        update();
    }

    Button(uint16_t pin, bool active_low)
        : pin(pin), active_low(active_low)
    {
        pinMode(pin, INPUT);
        update();
    }

    void config(bool active_low, uint16_t double_press_window, uint16_t hold_window)
    {
        this->active_low = active_low;
        this->double_press_window = double_press_window;
        this->hold_window = hold_window;
    }

    void clear_flags()
    {
        
        if (double_press_flag)
            double_press_timer = 0;

        if (hold_flag)
            hold_cleared = true;

        press_flag = false;
        release_flag = false;
        double_press_flag = false;
        hold_flag = false;
        
    }

    void update()
    {
        last_state = state;
        state = active_low ^ digitalRead(pin);

        press = (state & !last_state);
        release = (!state & last_state);

        if(release & !prevent_next_release)
            release_flag = true;

        if(press)
        {
            press_flag = true;
            prevent_next_release = false;

            if (double_press_timer > 0)
                double_press_flag = true;

            hold_cleared = false;
            hold_timer = hold_window;
            double_press_timer = double_press_window;
        }

        if ((hold_timer==0) & state & !hold_cleared)
        {
            hold_flag = true;
            prevent_next_release = true;
        }


        double_press_timer--;
        if (double_press_timer <= 0)
            double_press_timer = 0;

        hold_timer--;
        if (hold_timer <= 0)
            hold_timer = 0;

    }
};