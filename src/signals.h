/*
  h - signals_h
  Created by Jackson Lago, Dez 20, 2021.
  Not released into the public domain.
*/
#ifndef signals_h
#define signals_h

#include <Arduino.h>
#include "Vec.h"
#include "CosSin.h"



namespace signals 
{
    template<typename F>
    inline Vec2<F> alphabeta_dq(Vec2<F> &alphabeta, CosSin<F> th)
    {
        Vec2<F> dq;

        dq[0] =  th.cos() * alphabeta[0] + th.sin() * alphabeta[1];
        dq[1] = -th.sin() * alphabeta[0] + th.cos() * alphabeta[1];

        return dq;
    }

    
template<typename F>
inline Vec3<F> abc_dq0(const Vec3<F> &abc, const CosSin<F>& th)
{
   
  Vec3<F> dq0;
  
  F alphabeta[2];
  alphabeta[0] = (2*abc[0] - abc[1] - abc[2]) * 0.33333333333333333333333333333333f;
  alphabeta[1] = (abc[1] - abc[2])            * 0.57735026918962576450914878050196f;

  dq0[0] =  th.cos() * alphabeta[0] + th.sin() * alphabeta[1];
  dq0[1] = -th.sin() * alphabeta[0] + th.cos() * alphabeta[1];
  dq0[2] = (abc[0] + abc[1] - abc[2]) * 0.33333333333333333333333333333333f;

  return dq0;
}



    template<typename F>
    class Integral
    /*
     *    out = (in + in_1) * kT + out_1
     *
     *    kT = 1 / (2*Fs)
     */
    {
        private:
        F kT;
        F in_1;

        public:
        F out;

        Integral(){};

        Integral(F fs)
        {
            setFs(fs);
        };

        void reset()
        {
            in_1 = 0;
            out = 0;
        }

        void setFs(F fs)
        {
            kT = 0.5 / fs;
            reset();
        }

        F step(F in)
        {
            out += (in + in_1) * kT;
            in_1 = in;
            return out;
        }
    };

    template<typename F>
    class WrappedIntegral
    /*
     *    out = (in + in_1) * kT + out_1
     *
     *    kT = 1 / (2*Fs)
     *    out wrapp around {out_min, out_max}
     */
    {
        private:
        F kT;
        F in_1;
        F wrapp_lim[2];
        F wrapp_delta;

        public:
        F out;

        WrappedIntegral(){}

        WrappedIntegral(F fs, F out_min, F out_max)
        {
            setParameters(fs, out_min, out_max);
        }

        void reset()
        {
            in_1 = 0;
            out = 0;
        }

        void setParameters(F fs, F out_min, F out_max)
        {
            setLims(out_min, out_max);
            setFs(fs);
        }

        void setLims(F out_min, F out_max)
        {
            wrapp_lim[0] = out_min;
            wrapp_lim[1] = out_max;
            wrapp_delta = out_max - out_min;
        }

        void setFs(F fs)
        {
            kT = 0.5 / fs;
            reset();
        }

        F step(F in)
        {
            out += (in + in_1) * kT;
            in_1 = in;

            if(out<wrapp_lim[0])
                out += wrapp_delta;
            else if (out>wrapp_lim[1])
                out -= wrapp_delta;

            return out;
        }
    };

    template<typename T, uint16_t order>
    class TF
    {
        /*
        *   Nth order Transfer function:
        *
        *               out   kn0*z^O + kn1*z ... + knN
        *        G(z) = --- = ---------------------------
        *               in    kd0*z^O + kd1*z ... + kdN
        * 
        *        out[k] = kn0*in[k-0] + kn1*in[k-1] ... + knN*in[k-N] +
        *                   -kd1*out[k-1] - kd2*out[k-2] ... -kdN*out[k-N]
        */
        
        public:
        T knum[order+1]; //[kn0, kn1, ..., knN]
        T kden[order+1]; //[ 1 , kd1, ..., kdN]

        T in[order+1];  //[in(k), in(k-1), ..., in(k-N)]
        T out[order+1]; //[out(k), out(k-1), ..., out(k-N)]

        TF(){}

        TF(T num[order+1], T den[order+1])
        {
            setNumDen(num, den);
        }

        void setNumDen(T num[order+1], T den[order+1])
        {
            for(uint16_t k=0; k<order+1; k++)
            {
            knum[k] = num[k];
            kden[k] = den[k];
            }
            reset();
        }

        void reset()
        {
            for(uint16_t k=0; k<order+1; k++)
            {
            in[k] = 0;
            out[k] = 0;
            }
        }

        inline T step(T input)
        {
            for(uint16_t k=order; k>0; k--)
            {
            in[k] = in[k-1];
            out[k] = out[k-1];
            }
            in[0] = input;
            
            T o = knum[0]*in[0];
            for(uint16_t k=1; k <= order; k++)
            o += knum[k]*in[k] - kden[k]*out[k];

            //T o = knum[0]*in[0] + knum[1]*in[1] + knum[2]*in[2] - kden[1]*out[1] - kden[2]*out[2];

            out[0] = o;
            return o;
        }
    };
    
    template<typename F>
    class SOGI
    {
        /*
        *  Second Order Generalized Integrator
        */

        Integral<F> integA;
        Integral<F> integB;
        F sci = 0.7;

        public:
        Vec2<F> out;

        SOGI(){}

        void setFs(F fs)
        {
            integA.setFs(fs);
            integB.setFs(fs);
        }

        SOGI(F fs)
        {
            setFs(fs);
        }

        inline void step(F in, F w)
        {
            F err1 = (in - integA.out) * sci;
            F err2 = err1 - integB.out;

            integA.step(err2*w);
            integB.step(integA.out*w);

            out[0] = integA.out;
            out[1] = integB.out;
        }
    };

    template<typename F>
    class PLLdq
    {
    /*
    * SOGI -> PARK -> PLL
    */
        private:
        F m_fs;

        const F dMIN = 30;
        const F qMAX = 10;

        F pi_num[2] = {0.080018088888889f,	-0.079981911111111f};
        F pi_den[2] = {1.0f, -1.0f};

        WrappedIntegral<F> integ;

        const F w0 = 2*PI*60;

        public:
        bool align_sin = false;
        SOGI<F> sogi;
        TF<F, 1> pi;
        
        F w = 0;
        Vec2<F> dq;
        CosSin<F> th;
        
        bool out = false;
        bool locked = false;

        PLLdq(){}

        PLLdq(F fs)
        {
            setFs(fs);
        }

        PLLdq(F fs, bool align_sin)
            :align_sin(align_sin)
        {
            setFs(fs);
        }

        void setFs(F fs)
        {
            sogi.setFs(fs);
            pi.setNumDen(pi_num, pi_den);
            integ.setParameters(fs, -fast_math::pi, fast_math::pi);
            m_fs = fs;
        }

        F getFs()
        {
            return m_fs;
        }

        inline bool step(F in)
        {
            sogi.step(in, w);

            dq = alphabeta_dq(sogi.out, th);

            if(align_sin)
            {
                w = pi.step(dq[0]) + w0;
                th = integ.step(w);
                locked = (fabs(dq[0])<qMAX) & ((-dq[1])>dMIN);
            }
            else
            {
                w = pi.step(dq[1]) + w0;
                th = integ.step(w);
                locked = ((dq[0])>dMIN) & (fabs(dq[1])<qMAX);
            }

            out = th.cos() >= 0;
            

            return out;
        }

        String str()
        {
            return String("PLL: " + String(w*0.15915494309189533576888376337251f) + " Hz (" + ((locked) ? "locked)" : "unlocked)"));
        }
    };

    template<typename F>
    class Window
    {
        private:
        const int16_t min_cycles = 3;   // define f1 min
        const int16_t max_cycles = 20;  // define f1 max

        
        int16_t m_k;
        int16_t m_kstart;
        int16_t m_kend;
        int16_t m_cycle_id;   // -1 enquando início de ciclo não for detectado
        int16_t m_max_steps;
        bool m_triggered;     // o primeiro cruzamento por zero dentro da janela já foi detectado
        


        public:
        PLLdq<F> pll;
        float fs;
        bool valid; // janela anterior válida?
        bool sync;   // pulse no início da janela
        int16_t cycles;
        int16_t start;
        int16_t end;

        Window(){};

        Window(F fs, int16_t max_steps)
            : m_max_steps(max_steps)
        {
            setFs(fs);
        }

        void setLenght(int16_t max_steps)
        {
            m_max_steps = max_steps;
            reset();
        }

        void setFs(F fs)
        {
            this->fs = fs;
            pll.setFs(fs);
            reset();
        }

        void reset()
        {
            m_k = 0;
            m_kstart = 0;
            m_kend = 0;
            m_cycle_id = -1;
            m_triggered = false;

            valid = false;
            sync = false;
            cycles = 0;
            start = 0;
            end = 0;
        }

        uint16_t len()
        {
            return m_max_steps;
        }


        void step(F in)
        {
            bool pll_polarity_1 = pll.out;    // polaridade do pll no step anterior
            pll.step(in);

            bool cross = pll.out & !pll_polarity_1; // início de ciclo
            
            if(cross)
            {
                m_cycle_id++;
                if(!m_triggered)
                    m_kstart = m_k;
                
                m_kend = m_k - 1;
                m_triggered = true;
            }
            

            if (++m_k < m_max_steps)
                sync = false;
            else
            {
                sync = true;
                cycles = m_cycle_id;
                valid = pll.locked & (cycles > min_cycles) & (cycles < max_cycles);

                if (valid)
                {
                    start = m_kstart;
                    end = m_kend;
                }
                else
                {
                    start = 0;
                    end = m_max_steps-1;
                    cycles = 0;
                }

                m_k = 0;
                m_kstart = 0;
                m_kend = 0;
                m_cycle_id = -1;
                m_triggered = false;
            }

        }

        void dummy_step()
        {
            if (++m_k < m_max_steps)
                sync = false;
            else
            {
                sync = true;
                m_k = 0;

            }

        }

        uint16_t id()
        {
            return m_k;
        }


        String str()
        {
            return String(pll.str() + ", Window: (valid: " + String(valid) + ", cycles: " + String(cycles) +  ", start: " + String(start) + ", end: " + String(end) +  ")");
        }

    };


    template<typename F>
    inline F rms(int16_t* data, Window<F>& window)
    {
        Integral<float> integ(window.fs);
        int16_t x;

        float ganho_per = window.fs / (window.end - window.start);
        for(uint32_t k = window.start; k < window.end; k++)
        {
            x = data[k];
            integ.step(x*x);
        }
        return sqrt(integ.out*ganho_per);
    }

    template<typename F>
    inline float avg(int16_t* data, Window<F>& window)
    {
    Integral<float> integ(window.fs);

    float ganho_per = window.fs / (window.end - window.start);
    for(uint32_t k=window.start; k < window.end; k++)
    {
        integ.step(data[k]);
    }
    return (integ.out*ganho_per);
    }

    template<typename F>
    inline float active_power(int16_t* v, int16_t* i, Window<F>& window)
    {
        Integral<F> integ(window.fs);
        float ganho_per = window.fs / (window.end - window.start);
        for(uint32_t k=window.start; k < window.end; k++)
        {
            integ.step(v[k]*i[k]);
        }
        return (integ.out*ganho_per);
    }

    template<typename F>
    inline F energy(int16_t* v, int16_t* i, Window<F>& window)
    {
        Integral<F> integ(window.fs);
        for(uint32_t k=0; k < window.len(); k++)
        {
            integ.step(v[k]*i[k]);
        }
        return (integ.out);
    }

};
#endif