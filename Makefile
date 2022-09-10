all: zxedit.tap ed.com zxedit.d64

zxedit.d64: ed.prg
	c1541 -format diskname,id d64 zxedit.d64 -attach zxedit.d64 -write ed.prg ed

ed.com: zxedit.c
	zcc +cpm -DAMALLOC1 -o ed.com zxedit.c

ed.prg: zxedit.c
	cl65 -t c64 -o ed.prg zxedit.c

zxedit.tap: ed edplus3.bin edesx.bin edresi.bin
	sh -c "(echo zxedit.tap > archived.txt) && (ls ed >> archived.txt) && (ls edplus3.bin >> archived.txt) && (ls edresi.bin >> archived.txt) && (ls edesx.bin >> archived.txt) && (echo "" >> archived.txt) && (cat archived.txt | ./archiver)"
	rm archived.txt

edplus3.bin: zxedit.c
	zcc +zx -lp3 -DAMALLOC1 -clib=ansi -pragma-define:ansicolumns=32 -pragma-define:ansifont=15616 -pragma-define:ansifont_is_packed=0 -zorg=49152 zxedit.c
	z88dk-appmake +zx --dos --org 49152 --binfile a.bin --output edplus3.bin

edresi.bin: zxedit.c
	zcc +zx -DRESIDOS -lp3 -clib=ansi -pragma-define:ansicolumns=32 -pragma-define:ansifont=15616 -pragma-define:ansifont_is_packed=0 -DAMALLOC1 -zorg=49152 zxedit.c
	z88dk-appmake +zx --dos --org 49152 --binfile a.bin --output edresi.bin

edesx.bin: zxedit.c
	zcc +zx -lesxdos -clib=ansi -pragma-define:ansicolumns=32 -pragma-define:ansifont=15616 -pragma-define:ansifont_is_packed=0 -DAMALLOC1 -zorg=49152 zxedit.c
	z88dk-appmake +zx --dos --org 49152 --binfile a.bin --output edesx.bin

ed: zxedit2.bas
	./zmakebas -a 10 -n ed -p -o ed zxedit2.bas

zxedit2.bin: zxedit2.asm
	z88dk-z80asm -b zxedit2.asm

zxedit.bin: zxedit2.bin zxedit.asm
	z88dk-z80asm -b zxedit.asm

zxedit2.bas: zxedit.bin zxedit.bas
	printf '1 REM' > zxedit2.bas
	xxd -ps -u -c 100000 zxedit.bin | sed -e "s/\(..\)/\\\{0x\1\}/g" >> zxedit2.bas
	cat zxedit.bas >> zxedit2.bas

clean:
	rm -f zxedit2.bas ed zxedit.tap ed.com ed.prg zxedit.d64 *.bin *.o *.inc *.map
