#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <time.h>

#if defined(_WIN32) || defined(__WIN32__)
    #include <curses.h>   
    #include <windows.h>
    #define ESPERAR_MS(ms) Sleep(ms)
#else
    #include <ncurses.h>
    #include <unistd.h>   
    #define ESPERAR_MS(ms) usleep((ms) * 1000) 
#endif

#define ALTURA 30
#define LARGURA 100
#define MILISSEGUNDOS 50

typedef struct{
	int x, y;
} Player;

typedef struct{
	int x, y;
	int ativo;
	int direcao;
} Inimigo;

typedef struct{
	int x, y;
	int ativo;
	int direcao;
} Mergulador;

// protótipos das funções
void desenhar_borda();
void menu();
void game();

void menu(){
	int op = 0, conjunto_op = 0;

	while(1){
		clear();
		desenhar_borda();
		mvprintw(ALTURA / 3 - 8, (LARGURA / 2) - 5, "SEAQUEST");

		if (op == 0) {
            attron(A_REVERSE);
            mvprintw(ALTURA / 2 - 1, (LARGURA / 2) - 4, " START ");
            attroff(A_REVERSE); 
            mvprintw(ALTURA / 2 + 1, (LARGURA / 2) - 4, "  QUIT  ");
        } else {
            mvprintw(ALTURA / 2 - 1, (LARGURA / 2) - 4, "  START  ");
            attron(A_REVERSE);
            mvprintw(ALTURA / 2 + 1, (LARGURA / 2) - 4, "  QUIT  ");
            attroff(A_REVERSE);
        }

		mvprintw(ALTURA / 2 + 8, (LARGURA / 2) - 10, "Use as Setas e Enter");
        refresh();

        int c = getch();
        switch (c) {
            case KEY_UP: 
                op = 0;
                break;
            case KEY_DOWN: 
                op = 1;
                break;
            case 10:
            #if defined(_WIN32)
                case 13:
            #endif
                conjunto_op = 1;
                break;
        }
        
        if (conjunto_op) {
            if (op == 0) {
                game(); 
                conjunto_op = 0;
            } else {
                break; 
            }
        } 
	} 
}

void desenhar_borda() {
    for (int i = 0; i <= LARGURA; i++) {
        mvaddch(0, i, '#');
        mvaddch(ALTURA, i, '#');
    }
    for (int i = 0; i <= ALTURA; i++) {
        mvaddch(i, 0, '#');
        mvaddch(i, LARGURA, '#');
    }
} 

void game() {
    nodelay(stdscr, TRUE);
    Player submarino = {10, ALTURA / 2}; 
    Inimigo tubarao = {LARGURA - 4, 8, 1, -1}; 
    Mergulador humano = {2, 14, 1, 1}; 
    Inimigo sub_enemy = {LARGURA - 2, 5, 1, -1};

    int pontuacao = 0;
    int mergulhadores_salvos = 0;
    int oxigenio = 100;
    int game_over = 0;
    int loop_count = 0;

    while (!game_over) {
        clear();
        desenhar_borda();
        
        mvprintw(0, 25, "Pontos: %d | Mergulhadores: %d | Oxigenio: ", pontuacao, mergulhadores_salvos);
                
        int par_cor = 1;
        if (oxigenio <= 20) {
            par_cor = 3; 
        } else if (oxigenio <= 50) {
            par_cor = 2;
        }

        attron(COLOR_PAIR(par_cor));

        int tamanho_barra = oxigenio / 10;
        mvprintw(0, 65, "[");
        for (int b = 0; b < 10; b++) {
            if (b < tamanho_barra) {
                printw("#"); 
            } else {
                printw(" "); 
            }
        }
        printw("] %d%%", oxigenio);
        
        attroff(COLOR_PAIR(par_cor));
        
        mvhline(3, 1, '~', 99);

        mvprintw(submarino.y, submarino.x, "=0=");

        if (tubarao.ativo) {
            if(tubarao.direcao == -1){
                mvprintw(tubarao.y, tubarao.x, "<><");
            } else {
                mvprintw(tubarao.y, tubarao.x, "><>");
            }
            if (loop_count % 2 == 0) { 
                tubarao.x += tubarao.direcao;
            }
            if(tubarao.x <= 1 || tubarao.x >= LARGURA - 4){
                tubarao.direcao = (rand() % 2 == 0) ? -1 : 1;
                if(tubarao.direcao == -1){
                    tubarao.x = LARGURA - 4;
                } else {
                    tubarao.x = 2;
                }
                tubarao.y = (rand() % (ALTURA - 6)) + 4;
            }
        }
        
        if (sub_enemy.ativo) {
            if(sub_enemy.direcao == -1){
                mvprintw(sub_enemy.y, sub_enemy.x, "=0+"); 
            } else {
                mvprintw(sub_enemy.y, sub_enemy.x, "+0="); 
            }
            if (loop_count % 3 == 0) { 
             
                sub_enemy.x += sub_enemy.direcao;
            }
            if(sub_enemy.x <= 1 || sub_enemy.x >= LARGURA - 4){
                sub_enemy.direcao = (rand() % 2 == 0) ? -1 : 1;
                if(sub_enemy.direcao == -1){
                    sub_enemy.x = LARGURA - 4;
                } else {
                    sub_enemy.x = 2;
                }
                sub_enemy.y = (rand() % (ALTURA - 6)) + 4;
            }
        } 

        if (humano.ativo) {
            mvprintw(humano.y, humano.x, "oOo");
            if (loop_count % 3 == 0) { 
                humano.x += humano.direcao;
            }
            if (humano.x <= 1 || humano.x >= LARGURA - 2) {
                humano.direcao = (rand() % 2 == 0) ? -1 : 1;
                if(humano.direcao == -1){
                    humano.x = LARGURA - 2; 
                } else{
                  humano.x = 2; 
                }
                humano.y = (rand() % (ALTURA - 6)) + 4; 
            }
        }
        
        if (humano.ativo && submarino.y == humano.y && (submarino.x + 2 >= humano.x && submarino.x <= humano.x + 2)) {
            mergulhadores_salvos++;
            pontuacao += 100;
            humano.direcao = (rand() % 2 ==0) ? -1 : 1;
            humano.x = (humano.direcao == -1) ? LARGURA -2 : 2;
            humano.y = (rand() % (ALTURA - 6)) + 4;
        }

        // sistema de colisão
        if (tubarao.ativo && submarino.y == tubarao.y && (submarino.x + 2 >= tubarao.x && submarino.x <= tubarao.x + 2)) {
            game_over = 1;
        }

        if (sub_enemy.ativo && submarino.y == sub_enemy.y && (submarino.x + 2 >= sub_enemy.x && submarino.x <= sub_enemy.x + 2)) {
            game_over = 1;
        }

        // oxigênio
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
        
        refresh();
        napms(MILISSEGUNDOS);
        loop_count++;

        // Controles
        int ch;
        while ((ch = getch()) != ERR) {
            switch (ch) {
                case KEY_UP:
                case 'w':
                    if (submarino.y > 1) submarino.y--;
                    break;
                case KEY_DOWN:
                case 's':
                    if (submarino.y < ALTURA - 1) submarino.y++;
                    break;
                case KEY_LEFT:
                case 'a':
                    if (submarino.x > 1) submarino.x--;
                    break;
                case KEY_RIGHT:
                case 'd':
                    if (submarino.x < LARGURA - 4) submarino.x++;
                    break;
                case 'q':
                    game_over = 1;
                    break;
            }    
        }   
    }

    // tela de GAME OVER
    nodelay(stdscr, FALSE);
    clear();
    desenhar_borda();
    mvprintw(ALTURA / 2 - 1, (LARGURA / 2) - 5, "GAME OVER");
    mvprintw(ALTURA / 2 + 1, (LARGURA / 2) - 9, "Pontuacao Final: %d", pontuacao);
    mvprintw(ALTURA / 2 + 3, (LARGURA / 2) - 14, "Pressione qualquer tecla");
    refresh();
    getch();
}

int main(){
	initscr();
	cbreak(); 
	noecho(); 
	keypad(stdscr, TRUE); 
	curs_set(0); 
	srand(time(NULL)); 

	// esquema de cores
	if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
    } 
	
	menu();
	endwin(); 
	return 0;
}
