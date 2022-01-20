#include "stm8s.h"
#include "assert.h"
#include "milis.h"
#include "stm8_hd44780.h"
#include <stdio.h>
//piny na posilující tranzistory
#define LEDR_PORT GPIOD
#define LEDR_PIN GPIO_PIN_4
#define LEDB_PORT GPIOA
#define LEDB_PIN GPIO_PIN_4
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

uint32_t timeA = 0;
uint32_t timeB = 0;
uint32_t timeC = 0;
uint16_t PWM_R = 0;
uint16_t PWM_G = 0;
uint16_t PWM_B = 0;
uint8_t a_stav = 0;
uint8_t m_stav = 0;
uint8_t vyberA = 0;
int8_t cisloA = 0;
int8_t jas = 0;	

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

	TIM2_TimeBaseInit(TIM2_PRESCALER_16, 1600 - 1);//10kHz
	
	TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 1056, TIM2_OCPOLARITY_HIGH);
	TIM2_OC2Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 1056, TIM2_OCPOLARITY_HIGH);
	TIM2_OC3Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 1056, TIM2_OCPOLARITY_HIGH);
	
	TIM2_OC1PreloadConfig(ENABLE);
	TIM2_OC2PreloadConfig(ENABLE);
	TIM2_OC3PreloadConfig(ENABLE);
	
	TIM2_Cmd(ENABLE);
}

int8_t kontrola_ncoder(void)
{
	static int m=0;

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
	uint16_t n=0;
	uint32_t timeA=0;
	uint32_t timeB=0;

	setup();

	lcd_init();

	while (1) {
		if (milis() - timeA > 50){
			timeA = milis();

			if (VYBER_n){//tlačítko vyběru pozor!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				a_stav = 1;
			}
			else{
				a_stav = 0;
			}
			if(m_stav == 1 && a_stav == 0){
				vyberA =1;
			}
			m_stav = a_stav;
			if (vyberA == 5){
				vyberA = 0;
			}
		}
		if (milis() - timeB > 60){
			timeB = milis();
			if (vyberA == 0){
				lcd_gotoxy(0, 0);
				lcd_puts("Klik pro nas");
			}
			if (vyberA == 1){
				cisloA += kontrola_ncoder();
				lcd_gotoxy(0, 1);
				sprintf(text, "Cervena=%5d", cisloA);
				lcd_puts(text);
				PWM_R = cisloA;
				if (PWM_R > 2000){
					PWM_R = 2000;
				}
			}
			if (vyberA == 2){
				cisloA += kontrola_ncoder();
				lcd_gotoxy(0, 0);
				sprintf(text, "Cervena=%5d", PWM_R);
				lcd_puts(text);
				lcd_gotoxy(0, 1);
				sprintf(text, "Zelena=%5d", cisloA);
				lcd_puts(text);
				PWM_G = cisloA;
				if (PWM_G > 2000){
					PWM_G = 2000;
				}
			}
			if (vyberA == 3){
				cisloA += kontrola_ncoder();
				lcd_gotoxy(0, 0);
				sprintf(text, "Zelena=%5d", PWM_G);
				lcd_puts(text);
				lcd_gotoxy(0, 1);
				sprintf(text, "Modra=%5d", cisloA);
				lcd_puts(text);
				PWM_B = cisloA;
				if (PWM_B > 2000){
					PWM_B = 2000;
				}
			}
			if (vyberA == 4){
				cisloA += kontrola_ncoder();
				lcd_gotoxy(0, 0);
				sprintf(text, "Modra=%5d", cisloA);
				lcd_puts(text);
				lcd_gotoxy(0, 1);
				sprintf(text, "Jas=%5d", cisloA);
				lcd_puts(text);
				jas=cisloA;
				PWM_R +=jas;
				PWM_G +=jas;
				PWM_B +=jas;
				if (PWM_R > 2000 && PWM_G > 2000 && PWM_B > 2000){
					PWM_R =2000;
					PWM_G =2000;
					PWM_B =2000;
				}
			}
		}
		if (milis() - timeC > 150){//nastavení příslušné barvy
			timeC = milis();
			TIM2_SetCompare1(PWM_R);
			TIM2_SetCompare2(PWM_G);
			TIM2_SetCompare3(PWM_B);
		}
	}
}