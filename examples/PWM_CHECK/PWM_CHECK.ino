
#include <UNOQ_STM32_HAL.h>

struct PwmCh {
  const char*  name;
  GP_Pin       pin;
  TIM_TypeDef* timer;
  uint8_t      ch;
  uint8_t      af;
  uint32_t     freq;  
  uint8_t      duty;  
};

struct CapCh {
  const char*  name;
  GP_Pin       pin;
  TIM_TypeDef* timer;
  uint8_t      ch;
  uint8_t      af;
};

static const PwmCh PWM_CH[4] = {
  {"PA8", PA8, TIM1, 1, 1, 1000, 25},
  {"PC7", PC7, TIM8, 2, 3, 2000, 50},
  {"PC8", PC8, TIM8, 3, 3, 2000, 75},
  {"PC9", PC9, TIM8, 4, 3, 2000, 40},
};

static const CapCh CAP_CH[4] = {
  {"PE3", PE3, TIM3, 1, 2},
  {"PE4", PE4, TIM3, 2, 2},
  {"PE5", PE5, TIM3, 3, 2},
  {"PE6", PE6, TIM3, 4, 2},
};

static bool pwmOk[4];
static bool capOk[4];

static const uint32_t REPORT_MS = 500;

static const uint8_t CAP_FILTER = 0xF;

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  Serial.println("UNO Q: 4x PWM out + 4x PWM capture");

  PWM_Capture_SetTimeout(1000);

  for (uint8_t i = 0; i < 4; i++) {
    const PwmCh& p = PWM_CH[i];
    pwmOk[i] = PWM_Setup_STM(p.pin, p.timer, p.ch, false, p.freq, p.duty, p.af);
    Serial.print("PWM "); Serial.print(p.name);
    Serial.println(pwmOk[i] ? " ok" : " FAILED");
  }

  for (uint8_t i = 0; i < 4; i++) {
    const CapCh& c = CAP_CH[i];
    capOk[i] = PWM_CaptureCh_Init_Ex(c.pin, c.timer, c.ch, c.af, 15, CAP_FILTER);
    Serial.print("CAP "); Serial.print(c.name);
    Serial.println(capOk[i] ? " ok" : " FAILED");
  }


  Serial.println();
}

void loop()
{
  const uint32_t t0 = millis();
  while (millis() - t0 < REPORT_MS) PWM_CaptureCh_PollAll();

  for (uint8_t i = 0; i < 4; i++) {
    const PwmCh& p = PWM_CH[i];
    const CapCh& c = CAP_CH[i];

    Serial.print(p.name); Serial.print(" -> "); Serial.print(c.name);
    Serial.print("  set "); Serial.print(p.freq); Serial.print(" Hz ");
    Serial.print(p.duty);   Serial.print("%  |  ");

    if (!pwmOk[i] || !capOk[i]) {
      Serial.println("init FAILED");
      continue;
    }

    const float f = PWM_CaptureCh_GetFrequency(c.timer, c.ch);   /* 0 = no signal */
    if (f <= 0.0f) {
      Serial.println("no signal");
      continue;
    }
    const float d = PWM_CaptureCh_GetDuty(c.timer, c.ch);

    const bool fOk = fabsf(f - (float)p.freq) <= (float)p.freq * 0.01f;   /* +-1 %   */
    const bool dOk = fabsf(d - (float)p.duty) <= 1.0f;                    /* +-1 pt  */

    Serial.print("got "); Serial.print(f, 1); Serial.print(" Hz ");
    Serial.print(d, 1);   Serial.print("%  ");
    Serial.println((fOk && dOk) ? "PASS" : "MISMATCH");
  }
  Serial.println();
}
