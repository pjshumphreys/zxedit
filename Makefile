all: zxedit.zip

zxedit.zip: zxedit.tap residos.tap plus3dos.tap esxdos.tap ed.com $$.ED $$.ED.INF zxedit.d64
	zip zxedit.zip zxedit.tap residos.tap plus3dos.tap esxdos.tap ed.com $$.ED $$.ED.INF zxedit.d64

$$.ED.INF: $$.ED
	sh -c "printf '$$.ED      001900 001900 00%X\n' `stat -c "%s" $$.ED` > $$.ED.INF"

$$.ED: zxedit.c
	cl65 -t bbc -lbbc -o $$.ED zxedit.c

zxedit: zxedit.c
	gcc -o zxedit zxedit.c

zxedit.d64: ed.prg
	c1541 -format diskname,id d64 zxedit.d64 -attach zxedit.d64 -write ed.prg ed

ed.com: zxedit.c
	zcc +cpm -DAMALLOC1 -o ed.com zxedit.c

ed.prg: zxedit.c
	cl65 -t c64 -o ed.prg zxedit.c

ed.bbc: zxedit.c
	cl65 -t bbc -lbbc -o ed.bbc zxedit.c

zxedit.tap: archiver ed edp.bin ede.bin edr.bin
	sh -c "(echo zxedit.tap > archived.txt) && (ls ed >> archived.txt) && (ls edp.bin >> archived.txt) && (ls edr.bin >> archived.txt) && (ls ede.bin >> archived.txt) && (echo "" >> archived.txt) && (cat archived.txt | ./archiver)"
	rm archived.txt

edp.bin: zxedit.c
	zcc +zx -DPLUS3DOS -lp3 -O0 -clib=ansi -pragma-define:ansicolumns=32 -pragma-define:ansifont=15616 -pragma-define:ansifont_is_packed=0 -DAMALLOC1 -zorg=32768 zxedit.c
	z88dk-appmake +zx --dos --org 32768 --binfile a.bin --output edp.bin

edr.bin: zxedit.c
	zcc +zx -DRESIDOS -lp3 -O0 -clib=ansi -pragma-define:ansicolumns=32 -pragma-define:ansifont=15616 -pragma-define:ansifont_is_packed=0 -DAMALLOC1 -zorg=32768 zxedit.c
	z88dk-appmake +zx --dos --org 32768 --binfile a.bin --output edr.bin

ede.bin: zxedit.c
	zcc +zx -lesxdos -O0 -clib=ansi -pragma-define:ansicolumns=32 -pragma-define:ansifont=15616 -pragma-define:ansifont_is_packed=0 -DAMALLOC1 -zorg=32768 zxedit.c
	z88dk-appmake +zx --dos --org 32768 --binfile a.bin --output ede.bin

ed: zxedit.bas
	./zmakebas -a 10 -n ed -p -o ed zxedit.bas

zxedit2.bin: zxedit2.asm
	z88dk-z80asm -b zxedit2.asm

zxedit.bin: zxedit2.bin zxedit.asm
	z88dk-z80asm -b zxedit.asm

zxedit.bas: zxedit.bin
	printf '10 CLEAR VAL "32767"\n20 RANDOMIZE USR VAL "\\{0xBE}23635+256*\\{0xBE}23636+51"\n30 REM' > zxedit.bas
	xxd -ps -u -c 100000 zxedit.bin | sed -e "s/\(..\)/\\\{0x\1\}/g" >> zxedit.bas

archiver: archiver.c
	gcc -o archiver archiver.c

residos.tap: unarchiver.c
	zcc +zx -DRESIDOS -lp3 -pragma-define:CRT_ON_EXIT=0x10002 -pragma-redirect:fputc_cons=fputc_cons_rom_rst -DAMALLOC -o residos -create-app unarchiver.c

plus3dos.tap: unarchiver.c
	zcc +zx -O0 -lp3 -pragma-define:CRT_ON_EXIT=0x10002 -pragma-redirect:fputc_cons=fputc_cons_rom_rst -DAMALLOC -o plus3dos -create-app unarchiver.c

esxdos.tap: unarchiver.c
	zcc +zx -lesxdos -pragma-define:CRT_ON_EXIT=0x10002 -pragma-redirect:fputc_cons=fputc_cons_rom_rst -DAMALLOC -o esxdos -create-app unarchiver.c

clean:
	rm -f zxedit.zip zxedit.bas ed ed.com ed.prg zxedit.d64 archiver esxdos plus3dos residos $$.ED.INF $$.ED *.tap *.bin *.o *.inc *.map
