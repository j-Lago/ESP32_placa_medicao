/*
 * Ferramentas ->
 *    placa: DOIT ESP32 DEVKIT V1
 *    apertar gravação enqunato "Connecting....."
 */
#define CONFIG_DISABLE_HAL_LOCKS 1

#include "globals.h"


void setup(void);
void IRAM_ATTR onTimer(void);
void loop(void);
void loop0(void*);
void loop1(void*);

void cpu0();
void cpu1();



void setup() {
  disableCore0WDT();

  led.mode = blink_fast;
  led.set();
  
  //conector GPIO
  pinMode(22, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(34, INPUT);
  pinMode(35, INPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);

  // initialize serial communication at 9600 bits per second:
  Serial.begin(USB_BAULD_RATE);  // USB
  Serial2.begin(HMI_BAULD_RATE); // DISPLAY



  spi_adc.setup();
  spi_adc.loadOffsets();

  med.window.pll.align_sin = true; //saída do ângulo do pll alinhada com seno



  
  delay(2000);
  Serial2.print("rest");
  Serial2.write(0xff); Serial2.write(0xff); Serial2.write(0xff);
  Serial2.print("rest");
  Serial2.write(0xff); Serial2.write(0xff); Serial2.write(0xff);
  delay(500);
  Serial2.write(0xff); Serial2.write(0xff); Serial2.write(0xff);


  //Timer
  timer = timerBegin(0, TIMER_PRESCALE, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, TIMER_PERIOD, true); 
  timerAlarmEnable(timer);
  timerMux = portMUX_INITIALIZER_UNLOCKED;

  Serial.println("end of config!\n\n");

  xTaskCreatePinnedToCore(loop0, "loop0", 8192, NULL, 10, NULL, 0); //Cria a tarefa "loop2()" com prioridade 1, atribuída ao core 0
  xTaskCreatePinnedToCore(loop1, "loop1", 8192, NULL, 3, NULL, 1); //Cria a tarefa "loop1()" com prioridade 1, atribuída ao core 1
  delay(10);
}

// Timer
void IRAM_ATTR onTimer() {
  //digitalWrite(21, HIGH);
  //digitalWrite(21, LOW);
  //portENTER_CRITICAL_ISR(&timerMux);
  new_timer_interrupt = true;
  //portEXIT_CRITICAL_ISR(&timerMux);
  
}


/*
 * CPU1 loop
 */ 
void loop1(void *pvParameters){
  

  while(true)
    if(prerare_next)
    {
      digitalWrite(32, HIGH);

      //fft.Compute(FFT_FORWARD); 
      //fft.ComplexToMagnitude();

      cpu1();


      //portENTER_CRITICAL(&dataMux);
      new_data = false;
      prerare_next = false;
      //portEXIT_CRITICAL(&dataMux);
      digitalWrite(32, LOW);
    }
 }


/*
 * CPU0 loop
 */ 
void loop0(void *pvParameters) {
  while(true){
    if (new_timer_interrupt) {
      digitalWrite(22, HIGH);

      cpu0();
      
      

      //portENTER_CRITICAL(&dataMux); // ~1us
      if(med.window.id() > BUFFER_SIZE-5)
        prerare_next = true;

      if(med.window.id() == 0)
        new_data = true;


      //portEXIT_CRITICAL(&dataMux);
      //portENTER_CRITICAL(&timerMux); // ~1us
      new_timer_interrupt=false;
      //portEXIT_CRITICAL(&timerMux);
      digitalWrite(22, LOW);
    }
  }
}

void loop() {}

#include "cpu0.tpp"
#include "cpu1.tpp"
