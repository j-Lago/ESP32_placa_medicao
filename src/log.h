/*
  h - log

  funcionalidades para Log via USB  

  Created by Jackson Lago, Dez 22, 2021.
  Not released into the public domain.
*/

#pragma once
#include "Arduino.h"

class SerialLog
{
    private:
    String str;
    HardwareSerial *port;

    public:
    SerialLog(HardwareSerial* port)
    : port(port)
    {
        clear();
    }

    void clear()
    {
        str = "";
    }

    void add(String s)
    {
        str += s;
    }

    void operator()(String s)
    {
        add(s);
    }

    void operator()(float f)
    {
        add(String(f, 5));
    }

    void operator()(int i)
    {
        add(String(i));
    }

    void print()
    {
        port->print(str);
        clear();
    }

    void println()
    {
        port->println(str);
        clear();
    }
};