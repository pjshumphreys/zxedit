#include <stdio.h>
#include <stdlib.h>   /* malloc, free */
#include <string.h>   /* strlen, memmove */

#define freeAndZero(p) { free(p); p = 0; }

#ifdef __CC65__
  #include <dirent.h>   /* opendir, readdir, closedir */
  #include <conio.h>    /* cgetc */
  #include <unistd.h>   /* chdir */

  char* __fastcall__ myfgets (char* s, unsigned size, register FILE* f)
  {
      register char* p = s;
      unsigned i;
      int c;

      if (size == 0) {
          /* Invalid size */
          return (char*) _seterrno (EINVAL);
      }

      /* Read input */
      i = 0;
      while (--size) {

          /* Get next character */
          if ((c = fgetc (f)) == EOF) {
              /* Error or EOF */
              if (i == 0) {
                  /* EOF on first char */
                  *p = '\0';
                  return 0;
              } else {
                  /* EOF with data already read */
                  break;
              }
          }

          /* One char more */
          *p = c;
          ++p;
          ++i;

          /* Stop at end of line */
          if ((char)c == '\r' || (char)c == '\n') {
              break;
          }
      }

      /* Terminate the string */
      *p = '\0';

      /* Done */
      return s;
  }

  #define fgets myfgets

  /* cc65's fgets does whitespace trimming on the c64 version for stdin, so isn't really usable for us. so we use cgetc in a loop and build a line buffer ourselves */
  int d_fgets2(char** ws) {
    unsigned char buf[81];
    int c;
    int currentIndex = 0;
    int maxIndex = 0;

    memset(buf, 0, 81);

    do {
      switch((c = cgetc())) {
        case 145: /* up */
        case 17: {  /* down */
          /* do nothing */
        } break;

        case 157: { /* left */
          if(currentIndex) {
            --currentIndex;

            printf("%c", 157);
          }
        } break;

        case 20: { /* backspace */
          if(currentIndex) {
            if(currentIndex != maxIndex) {
              memmove(&(buf[currentIndex-1]), &(buf[currentIndex]), maxIndex-currentIndex+1);

              --maxIndex;
              --currentIndex;
            }
            else {
              --maxIndex;
              --currentIndex;

              buf[currentIndex] = 0;
            }

            printf("%c", 20);
          }
        } break;

        case 29: {  /* right */
          if(currentIndex < maxIndex) {
            ++currentIndex;

            printf("%c", 29);
          }
        } break;

        case 13: { /* enter */
          if(*ws) {
            free(*ws);
          }

          *ws = strdup(buf);

          printf("%c", 13);
          return 1;
        } break;

        default: {
          /*
            remap uppercase scancodes to upper case petscii
            A-Z = 193-218 should be 97...
          */
          if(c> 192 && c < 219) {
            c -= 96;
          }

          if(c > 31 && c < 127) {
            if(currentIndex != maxIndex) {
              if(maxIndex < 80) {
                memmove(&(buf[currentIndex+1]), &(buf[currentIndex]), maxIndex-currentIndex);
                buf[currentIndex] = c;
                printf("%c%c", 148, c);

                ++maxIndex;
                ++currentIndex;
              }
            }
            else if(currentIndex < 80) {
              buf[currentIndex] = c;
              printf("%c", c);

              ++currentIndex;
              ++maxIndex;
            }
          }
          /*
          else {
            printf("%d", c);
          }
          */
        } break;
      }

    } while(1);
  }
#endif

int d_fgets(char** ws, FILE* stream) {
  char buf[80];
  char* newWs = NULL;
  char* potentialNewWs = NULL;
  size_t totalLength = (size_t)0;
  size_t potentialTotalLength = (size_t)0;
  size_t bufferLength;
  int c;

  /* check sanity of inputs */
  if(ws == NULL) {
    return 0;
  }

  #ifdef __CC65__
    if(stream == stdin) {
      return d_fgets2(ws);
    }
  #endif

  /* try reading some text from the file into the buffer */
  while(fgets((char *)&(buf[0]), 80, stream) != NULL) {

    /* get the length of the string in the buffer */
    bufferLength = strlen((char *)&(buf[0]));

    /*
      add it to the potential new length.
      this might not become the actual new length if the realloc fails
    */
    potentialTotalLength+=bufferLength;

    /*
      try reallocating the output string to be a bit longer
      if it fails then set any existing buffer to the return value and return true
    */
    if((potentialNewWs = (char*)realloc(newWs, ((potentialTotalLength+1)*sizeof(char)))) == NULL) {

      /* if we've already retrieved some text */
      if(newWs != NULL) {
        /* set the output string pointer and return true */
        if(*ws) {
          free(*ws);
        }
        *ws = newWs;

        return 2;
      }

      /*
        otherwise no successful allocation was made.
        return false without modifying the output string location
      */
      return 0;
    }

    /* copy the buffer data into potentialNewWs */
    memcpy(potentialNewWs+totalLength, &buf, bufferLength*sizeof(char));

    /* the potential new string becomes the actual one */
    totalLength = potentialTotalLength;
    newWs = potentialNewWs;

    /* if soft eof was found then truncate the returned match */
    if((potentialNewWs = strchr(newWs, '\x1a')) != NULL) {
      /* ensure null termination of the string */
      potentialNewWs[0] = '\0';

      /* set the output string pointer and return true */
      if(*ws) {
        free(*ws);
      }

      *ws = newWs;

      return 2;
    }

    /* ensure null termination of the string */
    newWs[totalLength] = '\0';

    if(newWs[totalLength-1] == '\r') {
      if((c = fgetc(stream)) != '\n') {
        ungetc(c, stream);
      }

      /* ensure null termination of the string */
      newWs[totalLength-1] = '\0';

      if(newWs[totalLength-2] == '\n') {
        newWs[totalLength-2] = '\0';
      }

      /* set the output string pointer and return true */
      if(*ws) {
        free(*ws);
      }

      *ws = newWs;

      return 1;
    }

    /* if the last character is '\n' (ie we've reached the end of a line) then return the result */
    if(newWs[totalLength-1] == '\n') {
      if((c = fgetc(stream)) != '\r') {
        ungetc(c, stream);
      }

      /* ensure null termination of the string */
      newWs[totalLength-1] = '\0';

      if(newWs[totalLength-2] == '\r') {
        newWs[totalLength-2] = '\0';
      }

      /* set the output string pointer and return true */
      if(*ws) {
        free(*ws);
      }

      *ws = newWs;

      return 1;
    }
  }

  /* if we've already retrieved some text */
  if(newWs != NULL) {
    /* set the output string point and return true */
    if(*ws) {
      free(*ws);
    }
    *ws = newWs;

    return 2; /* last string in the file */
  }

  /*
    otherwise no successful allocation was made.
    return false without modifying the output string location
  */
  return 0;
}

int mygetch(void) {
  int c;
  char* temp = NULL;

  d_fgets(&temp, stdin);
  c = temp[0];
  freeAndZero(temp);

  return c;
}

FILE * readFrom;
FILE * writeTo;
char * newLine = NULL;
char * oldLine = NULL;
int firstLine;
int c;

void insertLines(void) {
  fprintf(stdout,
    "Enter lines now.\n"
    "First character is ignored.\n"
    "Enter empty string to finish.\n"
  );

  do {
    d_fgets(&newLine, stdin);

    if(strcmp("", newLine) == 0) {
      freeAndZero(newLine);
      return;
    }

    if(firstLine == 1) {
      fprintf(writeTo, "%s", newLine + 1);
      firstLine = 0;
    }
    else {
      fprintf(writeTo, "\n%s", newLine + 1);
    }
  } while(1);
}

void printHelp(int mode) {
  switch(mode) {
    case 0: {
      fprintf(stdout,
        "commands:\n"
        "(r)ead file\n"
        "(w)rite file\n"
        "(a)mend file\n"
        "(s)ingle line read file\n"
        "(d)elete file\n"

        #ifdef __CC65__
          "re(n)name file\n"
          "(c)hange directory\n"
          "(l)ist files\n"
        #endif

        "(h)elp\n"
        "(q)uit\n"
      );
    } break;

    case 1: {
      fprintf(stdout,
        "commands:\n"
        "(n)ext line (default)\n"
        "(s)kip line\n"
        "(i)nsert lines\n"
        "(a)ppend rest of old file\n"
        "(h)elp\n"
        "(q)uit\n"
      );
    } break;

    case 2: {
      fprintf(stdout,
        "commands:\n"
        "(i)nsert lines\n"
        "(h)elp\n"
        "(q)uit\n"
      );
    } break;
  }
}

int edit(void) {
  do {
    fprintf(stdout, "? ");

    switch(mygetch()) {
      case 'q':
      case 'Q': {
        return 0;
      } break;

      case 'a':
      case 'A': {
        {
          fprintf(stdout, "old filename?\n");
          d_fgets(&oldLine, stdin);

          if(strcmp("", oldLine) == 0) {
            freeAndZero(oldLine);
            return 1;
          }

          fprintf(stdout, "new filename?\n");
          d_fgets(&newLine, stdin);

          if(strcmp("", newLine) == 0) {
            freeAndZero(newLine);
            return 1;
          }

          if((readFrom = fopen(oldLine, "rb")) == NULL) {
            fprintf(stdout, "could not open file\n");
            freeAndZero(oldLine);
            return 1;
          }

          if((writeTo = fopen(newLine, "w")) == NULL) {
            fprintf(stdout, "could not open file\n");
            freeAndZero(newLine);
            return 1;
          }

          freeAndZero(oldLine);
          freeAndZero(newLine);

          printHelp(1);

          firstLine = 1;
          c = d_fgets(&oldLine, readFrom);
        }

        do {
          if(c == 0) {
            fprintf(stdout, "no more lines\n");
            printHelp(2);

            c = 1;

            do {
              fprintf(stdout, "? ");
              d_fgets(&newLine, stdin);

              switch(newLine[0]) {
                case 'i':
                case 'I':
                  insertLines();
                case 'q':
                case 'Q':
                  c = 0;
                break;

                default:
                  printHelp(2);
                break;
              }
            } while(c);

            #ifdef __Z88DK
              fprintf(writeTo, "\x1a");
            #endif

            #ifdef __CC65__
              fprintf(writeTo, "\n");
            #endif

            fclose(readFrom);
            fclose(writeTo);

            freeAndZero(newLine);
            freeAndZero(oldLine);

            printHelp(0);

            return 1;
          }
          else {
            fprintf(stdout, "%s\n", oldLine);
            fprintf(stdout, "? ");
          }

          d_fgets(&newLine, stdin);

          switch(newLine[0]) {
            case 'h':
            case 'H': {
              printHelp(1);
            } break;

            case 's':
            case 'S': {
              c = (c == 2) ? 0 : d_fgets(&oldLine, readFrom);
            } break;

            case 'i':
            case 'I': {
              insertLines();
            } continue;

            case 'a':
            case 'A': {
              do {
                switch(c) {
                  case 1: {
                    fprintf(writeTo, firstLine == 1 ? "%s" : "\n%s", oldLine);
                    firstLine = 0;

                    if((c = d_fgets(&oldLine, readFrom))) {
                      fprintf(stdout, "%s\n", oldLine);
                    }
                  } break;

                  case 2: {
                    fprintf(writeTo, firstLine == 1 ? "%s" : "\n%s", oldLine);
                    c = 0;
                  } break;
                }
              } while(c);
            } break;

            case 'q':
            case 'Q': {
              #ifdef __Z88DK
                fprintf(writeTo, "\x1a");
              #endif

              #ifdef __CC65__
                fprintf(writeTo, "\n");
              #endif

              fclose(readFrom);
              fclose(writeTo);

              freeAndZero(newLine);
              freeAndZero(oldLine);

              printHelp(0);

            } return 1;

            /*
              case 'n':
              case 'N':
            */
            default: {
              fprintf(writeTo, firstLine == 1 ? "%s" : "\n%s", oldLine);
              firstLine = 0;
              c = (c == 2) ? 0 : d_fgets(&oldLine, readFrom);
            } break;
          }
        } while(1);
      } break;

      case 'w':
      case 'W': {
        fprintf(stdout, "filename to write to?\n");

        d_fgets(&newLine, stdin);

        if(strcmp("", newLine) == 0) {
          freeAndZero(newLine);
          return 1;
        }

        if((writeTo = fopen(newLine, "w")) == NULL) {
          fprintf(stdout, "could not open file\n");
          freeAndZero(newLine);
          return 1;
        }

        firstLine = 1;
        insertLines();

        #ifdef __Z88DK
          fprintf(writeTo, "\x1a");
        #endif

        #ifdef __CC65__
          fprintf(writeTo, "\n");
        #endif

        fclose(writeTo);
      } return 1;

      case 'r':
      case 'R': {
        fprintf(stdout, "filename to read from?\n");

        d_fgets(&newLine, stdin);

        if((readFrom = fopen(newLine, "rb")) == NULL) {
          fprintf(stdout, "could not open file\n");
          freeAndZero(newLine);
          return 1;
        }

        do {
          c = d_fgets(&newLine, readFrom);

          switch(c) {
            case 1:
              fprintf(stdout, "%s\n", newLine);
            break;

            case 2:
              fprintf(stdout, "%s\n", newLine);

            default:
              freeAndZero(newLine);
              fclose(readFrom);
              return 1;
          }
        } while(1);
      } break;

      case 's':
      case 'S': {
        fprintf(stdout, "filename to read from?\n");

        d_fgets(&newLine, stdin);

        if((readFrom = fopen(newLine, "rb")) == NULL) {
          fprintf(stdout, "could not open file\n");
          freeAndZero(newLine);
          return 1;
        }

        fprintf(stdout,
          "enter empty string to show\n"
          "next line or anything else\n"
          "to quit\n");

        do {
          c = d_fgets(&oldLine, readFrom);

          switch(c) {
            case 1:
              fprintf(stdout, "%s", oldLine);
            break;

            case 2:
              fprintf(stdout, "%s", oldLine);

            default:
              freeAndZero(newLine);
              freeAndZero(oldLine);
              fclose(readFrom);
              return 1;
          }

          d_fgets(&newLine, stdin);

          if(strcmp("", newLine) != 0) {
            fprintf(stdout, "\n");

            freeAndZero(newLine);
            freeAndZero(oldLine);
            fclose(readFrom);
            return 1;
          }
        } while(1);
      } break;

      #ifdef __CC65__
        case 'c':
        case 'C': {
          fprintf(stdout, "drive to change to?\n");
          d_fgets(&newLine, stdin);

          chdir(newLine);

          freeAndZero(newLine);
        } break;

        case 'n':
        case 'N': {
          fprintf(stdout, "old filename?\n");
          d_fgets(&oldLine, stdin);

          fprintf(stdout, "new filename?\n");
          d_fgets(&newLine, stdin);

          rename(oldLine, newLine);
          freeAndZero(newLine);
          freeAndZero(oldLine);
        } return 1;

        case 'l':
        case 'L': {
          DIR *d;
          struct dirent *dir;

          if((d = opendir("."))) {
            while ((dir = readdir(d)) != NULL) {
              if(dir->d_name[0] == '.') {
                if(
                  strcmp(".", dir->d_name) == 0 ||
                  strcmp("..", dir->d_name) == 0
                ) {
                  continue;
                }
              }

              fprintf(stdout, "%s\n", dir->d_name);
            }

            closedir(d);
          }
        } break;
      #endif

      case 'd':
      case 'D': {
        fprintf(stdout, "filename to delete?\n");
        d_fgets(&newLine, stdin);

        if(strcmp("", newLine) == 0) {
          fprintf(stdout, "no filename supplied\n");
          freeAndZero(newLine);
          return 1;
        }

        #if (defined(PLUS3DOS) || defined(RESIDOS))
          c = strlen(newLine);
          newLine[c] = -1;  // the filespec for residos and plus3dos must be terminated with 0xff
        #endif

        remove(newLine);

        freeAndZero(newLine);
      } return 1;

      default: {
        printHelp(0);
      } break;
    }
  } while(1);
}

void main(void) {
  #ifdef __CC65__
    cursor(1);
  #endif

  printHelp(0);

  #ifdef __CC65__
    kbrepeat(KBREPEAT_NONE);
  #endif

  while(edit()) {};
}
