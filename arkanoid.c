#include <mlx.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define LEFT_ARROW 65361
#define RIGHT_ARROW 65363

typedef struct	s_data {
    void	*img;
    char	*addr;
    int		bits_per_pixel;
    int		line_length;
    int		endian;
}				t_data;

typedef struct	s_ball {
    double x;
    double y;
    double dx;
    double dy;
    int radius;
}				t_ball;

typedef struct	s_vars {
    void	*mlx;
    void	*win;
    t_data	img;
    int		paddle_x;
    int		paddle_y;
    int		paddle_width;
    int		paddle_height;
    t_ball	ball;
    int     paddle_left;
    int     paddle_right;
    int     bounce_count;
    int     game_started;
}				t_vars;

int key_press(int keycode, t_vars *vars)
{
    if (keycode == LEFT_ARROW)
        vars->paddle_left = 1;
    else if (keycode == RIGHT_ARROW)
        vars->paddle_right = 1;
    return (0);
}

int key_release(int keycode, t_vars *vars)
{
    if (keycode == LEFT_ARROW)
        vars->paddle_left = 0;
    else if (keycode == RIGHT_ARROW)
        vars->paddle_right = 0;
    return (0);
}

void my_mlx_pixel_put(t_data *data, int x, int y, int color)
{
    if (x >= 0 && x < 800 && y >= 0 && y < 800)
    {
        char *dst = data->addr + (y * data->line_length + x * (data->bits_per_pixel / 8));
        *(unsigned int*)dst = color;
    }
}

void draw_button(t_vars *vars)
{
    int button_width = 100;
    int button_height = 50;
    int button_x = 350;
    int button_y = 375;
    for (int y = button_y; y < button_y + button_height; ++y)
        for (int x = button_x; x < button_x + button_width; ++x)
            my_mlx_pixel_put(&vars->img, x, y, 0x00FFFFFF);
    mlx_string_put(vars->mlx, vars->win, button_x + 10, button_y + 25, 0x66666666, "START");
}

int mouse_click(int button, int x, int y, t_vars *vars)
{
    if (button == 1 && x >= 350 && x <= 450 && y >= 375 && y <= 425 && !vars->game_started)
    {
        vars->game_started = 1;
    }
    return (0);
}

int update(void *param)
{
    t_vars *vars = (t_vars *)param;
    t_ball *ball = &vars->ball;

    if (!vars->game_started)
    {
        draw_button(vars);
        mlx_put_image_to_window(vars->mlx, vars->win, vars->img.img, 0, 0);
        return (0);
    }

    // Перемещение ракетки
    if (vars->paddle_left)
    {
        vars->paddle_x -= 1;
        if (vars->paddle_x < 0)
            vars->paddle_x = 0;
    }
    if (vars->paddle_right)
    {
        vars->paddle_x += 1;
        if (vars->paddle_x > 800 - vars->paddle_width)
            vars->paddle_x = 800 - vars->paddle_width;
    }

    ball->x += ball->dx;
    ball->y += ball->dy;

    // Столкновение со стенками
    if (ball->x - ball->radius < 0 || ball->x + ball->radius > 800)
        ball->dx = -ball->dx;
    if (ball->y - ball->radius < 0)
        ball->dy = -ball->dy;

    // Столкновение с ракеткой
    if (ball->y + ball->radius >= vars->paddle_y &&
        ball->x >= vars->paddle_x &&
        ball->x <= vars->paddle_x + vars->paddle_width &&
        ball->dy > 0)
    {
        ball->dy = -ball->dy;
        vars->bounce_count++;
    }
    else if (ball->y + ball->radius > 800)
    {
        mlx_destroy_window(vars->mlx, vars->win);
        exit(0);
    }

    // Очистка изображения
    memset(vars->img.addr, 0, 800 * vars->img.line_length);

    // Отрисовка ракетки
    for (int y = vars->paddle_y; y < vars->paddle_y + vars->paddle_height; ++y)
        for (int x = vars->paddle_x; x < vars->paddle_x + vars->paddle_width; ++x)
            my_mlx_pixel_put(&vars->img, x, y, 0x00FFFDD0);

    // Отрисовка мяча
    for (int dy = -ball->radius; dy <= ball->radius; ++dy)
        for (int dx = -ball->radius; dx <= ball->radius; ++dx)
            if (dx * dx + dy * dy <= ball->radius * ball->radius)
                my_mlx_pixel_put(&vars->img, (int)(ball->x + dx), (int)(ball->y + dy), 0x00FF0000);

    mlx_put_image_to_window(vars->mlx, vars->win, vars->img.img, 0, 0);

    // Отображение счетчика отбиваний
    char count_str[50];
    sprintf(count_str, "%d", vars->bounce_count);
    mlx_string_put(vars->mlx, vars->win, 10, 10, 0xFFFFFF, count_str);

    return (0);
}

int	main(void)
{
    t_vars	var;

    var.mlx = mlx_init();
    if (!var.mlx)
        return (1);

    var.win = mlx_new_window(var.mlx, 800, 800, "Arkanoid");
    if (!var.win)
        return (1);

    var.img.img = mlx_new_image(var.mlx, 800, 800);
    if (!var.img.img)
        return (1);

    var.img.addr = mlx_get_data_addr(var.img.img,
                                    &var.img.bits_per_pixel,
                                    &var.img.line_length,
                                    &var.img.endian);
    if (!var.img.addr)
        return (1);

    var.paddle_x = 0;
    var.paddle_y = 700;
    var.paddle_left = 0;
    var.paddle_right = 0;
    var.paddle_width = 250;
    var.paddle_height = 25;
    var.ball.x = 300;
    var.ball.y = 100;
    var.ball.dx = 0.3;
    var.ball.dy = 0.3;
    var.ball.radius = 10;
    var.bounce_count = 0;
    var.game_started = 0;

    mlx_hook(var.win, 2, 1L<<0, key_press, &var);
    mlx_hook(var.win, 3, 1L<<1, key_release, &var);
    mlx_hook(var.win, 4, 1L<<2, mouse_click, &var);
    mlx_loop_hook(var.mlx, update, &var);
    mlx_loop(var.mlx);

    return (0);
}