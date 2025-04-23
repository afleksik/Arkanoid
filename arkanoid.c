#include <mlx.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define WIN_SIZE            800
#define PADDLE_SPEED        1

#define BUTTON_WIDTH        500
#define BUTTON_HEIGHT       100
#define BUTTON_X            ((WIN_SIZE - BUTTON_WIDTH) / 2)
#define BUTTON_Y            ((WIN_SIZE - BUTTON_HEIGHT) / 2)

#define NEW_BUTTON_WIDTH    BUTTON_WIDTH
#define NEW_BUTTON_HEIGHT   50
#define NEW_BUTTON_X        BUTTON_X
#define NEW_BUTTON_Y        (BUTTON_Y + BUTTON_HEIGHT + 20)

#define LEFT_ARROW          65361
#define RIGHT_ARROW         65363

#define COLOR_BG            0x00000000
#define COLOR_BUTTON        0x00FFFFFF
#define COLOR_TEXT          0x00000000
#define COLOR_PADDLE        0x00FFFDD0
#define COLOR_BALL          0x00FF0000
#define COLOR_COUNTER       0x00FFFFFF

typedef struct s_data {
    void *img;
    char *addr;
    int bits_per_pixel;
    int line_length;
    int endian;
} t_data;

typedef struct s_ball {
    double x;
    double y;
    double dx;
    double dy;
    int radius;
} t_ball;

typedef struct s_vars {
    void *mlx;
    void *win;
    t_data img;
    int paddle_x;
    int paddle_y;
    int paddle_width;
    int paddle_height;
    t_ball ball;
    int paddle_left;
    int paddle_right;
    int bounce_count;
    int game_started;
    int game_over;
    time_t game_over_time;
} t_vars;

/* пиксельный 5x7 шрифт: ' ', '!', 'A', 'E', 'G', 'M', 'O', 'R', 'S', 'T', 'V' */
static const char *font5x7[][7] = {
    { "00000", "00000", "00000", "00000", "00000", "00000", "00000" },
    { "00100", "00100", "00100", "00100", "00000", "00100", "00000" },
    { "01110", "10001", "10001", "11111", "10001", "10001", "10001" },
    { "11111", "10000", "11110", "10000", "10000", "10000", "11111" },
    { "01110", "10001", "10000", "10111", "10001", "10001", "01110" },
    { "10001", "11011", "10101", "10101", "10001", "10001", "10001" },
    { "01110", "10001", "10001", "10001", "10001", "10001", "01110" },
    { "11110", "10001", "10001", "11110", "10100", "10010", "10001" },
    { "01111", "10000", "10000", "01110", "00001", "00001", "11110" },
    { "11111", "00100", "00100", "00100", "00100", "00100", "00100" },
    { "10001", "10001", "10001", "10001", "01010", "00100", "00100" },
    { "10001", "11001","10101","10011","10001","10001","10001" },
    { "10001",  "10001",  "10001",  "10001",  "10101",  "11011",  "10001" }
};

int char_index(char c)
{
    if (c == ' ')
        return 0;
    if (c == '!')
        return 1;
    if (c == 'A')
        return 2;
    if (c == 'E')
        return 3;
    if (c == 'G')
        return 4;
    if (c == 'M')
        return 5;
    if (c == 'O')
        return 6;
    if (c == 'R')
        return 7;
    if (c == 'S')
        return 8;
    if (c == 'T')
        return 9;
    if (c == 'V')
        return 10;
    if (c == 'N')
        return 11;
    if (c == 'W')
        return 12;
    return 0;
}

void my_mlx_pixel_put(t_data *d, int x, int y, int color)
{
    if (x >= 0 && x < WIN_SIZE && y >= 0 && y < WIN_SIZE) {
        char *dst = d->addr + (y * d->line_length
                               + x * (d->bits_per_pixel / 8));
        *(unsigned int *) dst = color;
    }
}

void draw_char(t_data *img, char c, int x, int y, int scale, int color)
{
    int idx = char_index(c);

    for (int r = 0; r < 7; r++)
        for (int co = 0; co < 5; co++)
            if (font5x7[idx][r][co] == '1')
                for (int dy = 0; dy < scale; dy++)
                    for (int dx = 0; dx < scale; dx++)
                        my_mlx_pixel_put(img,
                                         x + co * scale + dx,
                                         y + r * scale + dy, color);
}

void draw_text(t_data *img, const char *text,
               int x, int y, int scale, int color)
{
    int len = strlen(text);

    for (int i = 0; i < len; i++)
        draw_char(img, text[i],
                  x + i * (5 * scale + scale), y, scale, color);
}

void draw_button(t_vars *v)
{
    for (int yy = BUTTON_Y; yy < BUTTON_Y + BUTTON_HEIGHT; yy++)
        for (int xx = BUTTON_X; xx < BUTTON_X + BUTTON_WIDTH; xx++)
            my_mlx_pixel_put(&v->img, xx, yy, COLOR_BUTTON);

    const char *t = v->game_over ? "GAME OVER!" : "START";
    int sc = 8;
    int tw = strlen(t) * (5 * sc + sc) - sc;
    int th = 7 * sc;
    int tx = BUTTON_X + (BUTTON_WIDTH - tw) / 2;
    int ty = BUTTON_Y + (BUTTON_HEIGHT - th) / 2;

    draw_text(&v->img, t, tx, ty, sc, COLOR_TEXT);
}

void draw_new_button(t_vars *v)
{
    for (int yy = NEW_BUTTON_Y;
         yy < NEW_BUTTON_Y + NEW_BUTTON_HEIGHT; yy++)
        for (int xx = NEW_BUTTON_X;
             xx < NEW_BUTTON_X + NEW_BUTTON_WIDTH; xx++)
            my_mlx_pixel_put(&v->img, xx, yy, COLOR_BUTTON);

    const char *t = "NEW GAME";
    int sc = 6;
    int tw = strlen(t) * (5 * sc + sc) - sc;
    int th = 7 * sc;
    int tx = NEW_BUTTON_X + (NEW_BUTTON_WIDTH - tw) / 2;
    int ty = NEW_BUTTON_Y + (NEW_BUTTON_HEIGHT - th) / 2;

    draw_text(&v->img, t, tx, ty, sc, COLOR_TEXT);
}

void draw_counter(t_vars *v)
{
    char buf[16];
    sprintf(buf, "%d", v->bounce_count);
    int len = strlen(buf);
    int sc = 6;

    for (int i = 0; i < len; i++)
        draw_char(&v->img, buf[i],
                  10 + i * (3 * sc + sc), 10, sc, COLOR_COUNTER);
}

int mouse_click(int b, int x, int y, t_vars *v)
{
    if (b == 1) {
        if (!v->game_started && !v->game_over
            && x >= BUTTON_X && x <= BUTTON_X + BUTTON_WIDTH
            && y >= BUTTON_Y && y <= BUTTON_Y + BUTTON_HEIGHT) {
            v->game_started = 1;
            v->bounce_count = 0;
            v->ball.x = WIN_SIZE / 2;
            v->ball.y = WIN_SIZE / 2;
            v->ball.dx = 0.3;
            v->ball.dy = 0.3;
        } else if (v->game_over
                   && x >= NEW_BUTTON_X
                   && x <= NEW_BUTTON_X + NEW_BUTTON_WIDTH
                   && y >= NEW_BUTTON_Y
                   && y <= NEW_BUTTON_Y + NEW_BUTTON_HEIGHT) {
            v->game_over = 0;
            v->game_started = 1;
            v->bounce_count = 0;
            v->ball.x = WIN_SIZE / 2;
            v->ball.y = WIN_SIZE / 2;
            v->ball.dx = 0.3;
            v->ball.dy = 0.3;
        }
    }
    return 0;
}

int key_press(int k, t_vars *v)
{
    if (k == LEFT_ARROW)
        v->paddle_left = 1;
    if (k == RIGHT_ARROW)
        v->paddle_right = 1;
    return 0;
}

int key_release(int k, t_vars *v)
{
    if (k == LEFT_ARROW)
        v->paddle_left = 0;
    if (k == RIGHT_ARROW)
        v->paddle_right = 0;
    return 0;
}

int update(void *p)
{
    t_vars *v = (t_vars *) p;
    t_ball *b = &v->ball;

    memset(v->img.addr, 0, WIN_SIZE * v->img.line_length);

    if (v->game_over) {
        if (time(NULL) - v->game_over_time >= 3)
            exit(0);
        draw_button(v);
        draw_new_button(v);
        mlx_put_image_to_window(v->mlx, v->win, v->img.img, 0, 0);
        return 0;
    }

    if (!v->game_started) {
        draw_button(v);
        mlx_put_image_to_window(v->mlx, v->win, v->img.img, 0, 0);
        return 0;
    }

    if (v->paddle_left)
        v->paddle_x = fmax(0, v->paddle_x - PADDLE_SPEED);
    if (v->paddle_right)
        v->paddle_x = fmin(WIN_SIZE - v->paddle_width,
                           v->paddle_x + PADDLE_SPEED);

    b->x += b->dx;
    b->y += b->dy;

    if (b->x - b->radius < 0 || b->x + b->radius > WIN_SIZE)
        b->dx = -b->dx;
    if (b->y - b->radius < 0)
        b->dy = -b->dy;

    if (b->y + b->radius >= v->paddle_y &&
        b->x >= v->paddle_x &&
        b->x <= v->paddle_x + v->paddle_width && b->dy > 0) {
        b->dy = -b->dy;
        v->bounce_count++;
    } else if (b->y - b->radius > v->paddle_y + v->paddle_height) {
        v->game_over = 1;
        v->game_started = 0;
        v->game_over_time = time(NULL);
    }

    for (int yy = v->paddle_y; yy < v->paddle_y + v->paddle_height; yy++)
        for (int xx = v->paddle_x;
             xx < v->paddle_x + v->paddle_width; xx++)
            my_mlx_pixel_put(&v->img, xx, yy, COLOR_PADDLE);

    for (int dy = -b->radius; dy <= b->radius; dy++)
        for (int dx = -b->radius; dx <= b->radius; dx++)
            if (dx * dx + dy * dy <= b->radius * b->radius)
                my_mlx_pixel_put(&v->img,
                                 (int) (b->x + dx),
                                 (int) (b->y + dy), COLOR_BALL);

    mlx_put_image_to_window(v->mlx, v->win, v->img.img, 0, 0);
    draw_counter(v);
    return 0;
}

int main(void)
{
    t_vars v;

    v.mlx = mlx_init();
    if (!v.mlx)
        return 1;

    v.win = mlx_new_window(v.mlx, WIN_SIZE, WIN_SIZE, "Arkanoid");
    if (!v.win)
        return 1;

    v.img.img = mlx_new_image(v.mlx, WIN_SIZE, WIN_SIZE);
    v.img.addr = mlx_get_data_addr(v.img.img,
                                   &v.img.bits_per_pixel,
                                   &v.img.line_length, &v.img.endian);

    v.paddle_width = 250;
    v.paddle_height = 25;
    v.paddle_x = (WIN_SIZE - v.paddle_width) / 2;
    v.paddle_y = 700;

    v.ball.x = WIN_SIZE / 2;
    v.ball.y = WIN_SIZE / 2;
    v.ball.dx = 0.3;
    v.ball.dy = 0.3;
    v.ball.radius = 10;

    v.bounce_count = 0;
    v.game_started = 0;
    v.game_over = 0;
    v.paddle_left = 0;
    v.paddle_right = 0;
    v.game_over_time = 0;

    mlx_hook(v.win, 2, 1L << 0, key_press, &v);
    mlx_hook(v.win, 3, 1L << 1, key_release, &v);
    mlx_hook(v.win, 4, 1L << 2, mouse_click, &v);
    mlx_loop_hook(v.mlx, update, &v);
    mlx_loop(v.mlx);
    return 0;
}
