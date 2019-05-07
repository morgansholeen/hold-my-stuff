// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 11/20/2018 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2018

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2018

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Timer2.h"
#include "Timer3.h"
#include "Timer4.c"

#define zombie1 0
#define plant1 1
#define sunflower1 2
#define walnut1 3

void Timer4_Init(void(*task)(void), uint32_t period);
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
volatile uint32_t FallingEdges = 0;
uint32_t zc = 0; //zombie counter
uint32_t pc = 0; //pea counter
uint32_t plc = 0; //plant counter
uint32_t cc=0; //coords counter
uint32_t zk=0; //zombies killed
uint32_t zw=0; //how many zombies have won
uint32_t sc=0; //sunflower counter
uint32_t wc=0; //walnut counter

void DAC_Out(uint32_t data);

uint32_t countersound = 0;


void dosound(void){
	DAC_Out(Theme[countersound]);
	countersound = (countersound+1)%49993;
}

//structs of sprites
struct coord {
	uint8_t type;
	uint8_t x;
	uint8_t y;
  uint8_t width;
	uint8_t height;
	uint8_t health;
};
typedef struct coord coord_t;
coord_t coords[15];

struct plant {
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
	uint8_t health;
	const unsigned short *images[20*20];
};
typedef struct plant plant_t;

plant_t plants[15];

struct sunflower {
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
	uint8_t health;
	const unsigned short *images[20*20];
};
typedef struct sunflower sunflower_t;

sunflower_t sunflowers[15];

struct zombie {
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
	uint8_t health;
	const unsigned short *images[20*20];
};
typedef struct zombie zombie_t;

zombie_t zombies[10];

struct walnut {
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
	uint8_t health;
	const unsigned short *images[20*20];
};
typedef struct walnut walnut_t;

walnut_t walnuts[15];

struct pea {
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
	uint8_t offmap;
	const unsigned short *images[100];
};
typedef struct pea pea_t;

pea_t pea[100];
//end of sprites structs




//init routines
void SysTick_Init(void){
	NVIC_ST_RELOAD_R = 1325000; // this is 12.5/16.67 * 1,000,000 adjusted to get 60 Hz
	NVIC_ST_CTRL_R = 0X07;
}

void Button_Init(void) {
	SYSCTL_RCGCGPIO_R |= 0x10;
	volatile int p =1;
	p++;
	FallingEdges = 0; 
	GPIO_PORTE_DIR_R &= ~0x03;
	GPIO_PORTE_AFSEL_R &= ~0x03;
	GPIO_PORTE_DEN_R |= 0x03;
	GPIO_PORTE_PCTL_R &= ~0x000000FF;
	GPIO_PORTE_AMSEL_R &= ~0x03;
	GPIO_PORTE_IS_R &= ~0x03;
	GPIO_PORTE_IBE_R&= ~0X03;
	GPIO_PORTE_IEV_R &= ~0X03;
	GPIO_PORTE_ICR_R = 0X03;
	GPIO_PORTE_IM_R |= 0X03;
	NVIC_PRI1_R = (NVIC_PRI1_R & 0XFFFFFF00) | 0X000000C0; //sets priority
	NVIC_EN0_R = 0X00000010;
}
//end of init routines






//handler routines
void GPIOPortE_Handler(void){ //need to add the new plant to the coord array
	//put code here
	if((GPIO_PORTE_DATA_R & 0x03) == 0x01){

	}
  if((GPIO_PORTE_DATA_R & 0x03) == 0x02){
		
	}
}
//end of handler routines



//other functions


//generate funcitons
void GeneratePeas(void){ //generate peas to shoot
	for(uint8_t i=0; i<plc; i++){
		if(plants[i].health > 0){	
			pea[pc].x=plants[i].x;
			pea[pc].y=plants[i].y;
			pea[pc].height = 10;
			pea[pc].width = 10;
			pea[pc].offmap = 0;
			pc++;
		}
	}
}

void GeneratePlant(uint8_t type1){
	if(type1 == plant1){
		coords[cc].type = plant1;
		coords[cc].x = 42; //configure
		coords[cc].y = 42; //configure
		coords[cc].height = 20;
		coords[cc].width = 20;
		coords[cc].health = 2;
		cc++;
	}
		if(type1 == sunflower1){
		coords[cc].type = sunflower1;
		coords[cc].x = 42; //configure
		coords[cc].y = 42; //configure
		coords[cc].height = 20;
		coords[cc].width = 20;
		coords[cc].health = 2;
		cc++;
	}
	if(type1 == walnut1){
		coords[cc].type = walnut1;
		coords[cc].x = 42; //configure
		coords[cc].y = 42; //configure
		coords[cc].height = 20;
		coords[cc].width = 20;
		coords[cc].health = 4;
	  cc++;
	}

}

//end


//erase functions
void ZombiesDead(uint8_t number, uint8_t number2) {
	if(number < 42 || number > 84){
		ST7735_FillRect(zombies[number2].x, zombies[number2].y, zombies[number2].width, zombies[number2].height, 0x4D84);
	}
	else{
		ST7735_FillRect(zombies[number2].x, zombies[number2].y, zombies[number2].width, zombies[number2].height, 0x1F36);
	}
	zk++;
}

void PlantDied(uint8_t number, uint8_t number2){
		if(number < 42 || number > 84){
		ST7735_FillRect(zombies[number2].x, zombies[number2].y, zombies[number2].width, zombies[number2].height, 0x4D84);
	}
	else{
		ST7735_FillRect(zombies[number2].x, zombies[number2].y, zombies[number2].width, zombies[number2].height, 0x1F36);
	}
}


void ErasePea(uint8_t number, uint8_t number2){
			if(number < 42 || number > 84){
		ST7735_FillRect(pea[number2].x, pea[number2].y, pea[number2].width, pea[number2].height, 0x4D84);
	}
	else{
		ST7735_FillRect(pea[number2].x, pea[number2].y, pea[number2].width, pea[number2].height, 0x1F36);
	}
}
//end


uint8_t ZombieCollide(uint8_t i){
		for(uint8_t j = 0; j<cc; j++){
			if(zombies[i].y == coords[j].y){
				coords[j].health--;
				if(coords[j].health == 0){
					PlantDied(coords[j].x, j);
				}
				return 1;
			}
		}
	return 0;
}

void ZombiesWin(void){
	//add the code for player losing
}


//move functions
void MoveZombies(void){ int i; //need to check if past mowers
  for(i=0;i<zc;i++){
    if(zombies[i].health != 0){
			if( ZombieCollide(i) == 0){
				zombies[i].y -= 2;
				if(zombies[i].y < 20){
					zw++;
					if(zw>3){
						ZombiesWin();
					}
				}
			}
    }
  }
}



void MovePea(void){ int i; //pea no longer in screen
  for(i=0;i<pc;i++){
		if(pea[i].offmap == 1){}
		else{
			for(uint8_t p=0;p<zc;p++){
				if(pea[i].y == zombies[p].y){
					zombies[p].health -= 1;
					pea[i].offmap = 1;
					ErasePea(pea[i].x, i);
					if(zombies[p].health ==0){
						ZombiesDead(zombies[p].x, p);
					}
				}
				else{
					pea[i].y +=2;
					if(pea[i].y >150){
						ErasePea(pea[i].x, i);
					}
				}
			}
		}
  }
}
//end







int main(void){
	
	Timer4_Init(dosound(), 360);
	Timer1_Init(something, 320000000);
	Timer0_Init(something, 400000000);
	Timer2_Init(something, 160000000);
	Timer3_Init(something, 40000000);
	
	
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Random_Init(1);

  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
  
  ST7735_DrawBitmap(52, 159, ns, 18,8); // player ship middle bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);

  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);

  Delay100ms(50);              // delay 5 sec at 80 MHz

  ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  LCD_OutDec(1234);
  while(1){
		MoveZombies();
		MovePea();
  }

}


// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
