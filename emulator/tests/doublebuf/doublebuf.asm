.include "../sys/sys.asm"

; display buffers at beginning of RAM
.const buffer0  0x2000000
.const buffer1  0x2012C00
.const pal      0x2025800
.const buffer   0x2025B00

.const rnd_size 1024

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

swap_buffers:
	ife [VDP0_DATA], buffer0
		set.l pc, swap_to_1

	; swap to buffer 0
	set.l [VDP0_DATA], buffer0
	set.l [buffer], buffer1
			
	set.l pc, pop

	swap_to_1:
		; swap to buffer 1
		set.l [VDP0_DATA], buffer1
		set.l [buffer], buffer0

		set.l pc, pop

start:
	; inititalize layer 0
	set.l [VDP0_MODE], 1
	set.l [VDP0_WIDTH], DISPLAY_W
	set.l [VDP0_HEIGHT], DISPLAY_H
	set.l [VDP0_PAL], pal
        set.l [VDP0_DATA], buffer0

	; set up interrupt vector
	set.l [IRQ_VBLANK], handle_vsync

	set.l [buffer], buffer1

	; generate palettes
	set.l x, pal
	jsr gen_palette

	set.l c, 0

	set.l c, 0
	loop:
		set.l i, [buffer]
		set.l j, i
		add j, 76800

		add c, 1
		mod c, rnd_size

		copy_loop:
			set.b [i], [rnd+c]

			add i, 1
			add z, 1

			ifn i, j
				set.l pc, copy_loop
						

		; wait for VSYNC
		sys SYS_WAIT_INTERRUPT
		jsr swap_buffers

		set.l pc, loop

msg: .dw      "test", 10, 0
rnd: .incbin  "rnd.dat"
