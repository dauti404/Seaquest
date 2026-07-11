#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>

// difinições do jogo
#define ALTURA 30
#define LARGURA 100

// Entidades
typedef struct{
	int x, y;
} PLAYER;

typedef struct{
	int x, y;
	int ativo;
} INIMIGO;

typedef struct{
	int x, y;
	int ativo;
} MERGULHADOR;

// Prototipos das funções 
void desenhar_borda();
void menu();
void game();

// Função do MENU
void menu(){
	// Variáveis de escolhas
	int op = 0, conjunto_op = 0;

	while(1){
		clear();
		desenhar_borda();

		// Definir o Título do Jogo
		mvprintw(
			// Mantém centralizado os caracteres na tela
			ALTURA / 
			// o primeiro move de forma agressiva os caracteres
			3 
			// o segundo move de forma suave os caraceters
			- 8, (LARGURA / 2) - 5, "SEAQUEST");

		 // Opções do Menu
		if (op == 0) {
            attron(A_REVERSE); // Destaca a opção selecionada
			// Opção start
            mvprintw(ALTURA / 2 - 1, (LARGURA / 2) - 4, " START ");
            attroff(A_REVERSE); // Destacamento da escolha
			// opção quit
            mvprintw(ALTURA / 2 + 1, (LARGURA / 2) - 4, "  QUIT  ");
        } else {
			// Faz o inverso das funções acima
            mvprintw(ALTURA / 2 - 1, (LARGURA / 2) - 4, "  START  ");
            attron(A_REVERSE);
            mvprintw(ALTURA / 2 + 1, (LARGURA / 2) - 4, "  QUIT  ");
            attroff(A_REVERSE);
        }

		// Legenda de ajuda para o usuário
		mvprintw(ALTURA / 2 + 8, (LARGURA / 2) - 10, "Use as Setas e Enter");
        refresh();
        
		// Variável que guarda a tecla do usuário
        int c = getch();
		// Casos para uso de teclas pelo usuário
        switch (c) {
            case KEY_UP: // seta para cima
                op = 0;
                break;
            case KEY_DOWN: // seta para baixo
                op = 1;
                break;
            case 10: // Tecla Enter
                conjunto_op = 1;
                break;
        }
        
        if (conjunto_op) { // início do IF
            if (op == 0) {
                game(); // Inicia o jogo
                conjunto_op = 0; // Volta para o menu se o jogo acabar
            } else {
                break; // Sai do loop e fecha o programa
            }
        } // fim do IF

	} // fim do WHILE
} // fim da função do MENU

// desenha a borda da tela
void desenhar_borda() {
	// Largura da tela
    for (int i = 0; i <= LARGURA; i++) {
        mvaddch(
			// Linha inicial
			0, 
			// Coluna preenchidas com o caracter desejado
			i, 
			// Caracter ACSII
			'&');
		// Linha final
        mvaddch(
			// Valor da altura
			ALTURA, 
			// Linha final de caracteres
			i, 
			// Caracter desejado ACSII
			'&');
    }
	// Altura da tela
    for (int i = 0; i <= ALTURA; i++) {
        mvaddch(i, 0, '#');
        mvaddch(i, LARGURA, '#');
    }
} // fim da função que desenha a borda

// função que faz o jogo funcionar
void game() {
    // Configura o getch para não travar o jogo esperando input
    nodelay(stdscr, TRUE);
    
    PLAYER submarino = {10, ALTURA / 2};
    INIMIGO tubarao = {LARGURA - 4, 8, 1};
    MERGULHADOR humano = {LARGURA - 2, 14, 1};
    
    int pontuacao = 0;
    int mergulhadores_salvos = 0;
    int oxigenio = 100;
    int game_over = 0;
    int loop_count = 0;

    while (!game_over) {
        clear();
        desenhar_borda();
        
        // Interface de Status
        mvprintw(0, 2, " Pontos: %d | Mergulhadores: %d | Oxigenio: %d%% ", pontuacao, mergulhadores_salvos, oxigenio);
        // Linha da superfície da água
        mvprintw(3, 1, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

        // Desenha o Jogador (Submarino)
        mvprintw(submarino.y, submarino.x, "=0=");

        // Movimenta e Desenha o Tubarão
        if (tubarao.ativo) {
            mvprintw(tubarao.y, tubarao.x, "<><");
            if (loop_count % 2 == 0) { // Controla a velocidade do tubarão
                tubarao.x--;
            }
            if (tubarao.x <= 1) {
                tubarao.x = LARGURA - 4;
                tubarao.y = (rand() % (ALTURA - 6)) + 4; // Spawna em altura aleatória abaixo da água
            }
        }

        // Movimenta e Desenha o Mergulhador
        if (humano.ativo) {
            mvprintw(humano.y, humano.x, "o");
            if (loop_count % 3 == 0) {
                humano.x--;
            }
            if (humano.x <= 1) {
                humano.x = LARGURA - 2;
                humano.y = (rand() % (ALTURA - 6)) + 4;
            }
        }

        // --- SISTEMA DE COLISÕES ---
        
        // Submarino pega mergulhador
        if (humano.ativo && submarino.y == humano.y && (submarino.x <= humano.x && submarino.x + 2 >= humano.x)) {
            mergulhadores_salvos++;
            pontuacao += 100;
            // Respawna o mergulhador
            humano.x = LARGURA - 2;
            humano.y = (rand() % (ALTURA - 6)) + 4;
        }

        // Submarino bate no tubarão (Game Over)
        if (tubarao.ativo && submarino.y == tubarao.y && (submarino.x + 2 >= tubarao.x && submarino.x <= tubarao.x + 2)) {
            game_over = 1;
        }

        // Submarino volta à superfície
        if (submarino.y <= 3) {
            if (mergulhadores_salvos > 0) {
                pontuacao += mergulhadores_salvos * 200;
                mergulhadores_salvos = 0;
            }
            if (oxigenio < 100) oxigenio += 5;
        } else {
            // Consumo de oxigênio abaixo da água
            if (loop_count % 10 == 0) oxigenio--;
        }

        if (oxigenio <= 0) game_over = 1;

        // Existi um problema aqui
        // a função 'usleep' não funciona nativamente
        refresh();
        // usleep
        // a função sleep() no ncurses funciona diferente do que no windows.h
        // o certo é usar o napms()
        napms(50); // Ritmo do jogo (50ms)
        loop_count++;

        // --- CONTROLES ---
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (submarino.y > 1) submarino.y--;
                break;
            case KEY_DOWN:
                if (submarino.y < ALTURA - 1) submarino.y++;
                break;
            case KEY_LEFT:
                if (submarino.x > 1) submarino.x--;
                break;
            case KEY_RIGHT:
                if (submarino.x < LARGURA - 4) submarino.x++;
                break;
            case 'q': // Atalho para sair voluntariamente
                game_over = 1;
                break;
        }
    }

    // Tela de Game Over temporária antes de voltar ao menu
    nodelay(stdscr, FALSE);
    clear();
    desenhar_borda();
    mvprintw(ALTURA / 2 - 1, (LARGURA / 2) - 5, "GAME OVER");
    mvprintw(ALTURA / 2 + 1, (LARGURA / 2) - 9, "Pontuacao Final: %d", pontuacao);
    mvprintw(ALTURA / 2 + 3, (LARGURA / 2) - 14, "Pressione qualquer tecla");
    refresh();
    getch();
}

// Função principal main
int main(){
	// Iniciar a tela
	initscr();
	// 
	cbreak();
	// Não imprime as teclas digitadas pelo usuário
	noecho();
	// Ativa as teclas
	keypad(stdscr, TRUE);
	curs_set(0); // Esconde o cursor do terminal
	srand(time(NULL));

	menu();
	// Finalização
	endwin();

	return 0;
} // fim da função principal
