	INPUT = 12584960	; Constant to be changed by SH2 before program load.
	VERTTBL = 12584960	; Constant to be changed by SH2 before program load.
	PORTTBL = 12584960	; Constant to be changed by SH2 before program load.
	REPORT = 12584960	; End stat DMA address
	CLIP_MARKER = 16
	;------------------------------------------------------------------------ P64 PROGRAM ADDRESS HEADER
	MVI INPUT,PL															; Note that addresses must be shifted left one
	ADD				NOP					MOV ALU,A		MOV 60,CT3			; to get the real address
	SL				NOP					MOV ALU,A		MOV 0,CT0			; 
	NOP				NOP					NOP				MOV ALL,RA0			; 
	NOP				NOP					CLR A			MOV ALL,MC3			;
	MVI VERTTBL,PL															; CT3 = 61
	ADD				NOP					MOV ALU,A		MOV 0,CT1			; 
	SL				NOP					MOV ALU,A		MOV 0,CT2			;
	NOP				NOP					NOP										;
	NOP				NOP					CLR A			MOV ALL,MC3			; 
	MVI PORTTBL,PL															; CT3 = 62
	ADD									mov alu,a							;
	sl									mov alu,a							;
										clr a			mov all,mc3			; 
	MVI REPORT,PL															; CT3 = 63
	ADD									mov alu,a							;
	sl									mov alu,a							;
														MOV ALL,WA0			;
										clr a			mov all,mc3			; 
	;------------------------------------------------------------------------ CT3 = ??
	; RAM3 60: Control inputs address (Read)
	; RAM3 61: Vertex table address  (Read), becomes Vertex Clipping Address (Write)
	; RAM3 62: Portal table address (Read)
	; RAM3 63: Reporting address (Write)
	; Opening RA0: Inputs
	; Opening WA0: Report
	;------------------------------------------------------------------------
	; Long-term purpose of this program:
	; To work with the heightmap vertex struct and the portal struct to calculate if the vertices are in or out of the portal.
	;	//////////////////////////////////
	;	// Post-transformed vertice data struct
	;	//////////////////////////////////
	;	typedef struct {
	;		POINT  pnt;
	;		int clipFlag;
	;	} vertex_t; //16 bytes each
	;	//////////////////////////////////
	;	// Basic Portal/Occluder Stuff
	;	//////////////////////////////////
	;	typedef struct 
	;	{
	;		int verts[4][3];
	;		int depth;
	;		unsigned char type;
	;		unsigned char backface;
	;		unsigned char sectorA;
	;		unsigned char sectorB;
	;		short min[2];
	;		short max[2];
	;	} _portal;
	;
	;	Next steps:
	;	To allow communicating which portal(s) to test against in the list
	;	Enable, or disable, testing if vertex is above or below portal's Z
	;	Enable communication of the clip flags to set if clip out or in
	;	Enable setting (in portal struct) of clip out or in
	;
	;------------------------------------------------------------------------
	;
	; Note this program is running tandem with another DSP program.
	; Because of that, nothing can be assumed zero or non-zero unless written to.
	;
	;------------------------------------------------------------------------
	; Clear RAM3[30] to use as reporting data: reports the # of vertices marked as within the portal/occluder
								clr a		mov 30,ct3
											mov all,mc3		; CT3 = 30
	DMAH2 D0,MC0,1											; CT3 = 31
											mov 62,ct3		; CT0 = 1 ; Inputs to RAM0 0 ; 
											mov M3,RA0		; CT3 = 62
	DMAH2 D0,MC1,16											; Portal address (RAM3 62) in RA0
											mov 0,ct0		; CT1 = 12 ; DMA in verts[4][3] (12 ints) to RAM1[0] - RAM1[11]
											mov 61,ct3		; CT0 = 0
								mov m3,a	mov 3,PL		; CT3 = 61 (&vertex_t[0])
	ad2							mov alu,a	mov m3,ra0		; Move &vertex_t[0] to A, and 3 to P
											mov all,wa0		; Add 3 to &vertex_t[0] to reach &vertex_t[0].clipFlag
											mov all,mc3		; move &vertex_t[0].clipFlag to WA0 
											mov 0,ct1		; CT3 = 62 ; &vertex_t[0].clipFlag back to RAM3 61
															; CT1 = 0
	; -----------------------------------------------------------------------
	; Take special note that RA0 and WA0 increment separately.
	; Because of this, I have to increment the addresses mathematically.
	; Alternatively: DMA2 will increment one unit (four bytes). DMA8 will increment four units (16 bytes).
	; DMA8 will correctly set the offset?
	; -----------------------------------------------------------------------
	; Intent: Use RAM0 0 as loop counter (# of vertices)
	; for(int ram0[0]; ram0[0] > 0; ram0[0]--)
	; Why not use "BTM" and "LOP" here?
	; Information:
	; Loop count			->	RAM0[0]
	; Portal information	->	RAM1[0] - RAM1[15]
	; Vertex				->	RAM2[0] - RAM2[3]
	; &vertex_t				->	RA0 (Read)
	; &vertex_t.clipFlag	->	WA0	(Write)
	; -----------------------------------------------------------------------
	;	CT0 = 0 ; CT1 = ?? ; CT2 = ?? CT3 = ?? ; All but CT0 are undefined.
	FOR_ENTRY1: ; Test the condition "ram0[0] > 0"
					mov m0,p	clr a						; 
	ad2							mov alu,a	mov 1,PL		; RAM0[0] in P as loop counter
	jmp ZS,FOR_EXIT1										; RAM0[0] moved to A with AD2 instruction ; 1 in PL
	nop														; If RAM0[0] was zero or negative, jump to loop exit	
	sub							mov alu,a	mov 0,ct2		; (nop inserted for jump safety)
								clr a		mov all,mc0		; CT2 = 0 ; Perform RAM0[0] - 1
															; CT0 = 1 ; Result to RAM0[0] ; 
	;	---------------------------------------------------------------------
	;	Loop body
	;	Step 1: DMA in vertex_t[i]. Four units ( 16 bytes ).
	;	DMA with address increment will be used.
	;	---------------------------------------------------------------------
	DMA2 D0,MC2,4
	;	---------------------------------------------------------------------
	;	Step 2: Chirality check maths
	;
	;	point[X] is RAM2[0], point[Y] is RAM2[1]
	;	p0[X] is RAM1[0], p0[Y] is RAM1[1]
	;	p1[X] is RAM1[3], p1[Y] is RAM1[4]
	;	p2[X] is RAM1[6], p2[Y] is RAM1[7]
	;	p3[X] is RAM1[9], p3[Y] is RAM1[10]
	;	First do 0 -> 1, then 1 -> 2, then 2 -> 3, then 3 -> 0
	;	RAM2[4], pointY
	;	RAM2[5], edgeAX
	;	RAM2[6], edgeBY
	;	RAM1[16], edgeAY
	;	RAM1[17], edgeBX
	;	RAM1[18], pointX
	;	RAM3[2], side
	;	---------------------------------------------------------------------
											mov 1,ct2
							mov m2,a		mov 4,ct2	; CT2 = 1
											mov all,mc2	; CT2 = 4 ; RAM2[1] to A (pointY)
											mov 0,ct1	; CT2 = 5 ; RAM2[1] from A to RAM2[4] (pointY)
							mov m1,a		mov 4,ct1	; CT1 = 0
							mov m1,a		mov all,mc2	; CT1 = 4 ; RAM1[0] to A (p0[X])
											mov all,mc2 ; CT2 = 6 ; RAM1[0] from A to RAM2[5] (p0[X]) ; RAM1[4] to A (p1[Y])
											mov 1,ct1	; CT2 = 7 ; RAM1[4] from A to RAM2[6] (p1[Y])
							mov m1,a		mov 16,ct1	; CT1 = 1 ;
											mov all,mc1	; CT1 = 16 ; RAM1[1] to A (p0[Y])
											mov 3,ct1	; CT1 = 17 ; RAM1[1] from A to RAM1[16] (p0[Y])
							mov m1,a		mov 17,ct1	; CT1 = 3
											mov all,mc1	; CT1 = 17 ; RAM1[3] to A (p1[X])
											mov 0,ct2	; CT1 = 18 ; RAM1[3] from A to RAM1[17] (p1[X])
							mov m2,a		mov 18,ct1	; CT2 = 0
							clr a			mov all,mc1 ; CT1 = 18 ; RAM2[0] to A (pointX)
	mvi 1,LOP											; CT1 = 19 ; RAM2[0] from A to RAM1[18] (pointX)
	mvi CHIRALITY_ONE_SIDE,PC							; Jump to subroutine (note: DO NOT USE `jmp` instruction)
	nop													; CT0 = 0 ; CT1 = 0 ; CT2 = 5 ; CT3 = 2
	; -----------------------------------------------------------------------
	;	Next: If RAM3[2] is < 0, jump to CONTINUE.
	;	If RAM3[2] is >= 0, set-up and test p1 -> p2.
				mov m3,p	clr a			
	ad2						mov alu,a		
	jmp S,CONTINUE
	;	RAM2[4], pointY from RAM2[1]
	;	RAM2[5], edgeAX	from RAM1[3]
	;	RAM2[6], edgeBY from RAM1[7]
	;	RAM1[16], edgeAY from RAM1[4]
	;	RAM1[17], edgeBX from RAM1[6]
	;	RAM1[18], pointX from RAM2[0]
											mov 3,ct1	; (warning: executes after jump)
							mov m1,a		mov 7,ct1	; CT1 = 3
							mov m1,a		mov all,mc2	; CT1 = 7 ; RAM1[3] to A (p1[X])
											mov all,mc2 ; CT2 = 6 ; RAM1[3] from A to RAM2[5] (p1[X]) ; RAM1[7] to A (p2[Y])
											mov 4,ct1	; CT2 = 7 ; RAM1[7] from A to RAM2[6] (p2[Y])
							mov m1,a		mov 16,ct1	; CT1 = 4 ;
											mov all,mc1	; CT1 = 16 ; RAM1[4] to A (p1[Y])
											mov 6,ct1	; CT1 = 17 ; RAM1[4] from A to RAM1[16] (p1[Y])
							mov m1,a		mov 17,ct1	; CT1 = 6
											mov all,mc1	; CT1 = 17 ; RAM1[6] to A (p2[X])
	mvi 1,LOP											; CT1 = 18  ; RAM1[6] from A to RAM1[17] (p2[X])
	mvi CHIRALITY_ONE_SIDE,PC							; Jump to subroutine (note: DO NOT USE `jmp` instruction)
	nop
	; -----------------------------------------------------------------------
	;	Next: If RAM3[2] is < 0, jump to CONTINUE.
	;	If RAM3[2] is >= 0, set-up and test p2 -> p3.
				mov m3,p	clr a			
	ad2						mov alu,a		
	jmp S,CONTINUE
	;	RAM2[4], pointY from RAM2[1]
	;	RAM2[5], edgeAX	from RAM1[6]
	;	RAM2[6], edgeBY from RAM1[10]
	;	RAM1[16], edgeAY from RAM1[7]
	;	RAM1[17], edgeBX from RAM1[9]
	;	RAM1[18], pointX from RAM2[0]
											mov 6,ct1	; (warning: executes after jump)
							mov m1,a		mov 10,ct1	; CT1 = 6
							mov m1,a		mov all,mc2	; CT1 = 10 ; RAM1[6] to A (p2[X])
											mov all,mc2 ; CT2 = 6 ; RAM1[6] from A to RAM2[5] (p2[X]) ; RAM1[10] to A (p3[Y])
											mov 7,ct1	; CT2 = 7 ; RAM1[10] from A to RAM2[6] (p3[Y])
							mov m1,a		mov 16,ct1	; CT1 = 7 ;
											mov all,mc1	; CT1 = 16 ; RAM1[7] to A (p2[Y])
											mov 9,ct1	; CT1 = 17 ; RAM1[7] from A to RAM1[16] (p2[Y])
							mov m1,a		mov 17,ct1	; CT1 = 9
											mov all,mc1	; CT1 = 17 ; RAM1[9] to A (p3[X])
	mvi 1,LOP											; CT1 = 18 ; RAM1[9] from A to RAM1[17] (p3[X])
	mvi CHIRALITY_ONE_SIDE,PC							; Jump to subroutine (note: DO NOT USE `jmp` instruction)
	nop
	; -----------------------------------------------------------------------
	;	Next: If RAM3[2] is < 0, jump to CONTINUE.
	;	If RAM3[2] is >= 0, set-up and test p3 -> p0.
				mov m3,p	clr a			
	ad2						mov alu,a		
	jmp S,CONTINUE
	;	RAM2[4], pointY from RAM2[1]
	;	RAM2[5], edgeAX	from RAM1[9]
	;	RAM2[6], edgeBY from RAM1[1]
	;	RAM1[16], edgeAY from RAM1[10]
	;	RAM1[17], edgeBX from RAM1[0]
	;	RAM1[18], pointX from RAM2[0]
											mov 9,ct1	; (warning: executes after jump)
							mov m1,a		mov 1,ct1	; CT1 = 9
							mov m1,a		mov all,mc2	; CT1 = 1 ; RAM1[9] to A (p3[X])
											mov all,mc2 ; CT2 = 6 ; RAM1[9] from A to RAM2[5] (p3[X]) ; RAM1[1] to A (p0[Y])
											mov 10,ct1	; CT2 = 7 ; RAM1[1] from A to RAM2[6] (p0[Y])
							mov m1,a		mov 16,ct1	; CT1 = 10 ;
											mov all,mc1	; CT1 = 16 ; RAM1[10] to A (p3[Y])
											mov 0,ct1	; CT1 = 17 ; RAM1[10] from A to RAM1[16] (p3[Y])
							mov m1,a		mov 17,ct1	; CT1 = 0
											mov all,mc1	; CT1 = 17 ; RAM1[0] to A (p0[X])
	mvi 1,LOP											; CT1 = 18 ; RAM1[0] from A to RAM1[17] (p0[X])
	mvi CHIRALITY_ONE_SIDE,PC							; Jump to subroutine (note: DO NOT USE `jmp` instruction)
	nop
	; -----------------------------------------------------------------------
	;	Next: If RAM3[2] is < 0, jump to CONTINUE.
	;	If RAM3[2] is >= 0, all edges have passed the test.
	;	We shall then mark RAM2[3], and DMA it out.
				mov m3,p	clr a			
	ad2						mov alu,a		
	jmp S,CONTINUE
											mov 3,ct2
	mvi CLIP_MARKER,PL
							mov m2,a
	or						mov alu,a		mov 30,ct3
							clr a			mov all,mc2
											mov 3,ct2
	DMAH2 MC2,D0,1
	;	In this case, I also want to use, say, RAM3[30] to store the # of hits.
							mov m3,a		mov 1,PL
	ad2						mov alu,a		
											mov all,mc3
	CONTINUE:
	;	---------------------------------------------------------------------
	;	CT0 = ?? ; CT1 = ?? ; CT2 = ?? ; CT3 = ?? ; Treat all counters as undefined.
	;	We need to ensure we increment the DMA address correctly, whether the vertex is clipped or not.
	;	RAM3[61] -> Clip flag address of vertex_t[i]
											mov 61,ct3
							mov m3,a		mov 4,PL
	ad2						mov alu,a		
											mov all,wa0
							clr a			mov all,mc3
	jmp FOR_ENTRY1 	; Return to top of loop
											mov 0,ct0
	;	---------------------------------------------------------------------
	FOR_EXIT1:
											mov 63,ct3	; 
											mov m3,wa0	; Procedure to move reporting address back to WA0
											mov 30,ct3	;
	DMAH2 MC3,D0,1										; Procedure to DMA RAM3[30] out to the reporting address
	END
	nop
	;	---------------------------------------------------------------------
	;	Single-side subroutine execution
	;	(y - y0) * (x1 - x0) - (x - x0) * (y1 - y0)
	;	left = (point[Y] - edgeA[Y]) * (edgeB[X] - edgeA[X])
	;	right = (point[X] - edgeA[X]) * (edgeB[Y] - edgeA[Y])
	;	If portal is not backfaced:
	;	side = left - right
	;	If portal is backfaced:
	;	side = right - left
	;	If(side >= 0) then the vertex is "inside" the edge, we can test the next edge.
	;	If(side < 0) then the vertex is "outside" the edge. We must then move on to the next vertex.
	;	BACKFACE is bits 16-23 of RAM1[13]: If backfaced is TRUE, bit 16 will be 1. If false, will be 0.
	;	Cannot write to:
	;	RAM3[60] - RAM3[63], RAM0[0], RAM1[0] - RAM1[15], RAM2[0] - RAM2[3]
	;	Enters with:
	;	(Sets all CT, no need to control)
	;	Function Inputs:
	;	RAM2[4], pointY
	;	RAM2[5], edgeAX
	;	RAM2[6], edgeBY
	;	RAM1[16], edgeAY
	;	RAM1[17], edgeBX
	;	RAM1[18], pointX
	;	Function uses temporarily:
	;	RAM0[1,2], RAM3[0,1]
	;	Function outputs:
	;	CT0 = 0 ; CT1 = 0 ; CT2 = 5 ; CT3 = 2
	;	RAM3[2] = (side)
	CHIRALITY_ONE_SIDE:
											mov 16,ct1	; 
									clr a	mov 4,ct2	; CT1 = 16
	; left = (pointY - edgeAY) * (edgeBX - edgeAX) = RAM0[2]
	; To calculate: pointY - edgeAY
	;	pointY = RAM2[4]
	;	edgeAY = RAM1[16]
	;	Stored to: RAM3[0]
					mov mc1,p	mov mc2,a	mov 0,ct3	; CT2 = 4
	sub							mov alu,a				; CT2 = 5 ; CT1 = 17 ; CT3 = 0
								clr a		mov all,mc3	;
	; To calculate: edgeBX - edgeAX 
	;	edgeBX = RAM1[17]
	;	edgeAX = RAM2[5]
	;	Stored to: RX
					mov m2,p	mov mc1,a				; CT3 = 1
	sub							mov alu,a	mov 0,ct3	; CT1 = 18
								mov m3,y	mov all,rx	; CT3 = 0
	; To multiply those results:
	; pointY - edgeAY =	RAM3[0]
	; edgeBX - edgeAX = RX
					mov mul,p	clr a		mov 2,ct0	
	ad2							mov alu,a				; CT0 = 2
								clr a		mov all,mc0
	; LEFT = RAM0[2]
	; CT0 = 3, CT1 = 18, CT2 = 5, CT3 = 0
	; right = (pointX - edgeAX) * (edgeBY - edgeAY) = RAM3[1]
	; Established: edgeAX in RAM2 5, edgeAY in RAM1[16]
	; To calculate: pointX - edgeAX 
	;	pointX = RAM1[18]
	;	edgeAX = RAM2[5]
	;	Stored to RAM3[0]
					mov mc2,p	mov m1,a				; CT0 = 3
	sub							mov alu,a	mov 16,ct1	; CT2 = 6
								clr a		mov all,mc3	; CT1 = 16
	; To calculate: edgeBY - edgeAY 
	;	edgeBY = RAM2[6]
	;	edgeAY = RAM1[16]
	;	Stored to RX
					mov m1,p	mov m2,a				; CT3 = 1
	sub							mov alu,a	mov 0,ct3	
								mov mc3,y	mov all,rx	; CT3 = 0
	; To multiply those results:
	;	pointX - edgeAX = RAM3[0]
	;	edgeBY - edgeAY = RX
					mov mul,p	clr a					; CT3 = 1
	ad2							mov alu,a	mov 13,ct1
								clr a		mov all,mc3	; CT1 = 13
	; RIGHT = RAM3[1]
	;	CT0 = 3 ; CT1 = 13 ; CT2 = 6 ; CT3 = 2
	; Is the portal (occluder) backfaced?
	; Check bit 8 of RAM1[13].
	; This is for the (left - right) or (right - left) test.
	; Left = RAM0[2]
	; Right = RAM3[1]
	mvi $10000,PL										; CT3 = 2
								mov m1,a	mov 5,ct2
	and							mov alu,a	mov 1,ct3	; CT2 = 5	
	jmp Z,BACKFACED										; CT3 = 1
								clr a		mov 2,ct0	;
	;	The portal, at this point, was not backfaced.
	;	So we should perform (left - right).
	;	We will store the result in RAM3[2].
				mov mc3,p		mov m0,a	mov 0,ct0	; CT0 = 2
	sub							mov alu,a	mov 0,ct1	; CT0 = 0 ; CT3 = 2
								clr a		mov all,mc3	; CT1 = 0
	jmp FUNC_EXIT										; CT3 = 3
	nop
	BACKFACED:
	;	The portal, at this point, was backfaced.
	;	So we should perform (right - left).
	;	We will store the result in RAM3[2].
				mov m0,p		mov mc3,a	mov 0,ct0
	sub							mov alu,a	mov 0,ct1	; CT0 = 0 ; CT3 = 2
								clr a		mov all,mc3	; CT1 = 0
	;--------------------------------------------------------------
	; Subroutine end, return.
	FUNC_EXIT:
	BTM													; CT3 = 3
											mov 2,ct3	;
	nop													; CT3 = 2 (after function returns)
	
	
	
	
	
	
	