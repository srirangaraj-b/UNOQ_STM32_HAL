/* ============================================================================
 * PWM_UART_Control
 *
 * 4-channel PWM over UART, using the UNOQ_STM32_HAL library.
 * Same pin mapping and command set as the original hand-rolled demo:
 *
 *   CH1: TIM1_CH1  -> PA8   / TIM1_CH1N -> PB13
 *   CH2: TIM8_CH2  -> PC7   / TIM8_CH2N -> PB0
 *   CH3: TIM8_CH3  -> PC8   / TIM8_CH3N -> PB1
 *   CH4: TIM8_CH4  -> PC9   / TIM8_CH4N -> PB2
 *
 * NOTE: verify PC6 vs PC7 for CH2 against your schematic - per ST's AF table,
 * PC6=TIM8_CH1, PC7=TIM8_CH2.
 *
 * Commands over Serial (115200 baud):
 *   '<ch 1-4> <duty 0-100>'   e.g. "1 50"     -> set channel duty
 *   'p <freq_hz>'             e.g. "p 2000"   -> set frequency for all channels
 * ==========================================================================*/

#include <UNOQ_STM32_HAL.h>

static uint8_t duty_pct[4] = {0, 0, 0, 0};

#define RX_BUF_SIZE 64
static char rx_buf[RX_BUF_SIZE];
static int rx_pos = 0;

static void apply_duty(int ch)
{
    switch (ch) {
        case 0: PWM_SetDuty(TIM1, 1, true, duty_pct[0]); break;
        case 1: PWM_SetDuty(TIM8, 2, true, duty_pct[1]); break;
        case 2: PWM_SetDuty(TIM8, 3, true, duty_pct[2]); break;
        case 3: PWM_SetDuty(TIM8, 4, true, duty_pct[3]); break;
    }
    Serial.print("CH"); Serial.print(ch + 1);
    Serial.print(" -> duty="); Serial.print(duty_pct[ch]); Serial.println("%");
}

static void process_line(char *line)
{
    int ch, val;
    while (*line == ' ') line++;

    if (line[0] == 'p' || line[0] == 'P') {
        if (sscanf(line + 1, "%d", &val) == 1 && val > 0) {
            /* Frequency is per-timer; TIM1 and TIM8 are set independently
             * so they stay in lockstep the same way the original demo did. */
            PWM_SetFrequency(TIM1, 1, true, (uint32_t)val);
            PWM_SetFrequency(TIM8, 2, true, (uint32_t)val);
            for (int i = 0; i < 4; i++) apply_duty(i);
            Serial.print("Frequency set to "); Serial.print(val); Serial.println(" Hz");
        } else {
            Serial.println("Usage: p <freq_hz>");
        }
        return;
    }

    if (sscanf(line, "%d %d", &ch, &val) == 2) {
        if (ch >= 1 && ch <= 4 && val >= 0 && val <= 100) {
            duty_pct[ch - 1] = (uint8_t)val;
            apply_duty(ch - 1);
        } else {
            Serial.println("Invalid: channel 1-4, duty 0-100");
        }
        return;
    }

    Serial.println("Unknown cmd. Use: '<ch 1-4> <duty 0-100>' or 'p <freq_hz>'");
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    /* CH1 on TIM1 (advanced timer, complementary + dead-time capable) */
    PWM_Setup_STM(PA8, TIM1, 1, true, 1000, 0, 1);   /* PA8  = TIM1_CH1,  AF1 */
    PWM_Setup_STM(PB13, TIM1, 1, true, 1000, 0, 1);  /* PB13 = TIM1_CH1N, AF1 - shares CH1 config */
    PWM_SetDeadTime(TIM1, 500, UNOQ_TIM_CLK_HZ);      /* 500ns dead time */

    /* CH2-CH4 on TIM8 - first PWM_Setup_STM configures the timer's PSC/ARR,
     * the rest are added with PWM_AddChannel_STM so they share the frequency. */
    PWM_Setup_STM(PC7, TIM8, 2, true, 1000, 0, 3);    /* PC7 = TIM8_CH2, AF3 */
    PWM_Setup_STM(PB0, TIM8, 2, true, 1000, 0, 3);    /* PB0 = TIM8_CH2N, AF3 */

    PWM_AddChannel_STM(PC8, TIM8, 3, true, 0, 3);     /* PC8 = TIM8_CH3, AF3 */
    PWM_AddChannel_STM(PB1, TIM8, 3, true, 0, 3);     /* PB1 = TIM8_CH3N, AF3 */

    PWM_AddChannel_STM(PC9, TIM8, 4, true, 0, 3);     /* PC9 = TIM8_CH4, AF3 */
    PWM_AddChannel_STM(PB2, TIM8, 4, true, 0, 3);     /* PB2 = TIM8_CH4N, AF3 */

    PWM_SetDeadTime(TIM8, 500, UNOQ_TIM_CLK_HZ);

    Serial.println("Ready. CH1=TIM1_CH1(PA8/PB13) CH2=TIM8_CH2(PC7/PB0) CH3=TIM8_CH3(PC8/PB1) CH4=TIM8_CH4(PC9/PB2)");
    Serial.println("Commands: '<ch 1-4> <duty 0-100>'  e.g. '1 50'");
    Serial.println("          'p <freq_hz>'            e.g. 'p 2000'");
}

void loop()
{
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\r') continue;
        if (c == '\n') {
            rx_buf[rx_pos] = '\0';
            if (rx_pos > 0) process_line(rx_buf);
            rx_pos = 0;
        } else if (rx_pos < RX_BUF_SIZE - 1) {
            rx_buf[rx_pos++] = c;
        } else {
            rx_pos = 0;
        }
    }
}
