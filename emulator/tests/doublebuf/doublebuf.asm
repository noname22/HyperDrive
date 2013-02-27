.include "../sys/sys.asm"

; display buffers at beginning of RAM
.const buffer0 0x2000000
.const buffer1 0x2012C00
.const pal0    0x2025800
.const pal1    0x2025900

; generate a palette, x -> target addr
gen_palette:
	set.l i, 0

	gp_loop:
		set.b [x], i
		add i, 1
		add x, 3
		ifn i, 256
			set.l pc, gp_loop
	
	set.l pc, pop

handle_vsync:
	set.l pc, pop

buffer:        .dw 0, 0, 0, 0
buf_num:       .dw 0, 0, 0, 0
buf_ptr:       .dw 0, 0, 0, 0
buf_ptr_1:     .dw 0, 0, 0, 0

swap_buffers:
	add [buf_num], 4
	mod [buf_num], 8

	set.l i, [buf_num]
	set.l [buffer], [buf_ptr+i]

	; return
	set.l pc, pop

start:
	; inititalize layer 0
	set.l [VDP0_MODE], 1
	set.l [VDP0_WIDTH], DISPLAY_W
	set.l [VDP0_HEIGHT], DISPLAY_H
	set.l [VDP0_PAL], pal0
        set.l [VDP0_DATA], buffer0

	; set up interrupt vector
	set.l [IRQ_VBLANK], handle_vsync

	; set up buffers
	set.l [buf_ptr], buffer0
	set.l [buf_ptr_1], buffer1

	; generate palettes
	set.l x, pal0
	jsr gen_palette

	set.l x, pal1
	jsr gen_palette

	set.l a, 0

	loop:
		set.l x, 0
		set.l y, 0

		y_loop:
			set.l i, DISPLAY_W
			mul i, y
			add i, 160

			set.b [buffer+i], 255

			add y, 1
			ifn y, DISPLAY_W
				set.l pc, y_loop
			


		; wait for VSYNC
		sys SYS_WAIT_INTERRUPT
		set.l pc, loop
