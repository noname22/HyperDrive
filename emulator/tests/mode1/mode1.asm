.const VDP_BASE 0x3000100

.const VDP0_X    0x300010C
.const VDP0_Y    0x3000110

.const VDP1_X    0x300012C
.const VDP1_Y    0x3000130

.const VDP_LAYER1 0x3000120

.const IRQ_HBLANK 0x3000000
.const IRQ_VBLANK 0x3000004

start:
	; ==== Set up hblank and vblank interrupt vector ====
	set.l [IRQ_HBLANK], hblank 
	set.l [IRQ_VBLANK], vblank 

	; ==== Initialize VDP                            ====
	set.l a, VDP_BASE
	
	; Layer 0 mode 1, bitmap
	set.l [a], 1
	add a, 4

	; Width
	set.l [a], 128
	add a, 4
	
	; Height
	set.l [a], 128
	add a, 4
	
	; X
	set.l [a], 96
	add a, 4
	
	; Y
	set.l [a], 56
	add a, 4

	; Tileset (not used)
	set.l [a], 0
	add a, 4
	
	; Palette
	set.l [a], pal
	add a, 4

	; Bitmap data
	set.l [a], bitmap
	add a, 4

	; Color key
	set.b [a], 0
	add a, 1
	
	; Blend mode
	set.b [a], 1
	add a, 1

	set.l a, VDP_LAYER1 
	
	; Layer 1 mode 1, bitmap
	set.l [a], 1
	add a, 4

	; Width
	set.l [a], 128
	add a, 4
	
	; Height
	set.l [a], 128
	add a, 4
	
	; X
	set.l [a], 96
	add a, 4
	
	; Y
	set.l [a], 56
	add a, 4

	; Tileset (not used)
	set.l [a], 0
	add a, 4
	
	; Palette
	set.l [a], pal
	add a, 4

	; Bitmap data
	set.l [a], bitmap
	add a, 4

	; Color key
	set.b [a], 255
	add a, 1
	
	; Blend mode
	set.b [a], 2
	add a, 1


	; ==== Initialize sintab index registers       ====

	set.l x, 0
	set.l z, 0

	; ==== Loop and halt the CPU                   ====
	;         (interrupts will keep waking it up)

loop:
	sys 0
	set.l pc, loop

vblank:
	add z, 8       ; increase index into the sintab
		       ; that X gets reset.l to (to create the wave effect)

	set.l x, z       ; reset.l the index into the sintab to Z
	set.l pc, pop

hblank:
	mod x, 1024    ; make sure the index stays within the sintab

	; Set the VDP X and Y position registers to the sintab value
	; Plus the width/height of the bitmap - width/height of the screen / 2

	set.l [VDP1_X], [sintab+x]
	add [VDP1_X], 96
	
	set.l [VDP1_Y], [sintab+x]
	add [VDP1_Y], 56
	
	add x, 4

	set.l pc, pop

bitmap: .incbin "bear.dat"
pal:    .incbin "bear.pal"
sintab: .incbin "sintab.dat"
