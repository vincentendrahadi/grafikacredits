#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

typedef struct Characters {
    char content;
    short red, green, blue;
    short i,j;
} Character;

// global variable
char *fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
short alphabets[26][28][20];
short numbers[10][28][20];
short symbols[3][28][20];
Character credit[255];

void loadCreditContent(int i_start, int j_start) {
    FILE *fp;
    char x;
    int c = -1;
    int i = i_start,j= j_start;
    int row = 0;
    int red = 0, green = 240, blue = 200;
    fp = fopen("credit_content.txt", "r");
    while(fscanf(fp, "%c", &x) != EOF) {
        if (x != '\n') {
            ++c;
            credit[c].content = x;
            credit[c].red = red;
            credit[c].green = green;
            credit[c].blue = blue;
            credit[c].i = i;
            credit[c].j = j;
            j += 22;
        } else {
            j = j_start;
            i += 112;
            ++row;
            if (row == 1) {
                red = 255;
                green = 255;
                blue = 255;
            } else if (row == 2) {
                red = 160;
                green = 200;
                blue = 220;
            } else if (row == 3) {
                red = 240;
                green = 120;
                blue = 0;
            } else if (row == 4) {
                red = 250;
                green = 0;
                blue = 200;
            } else if (row == 5) {
                red = 0;
                green = 255;
                blue = 0;
            } else if (row == 6) {
                red = 160;
                green = 0;
                blue = 0;
            } else if (row == 7) {
                red = 30;
                green = 60;
                blue = 240;
            } else {
                red = 180;
                green = 160;
                blue = 255;
            }
        }
    }
}

void loadCharacters() {
    FILE *fp;

    for (char c = 'A'; c <= 'Z'; c++) {
        char x;
        int i = 0, j = 0;
        char filename[9];
        filename[0] = 'a';
        filename[1] = 's';
        filename[2] = 's';
        filename[3] = 'e';
        filename[4] = 't';
        filename[5] = 's';
        filename[6] = '/';
        filename[7] = c;
        filename[8] = '\0';
        fp = fopen(filename, "r");
        while (fscanf(fp, "%c", &x) != EOF) {
            if (x == 'X') {
                alphabets[c - 'A'][i][j] = 1;
            } else if (x == '\n') {
                i++;
                j = -1;
            } else {
                alphabets[c - 'A'][i][j] = 0;
            }
            j++;
        }
        fclose(fp);
    }

    for (char c = '0'; c <= '9'; c++) {
        char x;
        int i = 0, j = 0;
        char filename[9];
        filename[0] = 'a';
        filename[1] = 's';
        filename[2] = 's';
        filename[3] = 'e';
        filename[4] = 't';
        filename[5] = 's';
        filename[6] = '/';
        filename[7] = c;
        filename[8] = '\0';
        fp = fopen(filename, "r");
        while (fscanf(fp, "%c", &x) != EOF) {
            if (x == 'X') {
                numbers[c - '0'][i][j] = 1;
            } else if (x == '\n') {
                i++;
                j = -1;
            } else {
                numbers[c - '0'][i][j] = 0;
            }
            j++;
        }
        fclose(fp);
    }

    char x;
    int i = 0, j = 0;
    char filename[9];
    filename[0] = 'a';
    filename[1] = 's';
    filename[2] = 's';
    filename[3] = 'e';
    filename[4] = 't';
    filename[5] = 's';
    filename[6] = '/';
    filename[7] = ':';
    filename[8] = '\0';
    fp = fopen(filename, "r");
    while (fscanf(fp, "%c", &x) != EOF) {
        if (x == 'X') {
            symbols[0][i][j] = 1;
        } else if (x == '\n') {
            i++;
            j = -1;
        } else {
            symbols[0][i][j] = 0;
        }
        j++;
    }
    fclose(fp);

    i = 0;
    j = 0;
    filename[7] = '-';
    fp = fopen(filename, "r");
    while (fscanf(fp, "%c", &x) != EOF) {
        if (x == 'X') {
            symbols[1][i][j] = 1;
        } else if (x == '\n') {
            i++;
            j = -1;
        } else {
            symbols[1][i][j] = 0;
        }
        j++;
    }
    fclose(fp);
}

void printPixel(int i, int j, int opacity, int blue, int green, int red) {
    int location;
    if (j < 0 || j >= 1366 || i < 0 || i >= 760) {
        // out of bound, no need to print
    } else {
        location = (j+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                   (i+vinfo.yoffset) * finfo.line_length;
        *(fbp + location) = blue;        // Some blue
        *(fbp + location + 1) = green;     // A little green
        *(fbp + location + 2) = red;    // A lot of red
        *(fbp + location + 3) = opacity;      // No transparency
    }
}

void printCharacter(int i_start, int j_start, int opacity, int blue, int green, int red, char content) {
    int i, j;
    for (i = 0; i < 28; i++) {
        for (j = 0; j < 20; j++) {
            if (content >= 'A' && content <= 'Z') {
                if (alphabets[content - 'A'][i][j] == 1) {
                    printPixel(i_start + i, j_start + j, 0, blue, green, red);
                } else {
                    printPixel(i_start + i, j_start + j, 0, 0, 0, 0);
                }
            } else if (content >= '0' && content <= '9') {
                if (numbers[content - '0'][i][j] == 1) {
                    printPixel(i_start + i, j_start + j, 0, blue, green, red);
                } else {
                    printPixel(i_start + i, j_start + j, 0, 0, 0, 0);
                }
            } else if (content == ' ') {
                printPixel(i_start + i, j_start + j, 0, 0, 0, 0);
            } else if (content == ':') {
                if (symbols[0][i][j] == 1) {
                    printPixel(i_start + i, j_start + j, 0, blue, green, red);
                } else {
                    printPixel(i_start + i, j_start + j, 0, 0, 0, 0);
                }
            } else if (content == '-') {
                if (symbols[1][i][j] == 1) {
                    printPixel(i_start + i, j_start + j, 0, blue, green, red);
                } else {
                    printPixel(i_start + i, j_start + j, 0, 0, 0, 0);
                }
            }
         }
    }
}

int main()
{
    int fbfd = 0;
    // struct fb_var_screeninfo vinfo;
    // struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    int x = 0, y = 0;
    int i_start, j_start;
    long int location = 0;

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (*fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    i_start = 800;
    j_start = 400;
    // load credit content from txt
    loadCreditContent(i_start,j_start);

    // load characters format from txt
    loadCharacters();

    for(long int a = 0 ; a < 999999999; a++ ) {
      for (y = 0; y < 760; y++) {
          for (x = 0; x < 1366; x++) {
              printPixel(y, x, 0, 0, 0, 0);
          }
      }

      for(int i = 0; i < 232; i++) {
          printCharacter(credit[i].i-a, credit[i].j, 0, credit[i].blue, credit[i].green, credit[i].red, credit[i].content);
      }
      usleep(1500);
    }

    munmap(fbp, screensize);
    close(fbfd);
    while (1) {
        char x;
        scanf("%c", &x);
    }
    return 0;
}
