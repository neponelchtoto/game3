/**
 * rabbit.c -- прототип игры управления персонажем
 * 
 * Copyright (c) 2022, Ангелина Кондратьева <akondrat@cs.petrsu.ru>
 *
 * This code is licensed under a MIT-style license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#define VXD 5
#define VYD 5
#define MAX_RAINS 5
#define MAX_CARROTS 5

/* Описание экрана игры */
typedef struct _gamescreen {
    /* Поверхность отрисовки */
    SDL_Surface *sprite;
} gamescreen;


/* Описание управляемого пользователем персонажа */
typedef struct _rabbitoo {
    /* Поверхность отрисовки */
    SDL_Surface *sprite;
    /* Координаты персонажа */
    int x;
    int y;
    /* Проекции скорости персонажа */
    int vx;
    int vy;
} rabbitoo;

/* Описание капель */
typedef struct _rain {
    /* Координаты капель */
    int x;
    int y;
    /* Проекции скорости капель */
    int vx;
    int vy;
} rain;

/* Описание спрайта-ресурса -- морковки */
typedef struct _carrot {
    /* Координаты ресурса */
    int x;
    int y;
    /* Проекции скорости ресурса */
    int vx;
    int vy;
} carrot;

/* Ресурсы и состояние игры  */
typedef struct _game {
    /* Экран игры */
    gamescreen *screen;
    
    /* Персонаж  пользователя */
    rabbitoo *rabbit;

    /* Капли и морковки */
    SDL_Surface *rain;
    rain rains[MAX_RAINS];
    SDL_Surface *carrot;
    carrot carrots[MAX_CARROTS];
    
    /* Фон игры */
    SDL_Surface *background;
    
    /* Таймер */
    int time;

    /* Счётчики */
    int won;
    int score;

    /* Шрифт */
    TTF_Font *font;
    
} game;

/**
 * Инициализирует игру
 * @returns g указатель на структуру состояния игры
 */
game *init();

/**
 * Инициализирует игру
 * @param g указатель на структуру состояния игры
 */
void run(game *g);

/**
 * Отрисовывает объекты в новой позиции
 * @param g указатель на структуру состояния игры
 */
void draw(game *g);


/**
 * Основная программа
 */
int main()
{
    /* Инициализируем игру */
    game *g = init();

    /* Запускаем цикл обработки событий игры */
    run(g);

    return 0;
}

/* Рандомайзер */
int randomizer(int b, int e)
{
    int r = rand();
    return r % (b - e + 1) + b;
}

/* Таймер */
Uint32 timer(Uint32 interval, void *p)
{
    game *g = p;
    g->time += interval / 1000;
    if (g->time > 10)
        g->won = 1;
    return interval;
}

/**
 * Инициализирует игру
 * @returns g указатель на структуру состояния игры
 */
game *init()
{
    /* Создаём структуру представления состояния игры */
    game *g;
    g = (game *) malloc(sizeof(game));
    if (g == NULL) {
        fprintf(stderr, "Not enough memory!");
        exit(EXIT_FAILURE);
    }

    g->won = 0;
    g->time = 0;

    /* Инициализируем библиотеку SDL, используем только видеоподсистему */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    /* Инициализируем SDL_ttf */
    if (TTF_Init() == -1) {
        fprintf(stderr, "Couldn't initialize TTF: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    /* Открываем шрифт, задаём размер */
    g->font = TTF_OpenFont("font.ttf", 40);
    if(!g->font) {
        fprintf(stderr, "Couldn't load the font: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    
    /* Регистрируем обработчик завершения программы */
    atexit(SDL_Quit);

    /* Выделяем память для структуры представления экрана */
    g->screen = (gamescreen *) malloc(sizeof(gamescreen));
    if (g->screen == NULL) {
        fprintf(stderr, "Not enough memory!");
        exit(EXIT_FAILURE);
    }    
    
    /* Инициализируем видеорежим */
    g->screen->sprite =
        SDL_SetVideoMode(1024, 768, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (g->screen->sprite == NULL) {
        fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    /* Выделяем память для структуры представления корабля */
    g->rabbit = (rabbitoo *) malloc(sizeof(rabbitoo));
    if (g->rabbit == NULL) {
        fprintf(stderr, "Not enough memory!");
        exit(EXIT_FAILURE);
    }

    /* Загружаем изображения */
    g->rabbit->sprite = SDL_LoadBMP("rabbit.bmp");
    if (g->rabbit->sprite == NULL) {
        fprintf(stderr, "Couldn't load a RABbitmap: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    g->background = SDL_LoadBMP("background.bmp");
    if (g->background == NULL) {
        fprintf(stderr, "Couldn't load a BBbitmap: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
    }
    
    g->rain = SDL_LoadBMP("rain.bmp");
    if (g->rain == NULL)
    {
        fprintf(stderr,"Couldn't load a RRbitmap: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    g->carrot = SDL_LoadBMP("carrot.bmp");
    if (g->carrot == NULL)
    {
        fprintf(stderr,"Couldn't load a CCbitmap: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);

    }

    /* Устанавливаем ключевой цвет (цвет прозрачных пикселей) */
    SDL_SetColorKey(g->carrot, SDL_SRCCOLORKEY, 0);
    SDL_SetColorKey(g->rain, SDL_SRCCOLORKEY, 0);
    SDL_SetColorKey(g->rabbit->sprite,SDL_SRCCOLORKEY, 0);

    /* Устанавливаем начальные координаты корабля */
    g->rabbit->x = 425;
    g->rabbit->y = 638;

    /* Устанавливаем таймер */
    SDL_AddTimer(1000, timer, g);

    /* Устанавливаем заголовок окна */
    SDL_WM_SetCaption("Rabbit", NULL);

    /* Задаем случайные значения для движения астероидов и монеток */
    for (int i = 0; i < MAX_RAINS; i++) {
        g->rains[i].x = randomizer(0, 1024);
        g->rains[i].y = randomizer(-300, -100);
        g->rains[i].vx = 0;
        g->rains[i].vy = randomizer(2, 5);
    }

    for (int i = 0; i < MAX_CARROTS; i++) {
        g->carrots[i].x = randomizer(0, 1024);
        g->carrots[i].y = randomizer(-300, -100);
        g->carrots[i].vx = 0;
        g->carrots[i].vy = randomizer(2, 5);
    }
    return g;
}

/* Вывод текста */
int display_text(const char* str, game *g, int x, int y)
{
    /* Устанавливаем цвет текста */
    SDL_Color color = {255,255,255};
    
    /* TTF_RenderText_Solid рендерит текст str шрифта font цветом color */
    SDL_Surface *text = TTF_RenderText_Solid(g->font, str, color);
    if(!text)
        return 0;
    
    int h = text->h;
    SDL_Rect rect = {x, y, 0, 0};
    
    /* Копируем всю поверхность текста*/
    SDL_BlitSurface(text, NULL, g->screen->sprite, &rect);
    SDL_FreeSurface(text);
    return h;
}

/* Рассматриваем случай выигрыша */
void win(game *g)
{
    int y = 0;
    
    /* Открываем файл для чтения и записи в конец */
    FILE *save = fopen("fixes.log", "a+");
    if (!save) {
        fprintf(stderr, "Couldn't open the save file\n");
        exit(EXIT_FAILURE);
    }
    
    /* Текущее время в секундах */
    time_t t = time(NULL);
    
    /* Возвращает указатель на структуру, содержащую преобразованное системное время в дату и местное время */
    struct tm *save_time = localtime(&t);
    
    /* Возвращает указатель на строку, представляющую преобразованную информацию, хранящуюся в структуре (на которую указывает аргумент) */
    char str[100] = "";
    char *str_time = asctime(save_time);
    /* Записываем текущий результат */
    sprintf(str, "%s %d\n", str_time, g->score);
    str_time[strlen(str_time) - 1] = '\0';
    
    /*
     FillRect заливает экран одним цветом
     BlitSurface копирует фон полностью на экран
    */
    SDL_FillRect(g->screen->sprite, NULL, 0x000000);
    SDL_BlitSurface(g->background, NULL, g->screen->sprite, NULL);

    /* Сообщение о выигрыше */
    y += display_text("You won", g, 0, 0);
    
    char last_res[100] = {' ', '\n', '\0'};
    
    /* Выводим предыдущий и текущий результаты */
    while (fgets(last_res, 100, save));
    y += display_text(last_res, g, 0, y);
    y += display_text(str, g, 0, y);
    
    /* Записываем текущий результат в fixes.log */
    fprintf(save, "%s %d\n", str_time, g->score);
    fclose(save);
    
    /* Отрисовка обновленного экрана */
    SDL_Flip(g->screen->sprite);
    SDL_Delay(7000);
}

/* Рассматриваем случай проигрыша */
void lose(game *g) {
    
    SDL_FillRect(g->screen->sprite, NULL, 0x000000);
    SDL_BlitSurface(g->background, NULL, g->screen->sprite, NULL);

    /* Сообщение о проигрыше */
    display_text("You lost!", g, 420, 350);
    
    /* Отрисовываем обновленный экран */
    SDL_Flip(g->screen->sprite);
    SDL_Delay(3000);
}

/**
 * Инициализирует игру
 * @param g указатель на структуру состояния игры
 */
void run(game * g)
{
    /* Флажок выхода */
    int done = 0;

    /* Продолжаем выполнение, пока не поднят флажок */
    while (!done) {
	if(g->won){
            win(g);
            break;
        }
        /* Структура описания события */
        SDL_Event event;

        /* Извлекаем и обрабатываем все доступные события */
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            /* Если нажали клавишу передвижения */
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    g->rabbit->vx += -VXD;
                    break;
                case SDLK_RIGHT:
                    g->rabbit->vx += VXD;
                    break;
                case SDLK_UP:
                    g->rabbit->vy += -VYD;
                    break;
                case SDLK_DOWN:
                    g->rabbit->vy += VYD;
                    break;
                case SDLK_ESCAPE:
                    done = 1;
                    break;
                }
                break;
            /* Если клавишу отпустили */
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    g->rabbit->vx += VXD;
                    break;
                case SDLK_RIGHT:
                    g->rabbit->vx += -VXD;
                    break;
                case SDLK_UP:
                    g->rabbit->vy += VYD;
                    break;
                case SDLK_DOWN:
                    g->rabbit->vy += -VYD;
                    break;
                default:
                    break;
                }
                break;
            /* Если закрыли окно */
            case SDL_QUIT:
                done = 1;
                break;
            default:
                break;
            }
        }
	
	/* Ограничиваем передвижение корабля */
        if(g->rabbit->x + g->rabbit->vx >= 0 && g->rabbit->x + g->rabbit->vx <= 874)
            g->rabbit->x += g->rabbit->vx;
        
        if(g->rabbit->y + g->rabbit->vy >= 450 && g->rabbit->y + g->rabbit->vy <= 638)
            g->rabbit->y += g->rabbit->vy;
	
	/* Ограничиваем количество спрайтов */
        for (int i = 0; i < MAX_RAINS; i++) {
            g->rains[i].x += g->rains[i].vx;
            g->rains[i].y += g->rains[i].vy;
        }

        for (int i = 0; i < MAX_CARROTS; i++) {
            g->carrots[i].x += g->carrots[i].vx;
            g->carrots[i].y += g->carrots[i].vy;
        }

        draw(g);

	/* Встреча с астероидами и их генерирование */
        for (int i = 0; i < MAX_RAINS; i++) {
	    /* Если встретились с астероидом, то проигрыш */
            if (g->rabbit->x + g->rabbit->sprite->w > g->rains[i].x
                && g->rabbit->x < g->rains[i].x + g->rain->w
                && g->rabbit->y + g->rabbit->sprite->h > g->rains[i].y
                && g->rabbit->y < g->rains[i].y + g->rain->h ) {
                    done = 1;
                    lose(g);
                    break;
            }
	    /* Генерируем появление астероидов */
            if (g->rains[i].y > g->screen->sprite->h) {
                g->rains[i].x = randomizer(0, 924);
                g->rains[i].y = randomizer(-200, -100);
                g->rains[i].vy = randomizer(2, 5);
            }
        }

	/* Если столкнулись с монеткой, то записываем в score, продолжаем их генерировать */
        for (int i = 0; i < MAX_CARROTS; i++) {
            if (g->rabbit->x + g->rabbit->sprite->w > g->carrots[i].x
            && g->rabbit->x < g->carrots[i].x + g->carrot->w 
            && g->rabbit->y + g->rabbit->sprite->h > g->carrots[i].y
            && g->rabbit->y < g->carrots[i].y + g->carrot->h ) {
                g->score++;
                g->carrots[i].y = g->screen->sprite->h;
            }
            if (g->carrots[i].y >= g->screen->sprite->h) {
                g->carrots[i].x = randomizer(0, 924);
                g->carrots[i].y = randomizer(-200, -100);
                g->carrots[i].vy = randomizer(2, 5);
	    }
	}
	
        SDL_Delay(10);
    }
}

/**
 * Отрисовывает объекты в новой позиции
 * @param g указатель на структуру состояния игры
 */
void draw(game * g)
{
    /* Прямоугольники, определяющие зону отображения */
    SDL_Rect src, dest;

    /* Персонажа отображаем целиком */
    src.x = 0;
    src.y = 0;
    src.w = g->rabbit->sprite->w;
    src.h = g->rabbit->sprite->h;

    /* в новую позицию */
    dest.x = g->rabbit->x;
    dest.y = g->rabbit->y;
    dest.w = g->rabbit->sprite->w;
    dest.h = g->rabbit->sprite->h;

    /* Выполняем отображение */
    SDL_FillRect(g->screen->sprite,NULL, 0x000000);
    SDL_BlitSurface(g->background,NULL,g->screen->sprite, NULL);
    SDL_BlitSurface(g->rabbit->sprite, &src, g->screen->sprite, &dest);

    
     /* Отрисовываем капли */
    for (int i = 0; i < MAX_RAINS; i++) {
        dest.x = g->rains[i].x;
        dest.y = g->rains[i].y;
        SDL_BlitSurface(g->rain, NULL, g->screen->sprite, &dest);
    }

    /* Отрисовываем морковки */
    for (int i = 0; i < MAX_CARROTS; i++) {
        dest.x = g->carrots[i].x;
        dest.y = g->carrots[i].y;
        SDL_BlitSurface(g->carrot, NULL, g->screen->sprite, &dest);
    }

    char score[10] = "";
    sprintf(score, "Score: %d", g->score);
    display_text(score, g, g->screen->sprite->w - 190, 0);

    char time[8] = "";
    sprintf(time,"%02d:%02d",g->time / 60, g->time % 60);
    display_text(time, g, 0, 0);
    
    /* Отрисовываем обновленный экран */
    SDL_Flip(g->screen->sprite);
}
