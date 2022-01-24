/*
  h - fft
  Created by Jackson Lago, Dez 22, 2021.
  Not released into the public domain.
*/

#pragma once
#include "Arduino.h"
#include "arduinoFFT.h"



template<typename F, uint16_t BuffSize>
class FFT
{
  public:
  Medidor<F, BuffSize> *med;

  static const uint16_t DOWNSAMPLES_RATIO = 1;
  static const uint16_t fft_SAMPLES =  2048;      // limite(tempo): 2048

  static const uint16_t Nbars = 46;

  uint16_t count_windows = DOWNSAMPLES_RATIO;

  double* fft_data_re;
  double* fft_data_im;

  double f1;
  double v1;
  double thd;

  bool saturated[Nbars];
  int v[Nbars];
  uint16_t j[Nbars];

  FFT(Medidor<F, BuffSize>& med)
      : med( &med ){}


  void _calc()
  {
    digitalWrite(21, HIGH);
    fft_data_re = new double[fft_SAMPLES];
    fft_data_im = new double[fft_SAMPLES];
    digitalWrite(21, LOW);
  
    arduinoFFT fft = arduinoFFT(fft_data_re, fft_data_im, fft_SAMPLES, FFT_FORWARD);

    digitalWrite(21, HIGH);

    fft.Compute(FFT_FORWARD); 

    digitalWrite(21, LOW);

    fft.ComplexToMagnitude();

    digitalWrite(21, HIGH);

    delete[] fft_data_re;
    delete[] fft_data_im;

    digitalWrite(21, LOW);
    
  }

  void calc()
  {

    float gain;
    if(med->pll_ch < 3)
      gain = med->gains.v_ch_scale_gain[med->pll_ch][med->gains.v_ch_scale_selection[med->pll_ch]];
    else if (med->pll_ch < 6)
      gain = med->gains.i_ch_scale_gain[med->pll_ch-3][med->gains.i_ch_scale_selection[med->pll_ch-3]];
    else
      gain = 0.0f;


    // copia apenas os dados da entrada selecionada para liberar memoria ocupado pelas demais
    int16_t *temp = new int16_t[fft_SAMPLES];   // aloca temporariamente 4kB (fft_SAMPLES=2048) /* 2*fft_SAMPLES  */
    if(med->pll_ch<6)                             
      for(uint16_t k=0; k<fft_SAMPLES; k++)
        temp[k] = med->data[med->pll_ch][k*DOWNSAMPLES_RATIO];  //todo antialiasing antes do downsample

        

    delete[] med->data; // libera 96kB (BUFFER_SIZE=8192)  /* 6*2*BUFFER_SIZE  */

    fft_data_re = new double[fft_SAMPLES]; // aloca 16kB (fft_SAMPLES=2048)  /* 8*fft_SAMPLES  */
    for(uint16_t k=0; k<fft_SAMPLES; k++)
      fft_data_re[k] = (double)(temp[k] * gain);

    delete[] temp;  // libera 4kB (fft_SAMPLES=2048)

    fft_data_im = new double[fft_SAMPLES];  // aloca 16kB (fft_SAMPLES=2048)  
    for(uint16_t k=0; k<fft_SAMPLES; k++)
      fft_data_im[k] = 0.0;


    arduinoFFT fft = arduinoFFT(fft_data_re, fft_data_im, fft_SAMPLES, FFT_FORWARD);
    //fft.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);	
    fft.Compute(FFT_FORWARD); // ~64ms (fft_SAMPLES=2048)  
    fft.ComplexToMagnitude();

    delete[] fft_data_im;  // libera 16kB (fft_SAMPLES=2048)  /* 8*fft_SAMPLES  */



    fft_data_re[0] *= .70710678118654752440084436210485f;

    uint16_t f_id = 0;
    double vmax = 0.0;
    for(uint16_t k=0; k<fft_SAMPLES/2; k++)
    {
        if(fft_data_re[k]>vmax)
        {
          vmax = fft_data_re[k];
          f_id = k;
        }
    }
    
    f1 = f_id * ((float)med->window.pll.getFs()) / ((float)(fft_SAMPLES * DOWNSAMPLES_RATIO)) + 0.5f;  //+0.5f aaredonda pra cima no truncamento
    v1 = vmax*6.9053396600248781679769957236802e-4f;
    double v1_sat = (v1>1)? v1 : 1.0f;

    double norm_factor = 1.0f / v1_sat * 6.9053396600248781679769957236802e-2f;

    thd = 0.0f;
    double amp;
    for(uint16_t k=0; k<Nbars; k++)
    {
      amp = fft_data_re[k*5]*norm_factor;
      if (k>1)
        thd += amp*amp;
      v[k] = (int)(log10(amp)*50.0f);
      if(v[k]<0)
        v[k] = 0;
      j[k] = v[k];

      if(j[k]>100)
      {
        saturated[k] = true;
        j[k]=100;
      }
      else
        saturated[k] = false;
        
    }
    thd = sqrt(thd); 

    delete[] fft_data_re;  // libera 16kB (fft_SAMPLES=2048)  /* 8*fft_SAMPLES  */

  }
};