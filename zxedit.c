#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

#ifdef __CC65__
  #include <unistd.h>
  #define getch cgetc
#endif

int d_fgets(char** ws, FILE* stream) {
  char buf[80];
  char* newWs = NULL;
  char* potentialNewWs = NULL;
  size_t totalLength = (size_t)0;
  size_t potentialTotalLength = (size_t)0;
  size_t bufferLength;

  /* check sanity of inputs */
  if(ws == NULL) {
    return 0;
  }

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

        /* ensure null termination of the string */
        newWs[totalLength] = '\0';

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

    /* if the last character is '\n' (ie we've reached the end of a line) then return the result */
    if(newWs[totalLength-1] == '\x1a') {
      /* ensure null termination of the string */
      newWs[totalLength-1] = '\0';

      /* set the output string pointer and return true */
      if(*ws) {
        free(*ws);
      }

      *ws = newWs;

      return 2;
    }
    else if(newWs[totalLength-1] == '\n' || newWs[totalLength-1] == '\r') {
      /* ensure null termination of the string */
      newWs[totalLength-1] = '\0';

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
    /* ensure null termination of the string */
    newWs[totalLength] = '\0';

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

#ifdef __CC65__
void changeDrive(char* filename) {
  char * temp;
  char temp2 = 0;

  /* if a comma then a valid drive number appears after the file name then change to that drive */
  if((temp = strchr(filename, ',')) != NULL) {
    *temp = '\0';
    temp++;

    if(isdigit(temp[0])) {
      temp2 = temp[0] - '0';

      if(temp[1]) {
        if(isdigit(temp[1]) && !temp[2]) {
          temp2 = (((temp2 << 2) + temp2) << 1) + (temp[1] - '0');
        }
        else {
          temp2 = 0;
        }
      }
    }
  }

  /* use drive 8 by default */
  if(temp2 < 9 || temp2 > 30) {
    temp = "8";
  }

  chdir(temp);
}
#endif


void main(void) {
  char * textLine = NULL;
  FILE * readFrom;
  int c;

  fputs(
    "(w)rite,\n"
    "(r)ead or \n"
    "(d)elete file?\n",
    stdout
  );

  do {
    switch(getch()) {
      case 'w':
      case 'W': {
        fputs("filename to write to?\n", stdout);

        d_fgets(&textLine, stdin);

        #ifdef __CC65__
          changeDrive(textLine);
        #endif

        if((readFrom = fopen(textLine, "wb")) == NULL) {
          fputs("could not open file\n", stdout);
          free(textLine);
          return;
        }

        fputs(
          "\n"
          "enter lines now.\n"
          "the first character is ignored.\n"
          "enter empty string to finish.\n",
          stdout
        );

        do {
          d_fgets(&textLine, stdin);

          if(strcmp("", textLine) == 0) {
            fprintf(readFrom, "\x1a");

            free(textLine);

            fclose(readFrom);

            return;
          }

          fprintf(readFrom, "%s\n", textLine + 1);

        } while(1);
      } break;

      case 'r':
      case 'R': {
        fputs("filename to read from?\n", stdout);

        d_fgets(&textLine, stdin);

        #ifdef __CC65__
          changeDrive(textLine);
        #endif

        if((readFrom = fopen(textLine, "rb")) == NULL) {
          fputs("could not open file\n", stdout);
          free(textLine);
          return;
        }

        do {
          c = d_fgets(&textLine, readFrom);

          if(c != 0) {
            printf("%s\n", textLine);
          }

          if(c != 1) {
            free(textLine);
            fclose(readFrom);
            return;
          }
        } while(1);
      } break;

      case 'd':
      case 'D': {
        fputs("filename to delete?\n", stdout);

        d_fgets(&textLine, stdin);

        #ifdef __CC65__
          changeDrive(textLine);
        #endif

        remove(textLine);
        free(textLine);
      } return;

      default:
        continue;
    }
  } while(1);
}
