port1 equ 0x7ffd  ; address of ROM/RAM switching port in I/O map
bankm equ 0x5b5c  ; system variable that holds the last value output to 7FFDh
PROG equ 0x5c53
offset equ 49

;prevent assembly code from being listed
defb 0x0d
defb 0xff

;save af, de and hl. BC will be our return value back to basic
push af
push de
push hl

;memcpy the main code to 8000 and above so we don't have to keep using relative jumps
ld hl, (PROG)
ld de, dataStart+offset
add hl, de
ld de, 0x8000
ld bc, dataEnd-dataStart
di
ldir
ei

;identify whether 128k of main memory is present and store the result for later
ld hl, (PROG)
ld de, checkmem+offset
add hl, de
call 0x8004 ; jp (hl)
ld (0x8003), a ; memoryType

;now figure out what disk system we have
jp 0x8000

checkmem:
  ld  a,(75)
  cp  191
  jr  z, noram
  ld  a,(23611)
  and 16
  jr  z, noram

  ld bc, port1  ; the horizontal ROM switch/RAM switch I/O address

  di
  ld a, (bankm)  ; system variable that holds current switch state
  res 4, a  ; move right to left in horizontal ROM switch (3 to 2)
  ld (bankm), a  ; must keep system variable up to date (very important)
  out (c), a

  ld a, (0x0008)
  cp 0x50  ; 'P' of 'PLUS3DOS'

  ld a, (bankm)  ; get current switch state
  set 4, a  ; move left to right (ROM 2 to ROM 3)
  jr z, plus3
  and 0xf8  ; also want RAM page 0
  ld (bankm), a  ; update the system variable (very important)
  out (c), a
  ei
  ld a, 1  ; 128k, but no plus3 dos
  ret

noram:
  ld a, 0  ; 48k only
  ret

plus3:
  and 0xf8  ; also want RAM page 0
  ld (bankm), a  ; update the system variable (very important)
  out (c), a
  ei
  ld a, 2  ; 128k with plus3dos
  ret

dataStart:
  binary "zxedit2.bin"
dataEnd:
