.include "../sys/sys.asm"

.const level   0x2000000

handle_vsync:
	set.l pc, pop

start:
	set.l [IRQ_VBLANK], handle_vsync
	
	; inititalize layer 0
	set.l [VDP0_MODE], 4 ; 16x16
	set.l [VDP0_WIDTH], 32
	set.l [VDP0_HEIGHT], 32
	set.l [VDP0_PAL], pal
        set.l [VDP0_DATA], level
	set.l [VDP0_TILESET], tileset
	set.l [VDP0_CKEY], 1

	lvl_loop:
		set.b [level+x], 1

		add x, 6
		ifg 1024, x
			set.l pc, lvl_loop
		

	loop:
		sub [VDP0_X], 1

		sys SYS_WAIT_INTERRUPT
		set.l pc, loop

tileset: .incbin "tileset.dat"
pal:     .incbin "tileset.pal"
