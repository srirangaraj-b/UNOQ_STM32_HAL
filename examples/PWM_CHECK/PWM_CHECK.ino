/*
 * PWM4_Capture4.ino
 * Arduino UNO Q (STM32U585) - 4 PWM outputs + 4 independent PWM captures.
 *
 * Loopback wiring (jumper each output to its capture input, common GND):
 *
 *   PWM CH1  PA8  (TIM1 CH1)  1 kHz  25 %  ->  PE3  (TIM3 CH1)
 *   PWM CH2  PC7  (TIM8 CH2)  2 kHz  50 %  ->  PE4  (TIM3 CH2)
 *   PWM CH3  PC8  (TIM8 CH3)  2 kHz  75 %  ->  PE5  (TIM3 CH3)
 *   PWM CH4  PC9  (TIM8 CH4)  2 kHz  40 %  ->  PE6  (TIM3 CH4)
 *
 * Notes
 *  - Channels of one timer share the period, so the three TIM8 outputs
 *    all run at 45 kHz. Duty is independent per channel.
 *  - Capture is polled, so loop() spin-polls PWM_CaptureCh_PollAll()
 *    continuously and only prints between polling windows. Keep the
 *    pulses (high AND low) longer than ~100 us.
 *  - Every capture input must be wired, or pulled down; a floating
 *    input reads noise.
 */
#include "UNOQ_STM32_HAL.h"

struct PwmCh {
  const char*  name;
  GP_Pin       pin;
  TIM_TypeDef* timer;
  uint8_t      ch;
  uint8_t      af;
  uint32_t     freq;   /* Hz, only the first channel set up per timer counts */
  uint8_t      duty;   /* percent */
};

struct CapCh {
  const char*  name;
  GP_Pin       pin;
  TIM_TypeDef* timer;
  uint8_t      ch;
  uint8_t      af;
};

static const PwmCh PWM_CH[4] = {
  {"PA8", PA8, TIM1, 1, 1, 50000, 25},
  {"PC7", PC7, TIM8, 2, 3, 45000, 50},
  {"PC8", PC8, TIM8, 3, 3, 45000, 75},
  {"PC9", PC9, TIM8, 4, 3, 45000, 40},
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

/* Input glitch filter (TIMx_CCMR ICxF). 0xF = sample at fDTS/32 and require
 * 8 equal samples. With the clock divider set below this rejects spikes
 * shorter than ~6 us (jumper-wire crosstalk, ringing) and delays both edges
 * equally, so period and duty are not affected. */
static const uint8_t CAP_FILTER = 0xF;

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  Serial.println("UNO Q: 4x PWM out + 4x PWM capture");

  /* Capture timeout: a channel with no edges for this long is "no signal" */
  PWM_Capture_SetTimeout(1000);

  /* ---- 4 PWM outputs ---- */
  for (uint8_t i = 0; i < 4; i++) {
    const PwmCh& p = PWM_CH[i];
    pwmOk[i] = PWM_Setup_STM(p.pin, p.timer, p.ch, false, p.freq, p.duty, p.af);
    Serial.print("PWM "); Serial.print(p.name);
    Serial.println(pwmOk[i] ? " ok" : " FAILED");
  }

  /* ---- 4 independent captures (all on TIM3) ---- */
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
  /* Spin-poll for the whole interval. Do NOT return to the Arduino core
   * between polls: its loop() cadence is much slower than the edge rate
   * (an edge every ~250 us), so edges would be lost and nothing would sync. */
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
