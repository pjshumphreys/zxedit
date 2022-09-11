ERR_SP equ 0x5c3d  ; BASIC system variables
ERR_NR equ 0x5c3a   ; BASIC system variables
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
  ld a, (memoryType)
  cp 0
  jr nz, esxgot128

  ld c, 0x04  ; '4'
  jp finalise

esxgot128:
  ld c, 0x05  ; '5'
  jp finalise

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
  ld a, (memoryType)
  cp 0
  jr nz, resigot128

  ld c, 0x02  ; '2'
  jp finalise

resigot128:
  ld c, 0x03  ; '3'
  jp finalise

;-------------------------------------
; plus3check

plus3check:
  ld a, 0xff
  ld (ERR_NR), a  ; clear error

  ; is plus3dos present?
  ld a, (memoryType)
  cp 2
  jr nz, plus3fail

  ld c, 0x01  ; '1'
  jp finalise

plus3fail:
  ; no disk io available at all
  ld c, 0x00  ; '0'

finalise:
  ; return a number to basic that represents the disk system we found
  ld b, 0
  pop hl
  pop de
  pop af
  ret
