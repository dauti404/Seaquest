/*
 * SEAQUEST - versao portatil (SEM ncurses, SEM PDCurses)
 *
 * Desenho sem flicker: cada celula da tela guarda seu proprio
 * caractere E sua propria cor (igual o flappy-ascii.c do professor).
 * Tudo e montado numa unica string e mandado de uma vez com fputs,
 * entao nao existe mais um segundo "passe" de cor por cima -
 * era isso que causava o piscar.
 *
 * Windows: windows.h + conio.h | Linux: termios.h + unistd.h
 * Nenhuma biblioteca externa precisa ser instalada.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define ESPERAR_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #include <termios.h>
    #define ESPERAR_MS(ms) usleep((ms) * 1000)
#endif

/* ------------------------------- Configuracao ---------------------------- */

#define ALTURA   30
#define LARGURA  100
#define DELAY_MS 50

#define TECLA_CIMA      1000
#define TECLA_BAIXO     1001
#define TECLA_ESQUERDA  1002
#define TECLA_DIREITA   1003
#define TECLA_ENTER     1004

/* codigos de cor ANSI usados no jogo */
#define COR_PADRAO    0   /* sem cor especial */
#define COR_REVERSO   7   /* destaque do menu (video reverso) */
#define COR_VERDE     32
#define COR_AMARELO   33
#define COR_VERMELHO  31

/* --------------------------------- Entidades ------------------------------ */

typedef struct { int x, y; } Player;
typedef struct { int x, y, ativo, direcao; } Inimigo;
typedef struct { int x, y, ativo, direcao; } Mergulhador;

/* ---------------------- Camada portatil (terminal / teclado) -------------- */

#ifdef _WIN32

void restaurar_terminal(void) { printf("\033[?25h\033[0m"); fflush(stdout); }

void preparar_terminal(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    GetConsoleMode(h, &modo);
    SetConsoleMode(h, modo | 0x0004);
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
    if (c == '\r' || c == '\n') return TECLA_ENTER;
    return c;
}

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

    if (c == 27) {
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
    if (c == '\n' || c == '\r') return TECLA_ENTER;
    return c;
}

#endif

/* -------------------- Buffer de tela: caractere + cor por celula ---------- */

static char tela_buf[ALTURA + 2][LARGURA + 2];  /* o que aparece */
static int  cor_buf [ALTURA + 2][LARGURA + 2];  /* a cor de cada posicao */
static char saida[(ALTURA + 2) * (LARGURA + 40)];

void limpa_buffer(void) {
    for (int y = 0; y <= ALTURA; y++) {
        for (int x = 0; x <= LARGURA; x++) {
            tela_buf[y][x] = ' ';
            cor_buf[y][x]  = COR_PADRAO;
        }
    }
}

/* escreve texto simples (sem cor especial) */
void escreve(int lin, int col, const char *texto) {
    if (lin < 0 || lin > ALTURA) return;
    int len = (int) strlen(texto);
    for (int i = 0; i < len; i++) {
        int c = col + i;
        if (c < 0 || c > LARGURA) continue;
        tela_buf[lin][c] = texto[i];
    }
}

/* escreve texto e marca a cor de cada celula ocupada */
void escreve_cor(int lin, int col, const char *texto, int codigo_cor) {
    if (lin < 0 || lin > ALTURA) return;
    int len = (int) strlen(texto);
    for (int i = 0; i < len; i++) {
        int c = col + i;
        if (c < 0 || c > LARGURA) continue;
        tela_buf[lin][c] = texto[i];
        cor_buf[lin][c]  = codigo_cor;
    }
}

void escreve_char(int lin, int col, char ch) {
    if (lin < 0 || lin > ALTURA || col < 0 || col > LARGURA) return;
    tela_buf[lin][col] = ch;
}

/* monta a tela inteira (caractere + cor) numa unica string e manda de uma vez */
void mostra_buffer(void) {
    char *p = saida;
    p += sprintf(p, "\033[H");

    for (int y = 0; y <= ALTURA; y++) {
        int cor_atual = -1; /* forca o primeiro codigo de cor a ser escrito */
        for (int x = 0; x <= LARGURA; x++) {
            if (cor_buf[y][x] != cor_atual) {
                cor_atual = cor_buf[y][x];
                p += sprintf(p, "\033[%dm", cor_atual);
            }
            *p++ = tela_buf[y][x];
        }
        p += sprintf(p, "\033[0m\n");
    }
    *p = '\0';
    fputs(saida, stdout);
    fflush(stdout);
}

/* -------------------------------- Jogo ----------------------------------- */

void desenhar_borda(void) {
    for (int i = 0; i <= LARGURA; i++) {
        escreve_char(0, i, '#');
        escreve_char(ALTURA, i, '#');
    }
    for (int i = 0; i <= ALTURA; i++) {
        escreve_char(i, 0, '#');
        escreve_char(i, LARGURA, '#');
    }
}

void game(void);

void menu(void) {
    int op = 0, confirmou = 0;

    while (1) {
        limpa_buffer();
        desenhar_borda();

        escreve(ALTURA / 3 - 8, (LARGURA / 2) - 5, "SEAQUEST");
        escreve(ALTURA / 2 + 8, (LARGURA / 2) - 10, "Use as Setas e Enter");

        /* destaque em video reverso na opcao selecionada - ja incluido
           no mesmo desenho, sem segundo passe */
        escreve_cor(ALTURA / 2 - 1, (LARGURA / 2) - 4, " START ",
                    (op == 0) ? COR_REVERSO : COR_PADRAO);
        escreve_cor(ALTURA / 2 + 1, (LARGURA / 2) - 4, "  QUIT  ",
                    (op == 1) ? COR_REVERSO : COR_PADRAO);

        mostra_buffer();

        int c;
        while ((c = ler_tecla()) == -1) ESPERAR_MS(20);

        if (c == TECLA_CIMA)  op = 0;
        if (c == TECLA_BAIXO) op = 1;
        if (c == TECLA_ENTER) confirmou = 1;

        if (confirmou) {
            if (op == 0) { game(); confirmou = 0; }
            else break;
        }
    }
}

void game(void) {
    Player submarino  = { 10, ALTURA / 2 };
    Inimigo tubarao    = { LARGURA - 4, 8, 1, -1 };
    Inimigo sub_enemy  = { LARGURA - 2, 5, 1, -1 };
    Mergulhador humano = { 2, 14, 1, 1 };

    int pontuacao = 0;
    int mergulhadores_salvos = 0;
    int oxigenio = 100;
    int game_over = 0;
    int loop_count = 0;

    while (!game_over) {
        limpa_buffer();
        desenhar_borda();

        escreve(3, 1, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        escreve(submarino.y, submarino.x, "=0=");

        if (tubarao.ativo) {
            escreve(tubarao.y, tubarao.x, (tubarao.direcao == -1) ? "<><" : "><>");
            if (loop_count % 2 == 0) tubarao.x += tubarao.direcao;
            if (tubarao.x <= 1 || tubarao.x >= LARGURA - 4) {
                tubarao.direcao = (rand() % 2 == 0) ? -1 : 1;
                tubarao.x = (tubarao.direcao == -1) ? LARGURA - 4 : 2;
                tubarao.y = (rand() % (ALTURA - 6)) + 4;
            }
        }

        if (sub_enemy.ativo) {
            escreve(sub_enemy.y, sub_enemy.x, (sub_enemy.direcao == -1) ? "=0+" : "+0=");
            if (loop_count % 3 == 0) sub_enemy.x += sub_enemy.direcao;
            if (sub_enemy.x <= 1 || sub_enemy.x >= LARGURA - 4) {
                sub_enemy.direcao = (rand() % 2 == 0) ? -1 : 1;
                sub_enemy.x = (sub_enemy.direcao == -1) ? LARGURA - 4 : 2;
                sub_enemy.y = (rand() % (ALTURA - 6)) + 4;
            }
        }

        if (humano.ativo) {
            escreve(humano.y, humano.x, "oOo");
            if (loop_count % 3 == 0) humano.x += humano.direcao;
            if (humano.x <= 1 || humano.x >= LARGURA - 2) {
                humano.direcao = (rand() % 2 == 0) ? -1 : 1;
                humano.x = (humano.direcao == -1) ? LARGURA - 2 : 2;
                humano.y = (rand() % (ALTURA - 6)) + 4;
            }
        }

        if (humano.ativo && submarino.y == humano.y &&
            (submarino.x + 2 >= humano.x && submarino.x <= humano.x + 2)) {
            mergulhadores_salvos++;
            pontuacao += 100;
            humano.direcao = (rand() % 2 == 0) ? -1 : 1;
            humano.x = (humano.direcao == -1) ? LARGURA - 2 : 2;
            humano.y = (rand() % (ALTURA - 6)) + 4;
        }

        if (tubarao.ativo && submarino.y == tubarao.y &&
            (submarino.x + 2 >= tubarao.x && submarino.x <= tubarao.x + 2)) {
            game_over = 1;
        }

        if (sub_enemy.ativo && submarino.y == sub_enemy.y &&
            (submarino.x + 2 >= sub_enemy.x && submarino.x <= sub_enemy.x + 2)) {
            game_over = 1;
        }

        if (submarino.y <= 3) {
            if (mergulhadores_salvos > 0) {
                pontuacao += mergulhadores_salvos * 200;
                mergulhadores_salvos = 0;
            }
            if (oxigenio < 100) oxigenio += 1;
        } else {
            if (loop_count % 10 == 0) oxigenio--;
        }

        if (oxigenio <= 0) game_over = 1;

        /* HUD: monta o texto e a barra ANTES do mostra_buffer, tudo no
           mesmo desenho - por isso nao pisca mais */
        char label[64];
        int label_len = snprintf(label, sizeof label,
            " Pontos: %d | Mergulhadores: %d | Oxigenio: ",
            pontuacao, mergulhadores_salvos);
        escreve(0, 2, label);

        int cor_oxig = COR_VERDE;
        if (oxigenio <= 20)      cor_oxig = COR_VERMELHO;
        else if (oxigenio <= 50) cor_oxig = COR_AMARELO;

        int tamanho_barra = oxigenio / 10;
        char barra[32];
        int p = 0;
        barra[p++] = '[';
        for (int b = 0; b < 10; b++) barra[p++] = (b < tamanho_barra) ? '#' : ' ';
        barra[p++] = ']';
        p += sprintf(barra + p, " %d%%", oxigenio);
        barra[p] = '\0';

        escreve_cor(0, 2 + label_len, barra, cor_oxig);

        mostra_buffer();

        ESPERAR_MS(DELAY_MS);
        loop_count++;

        int ch;
        while ((ch = ler_tecla()) != -1) {
            if (ch == TECLA_CIMA     && submarino.y > 1)           submarino.y--;
            if (ch == TECLA_BAIXO    && submarino.y < ALTURA - 1)  submarino.y++;
            if (ch == TECLA_ESQUERDA && submarino.x > 1)           submarino.x--;
            if (ch == TECLA_DIREITA  && submarino.x < LARGURA - 4) submarino.x++;
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
    while ((c = ler_tecla()) == -1) ESPERAR_MS(20);
}

int main(void) {
    srand((unsigned) time(NULL));
    preparar_terminal();
    menu();
    return 0;
}