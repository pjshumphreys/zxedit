all: zxedit.tap zxedit.com zxedit.prg

zxedit.com: zxedit.c
	zcc +cpm -DAMALLOC1 -o zxedit.com zxedit.c

zxedit.prg: zxedit.c
	cl65 -t c64 -o zxedit.prg zxedit.c

zxedit.tap: zxedit plus3.bin esxdos.bin residos.bin
	sh -c "(echo zxedit.tap > archived.txt) && (ls zxedit >> archived.txt) && (ls plus3.bin >> archived.txt) && (ls residos.bin >> archived.txt) && (ls esxdos.bin >> archived.txt) && (echo "" >> archived.txt) && (cat archived.txt | ./archiver)"
	rm archived.txt

plus3.bin: zxedit.c
	zcc +zx -lp3 -DAMALLOC1 -clib=ansi -pragma-define:ansicolumns=32 -pragma-define:ansifont=15616 -pragma-define:ansifont_is_packed=0 -zorg=49152 zxedit.c
	z88dk-appmake +zx --dos --org 49152 --binfile a.bin --output plus3.bin

residos.bin: zxedit.c
	zcc +zx -DRESIDOS -lp3 -clib=ansi -pragma-define:ansicolumns=32 -pragma-define:ansifont=15616 -pragma-define:ansifont_is_packed=0 -DAMALLOC1 -zorg=49152 zxedit.c
	z88dk-appmake +zx --dos --org 49152 --binfile a.bin --output residos.bin

esxdos.bin: zxedit.c
	zcc +zx -lesxdos -clib=ansi -pragma-define:ansicolumns=32 -pragma-define:ansifont=15616 -pragma-define:ansifont_is_packed=0 -DAMALLOC1 -zorg=49152 zxedit.c
	z88dk-appmake +zx --dos --org 49152 --binfile a.bin --output esxdos.bin

zxedit: zxedit2.bas
	./zmakebas -a 10 -n zxedit -p -o zxedit zxedit2.bas

zxedit2.bin: zxedit2.asm
	z88dk-z80asm -b zxedit2.asm

zxedit.bin: zxedit2.bin zxedit.asm
	z88dk-z80asm -b zxedit.asm

zxedit2.bas: zxedit.bin zxedit.bas
	printf '1 REM' > zxedit2.bas
	xxd -ps -u -c 100000 zxedit.bin | sed -e "s/\(..\)/\\\{0x\1\}/g" >> zxedit2.bas
	cat zxedit.bas >> zxedit2.bas 

clean:
	rm -f zxedit2.bas zxedit.tap zxedit.com zxedit.prg zxedit *.bin *.o *.inc *.map
