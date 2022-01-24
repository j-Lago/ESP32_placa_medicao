/*
  h - symmetrical_h
  Created by Jackson Lago, Dez 14, 2021.
  Not released into the public domain.
*/
#ifndef symmetrical_h
#define symmetrical_h

#include <Arduino.h>
#include "medidor.h"
#include "Vec.h"
#include "CosSin.h"


/*
 *
 * Instantaneous Symmetrical Components
 * 
 */
template<typename F>
class Symmetrical
{
    private:
    const F um_6 = 0.16666666666666666666666666666667;
    const F sqrt3_6 = 0.28867513459481288225457439025098;
    public:
    SOGI<F> sogi[3];
    Vec3<F> abc_p;
    Vec3<F> abc_n;
    Vec3<F> dq0_p;
    Vec3<F> dq0_n;

    Symmetrical(){};

    Symmetrical(F fs)
    {
        setFs(fs);
    };

    void setFs(F fs)
    {
        for(uint16_t k=0; k<3; k++)
            sogi[k].setFs(fs);
    }


    inline void step(const Vec3<F>& abc, F w, CosSin<F>& th)
    {
        for(uint16_t ph=0; ph<3; ph++)
        {
            sogi[ph].step(abc[ph], w);
        }

        F part[6];
        part[0] = (2*sogi[0].out[0]   - sogi[1].out[0]   - sogi[2].out[0]) * um_6;
        part[1] = ( -sogi[0].out[0] + 2*sogi[1].out[0]   - sogi[2].out[0]) * um_6;
        part[2] = ( -sogi[0].out[0]   - sogi[1].out[0] + 2*sogi[2].out[0]) * um_6;
        part[3] = (                -sogi[1].out[1] + sogi[2].out[1]) * sqrt3_6;
        part[4] = ( sogi[0].out[1]                 - sogi[2].out[1]) * sqrt3_6;
        part[5] = (-sogi[0].out[1] + sogi[1].out[1]                ) * sqrt3_6;

        abc_p[0] = part[0] + part[3];
        abc_p[1] = part[1] + part[4];
        abc_p[2] = part[2] + part[5];
        abc_n[0] = part[0] - part[3];
        abc_n[1] = part[1] - part[4];
        abc_n[2] = part[2] - part[5];

        dq0_p = abc_dq0(abc_p, th);

        digitalWrite(33, true);
        dq0_n = abc_dq0(abc_n, -th);
        digitalWrite(33, false);

    }
};

template<typename F, uint16_t BuffSize>
class ivSymmetrical 
{
    public:
    Medidor<F, BuffSize>* med;
    Symmetrical<float> symm_i;
    Symmetrical<float> symm_v;

    ivSymmetrical(Medidor<F, BuffSize>& med)
        : med(&med)
        {
            symm_v.setFs(med.window.pll.getFs());
            symm_i.setFs(med.window.pll.getFs());
        }
    
    void batch()
    {
        Vec3<float> abc_v;
        Vec3<float> abc_i;
        CosSin<F> th;
        for (uint16_t k=0; k<BuffSize; k++)
        {
            abc_v[0] = med->data[0][k] * med->gains.v[0];
            abc_v[1] = med->data[1][k] * med->gains.v[1];
            abc_v[2] = med->data[2][k] * med->gains.v[2];

            
            abc_i[0] = med->data[3][k] * med->gains.i[0];
            abc_i[1] = med->data[4][k] * med->gains.i[1];
            abc_i[2] = med->data[5][k] * med->gains.i[2];


            th = med->data[6][k] * 9.5873799242852576857380474343247e-5f;  //  2^15 -> pi
            F w = med->data[7][k] * 0.02300971181828461844577131384238f;              //  2^15 -> 2*pi*120Hz
            
            symm_v.step(abc_v, w, th);
            symm_i.step(abc_i, w, th);

        }
    }

};



#endif