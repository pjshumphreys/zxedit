all: zxedit.tap

#zcc +zx -lesxdos -DAMALLOC1 -create-app -zorg=49152 -Cz--merge -Czzxedit.tap zxedit.c

zxedit.tap: zxedit2.bas
	./zmakebas -a 10 -n zxedit -o zxedit.tap zxedit2.bas

zxedit2.bin: zxedit2.asm
	z88dk-z80asm -b zxedit2.asm

zxedit.bin: zxedit2.bin zxedit.asm
	z88dk-z80asm -b zxedit.asm

zxedit2.bas: zxedit.bin zxedit.bas
	printf '1 REM' > zxedit2.bas
	xxd -ps -u -c 100000 zxedit.bin | sed -e "s/\(..\)/\\\{0x\1\}/g" >> zxedit2.bas
	cat zxedit.bas >> zxedit2.bas 

clean:
	rm -f zxedit2.bas zxedit.tap *.bin *.o *.inc *.map
