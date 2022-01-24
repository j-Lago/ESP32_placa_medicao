/*
  h - cossin_h
  Created by Jackson Lago, Dez 15, 2021.
  Not released into the public domain.
*/
#ifndef cossin_h
#define cossin_h

#include <Arduino.h>


namespace fast_math 
{
    const float um_pi = 0.31830988618379067153776752674503;
    const float um_pi2 = 0.1591549430918953357688837633725;
    const float pi = 3.1415926535897932384626433832795;
    const float pi2 = 6.283185307179586476925286766559;
    const float pi_2 = 1.5707963267948966192313216916398;
    const float deg_rad = 0.01745329251994329576923690768489;
    const float rad_deg = 57.29577951308232087679815481410;
    
    const float coeff_arcsin[13]{   1.000000000000000000,    // x
                                    0.166666666666667000,    // x^3
                                    0.075000000000000000,    // x^5
                                    0.044642857142857100,    // x^7
                                    0.030381944444444400,    // x^9
                                    0.022372159090909100,    // x^11
                                    0.017352764423076900,    // x^13
                                    0.013964843750000000,
                                    0.011551800896139700,
                                    0.009761609529194080,
                                    0.008390335809616820,
                                    0.007312525873598850,
                                    0.006447210311889650
    };


    const float coeff_cossin[13]{ 0.0,                  //      x
                                  1.0,                  // x^2
                                 -0.500000000000000,    //      x^3
                                 -0.166666666666667,    // x^4
                                  0.0416666666666667,   //      x^5
                                  0.00833333333333333,	// x^6
                                 -0.00138888888888889,	//      x^7
                                 -0.000198412698412698, // x^8
                                  2.48015873015873e-05, //      x^9 
                                  2.75573192239859e-06, // x^10
                                 -2.75573192239859e-07, //      x^11
                                 -2.50521083854417e-08, // x^12
                                  2.08767569878681e-09  //      x^13
    };
};


template<typename T>
T fast_asin(T sinx)
{
    using namespace fast_math;
    T xx = sinx * sinx;
    T asinx =  sinx + sinx * (xx * (coeff_arcsin[1] + xx * (coeff_arcsin[2] + xx * (coeff_arcsin[3] + xx * (coeff_arcsin[4] + xx * (coeff_arcsin[5] + xx * (coeff_arcsin[6])))))));
    return asinx;
}

template<typename T>
T fast_acos(T cosx)
{
    using namespace fast_math;
    return pi_2 - fast_asin(cosx);
}

template<typename T>
T fast_sin(T rad)
{
    using namespace fast_math;
        
    int ndiv;
    T x;
    if(fabs(rad)>pi_2)
    {
        if(rad>=0)
            ndiv = (int)((rad+pi) * um_pi2);
        else
            ndiv = (int)((rad-pi) * um_pi2);

        x = rad - ndiv*pi2;
    }
    else
        x = rad;

    T xx = x * x;
    T sinx =  x + x * (xx * (coeff_cossin[3] + xx * (coeff_cossin[5] + xx * (coeff_cossin[7] + xx * (coeff_cossin[9] + xx * (coeff_cossin[11] + xx * (coeff_cossin[13])))))));

    return sinx;
}

template<typename T>
T fast_cos(T rad)
{
    using namespace fast_math;
        
    int ndiv;
    T x = fabs(rad);
    if(rad>pi_2)
    {
        ndiv = (int)((rad+pi) * um_pi2);
        x = rad - ndiv*pi2;
    }
        
    T xx = x * x;
    T cosx =  1 + (xx * (coeff_cossin[2] + xx * (coeff_cossin[4] + xx * (coeff_cossin[6] + xx * (coeff_cossin[8] + xx * (coeff_cossin[10] + xx * (coeff_cossin[12])))))));
    return cosx;
}

template<typename T>
class CosSin
{
    private:
    union {
        struct { T m_cos, m_sin;};
        T cossin[2];
    };
    T m_rad;

    public:
    CosSin()
    {
        m_rad = 0;
        m_sin = 0;
        m_cos = 1;
    }

    CosSin(const CosSin& cs)
    {
        m_sin = cs.sin();
        m_cos = cs.cos();
        m_rad = cs.rad();
    }

    CosSin(const CosSin *cs)
    {
        m_sin = cs->sin();
        m_cos = cs->cos();
        m_rad = cs->rad();
    }

    CosSin(T rad)
    : m_rad(rad)
    {
        fast_cossin(rad);
    }

    CosSin(T cos, T sin)
    :m_cos(cos), m_sin(sin)
    {
        cossin_to_angle();
    }

    T rad(void)  const
    {return m_rad;}

    T deg(void)  const
    {
        return m_rad * fast_math::rad_deg;
    }

    T sin(void)  const
    {return m_sin;}

    T cos(void)  const
    {return m_cos;}

    void rad(T rad)
    {
        this->m_rad = rad;
        fast_cossin(m_rad);
    }

    void neg()
    {
        m_sin = -m_sin;
        m_rad = m_rad;
    }

    void deg(T deg)
    {
        this->m_rad = rad * fast_math::deg_rad;
        fast_cossin(m_rad);
    }

    CosSin<T> operator-() const
    {
        CosSin<T> temp(this);
        temp.neg();
        return temp;
    }

    CosSin<T>& operator=(T rad)
    {
        this->rad(rad);
        return *this;
    }

    CosSin<T>& operator=(T cs[2])
    {
        setCosSin(cs[0], cs[1]);
        return *this;
    }
    
    void setCosSin(T cos, T sin)
    {
        this->m_sin = sin;
        this->m_cos = cos;
        cossin_to_angle();
    }

    T operator[](uint16_t id) const
    {
        return cossin[id];
    }


    private:
    void fast_cossin(T th)
    /*
     *  sin = x - 1/3! * x^3 + 1/5! * x^5 - 1/7! * x^7 +...
     *  cos = 1 - 1/2! * x^2 + 1/4! * x^4 - 1/6! * x^6 +...
     */
    {
        using namespace fast_math;
        
        int ndiv;
        if(fabs(th)>pi_2)
        {
            if(th>=0)
                ndiv = (int)((th+pi) * um_pi2);
            else
                ndiv = (int)((th-pi) * um_pi2);

            m_rad = th - ndiv*pi2;
        }
        else
            m_rad = th;
        

        T xx = m_rad * m_rad;
        m_cos =  1 + (xx * (coeff_cossin[2] + xx * (coeff_cossin[4] + xx * (coeff_cossin[6] + xx * (coeff_cossin[8] + xx * (coeff_cossin[10] + xx * (coeff_cossin[12])))))));
        m_sin =  m_rad + m_rad * (xx * (coeff_cossin[3] + xx * (coeff_cossin[5] + xx * (coeff_cossin[7] + xx * (coeff_cossin[9] + xx * (coeff_cossin[11] + xx * (coeff_cossin[13])))))));
    }

    T cossin_to_angle()
    {
        using namespace fast_math;
        bool sign_sin = (this->m_sin<0)? 0 : 1;
        bool sign_cos = (this->m_cos<0)? 0 : 1;
        if (abs(this->m_sin) <= abs(this->m_cos))  // melhor método (arcsin ou arccos) é o de menor valor entre sin ou cos
        {
            if(sign_cos)
                m_rad = fast_asin(this->m_sin);
            else if(sign_sin)
                m_rad = pi-fast_asin(this->m_sin);
            else
                m_rad = -pi-fast_asin(this->m_sin);
        }
        else 
        {
            if(sign_sin)
                m_rad = fast_acos(this->m_cos);
            else
                m_rad = -fast_acos(this->m_cos);
        }
        return m_rad;
    }

};

template<typename T>
String sincos2str(const CosSin<T> &sc)
{
    return String("rad=" + String(sc.rad(), 5) + " <-> [cos=" + String(sc.sin(),5) + ", sin=" + String(sc.cos(),5) + "]");
}




#endif