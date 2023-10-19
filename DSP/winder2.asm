	INPUT = 12584960	; Constant to be changed by SH2 before program load.
	VERTTBL = 12584960	; Constant to be changed by SH2 before program load.
	PORTTBL = 12584960	; Constant to be changed by SH2 before program load.
	REPORT = 12584960	; End stat DMA address
	PORTAL_ACTIVE = $100
	PORTAL_OR_OCCLUDE = $200
	PORTAL_TYPE_BACK = $400
	PORTAL_TYPE_DUAL = $800
	;------------------------------------------------------------------------ P64 PROGRAM ADDRESS HEADER
														mov 0,ct1
	MVI INPUT,MC1													;	CT1 = 0
	MVI VERTTBL,MC1													;	CT1 = 1
	MVI PORTTBL,MC1													;	CT1 = 2
	MVI REPORT,MC1													;	CT1 = 3
														mov 0,ct1	;	CT1 = 4
										mov mc1,a		mov 60,ct3	;	CT1 = 0
	sl									mov alu,a					;	CT1 = 1, CT3 = 60
														mov all,ra0
										mov mc1,a		mov all,mc3 ;	
	sl									mov alu,a		mov 0,ct2	;	CT1 = 2, CT3 = 61
										mov mc1,a		mov all,mc3	;		
	sl									mov alu,a		mov 0,ct0	;	CT1 = 3, CT3 = 62
										mov mc1,a		mov all,mc3 ;	
	sl									mov alu,a		mov 0,ct1	;	CT1 = 4, CT3 = 63
														mov all,mc3	;
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
	;		unsigned char sectorA;
	;		unsigned char sectorB;
	;		unsigned char type;
	;		unsigned char backface;
	;	} _portal;
	;	
	;	#define PORTAL_TYPE_ACTIVE	(1)		//Flag applied to active portals (1 = active)
	;	#define PORTAL_OR_OCCLUDE	(1<<1)	//Portal IN or OUT setting (portal or occluder). 1 = portal. 0 = occluder.
	;	#define PORTAL_TYPE_BACK	(1<<2)	// Portal on back-facing (only used if portal facing away from camera), otherwise front facing
	;	#define PORTAL_TYPE_DUAL	(1<<3)	// Portal on both front and back facing (portal always used if active)
	;
	;	Next steps:
	;	Enabling multiple portals to be processed
	;
	;	Communication Input Layout:
	;	DMA -> RAM0[0] -> # of vertices to clip
	;	DMA -> RAM0[1] -> Instruction time delay per vertex
	;	DMA -> RAM0[2] -> Flag applied all vertices checked
	;	DMA -> RAM0[3] -> Clip flag when clipped IN occluder
	;	DMA -> RAM0[4] -> Clip flag applied when clipped OUT of 0 -> 1 edge of portal
	;	DMA -> RAM0[5] -> Clip flag applied when clipped OUT of 1 -> 2 edge of portal
	;	DMA -> RAM0[6] -> Clip flag applied when clipped OUT of 2 -> 3 edge of portal
	;	DMA -> RAM0[7] -> Clip flag applied when clipped OUT of 3 -> 0 edge of portal
	;	
	;	
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
	DMAH2 D0,MC0,8											; CT3 = 31
											mov 62,ct3		; CT0 = 4 ; Inputs to RAM0 0 ; 
											mov M3,RA0		; CT3 = 62
	DMAH2 D0,MC1,14											; Portal address (RAM3 62) in RA0
											mov 0,ct0		; CT1 = 12 ; DMA in verts[4][3] (12 ints) to RAM1[0] - RAM1[11]
											mov 61,ct3		; CT0 = 0
								mov m3,a	mov 3,PL		; CT3 = 61 (&vertex_t[0])
	ad2							mov alu,a	mov m3,ra0		; Move &vertex_t[0] to A, and 3 to P
											mov all,wa0		; Add 3 to &vertex_t[0] to reach &vertex_t[0].clipFlag
											mov all,mc3		; move &vertex_t[0].clipFlag to WA0 
	; -------------------------------------------------------
	; Procedure to skip the portal if it is not marked as active.
											mov 13,ct1		; CT3 = 62 ; &vertex_t[0].clipFlag back to RAM3 61
	mvi PORTAL_ACTIVE,PL									; CT1 = 13
								mov m1,a	
	and							
	jmp Z,FOR_EXIT1
	nop										
	; -----------------------------------------------------------------------
	; Take special note that RA0 and WA0 increment separately.
	; Because of this, I have to increment the addresses mathematically.
	; -----------------------------------------------------------------------
	; Intent: Use RAM0 0 as loop counter (# of vertices)
	; for(int ram0[0]; ram0[0] > 0; ram0[0]--)
	; Information:
	; Loop count			->	RAM0[0]
	; Portal information	->	RAM1[0] - RAM1[13]
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
	;	SH2 Sync Delay
	;	Moves RAM0[1] to LOP, then processes a nop with lps here.
											mov 1,ct0
											mov m0,LOP	; CT0 = 1
	lps
	nop										mov 12,ct1	
	;	---------------------------------------------------------------------
	;	Loop body
	;	Step 1: DMA in vertex_t[i]. Four units ( 16 bytes ).
	;	DMA with address increment will be used.
	;	---------------------------------------------------------------------
	DMA2 D0,MC2,4
	;	---------------------------------------------------------------------
	;	Step 2: Check the vertex' depth against the portal's depth.
	;	If the vertex Z is lower than the portal's Z (read: nearer to camera), cease the test.
	;	point[Z] = RAM2[2]
	;	portal[i].depth = RAM1[12]
											mov 2,ct2	; CT1 = 12
					mov m1,p	mov m2,a				; CT2 = 2
	sub							mov alu,a	mov 1,ct2	;
	jmp S,UNCLIPPED										; CT2 = 1
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
	TEST_01:
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
														; CT0 = 0 ; CT1 = 13 ; CT2 = 5 ; CT3 = 2
	;	Next: Check the portal's clip OUT or clip IN setting.
	;	RAM1[13] -> Portal settings, bit 9: 0 is clip IN (occlude), 1 is clip OUT (portal)
	;	DMA -> RAM0[4] -> Clip flag applied when clipped OUT of 0 -> 1 edge of portal
							mov m1,a
	mvi PORTAL_OR_OCCLUDE,PL
	and
	jmp Z,OCCLUDE_01
				mov m3,p	clr a			
	ad2						mov alu,a		mov 4,ct0	
	jmp NS,TEST_12
	nop										mov 3,ct2	; CT0 = 4 (clip flag for OUT of 0->1)
				mov m0,p	mov m2,a					; CT2 = 3 (vertex clipFlag)
	or						mov alu,a		
											mov all,mc2
	jmp TEST_12											; CT2 = 4
	OCCLUDE_01:
	; -----------------------------------------------------------------------
	;	Next: If RAM3[2] is < 0, jump to UNCLIPPED.
	;	If RAM3[2] is >= 0, set-up and test p1 -> p2.
				mov m3,p	clr a			
	ad2						mov alu,a		
	jmp S,UNCLIPPED
	;	RAM2[4], pointY from RAM2[1]
	;	RAM2[5], edgeAX	from RAM1[3]
	;	RAM2[6], edgeBY from RAM1[7]
	;	RAM1[16], edgeAY from RAM1[4]
	;	RAM1[17], edgeBX from RAM1[6]
	;	RAM1[18], pointX from RAM2[0]
	TEST_12:
											mov 5,ct2
											mov 3,ct1	; CT2 = 5 (warning: executes after jump)
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
	;	Next: Check the portal's clip OUT or clip IN setting.
	;	RAM1[13] -> Portal settings, bit 9: 0 is clip IN (occlude), 1 is clip OUT (portal)
	;	DMA -> RAM0[5] -> Clip flag applied when clipped OUT of 1 -> 2 edge of portal
							mov m1,a
	mvi PORTAL_OR_OCCLUDE,PL
	and
	jmp Z,OCCLUDE_12
				mov m3,p	clr a			
	ad2						mov alu,a		mov 5,ct0	
	jmp NS,TEST_23
	nop										mov 3,ct2	; CT0 = 5 (clip flag for OUT of 1->2)
				mov m0,p	mov m2,a					; CT2 = 3 (vertex clipFlag)
	or						mov alu,a		
											mov all,mc2
	jmp TEST_23
	OCCLUDE_12:
	; -----------------------------------------------------------------------
	;	Next: If RAM3[2] is < 0, jump to UNCLIPPED.
	;	If RAM3[2] is >= 0, set-up and test p2 -> p3.
				mov m3,p	clr a			
	ad2						mov alu,a		
	jmp S,UNCLIPPED
	;	RAM2[4], pointY from RAM2[1]
	;	RAM2[5], edgeAX	from RAM1[6]
	;	RAM2[6], edgeBY from RAM1[10]
	;	RAM1[16], edgeAY from RAM1[7]
	;	RAM1[17], edgeBX from RAM1[9]
	;	RAM1[18], pointX from RAM2[0]
	TEST_23:
											mov 5,ct2
											mov 6,ct1	; CT2 = 5 (warning: executes after jump)
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
	;	Next: Check the portal's clip OUT or clip IN setting.
	;	RAM1[13] -> Portal settings, bit 9: 0 is clip IN (occlude), 1 is clip OUT (portal)
	;	DMA -> RAM0[6] -> Clip flag applied when clipped OUT of 2 -> 3 edge of portal
							mov m1,a
	mvi PORTAL_OR_OCCLUDE,PL
	and
	jmp Z,OCCLUDE_23
				mov m3,p	clr a			
	ad2						mov alu,a		mov 6,ct0	
	jmp NS,TEST_30
	nop										mov 3,ct2	; CT0 = 6 (clip flag for OUT of 2->3)
				mov m0,p	mov m2,a					; CT2 = 3 (vertex clipFlag)
	or						mov alu,a		
											mov all,mc2
	jmp TEST_30
	OCCLUDE_23:
	; -----------------------------------------------------------------------
	;	Next: If RAM3[2] is < 0, jump to UNCLIPPED.
	;	If RAM3[2] is >= 0, set-up and test p3 -> p0.
				mov m3,p	clr a			
	ad2						mov alu,a		
	jmp S,UNCLIPPED
	;	RAM2[4], pointY from RAM2[1]
	;	RAM2[5], edgeAX	from RAM1[9]
	;	RAM2[6], edgeBY from RAM1[1]
	;	RAM1[16], edgeAY from RAM1[10]
	;	RAM1[17], edgeBX from RAM1[0]
	;	RAM1[18], pointX from RAM2[0]
	TEST_30:
											mov 5,ct2
											mov 9,ct1	; CT2 = 5 (warning: executes after jump)
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
	;	Next: Check the portal's clip OUT or clip IN setting.
	;	RAM1[13] -> Portal settings, bit 9: 0 is clip IN (occlude), 1 is clip OUT (portal)
	;	DMA -> RAM0[7] -> Clip flag applied when clipped OUT of 3 -> 0 edge of portal
							mov m1,a
	mvi PORTAL_OR_OCCLUDE,PL
	and
	jmp Z,OCCLUDE_30
				mov m3,p	clr a			
	ad2						mov alu,a		mov 7,ct0	
	jmp NS,UNCLIPPED
	nop										mov 3,ct2	; CT0 = 7 (clip flag for OUT of 3->0)
				mov m0,p	mov m2,a					; CT2 = 3 (vertex clipFlag)
	or						mov alu,a		
											mov all,mc2
	jmp UNCLIPPED
	OCCLUDE_30:
	; -----------------------------------------------------------------------
	;	Next: If RAM3[2] is < 0, jump to UNCLIPPED.
	;	If RAM3[2] is >= 0, all edges have passed the test (in case of clip IN).
	;	We shall then mark RAM2[3] with the data DMA'd in at RAM0[3], and DMA it out. 
	;	DMA8 cannot be used here as the offset is unknown.
				mov m3,p	clr a			
	ad2						mov alu,a		mov 3,ct0
	jmp S,UNCLIPPED
											mov 3,ct2
				mov m0,p	mov m2,a
	or						mov alu,a		mov 30,ct3
							clr a			mov all,mc2
											mov 3,ct2
	DMAH2 MC2,D0,1
	;	In this case, I also want to use, say, RAM3[30] to store the # of hits.
							mov m3,a		mov 1,PL
	ad2						mov alu,a		
											mov all,mc3
	jmp CONTINUE:
	UNCLIPPED:
	;	In case the vertex is not clipped IN, mark the vertex as being checked with the data DMA'd in at RAM0[2], then continue.
	;	This branch will also be reached by portals that clip OUT.
											mov 3,ct2
											mov 2,ct0
				mov m0,p	mov m2,a
	or						mov alu,a		
							clr a			mov all,mc2
											mov 3,ct2
	DMAH2 MC2,D0,1
	;	---------------------------------------------------------------------
	;	CT0 = ?? ; CT1 = ?? ; CT2 = ?? ; CT3 = ?? ; Treat all counters as undefined.
	;	We must retrieve the output DMA address in RAM3[61] and add 4 to it then write it back.
	CONTINUE:
							clr a			mov 61,ct3
							mov m3,a		mov 4,PL
	add						mov alu,a
											mov all,wa0
							clr a			mov all,mc3
	jmp FOR_ENTRY1 	; Return to top of loop
											mov 0,ct0
	;	---------------------------------------------------------------------
	FOR_EXIT1:
											mov 63,ct3	; 
											mov m3,wa0	; Procedure to move reporting address back to WA0
											mov 29,ct3	; Procedure to move #1 to RAM3[29]
	mvi 1,MC3
											mov 29,ct3
	DMAH2 MC3,D0,2										; Procedure to DMA RAM3[29,30] out to the reporting address
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
	;	BACKFACE is bits 0-7 of RAM1[13]: If backfaced is TRUE, bit 0 will be 1. If false, will be 0.
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
	;	RAM2[7], RAM3[0,1]
	;	Function outputs:
	;	CT0 = 0 ; CT1 = 13 ; CT2 = 5 ; CT3 = 2
	;	RAM3[2] = (side)
	CHIRALITY_ONE_SIDE:
											mov 16,ct1	; 
									clr a	mov 4,ct2	; CT1 = 16
	; left = (pointY - edgeAY) * (edgeBX - edgeAX) = RAM2[7]
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
					mov mul,p	clr a		mov 7,ct2	
	ad2							mov alu,a				; CT2 = 7
								clr a		mov all,mc2
	; LEFT = RAM2[7]
	; CT0 = ??, CT1 = 18, CT2 = 8, CT3 = 0
	; right = (pointX - edgeAX) * (edgeBY - edgeAY) = RAM3[1]
	; Established: edgeAX in RAM2 5, edgeAY in RAM1[16]
	; To calculate: pointX - edgeAX 
	;	pointX = RAM1[18]
	;	edgeAX = RAM2[5]
	;	Stored to RAM3[0]
											mov 5,ct2	; CT2 = 8
					mov mc2,p	mov m1,a				; CT2 = 5
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
					mov mul,p	clr a		mov 13,ct1	; CT3 = 1
	ad2							mov alu,a	mov 1,PL	; CT1 = 13
								clr a		mov all,mc3	; PL = 1
	; RIGHT = RAM3[1]
	;	CT0 = ?? ; CT1 = 13 ; CT2 = 6 ; CT3 = 2
	; Is the portal (occluder) backfaced?
	; Check bit 0 of RAM1[13].
	; This is for the (left - right) or (right - left) test.
	; Left = RAM2[7]
	; Right = RAM3[1]
								mov m1,a	mov 7,ct2	; CT3 = 2
	and										mov 1,ct3	; CT2 = 7
	jmp Z,BACKFACED										; CT3 = 1
								clr a		
	;	The portal, at this point, was not backfaced.
	;	So we should perform (left - right).
	;	We will store the result in RAM3[2].
				mov mc3,p		mov m2,a	mov 0,ct0	;
	sub							mov alu,a	mov 13,ct1	; CT0 = 0 ; CT3 = 2
								clr a		mov all,mc3	; CT1 = 13
	jmp FUNC_EXIT										; CT3 = 3
	nop
	BACKFACED:
	;	The portal, at this point, was backfaced.
	;	So we should perform (right - left).
	;	We will store the result in RAM3[2].
				mov m2,p		mov mc3,a	mov 0,ct0
	sub							mov alu,a	mov 13,ct1	; CT0 = 0 ; CT3 = 2
								clr a		mov all,mc3	; CT1 = 13
	;--------------------------------------------------------------
	; Subroutine end, return.
	FUNC_EXIT:
	BTM													; CT3 = 3
											mov 2,ct3	;
	nop													; CT3 = 2 (after function returns)
	
	
	
	
	
	
	