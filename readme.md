# ZXEdit

ZXEdit is a simple text editor and file management tool for the Commodore 64 and ZX Spectrum with disk systems esxdos, residos or plus3dos

## Tools needed to compile

  - gcc (for building the included archiver tool for archiving files to .tap)
  - z88dk
  - cc65 (my fork from github.com/pjshumphreys/cc65)
  - zmakebas
  - c1541 (from the VICE emulator)

## How to build

```
make
```

## Installation instructions

All files needed are prebuilt in zxedit.zip for convienience

### Commodore 64

zxedit.zip contains zxedit.d64

### ZX Spectrum - esxdos, such as divmmc devices

1. Open esxdos.tap in the fuse ZX Spectrum emulator
2. Type in `LOAD ""`. Don't press anything yet.
3. Open zxedit.tap, then press 'q' to load the files
4. To run the program, type in `LOAD * "ed"`

### ZX Spectrum - residos

1. Open residos.tap in the fuse ZX Spectrum emulator
2. Type in `LOAD ""`. Don't press anything yet.
3. Open zxedit.tap, then press 'q' to load the files
4. To run the program, type in `LOAD % "ed"`
 
### ZX Spectrum +3

1. Open plus3dos.tap in the fuse ZX Spectrum emulator
2. Place a blank disk in drive A:
3. Type in `FORMAT "a:" : LOAD "t:" : LOAD ""`. Don't press anything yet.
4. Open zxedit.tap, then press 'a' to load the files
5. To run the program, type in `LOAD "a:": LOAD "ed"`

### ZX Spectrum +3e

1. Open plus3dos.tap in the fuse ZX Spectrum emulator
2. Type in `MOVE "c:" IN "<your partition name>" : LOAD "t:" : LOAD ""`. Don't press anything yet.
3. Open zxedit.tap, then press 'c' to load the files
4. To run the program, type in `LOAD "c:" : LOAD "ed"`