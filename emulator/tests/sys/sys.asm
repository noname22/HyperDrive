; jump past any code in this system header and begin at 
; the user defined label 'start'
entry:
	set.l pc, start

.const VDP_BASE      0x3000100

.const VDP0_MODE     0x3000100
.const VDP0_WIDTH    0x3000104
.const VDP0_HEIGHT   0x3000108
.const VDP0_X        0x300010C
.const VDP0_Y        0x3000110
.const VDP0_TILESET  0x3000114
.const VDP0_PAL      0x3000118
.const VDP0_DATA     0x300011C
.const VDP0_CKEY     0x3000120
.const VDP0_BMODE    0x3000121

.const VDP1_X        0x300012C
.const VDP1_Y        0x3000130

.const VDP_LAYER1    0x3000120

.const IRQ_HBLANK    0x3000000
.const IRQ_VBLANK    0x3000004

.const DISPLAY_W     320
.const DISPLAY_H     240

.const SYS_WAIT_INTERRUPT 0
.const SYS_PRINT          1
