	INPUT = 12584960	; Constant to be changed by SH2 before program load.
	SRCTBL = 12584960	; Constant to be changed by SH2 before program load.
	NOTI = 12584960	; End stat DMA address
	LINE_PIX = 21 ;Pixels in a line of the local map
	LINE_DIST = LINE_PIX >> 1 ;
	OUT_OF_BOUNDS_VALUE = -1;
	;------------------------------------------------------------------------ P64 PROGRAM DMA HEADER
	MVI INPUT,PL															; SH2 sets "PREAD" and "WRITE" constants before program load.
	ADD				NOP					MOV ALU,A		MOV 62,CT3			; MVI is 25-bit signed data. So it requires 3 shifts right to get high memory address in that bit depth.
	SL				NOP					MOV ALU,A		MOV 0,CT0			; As DSP reads data in DWORD units, the address actually must be on 4-byte boundaries,
	NOP				NOP					NOP				MOV ALL,RA0			; so we need to shift left one to get the proper address in RA0 and WA0.
	NOP				NOP					CLR A			MOV ALL,MC3			;
	MVI SRCTBL,PL															;
	ADD				NOP					MOV ALU,A		MOV 2,CT1			; CT3 = 63. Place origin DMA READ address at RAM3 62.
	SL				NOP					MOV ALU,A		MOV 0,CT2			;	
	NOP				NOP					NOP				MOV ALL,WA0			;
	NOP				NOP					CLR A			MOV ALL,MC3			; CT3 = ??. Place origin DMA WRITE address at RAM3 63 [end].
	MVI NOTI,PL																;
	ADD									mov alu,a		mov 58,ct3			;
	sl									mov alu,a		mov 0,mc1			;
										clr a			mov all,mc3			; Notification of end status address in RAM3 58
	;------------------------------------------------------------------------ MATH DATA TRANSFER
	DMAH2 D0,MC2,5															; DMA of inputs into the DSP via DMA Hold Address.
	;------------------------------------------------------------------------ DMAH prevents change to RA0/WA0 as a result of DMA.
	NOP				NOP					NOP				MOV 0,CT2			; CT3 = 0.
														MOV 0,CT1
	;------------------------------------------------------------------------
	;	Conditional statements need a "nop" after the jump, because the DSP pre-fetches instructions :)
	;	Subroutine \ Function execution: Place functions after main program "END". Move 1 to "LOP". Move the function label to the PC counter. Ensure a "nop" is after that command.
	;	Ensure functions end with "BTM" and "NOP" command. Because commands are pre-fetched, if you do not put a NOP after the BTM, it could hit an END.
	;-------------------------------------------------------------------------------------------------------------------------
	;	Notes
	;	Moving constants via D1 bus mov to registers RX and PL is allowed by the assembler, but is sometimes non-functional on real systems. Avoid before optimization.
	;
	;	Fixed-point math on DSP is done easily by taking register AHL instead of register ALL after copying mul result to a clear A with ad2 command.
	;-------------------------------------------------------------------------------------------------------------------------
	;	PROGRAM
	;
	;	Find pixels to sample from a heightmap, laid out in a linear fashion where every increment of the width of the image selects the pixel in the next row.
	;	The pixels to sample is a 25x25 square in that map, starting with its center in the center of the main map.
	;
	; 	startOffset = (main_map_total_pix>>1) - (main_map_x_pix * LINE_DIST) - LINE_DIST;
	;		y_pix_sample = (you.dispPos[Y] * (main_map_x_pix));
	;	for(int k = 0; k < LINE_PIX; k++){
	;		rowOffset = (k * main_map_x_pix) + startOffset;
	;		rowLimit = rowOffset / main_map_x_pix;
	;		for(int v = 0; v < LINE_PIX; v++){
	;			x_pix_sample = v + rowOffset + you.dispPos[X];
	;			RightBoundPixel = (x_pix_sample - (rowLimit * main_map_x_pix));
	;			src_pix = x_pix_sample - y_pix_sample;
	;			dst_pix = v+(k*LINE_PIX);
	;			}
	;	if this condition is NOT met
	;	if(temp_pix < main_map_total_pix && temp_pix >= 0 && RightBoundPixel < main_map_x_pix && RightBoundPixel >= 0) 
	;	src_pix = -1
	;	else
	;	src_pix = temp_pix;
	;
	;
	;	SYSTEM INPUT	[DMA LINE 15]
	;	main_map_total_pix	-> DMA from high memory into RAM2 0
	;	main_map_x_pix		-> DMA from high memory into RAM2 1
	;	main_map_y_pix		-> DMA from high memory into RAM2 2 [unused]
	;	you.CellPos[X]		-> DMA from high memory into RAM2 3
	;	you.CellPos[Y]		-> DMA from high memory into RAM2 4
	;
	;	FUNCTION RESERVED MEMORY
	;	DIVISION: [I] RAM0 0,1 [O] RAM1 1 [D] RAM0 2,3 RAM1 0
	;	SUMMARY: RAM0 0-3, RAM1 0-1
	;
	;	DATA 
	;	startOffset 		-> RAM0 4
	;	rowOffset			-> RAM0 5
	;	y_pix_sample 		-> RAM0 6
	;	rowLimit			-> RAM0 7
	;	x_pix_sample  		-> RAM0 8
	;	RightBoundPixel 	-> RAM0 9
	;	temp_pix			-> RAM0 10
	;	temp_ydata			-> RAM0 10
	;	K					-> RAM1 2
	;	V					-> RAM1 3
	;	src_pix				-> RAM3 0-24, then DMA out, repeat
	;
	;-------------------------------------------------------------------------------------------------------------------------
	; Start 	CT0,1,2,3 = 0
	; 	startOffset = (main_map_total_pix>>1) - (main_map_x_pix * LINE_DIST) - LINE_DIST;
	;	int k = 0;
	;	int v = 0;
	;	int g = 0;
	;	
	;	DATA
	;	startOffset			-> RAM0 4
	;	main_map_total_pix	-> RAM2 0
	;	main_map_x_pix		-> RAM2 1
	;	12					-> D1 mov SImm
	;	K					-> RAM1 2
	;	V					-> RAM1 3
	;-------------------------------------------------------------------------------------------------------------------------
			mov mc2,p							clr a 		mov 4,ct0	; ct2 = 1 ct0 = 4								
	ad2 						mov mc2,y		mov alu,a	mov LINE_DIST,RX	; ct2 = 2
	sr		mov mul,p							mov alu,a	mov 1,ct2	; ct2 = 1
	sub											mov alu,a	mov LINE_DIST,PL
	sub											mov alu,a	mov 2,ct1	; ct1 = 2
												clr a		mov all,mc0	; ct0 = 5
	;-------------------------------------------------------------------------------------------------------------------------
	; for K start 	
	;	INITIAL CONDITION: CT0 = 5, CT1 = 4, CT2 = 1 CT3 = 0
	;	CONDITION UPON LOOP:
	; 	for(  ; k < LINE_PIX; k++)
	;	
	;	DATA
	;	K	-> RAM1 2
	;	LINE_PIX	-> D1 mov SImm
	;-------------------------------------------------------------------------------------------------------------------------
	FOR_K:
															mvi LINE_PIX,PL	; If confused, please look at "for k return". K is still in A.
	sub											clr a		mov 2,ct1	; ct1 = 2 If K - LINE_PIX is zero or positive, K is LINE_PIX or >LINE_PIX.
	jmp NZS,FOR_KEND:
	nop														mov 4,ct0	; ct0 = 4 Instruction immaterial to program end jump	
	;-------------------------------------------------------------------------------------------------------------------------
	;	CT0 = 4, CT1 = 2, CT2 = 1, CT3 = 0 [gets set to 0 anyway]
	; 	rowOffset = (k * main_map_x_pix) + startOffset;	
	;	
	;	DATA
	;	rowOffset		->	RAM0 5
	;	k 	->				RAM1 2
	;	main_map_x_pix	->	RAM2 1
	;	startOffset		->	RAM0 4
	;-------------------------------------------------------------------------------------------------------------------------
			mov m2,x							mov m1,y	mov mc0,PL	; ct0 = 5
	ad2		mov mul,p				mov alu,a				mov 0,ct3	;
	ad2								mov alu,a							;
									clr a					mov all,mc0	; ct0 = 6
	;-------------------------------------------------------------------------------------------------------------------------
	;	CT0 = 6, CT1 = 2, CT2 = 1, CT3 = 0
	;	y_pix_sample = -(you.cellPos[Z] * (main_map_x_pix));
	;
	;	DATA
	;	y_pix_sample ->		RAM0 6
	;	you.cellPos[Y] ->	RAM2 4
	;	main_map_x_pix	->	RAM2 1
	;-------------------------------------------------------------------------------------------------------------------------
												mov m2,y	mov -1,RX	; Multiply by -1
			mov mul,p										mov 4,ct2	;
	ad2								mov alu,a	mov m2,y				;
									clr a					mov ALL,RX	;
			mov mul,p										mov 1,ct1	;
	ad2								mov alu,a				mov 1,ct2	; ct2 = 1
									clr a					mov all,mc0	; ct0 = 7
	;-------------------------------------------------------------------------------------------------------------------------
	;	CT0 = 7, CT1 = 1, CT2 = 1, CT3 = 0
	; 	rowLimit = rowOffset / main_map_x_pix;
	;
	;	DATA
	;	rowLimit ->			RAM0 7
	;	rowOffset ->		RAM0 5
	;	main_map_x_pix	->	RAM2 1
	;
	;	FUNCTION	DIVIDE
	;	Input DIVIDEND	->	RAM0 0
	;	Input DIVISOR	->	RAM0 1
	;	Output QUOTIENT	->	RAM1 1
	;
	;	MOV RAM0 5 to RAM0 0
	;	MOV RAM2 1 to RAM0 1
	;	ENSURE CT0 AND CT1 ARE 0
	;	RUN DIVIDE
	;	MOV RAM1 1 to RAM0 7
	;
	;-------------------------------------------------------------------------------------------------------------------------
															mov 5,ct0
			mov m0,p										mov 0,ct0
	ad2								mov alu,a				mov 0,mc1	; Clears RAM1 1 so the division function does not accumulate on loops
									clr a					mov all,mc0	;ct0 = 1
															mov mc2,mc0	;ct0 = 2 ct2 = 2
															mov 0,ct0
															mov 0,ct1
	mvi 1,LOP
	mvi DIVIDE,PC														; Division Process Execution
	nop																	; Division Return Point
															mov 1,ct1
															mov 7,ct0
									clr a					mov mc1,mc0 ; ct0 = 8 ct1 = 2 RAM1 1 to RAM0 7 ;
	;-------------------------------------------------------------------------------------------------------------------------
	;	v = 0;		reset V ct loop
	;	DATA V ->	RAM1 3
	;-------------------------------------------------------------------------------------------------------------------------
															mov 3,ct1
															mov 0,mc1	;ct1 = 4
	;-------------------------------------------------------------------------------------------------------------------------
	;	INITIAL CONDITION:	CT0 = 8, CT1 = 4, CT2 = 2, CT3 = 0
	;	CONDITION UPON LOOP:	
	; 		for( ; v < LINE_PIX; v++)
	;
	;	DATA
	;	v	->		RAM1 3
	;-------------------------------------------------------------------------------------------------------------------------
	FOR_V:
															mvi LINE_PIX,PL	; If confused, please look at "FOR_VEXI". V is still in A.
	sub								clr a					mov 3,ct1	; ct1 = 3 If V - LINE_PIX is zero or positive, V is LINE_PIX or >LINE_PIX.													
	jmp Z,FOR_VEXI:
	nop														
	;-------------------------------------------------------------------------------------------------------------------------
	;	CT0 = 8, CT1 = 3, CT2 = 2, CT3 = [Looping]
	; 	x_pix_sample = v + rowOffset + you.cellPos[X];
	;		
	;	DATA
	;	x_pix_sample  	->		RAM0 8
	;	V				->		RAM1 3
	;	rowOffset		->		RAM0 5
	;	you.CellPos[X]	->		RAM2 3
	;
	;-------------------------------------------------------------------------------------------------------------------------
				mov m1,p									mov 5,ct0
	ad2			mov m0,p			mov alu,a				mov 3,ct2
	ad2			mov m2,p			mov alu,a				mov 8,ct0
	ad2								mov alu,a				
									clr a					mov all,mc0	;ct0 = 9
	;-------------------------------------------------------------------------------------------------------------------------
	;	CT0 = 9, CT1 = 3, CT2 = 3, CT3 = [Looping]
	;	RightBoundPixel = (x_pix_sample - (rowLimit * main_map_x_pix));
	;
	;	DATA
	;	x_pix_sample	->		RAM0 8
	;	rowLimit		->		RAM0 7
	;	main_map_x_pix	->		RAM2 1
	;	RightBoundPixel ->		RAM0 9
	;-------------------------------------------------------------------------------------------------------------------------
															mov 8,ct0
				mov m0,p									mov 1,ct2
	ad2								mov alu,a				mov 7,ct0	; Bad order
				mov m0,x						mov m2,y	
				mov mul,p
	sub								mov alu,a				mov 9,ct0
									clr a					mov all,mc0 ;ct0 = 10
	;-------------------------------------------------------------------------------------------------------------------------
	;		CT0 = 10, CT1 = 3, CT2 = 1, CT3 = [Looping]
	; 		temp_pix = x_pix_sample + z_pix_sample;
	;
	;	DATA
	;	x_pix_sample	->		RAM0 8
	;	y_pix_sample	->		RAM0 6
	;	temp_pix		->		RAM0 10
	;-------------------------------------------------------------------------------------------------------------------------
															mov 8,ct0
				mov mc0,p									mov 6,ct0	;ct0 = 6
	ad2			mov mc0,p			mov alu,a							;ct0 = 7
	ad2								mov alu,a				mov 10,ct0	;ct0 = 10
									clr a					mov all,mc0	;ct0 = 11
	;-------------------------------------------------------------------------------------------------------------------------
	;		CT0 = 11, CT1 = 3, CT2 = 3, CT3 = [Looping]
	;	
	;	CONDITIONS - if...
	;	temp_pix < main_map_total_pix
	;	temp_pix >= 0
	;	RightBoundPixel < main_map_x_pix
	;	RightBoundPixel >= 0
	;	then
	;	src_pix = temp_pix;
	;	else
	;	temp_pix = -1
	;	src_pix = temp_pix;
	;
	;	DATA
	;	temp_pix		->		RAM0 10
	;	main_map_total_pix->	RAM2 0
	;	RightBoundPixel	->		RAM0 9
	;	main_map_x_pix	->		RAM2 1
	;
	;	OUTPUT
	;	src_pix			->		RAM3 0-24
	;	This loop will not set CT3 beyond MC3 command.
	;-------------------------------------------------------------------------------------------------------------------------
															mov 0,ct2	; CT0 = 2
															mov 10,ct0	; CT0 = 10
				mov m0,p			mov m2,a							;
	sub								clr a								;
	MVI OUT_OF_BOUNDS_VALUE,mc0,zs										; if main_map_total_pix - temp_pix is zero or negative...
	ad2								clr a					mov 10,ct0	; We need to reset CT0 as the MVI makes the state uncertain.
	MVI OUT_OF_BOUNDS_VALUE,mc0,s										; if temp_pix + 0 is negative...
															mov 1,ct2	; CT2 = 1
															mov 9,ct0	; CT0 = 9
				mov mc0,p			mov m2,a							; CT0 = 10
	sub								clr a								; 
	MVI OUT_OF_BOUNDS_VALUE,mc0,zs										; if main_map_x_pix - RightBoundPixel is zero or negative...
	ad2								mov alu,a				mov 10,ct0
	MVI OUT_OF_BOUNDS_VALUE,mc0,s
									clr a					mov 10,ct0
															mov mc0,mc3	; src_pix = temp_pix
	;-------------------------------------------------------------------------------------------------------------------------
	; CT0 = 11, CT1 = 3, CT2 = 1, CT3 = [Looping]
	; for V return; V++
	; V -> RAM1 3
	;-------------------------------------------------------------------------------------------------------------------------
															mov 3,ct1
				mov m1,p			clr a					mov 8,ct0
	ad2 							mov alu,a				mov 1,PL
	ad2								mov alu,a				mov 2,ct2
															mov all,mc1 ; ct1 = 4
	;-------------------------------------------------------------------------------------------------------------------------
	; For V exit
	;-------------------------------------------------------------------------------------------------------------------------
	JMP FOR_V:
	nop
	FOR_VEXI:
	;-------------------------------------------------------------------------------------------------------------------------
	; CT0 = 8, CT1 = 3, CT2 = 2, CT3 = [25?]
	; for K return
	; K -> RAM1 2
	; SRC PIXEL TABLE ADDR -> RAM3 63
	; "*output++ = *src_pix++" - In this case, DMA with address-increment is used as we want to save the changes.
	; Except this code also manipulates the address, as it is stored in RAM. Strange.
	; "&src_pix += LINE_PIX"
	;
	;-------------------------------------------------------------------------------------------------------------------------
															mov 0,ct3		; Re-select 0 for the output
	DMA2	MC3,D0,LINE_PIX													; DMA
															mov 63,ct3		; 
			mov m3,p														; 
	ad2								mov alu,a				mov LINE_PIX,PL	; Add LINE_PIX to src pixel address
	ad2								mov alu,a								;
									clr a					mov all,mc3		;
	;-------------------------------------------------------------------------------------------------------------------------
	;	K -> RAM1 2
	;	k++;
	;-------------------------------------------------------------------------------------------------------------------------
															mov 2,ct1
			mov m1,p				clr a					mov 5,ct0	
	ad2								mov alu,a				mov 1,PL	; D1 mov SImm 1
	ad2								mov alu,a				mov 1,ct2
															mov all,mc1 ; ct1 = 3 ; do not clear, see top of loop
	JMP FOR_K:
	FOR_KEND:
	nop														
	;-------------------------------------------------------------------------------------------------------------------------
	; Program End
	; DMA End-notification status out
	; DMA is via address-hold: the change to the address by DMA increment is not written back to RA0 or WA0
	;-------------------------------------------------------------------------------------------------------------------------
															mov 58,ct3
															mov mc3,wa0 ; Notifcation addr to wa0
															mov 0,ct0
															mov 32,mc0
															mov 0,ct0
	DMAH2 MC0,D0,1	;Write notification of end status "32"
	ENDI
	;-------------------------------------------------------------------------------------------------------------------------
	; Unsigned math only
	;-------------------------------------------------------------------------------------------------------------------------
	;	FUNCTION DIVIDE
	;
	;	INPUT
	;	Dividend ->			RAM0 0
	;	Divisor ->			RAM0 1
	;
	;	OUTPUT
	;	quotient ->			RAM1 1
	;	WARNING				Quotient is not cleared at program end or start thus will accumulate. Clear RAM1 1 before jumping to function!
	;
    ;	unsigned int tempdividend = dividend;
    ;	unsigned int tempdivisor = divisor;
	;	unsigned int tempquotient = 1;
	;	
	;	DATA
	;	tempdividend -> 	RAM0 2
	;	tempdivisor ->		RAM0 3
	;	tempquotient ->		RAM1 0
	;-------------------------------------------------------------------------------------------------------------------------
	DIVIDE: ; (this sequence seems weird but it works by using the ALU registers as a temporary stack while we increment some pointers)
					mov mc0,p 									clr a			mov 0,ct1 				;ct0 = 1
	ad2  			mov mc0,p 									mov alu,a		mov 1,mc1				;ct0 = 2 Store temporary quotient of 1 at RAM1 0
																clr a 			mov all,mc0				;ct0 = 3 Store copy of dividend at RAM0 2
	ad2 														mov alu,a								;
																clr a 			mov all,mc0				;ct0 = 4 Store copy of divisor at RAM0 3
	;-------------------------------------------------------------------------------------------------------------------------
    ;	if (tempdivisor >= tempdividend) {
	;		if(tempdivisor > tempdividend) tempquotient = 0;
	;		goto divend;
    ;	}
	;
	;	DATA
	;	tempdividend ->		RAM0 2
	;	tempdivisor ->		RAM0 3
	;	tempquotient ->		RAM1 0
	;-------------------------------------------------------------------------------------------------------------------------
																				mov 2,ct0				;ct0 = 2
																mov mc0,a								;ct0 = 3
					mov mc0,p 													mov 0,ct1				;ct0 = 4
	sub 														clr a			mov 3,ct0
	jmp ZS,DIVEND
	mvi 0,mc1,s	; Remember, the DSP caches instructions. Therefore, this instruction gets run even if we jump.
	;-------------------------------------------------------------------------------------------------------------------------
    ;	while (tempdivisor<<1 <= tempdividend)
    ;	{
    ;		tempdivisor = tempdivisor << 1;
    ;		tempquotient = tempquotient << 1;
    ;	}
	;
	;	DATA
	;	tempdividend ->		RAM0 2
	;	tempdivisor ->		RAM0 3
	;	tempquotient ->		RAN1 0
	;-------------------------------------------------------------------------------------------------------------------------
	jmp SLCOND:
	SLLOOP:
																mov m0,a		mov 0,ct1						; CT0 = 3
	sl 															mov alu,a										; CT1 = 0
																mov m1,a		mov all,mc0						; CT0 = 4 
	sl															mov alu,a		mov 3,ct0						; CT0 = 3
																clr a			mov all,mc1						; CT1 = 1
	SLCOND:
																mov m0,a		mov 2,ct0	; tempdivisor in A	; CT0 = 2
					mov m0,p																; tempdividend in P
	sl															mov alu,a					; tempdivisor<<1
	sub															mov alu,a		mov 0,ct1	; tempdivisor<<1  - tempdividend
	jmp S,SLLOOP:																			; if negative, jmp
																clr a			mov 3,ct0	;Prepares CT0 for next function segment
	;-------------------------------------------------------------------------------------------------------------------------
	;    // Call division recursively
	;		quotient += tempquotient;
    ;	    tempquotient = division(tempdividend-tempdivisor, divisor);
	;		
	;	DATA
	;	dividend ->		RAM0 0
	;	divisor ->		RAM0 1
	;	tempdividend -> RAM0 2
	;	tempdivisor	->	RAM0 3
	;	tempquotient->	RAM1 0
	;	quotient ->		RAM1 1
	;
	;	FUNCTION
	;	division 	-> label "DIVIDE"	
	;	Write:
	;	(tempdividend-tempdivisor) to RAM0 0
	;
	;-------------------------------------------------------------------------------------------------------------------------
					mov mc1,p																	; CT1 = 1
																mov m1,a						; (quotient is selected)
	ad2															mov alu,a		mov 2,ct0		; CT0 = 2 ; (quotient += tempquotient)
																clr a			mov all,mc1 	; (to quotient in RAM1 1) ; CT1 = 2	
																mov mc0,a		mov 0,ct1		; CT0 = 3 ; CT1 = 0 ; tempdividend to A
					mov m0,p																	; tempdivisor to P
	sub															mov alu,a		mov 0,ct0		; CT0 = 0 ; tempdividend - tempdivisor
																clr a			mov all,mc0		; to RAM0 0
	jmp DIVIDE:																					;
																				mov 0,ct0		;
	;-------------------------------------------------------------------------------------------------------------------------
	;	return tempquotient+quotient;
	;
	;	DATA
	;	tempquotient->	RAM1 0
	;	quotient ->		RAM1 1
	;	RETURN ->		RAM1 1
	;-------------------------------------------------------------------------------------------------------------------------
	DIVEND:
																clr a			mov 0,ct1		; CT1 = 0 (its state is uncertain before)
																mov mc1,a						; CT1 = 1
					mov m1,p													mov 0,ct0		; Return used RAM bank to zero (no results here)
	ad2															mov alu,a						; 
																clr a			mov all,mc1		; CT1 = 2
																				mov 1,ct1		; Return used RAM bank to result
	BTM
	nop
	