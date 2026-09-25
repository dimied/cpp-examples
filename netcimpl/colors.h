#ifndef COLORS
#define COLORS

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_PINK 5
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_TEAL 6
#define COLOR_WHITE 7

#define TEXT_EFFECT_BOLD 0x10 /*!<grey:bold black, orange:bold red*/
#define TEXT_EFFECT_FAINT 0x20
#define TEXT_EFFECT_NORMAL 0x30
#define TEXT_EFFECT_UNDERLINE 0x40
#define TEXT_EFFECT_BLINK 0x50
#define TEXT_EFFECT_NORMAL2 0x60
#define TEXT_EFFECT_NEGATIVE 0x70

void color_text(unsigned char color, char *text)
{
    if (color == 0)
    {
        printf("\x1B[0m%s\033[0m", text);
        return;
    }
    unsigned char effect = (color >> 4);

    if (effect == 0)
    {
        printf("\033[%dm%s\033[0m", (color & 0x7) + 30, text);
    }
    else
    {
        printf("\033[%d;%dm%s\033[0m", (color & 0x7) + 30, effect, text);
    }
}
void test_colors()
{
    char *effects[] = {"Normal", "Bold", "Faint", "Normal", "Underline", "Blink", "Norm2", "Negative"};
    char *colors[] = {"Black", "Red", "Green", "Yellow", "Blue", "Pink", "Cyan", "White", "A2", "A3"};

    int eIdx;
    printf("%10s", "");
    for (eIdx = 0; eIdx < 8; eIdx++)
    {
        printf("%12s", effects[eIdx]);
    }
    printf("\n");
    char example[12] = "  example   ";

    size_t cIdx;
    for (cIdx = 0; cIdx < sizeof(colors) / sizeof(colors[0]); cIdx++)
    {
        printf("%8s  ", colors[cIdx]);
        for (eIdx = 0; eIdx < 8; eIdx++)
        {

            unsigned char c = (eIdx << 4) | (unsigned char)cIdx;
            color_text(c, example);
        }
        printf("\n");
    }
    printf("\n");
}
#define print_colors_va(COLOR, TEXT, ARGS...)    \
    do                                           \
    {                                            \
        char text_buffer[128];                   \
        memset(text_buffer, 0, 128);             \
        sprintf(text_buffer, TEXT, ARGS); \
        color_text(COLOR, text_buffer);          \
    } while (0)

#endif