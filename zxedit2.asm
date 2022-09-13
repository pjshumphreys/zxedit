ERR_SP equ 0x5c3d  ; BASIC system variables
ERR_NR equ 0x5c3a
PROG equ 0x5c53
NEWPPC equ 0x5c42
NSPPC equ 0x5c44
VARS equ 0x5c4b
NXTLIN equ  0x5c55
RST_HOOK equ 8

__ESXDOS_SYS_M_GETSETDRV equ 0x89
HOOK_VERSION equ 0xfc

org 0x8000
  jp getDiskIO

memoryType:
  defb 0

  jp (hl)

;-------------------------------------
; getDiskIO

getDiskIO:
  ;check for residos. if it's not present then continue on to the check for esxdos
  ld a,(0x12a0)
  cp 0xcf
  jr nz, esxcheck

  ld a,(0x12a1)
  cp 0xfe
  jr z, resicheck

;-------------------------------------
; esxcheck

esxcheck:
  push ix
  push iy

  ; temporarily route error handling back here
  ld hl, (ERR_SP)
  push hl  ; save the existing ERR_SP

  ld hl, esxfail
  push hl

  ld hl, 0
  add hl, sp
  ld (ERR_SP), hl

  xor a  ; ld a, 0
  rst RST_HOOK
  defb __ESXDOS_SYS_M_GETSETDRV
  pop hl  ; if the code doesn't work, comment out this line
  jr esxcont

esxfail:
  xor a  ; ld a, 0

esxcont:
  pop hl
  ld (ERR_SP), hl

  pop iy
  pop ix

  cp 0
  jp z, plus3check

  ; is 128k present?
  ;ld a, (memoryType)
  ;cp 0
  ;jr nz, esxgot128

  ; '4'
  ;...
  ;jp reenter

;esxgot128:
  ; '5'
  jp reenter

;-------------------------------------
; resicheck

resicheck:
  ld hl, (ERR_SP)
  push hl  ; save the existing ERR_SP

  ld hl, resicont
  push hl  ; stack error-handler return address

  ld hl, 0
  add hl, sp
  ld (ERR_SP), hl  ; set the error-handler SP

  rst RST_HOOK  ; invoke the version info hook code
  defb HOOK_VERSION
  pop hl  ; ResiDOS doesn't return, so if we get
  jr plus3check  ; here, some other hardware is present

resicont:
  pop hl
  ld (ERR_SP), hl  ; restore the old ERR_SP
  ld a, (ERR_NR)
  inc a  ; is the error code now "OK"?
  jr nz, plus3check  ; if not, ResiDOS was not detected
  ex de, hl  ; get HL=ResiDOS version
  ld de, 0x0140  ; DE=minimum version to run with
  and a
  sbc hl, de
  jr c, plus3check

  ld a, 0xff
  ld (ERR_NR), a    ; clear er

  ; is 128k present?
  ;ld a, (memoryType)
  ;cp 0
  ;jr nz, resigot128

  ; '2'
  ;jp reenter

;resigot128:
  ; '3'
  ld hl, loadPrefix
  ld a, $25 ; '%'
  ld (hl), a
  ld hl, loadPostfix
  ld a, $72 ; 'r'
  ld (hl), a
  jp reenter

;-------------------------------------
; plus3check

plus3check:
  ld a, 0xff
  ld (ERR_NR), a  ; clear error

  ; is plus3dos present?
  ld a, (memoryType)
  cp 2
  jr nz, plus3fail

  ; '1'
  ld hl, loadPrefix
  ld a, $20 ; ' '
  ld (hl), a
  ld hl, loadPostfix
  ld a, $70 ; p
  ld (hl), a
  jp reenter

plus3fail:
  ; if its not then show a message
  ld bc, failProgEnd-failProg
  ld de, failProg
  jr loadBasic

reenter:
  ld bc, newProgEnd-newProg
  ld de, newProg

loadBasic:
  ;set NEWPPC to $000a (line 10)
  ld hl, $000a
  ld (NEWPPC), hl

  ;set NSPPC to 0 (first statement on line 10)
  push af
  ld a, 0
  ld (NSPPC), a
  pop af

  ;set VARS to prog + (newProgEnd-newProg)
  ld hl, (PROG)
  add hl, bc
  ld (VARS), hl

  ;set NXTLIN to prog + (newProgEnd-newProg) - 2
  dec hl
  dec hl
  ld (NXTLIN), hl

  ;copy the new program
  ld hl, (PROG)
  ex de, hl
  di
  ldir
  ei

  ;restore original registers
  ld a, 0xff
  ld (ERR_NR), a
  pop hl
  pop de
  pop af
  ret

newProg:
  ;10 CLEAR VAL "32767":LOAD *"ed-e.bin"CODE:RANDOMIZE USR VAL "32768"
  defb $00, $0a, $27, $00, $fd, $b0, $22, $33, $32, $37, $36, $37, $22, $3a
  defb $ef

loadPrefix:
  defb $2a, $22, "ed"

loadPostfix:
  defb "e.bin", $22, $af, $3a
  defb $f9, $c0, $b0, $22, $33, $32, $37, $36, $38, $22, $0d, $ff, $0d, $80
newProgEnd:

failProg:
  ;10 PRINT "Either ESXDOS, ResiDOS v1.40+ or PLUS3DOS is required"
  defb $00, $0a, $39, $00, $f5, $22, "Either ESXDOS, ResiDOS v1.40+ or PLUS3DOS is required", $22, $0d, $ff, $0d, $80
failProgEnd:
