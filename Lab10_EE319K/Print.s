; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
	  PUSH {R10, R12}
	  MOV R12, #0
	  SUB SP, #32
	  MOV R10, SP ; R10 is frame pointer
	  SUB SP, #32

	  CMP	R0, #0
	  BLT	negative

bloop
num EQU 0
	
	  CMP R0, #10
	  BLO OUT	  
	  STR R0, [SP, #num]
	  
loop4	  ;MOD the #
	  SUBS R0, #10
	  BGE loop4
	  ADD R0, #10
	  
	  
	  ADD R0, #0x30
	  ;PUSH {R12, LR}
	  ;BL ST7735_OutChar
	  ;POP{R12, LR}
	  STRB R0, [R10, R12]
	  ADD R12, #1
	  LDR R0, [SP, #num]
	  B DIV10
	  
done
	  ADDS R12, #0
	  BEQ done1
	  SUBS R12, #1
	  LDRB R0, [R10, R12]
	  PUSH{R9,LR}
	  BL ST7735_OutChar
	  POP{R9, LR}
	  ADDS R12, #0
	  BGT done
done1
	  ADD SP, #16
	  
	  POP{R10, R12}


      BX  LR
OUT
	ADD R0, #0x30
	PUSH{R9,LR}
	BL ST7735_OutChar
	POP{R9, LR}
	B  done
DIV10
	MOV R1, #-1
loop	
	ADD R1, #1
	SUBS R0, #10
	BGE loop
	MOV R0, R1
	B bloop
	
	
	
negative
;	B bloop
	  CMP R0, #10
	  BLO OUT
	  STR R0, [SP, #num]
loopc	  ;MOD the #
	  SUBS R0, #10
	  CMP R0, #10
	  BHI loopc
	  
	  ADD R0, #0x30
	  ;PUSH {R12, LR}
	  ;BL ST7735_OutChar
	  ;POP{R12, LR}
	  STRB R0, [R10, R12]
	  ADD R12, #1
	  LDR R0, [SP, #num]
	  MOV R1, #10
	  UDIV R0, R1
	  B negative

;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
cnt EQU 0
	PUSH{R11,LR}
	SUB SP, #8
	SUB R11, SP, #5
	SUB SP, #8
	MOV R1, #0
	STRB R1, [SP, #cnt]
	MOV R2, #0x2A
	STRB R2, [R11, #0]
	STRB R2, [R11, #1]
	STRB R2, [R11, #2]
	STRB R2, [R11, #4]
	MOV R2, #0x2E
	STRB R2, [R11, #3]
	CMP	R0, #0
	BLT	OUT2	
	SUB R2, R0, #3333
	SUB R2, #3333
	SUB R2, #3332
	SUBS R2, #1
	BGT OUT2
loop2	
	MOV R2, R0
loop5	  ;MOD the #
	SUBS R2, #10

	BGE loop5
	ADD R2, #10
	  
	ADD R2, #0x30
	LDRB R1, [SP, #cnt]
	STRB R2, [R11, R1]
	
	SUBS R2, R1, #4 ; check whether go to out2
	BEQ OUT2
	
	MOV R2, #-1 ;divide it by 10
loop3	
	ADD R2, #1
	SUBS R0, #10
	BGE loop3
	MOV R0, R2
	
	ADD R1, #1
	STRB R1, [SP, #cnt]
	SUBS R2, R1, #3
	BNE loop2 ; increment cnt again again
	ADD R1, #1
	STRB R1, [SP, #cnt]
	B loop2
	
OUT2
	LDRB R0, [R11, #4]
	BL ST7735_OutChar
	LDRB R0, [R11, #3]
	BL ST7735_OutChar
	LDRB R0, [R11, #2]
	BL ST7735_OutChar
	LDRB R0, [R11, #1]
	BL ST7735_OutChar
	LDRB R0, [R11, #0]
	BL ST7735_OutChar
	ADD SP, #8
	ADD SP, #8
	POP{R11,LR}

     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
