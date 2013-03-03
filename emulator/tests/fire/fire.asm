.include "../sys/sys.asm"

; display buffer at beginning of RAM
.const buffer   0x2000000
.const pal      0x2025800

.const rnd_size 1024

; generate a palette, x -> target addr, y -> num steps
gen_palette:
	set.l i, 0

	gp_loop:
		set.b [x], i
		add i, 1
		add x, 3
		ifn i, y
			set.l pc, gp_loop
	
	set.l pc, pop

start:
	; inititalize layer 0
	set.l [VDP0_MODE], 1
	set.l [VDP0_WIDTH], DISPLAY_W
	set.l [VDP0_HEIGHT], DISPLAY_H
	set.l [VDP0_PAL], pal
        set.l [VDP0_DATA], buffer

	; generate palettes
	set.l x, pal
	set.l y, 256
	jsr gen_palette

	set.l x, pal
	add x, 193
	set.l y, 192
	jsr gen_palette

	set.l c, 0

	set.l c, 0
	loop:
		; generate some junk

		set.l i, buffer
		add i, 76480 ; (320 * 240 - 320)
		set.l j, i
		add j, 320 

		gen_loop:
			set.b [i], [rnd+c]

			add c, 1
			mod c, rnd_size

			add i, 1
			ifn i, j
				set.l pc, gen_loop

		; copy and blur line below, lines 176 - 240

		set.l i, buffer
		set.l j, i
		add j, 76480

		add i, 66560

		set.l z, i
		add z, 319

		copy_loop:
			set.l x, z
			set.b y, [x]
			set.l a, y

			add x, 1
			set.b y, [x]
			add a, y
			
			add x, 1
			set.b y, [x]
			add a, y

			div a, 3
			ifg a, 4
				sub a, 4

			set.b [i], a

			add i, 1
			add z, 1

			ifn i, j
				set.l pc, copy_loop

		set.l pc, loop

msg: .dw      "test", 10, 0
rnd: .incbin  "rnd.dat"
