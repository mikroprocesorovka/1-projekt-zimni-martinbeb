#include "stm8s.h"
#include "assert.h"
#include "milis.h"
#include "stm8_hd44780.h"
#include <stdio.h>
//piny na posilující tranzistory
#define LEDR_PORT GPIOD
#define LEDR_PIN GPIO_PIN_4
#define LEDB_PORT GPIOA
#define LEDB_PIN GPIO_PIN_3
#define LEDG_PORT GPIOD
#define LEDG_PIN GPIO_PIN_3
//n-coder
#define NCODER_CLK_PORT GPIOD
#define NCODER_CLK_PIN GPIO_PIN_2
#define NCODER_GET_CLK (GPIO_ReadInputPin(NCODER_CLK_PORT, NCODER_CLK_PIN)!=RESET)

#define NCODER_DATA_PORT GPIOD
#define NCODER_DATA_PIN GPIO_PIN_0
#define NCODER_GET_DATA (GPIO_ReadInputPin(NCODER_DATA_PORT, NCODER_DATA_PIN)!=RESET)

//tlačitko vyberu
#define VYBER_PORT GPIOD
#define VYBER_PIN GPIO_PIN_1
#define VYBER_n (GPIO_ReadInputPin (VYBER_PORT, VYBER_PIN)==RESET)

//tlačítko specialnich funkcí
#define SPEC_PORT GPIOE
#define SPEC_PIN GPIO_PIN_4
#define VYBER_S (GPIO_ReadInputPin (SPEC_PORT, SPEC_PIN) == RESET)

//dekladace proměných
uint32_t timeA = 0;
uint32_t timeB = 0;
uint32_t timeC = 0;
uint16_t PWM_R = 0;
uint16_t PWM_G = 0;
uint16_t PWM_B = 0;
uint8_t a_stav = 0;
uint8_t m_stav = 0;
uint8_t vyberA = 0;
uint8_t cisloA = 0;
int16_t cisloB = 0;	
int16_t cisloB_m = 0;
static int m=0;
uint8_t cisloA_m = 0;
uint8_t a_stav1 = 0;
uint8_t m_stav1 = 0;
uint8_t vyberS = 0;
uint8_t cas_n = 0;
uint8_t cas_n1 = 0;
uint8_t s = 0;

/*  ----------- Nastavení --------------- */

void setup(void)
{
	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
	
	init_milis();
	
	GPIO_Init(LEDR_PORT, LEDR_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(LEDB_PORT, LEDB_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(LEDG_PORT, LEDG_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);

	GPIO_Init(LCD_RS_PORT, LCD_RS_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(LCD_RW_PORT, LCD_RW_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(LCD_E_PORT, LCD_E_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);

	GPIO_Init(LCD_D4_PORT, LCD_D4_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(LCD_D5_PORT, LCD_D5_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);	
	GPIO_Init(LCD_D6_PORT, LCD_D6_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(LCD_D7_PORT, LCD_D7_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);

	GPIO_Init(NCODER_CLK_PORT, NCODER_CLK_PIN, GPIO_MODE_IN_FL_NO_IT);
	GPIO_Init(NCODER_CLK_PORT, NCODER_DATA_PIN, GPIO_MODE_IN_FL_NO_IT);
	GPIO_Init(VYBER_PORT, VYBER_PIN, GPIO_MODE_IN_PU_NO_IT);

	TIM2_TimeBaseInit(TIM2_PRESCALER_1, 1600 - 1);
	
	TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 1056, TIM2_OCPOLARITY_HIGH);
	TIM2_OC2Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 1056, TIM2_OCPOLARITY_HIGH);
	TIM2_OC3Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 1056, TIM2_OCPOLARITY_HIGH);
	
	TIM2_OC1PreloadConfig(ENABLE);
	TIM2_OC2PreloadConfig(ENABLE);
	TIM2_OC3PreloadConfig(ENABLE);
	
	TIM2_Cmd(ENABLE);
}

/*  ----------- Funce konrola Ncoderu --------------- */

int8_t kontrola_ncoder(void)
{
	if(m==0 && NCODER_GET_CLK==1){
		m=1;
		if(NCODER_GET_DATA==0){
			return 1;
		}
		else{
			return -1;
		}
	}
	else if(m==1 && NCODER_GET_CLK==0){
		m=1;
		if(NCODER_GET_DATA==0){
			return -1;
		}
		else{
			return 1;
		}
	}
	return 0;
}

void main(void)
{
	char text[32];
	uint32_t timeA=0;
	uint32_t timeB=0;
	uint32_t timeC=0;

	setup();
	lcd_init();
	kontrola_ncoder();

	while (1) {

		/*  ----------- Kontrola Ncoder --------------- */

		if (milis() - timeC > 7){
			timeC = milis();
			cisloA += kontrola_ncoder();
			if (vyberA == 4){
				cisloB += kontrola_ncoder();
			}
		}

		/*  ----------- Kontrola tlačítka --------------- */

		if (milis() - timeA > 100){
			timeA = milis();

			if (VYBER_n){
				a_stav = 1;
			}
			else{
				a_stav = 0;
			}
			if(m_stav == 1 && a_stav == 0){
				vyberA +=1;
				cisloA_m =cisloA;
				cisloA=0;
			}
			m_stav = a_stav;
			if (vyberA == 5){
				vyberA = 0;
			}
			if (VYBER_S){
				a_stav1 = 1;
			}
			else{
				a_stav1 = 0;
			}
			if (a_stav1 == 0 && m_stav1 == 1){
				vyberS +=1;
			}
			m_stav1 = a_stav1;
			if (vyberS == 5){
				vyberS = 0;
				vyberA = 0;
			}
		}

		/*  ----------- Zápis na diplay --------------- */

		if (milis() - timeB > 333){
			timeB = milis();
			if (vyberA == 0){
				lcd_gotoxy(0, 0);
				lcd_puts("* * * Menu * * *");
				lcd_gotoxy(0, 1);
				lcd_puts("                ");
			}
			if (vyberA == 1){
				lcd_gotoxy(0, 1);
				sprintf(text, "Cervena=%5d", cisloA);
				lcd_puts(text);
				PWM_R= cisloA * 6;
				TIM2_SetCompare1(PWM_R);
			}
			if (vyberA == 2){
				lcd_gotoxy(0, 0);
				sprintf(text, "Cervena=%5d   ", cisloA_m);
				lcd_puts(text);
				lcd_gotoxy(0, 1);

				sprintf(text, "Zelena= %5d", cisloA);
				lcd_puts(text);
				PWM_G= cisloA * 6;
				TIM2_SetCompare2(PWM_G);
			}
			if (vyberA == 3){
				lcd_gotoxy(0, 0);
				sprintf(text, "Zelena= %5d", cisloA_m);
				lcd_puts(text);

				lcd_gotoxy(0, 1);
				sprintf(text, "Modra=  %5d", cisloA);
				lcd_puts(text);
				PWM_B= cisloA * 6;
				TIM2_SetCompare3(PWM_B);
			}
			if (vyberA == 4){
				lcd_gotoxy(0, 0);
				sprintf(text, "Modra=  %5d", cisloA_m);
				lcd_puts(text);

				lcd_gotoxy(0, 1);
				sprintf(text, "Jas=    %5d", cisloB);
				lcd_puts(text);
				if (cisloB != cisloB_m){
					if (cisloB > cisloB_m){
						cisloB_m = cisloB;
						TIM2_SetCompare1(PWM_R+=cisloB);
						TIM2_SetCompare2(PWM_G+=cisloB);
						TIM2_SetCompare3(PWM_B+=cisloB);
					}
					if (cisloB < cisloB_m){
						cisloB_m = cisloB;
						TIM2_SetCompare1(PWM_R-=cisloB);
						TIM2_SetCompare2(PWM_G-=cisloB);
						TIM2_SetCompare3(PWM_B-=cisloB);
					}
				}
				
			}
			if (vyberS == 1){
				lcd_gotoxy(0, 0);
				lcd_puts("SPECIALNI FUNKCE");
				lcd_gotoxy(0, 1);
				lcd_puts("                ");

				TIM2_SetCompare1(0);
				TIM2_SetCompare2(0);
				TIM2_SetCompare3(0);
			}
			if (vyberS == 2){
				lcd_gotoxy(0, 0);
				sprintf(text, "    POLICIE     ");
				lcd_puts(text);
				
				TIM2_SetCompare1(PWM_R = 0);
				TIM2_SetCompare2(PWM_G = 0);
				TIM2_SetCompare3(PWM_B = 0);

				cas_n +=1;
				if (cas_n == 3){
					TIM2_SetCompare3(1600);
					cas_n = 0;
				}
				if (cas_n ==2){
					TIM2_SetCompare1(1600);
				}
			}
			if (vyberS == 3){
				lcd_gotoxy(0, 0);
				sprintf(text, "     MAJAK      ");
				lcd_puts(text);
				
				TIM2_SetCompare1(PWM_R = 0);
				TIM2_SetCompare2(PWM_G = 0);
				TIM2_SetCompare3(PWM_B = 0);

				cas_n1 +=1;
				if (cas_n1 == 2){
					TIM2_SetCompare1(528);
					TIM2_SetCompare2(90);
				}
				if (cas_n1 == 4){
					TIM2_SetCompare1(528);
					TIM2_SetCompare2(90);
					cas_n1 = 0;
				}
			}
			if (vyberS == 4){
				lcd_gotoxy(0, 0);
				sprintf(text, "Pulzujici ruzova");
				lcd_puts(text);

				TIM2_SetCompare2(0);

				if (s == 0){
					TIM2_SetCompare3(PWM_B += 20);
					TIM2_SetCompare1(PWM_R += 20);
					if (PWM_B > 250){
						s = 1;
					}
				}
				if (s == 1){
					TIM2_SetCompare3(PWM_B -= 20);
					TIM2_SetCompare1(PWM_R -= 20);
					if (PWM_B < 50){
						s = 0;
					}
				}
			}
		}
	}
}