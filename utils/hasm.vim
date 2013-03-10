" Vim syntax file
" Language:	Nurd Hyperdrive Assembler
" By:	Fredrik Hultin <noname@nurd.se>, Jan Nárovec <finn@sendmail.cz> (avrasm file)
" Last Change:	2013 March 03

" put this file in ~/.vim/syntax and add
" au BufNewFile,BufRead *.hasm set filetype=hasm
" to ~/.vimrc

if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syn case ignore

syn match hAsmIdentifier	"[a-zA-Z][0-9a-zA-Z_]*"

"syn match decNumber		"0\+[1-7]\=[\t\n$,; ]"
syn match decNumber		"0\+\d*"
syn match decNumber		"[1-9]\d*"
syn match octNumber		"0[0-7][0-7]\+"
syn match hexNumber		"0x[0-9a-fA-F]\+"
syn match binNumber		"0[bB][0-1]*"

syn region hAsmString		start=/"/ end=/"/

syn match hAsmSpecialComment	";\*\*\*.*"
syn match hAsmComment		";.*"hs=s+1
syn match hAsmLab		"[a-zA-Z_][0-9a-zA-Z_]*:"he=e-1

syn match hAsmInclude		"\.include"
syn match hAsmEqu		"\.equ"
syn match hAsmSet		"\.set"
syn match hAsmType		"\.db"
syn match hAsmType		"\.dw"
syn match hAsmType		"\.dd"
syn match hAsmDef		"\.const"
syn match hAsmMacro		"\.macro"
syn match hAsmMacro		"\.end"
syn match hAsmMacro		"\.define"
syn match hAsmOrg		"\.org\s[0-9]\+"
syn match hAsmSeg		"\.cseg"
syn match hAsmSeg		"\.dseg"
syn match hAsmSeg		"\.eseg"
syn match hAsmAsm		"\.device"
syn match hAsmAsm		"\.exit"
syn match hAsmAsm		"\.list"
syn match hAsmAsm		"\.nolist"
syn match hAsmAsm		"\.listmac"

syn keyword hAsmReg		a  b  c  x  y  z  i  j 

syn keyword hAsmOpcode          jsr sys set add sub mul div mod shl shr and bor xor ife ifn ifg ifb

" syn keyword hAsmIOReg		ubrrh ubrrl ubrr ucr ucsrb ucsra udr usr
" syn keyword hAsmIOReg		adcl adch adcsr admux acsr
" syn keyword hAsmIOReg		spcr spsr spdr
" syn keyword hAsmIOReg		pina pinb pinc pind
" syn keyword hAsmIOReg		ddra ddrb ddrc ddrd
" syn keyword hAsmIOReg		porta portb portc portd
" syn keyword hAsmIOReg		eecr eedr eear eearl eearh 
" syn keyword hAsmIOReg		wdtcr
" syn keyword hAsmIOReg		assr ocr2 tccr2 tcnt2
" syn keyword hAsmIOReg		icr1l icr1h ocr1bl ocr1bh ocr1al ocr1ah tcnt1l tcnt1h tccr1b tccr1a
" syn keyword hAsmIOReg		tcnt0 tccr0
" syn keyword hAsmIOReg		mcusr mcucr
" syn keyword hAsmIOReg		tifr timsk gifr gimsk
" syn keyword hAsmIOReg		sp sph spl sreg

" syn keyword hAsmOperator	! ~ + - * / >> << < <= > >= == != & ^ | && ||

syn case match

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_asm_syntax_inits")
  if version < 508
    let did_asm_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  " The default methods for highlighting.  Can be overridden later
  HiLink hAsmSeg		Special
  HiLink hAsmOrg		Special
  HiLink hAsmComment		Comment
  HiLink hAsmAsm		PreCondit
  HiLink hAsmInclude		Include
  HiLink hAsmEqu		Typedef
  HiLink hAsmSet		Typedef
  HiLink hAsmDef		Typedef
  HiLink hAsmType		Type
  HiLink hAsmOpcode		Statement
  HiLink hAsmOperator		Operator

  HiLink hAsmInclude		Directive
  HiLink hAsmList		Directive
  HiLink hAsmMacro		Macro
  
  HiLink hAsmString		String

  HiLink hexNumber		Number
  HiLink decNumber		Number
  HiLink octNumber		Number
  HiLink binNumber		Number

  HiLink hAsmIdentifier	Normal

  HiLink hAsmReg		Identifier
  HiLink hAsmIOReg		Identifier
  HiLink hAsmLab		ModeMsg

  delcommand HiLink
endif

let b:current_syntax = "hasm"

" vim: ts=8
