/*
 * SEAQUEST - versao portatil (SEM ncurses, SEM PDCurses)
 *
 * Usa o mesmo truque do flappy-ascii.c do professor: no Windows usa
 * windows.h/conio.h (que ja vem com qualquer instalacao MinGW), no
 * Linux usa termios.h/unistd.h (que ja vem com qualquer gcc). Nao
 * precisa instalar nenhuma biblioteca extra em nenhum dos dois SOs.
 *
 * Desenho: em vez de mvprintw/refresh do curses, montamos a tela
 * inteira numa string e mandamos tudo de uma vez com fputs - isso
 * evita o efeito de "piscar" sem precisar de nenhum buffer especial
 * do Windows.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

/* ------------------------------- Configuracao ---------------------------- */

#define ALTURA   30
#define LARGURA  100
#define DELAY_MS 50

/* codigos de tecla normalizados (iguais nos dois SOs) */
#define TECLA_CIMA      1000
#define TECLA_BAIXO     1001
#define TECLA_ESQUERDA  1002
#define TECLA_DIREITA   1003
#define TECLA_ENTER     1004

/* --------------------------------- Entidades ------------------------------ */

typedef struct { int x, y; } PLAYER;
typedef struct { int x, y; int ativo; } INIMIGO;
typedef struct { int x, y; int ativo; } MERGULHADOR;

/* ---------------------- Camada portatil (terminal / teclado) -------------- */

#ifdef _WIN32

void restaurar_terminal(void) { printf("\033[?25h\033[0m"); fflush(stdout); }

void preparar_terminal(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    GetConsoleMode(h, &modo);
    SetConsoleMode(h, modo | 0x0004); /* ENABLE_VIRTUAL_TERMINAL_PROCESSING: liga os codigos ANSI no cmd */
    printf("\033[2J\033[?25l");
    atexit(restaurar_terminal);
}

int ler_tecla(void) {
    if (!_kbhit()) return -1;
    int c = _getch();
    if (c == 0 || c == 224) {
        int c2 = _getch();
        switch (c2) {
            case 72: return TECLA_CIMA;
            case 80: return TECLA_BAIXO;
            case 75: return TECLA_ESQUERDA;
            case 77: return TECLA_DIREITA;
        }
        return -1;
    }
    if (c == '\r') return TECLA_ENTER;
    return c;
}

void dormir(int ms) { Sleep(ms); }

#else /* Linux / macOS */

static struct termios termo_original;

void restaurar_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &termo_original);
    printf("\033[?25h\033[0m");
    fflush(stdout);
}

void preparar_terminal(void) {
    tcgetattr(STDIN_FILENO, &termo_original);
    struct termios cru = termo_original;
    cru.c_lflag &= ~(ICANON | ECHO);
    cru.c_cc[VMIN]  = 0;
    cru.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &cru);
    printf("\033[2J\033[?25l");
    atexit(restaurar_terminal);
}

int ler_tecla(void) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;

    if (c == 27) { /* possivel seta: ESC [ letra */
        unsigned char c2, c3;
        if (read(STDIN_FILENO, &c2, 1) != 1) return -1;
        if (read(STDIN_FILENO, &c3, 1) != 1) return -1;
        if (c2 == '[') {
            switch (c3) {
                case 'A': return TECLA_CIMA;
                case 'B': return TECLA_BAIXO;
                case 'D': return TECLA_ESQUERDA;
                case 'C': return TECLA_DIREITA;
            }
        }
        return -1;
    }
    if (c == '\n') return TECLA_ENTER;
    return c;
}

void dormir(int ms) { usleep(ms * 1000); }

#endif

/* ----------------------- Helpers de desenho (ANSI puro) -------------------- */

/* buffer onde a tela inteira e montada antes de imprimir de uma vez so */
static char tela_buf[ALTURA + 5][LARGURA + 40];

void limpa_buffer(void) {
    for (int y = 0; y < ALTURA + 2; y++)
        memset(tela_buf[y], ' ', LARGURA + 1), tela_buf[y][LARGURA + 1] = '\0';
}

/* escreve uma string na posicao (lin, col) do buffer, sem estourar a linha */
void escreve(int lin, int col, const char *texto) {
    if (lin < 0 || lin >= ALTURA + 2) return;
    int len = (int) strlen(texto);
    for (int i = 0; i < len; i++) {
        int c = col + i;
        if (c < 0 || c >= LARGURA + 1) continue;
        tela_buf[lin][c] = texto[i];
    }
}

void escreve_char(int lin, int col, char ch) {
    if (lin < 0 || lin >= ALTURA + 2 || col < 0 || col >= LARGURA + 1) return;
    tela_buf[lin][col] = ch;
}

/* manda o buffer inteiro pra tela de uma vez (cursor no topo, sem clear) */
void mostra_buffer(void) {
    printf("\033[H");
    for (int y = 0; y < ALTURA + 2; y++)
        printf("%s\n", tela_buf[y]);
    fflush(stdout);
}

/* -------------------------------- Jogo -------------------------------- */

void desenhar_borda(void) {
    for (int i = 0; i <= LARGURA; i++) {
        escreve_char(0, i, '&');
        escreve_char(ALTURA, i, '&');
    }
    for (int i = 0; i <= ALTURA; i++) {
        escreve_char(i, 0, '#');
        escreve_char(i, LARGURA, '#');
    }
}

void menu(void) {
    int op = 0, confirmou = 0;

    while (1) {
        limpa_buffer();
        desenhar_borda();

        escreve(ALTURA / 3 - 8, (LARGURA / 2) - 4, "SEAQUEST");

        if (op == 0) {
            escreve(ALTURA / 2 - 1, (LARGURA / 2) - 4, "[ START ]");
            escreve(ALTURA / 2 + 1, (LARGURA / 2) - 4, "  QUIT   ");
        } else {
            escreve(ALTURA / 2 - 1, (LARGURA / 2) - 4, "  START  ");
            escreve(ALTURA / 2 + 1, (LARGURA / 2) - 4, "[ QUIT ]");
        }

        escreve(ALTURA / 2 + 8, (LARGURA / 2) - 10, "Use as Setas e Enter");
        mostra_buffer();

        int c;
        while ((c = ler_tecla()) == -1) dormir(20); /* espera alguma tecla */

        if (c == TECLA_CIMA)   op = 0;
        if (c == TECLA_BAIXO)  op = 1;
        if (c == TECLA_ENTER)  confirmou = 1;

        if (confirmou) {
            if (op == 0) { extern void game(void); game(); confirmou = 0; }
            else break;
        }
    }
}

void game(void) {
    PLAYER submarino = { 10, ALTURA / 2 };
    INIMIGO tubarao = { LARGURA - 4, 8, 1 };
    MERGULHADOR humano = { LARGURA - 2, 14, 1 };

    int pontuacao = 0;
    int mergulhadores_salvos = 0;
    int oxigenio = 100;
    int game_over = 0;
    int loop_count = 0;

    while (!game_over) {
        limpa_buffer();
        desenhar_borda();

        char hud[128];
        snprintf(hud, sizeof hud, " Pontos: %d | Mergulhadores: %d | Oxigenio: %d%% ",
                 pontuacao, mergulhadores_salvos, oxigenio);
        escreve(0, 2, hud);

        escreve(3, 1, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

        escreve(submarino.y, submarino.x, "=0=");

        if (tubarao.ativo) {
            escreve(tubarao.y, tubarao.x, "<><");
            if (loop_count % 2 == 0) tubarao.x--;
            if (tubarao.x <= 1) {
                tubarao.x = LARGURA - 4;
                tubarao.y = (rand() % (ALTURA - 6)) + 4;
            }
        }

        if (humano.ativo) {
            escreve_char(humano.y, humano.x, 'o');
            if (loop_count % 3 == 0) humano.x--;
            if (humano.x <= 1) {
                humano.x = LARGURA - 2;
                humano.y = (rand() % (ALTURA - 6)) + 4;
            }
        }

        /* colisoes */
        if (humano.ativo && submarino.y == humano.y &&
            (submarino.x <= humano.x && submarino.x + 2 >= humano.x)) {
            mergulhadores_salvos++;
            pontuacao += 100;
            humano.x = LARGURA - 2;
            humano.y = (rand() % (ALTURA - 6)) + 4;
        }

        if (tubarao.ativo && submarino.y == tubarao.y &&
            (submarino.x + 2 >= tubarao.x && submarino.x <= tubarao.x + 2)) {
            game_over = 1;
        }

        if (submarino.y <= 3) {
            if (mergulhadores_salvos > 0) {
                pontuacao += mergulhadores_salvos * 200;
                mergulhadores_salvos = 0;
            }
            if (oxigenio < 100) oxigenio += 5;
        } else {
            if (loop_count % 10 == 0) oxigenio--;
        }

        if (oxigenio <= 0) game_over = 1;

        mostra_buffer();
        dormir(DELAY_MS);
        loop_count++;

        int ch;
        while ((ch = ler_tecla()) != -1) {
            if (ch == TECLA_CIMA    && submarino.y > 1)             submarino.y--;
            if (ch == TECLA_BAIXO   && submarino.y < ALTURA - 1)    submarino.y++;
            if (ch == TECLA_ESQUERDA && submarino.x > 1)            submarino.x--;
            if (ch == TECLA_DIREITA && submarino.x < LARGURA - 4)   submarino.x++;
            if (ch == 'q' || ch == 'Q') game_over = 1;
        }
    }

    limpa_buffer();
    desenhar_borda();
    escreve(ALTURA / 2 - 1, (LARGURA / 2) - 5, "GAME OVER");
    char placar[64];
    snprintf(placar, sizeof placar, "Pontuacao Final: %d", pontuacao);
    escreve(ALTURA / 2 + 1, (LARGURA / 2) - 9, placar);
    escreve(ALTURA / 2 + 3, (LARGURA / 2) - 14, "Pressione qualquer tecla");
    mostra_buffer();

    int c;
    while ((c = ler_tecla()) == -1) dormir(20);
}

int main(void) {
    srand((unsigned) time(NULL));
    preparar_terminal();
    menu();
    return 0;
}