// Ativa os recursos padrão do POSIX para o uso do usleep no Linux
#define _DEFAULT_SOURCE
// bibliotecas gerais
#include <stdlib.h>
#include <time.h>

// Defininções de SO
#if defined(_WIN32) || defined(__WIN32__)
    #include <curses.h>   // No Windows
    #include <windows.h>  // Necessário para a função Sleep do Windows para o jogo funcionar
    #define ESPERAR_MS(ms) Sleep(ms)
#else
    #include <ncurses.h>  // No Linux
    #include <unistd.h>   // Necessário para o usleep do Linux
    #define ESPERAR_MS(ms) usleep((ms) * 1000) // Converte milissegundos para microsegundos
#endif

// difinições da tela do jogo
#define ALTURA 30
#define LARGURA 100

// Entidades
typedef struct{
    // variáveis para guarda as coordenadas cartesianas do Jogador
    // em jogos 2d e de terminal, as coordenadas cartesianas são invertidas em relação a da matemática tradicional, mas funcionam de formas semelhantes
    // o x guarda a posição horizontal do jogador. x + 1 = move para direita e x - 1 = move para esquerda
    // o y guarda a posição vertical do jogador. y + 1 = move para baixo e y - 1 = move para cima
	int x, y;
} Player;

typedef struct{
	int x, y;
	int ativo;
	// muda a direção de spawn da entidade. Variável para guarda da posição do jogador
	int direcao;
} Inimigo;

typedef struct{
	int x, y;
	int ativo;
	// muda a direção de spawn da entidade
	int direcao;
} Mergulador;

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
            // a tecla Enter é capturado com código 13 no Windows/PDCurses (CR)
            #if defined(_WIN32)
                case 13:
            #endif
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
			'#');
		// Linha final
        mvaddch(
			// Valor da altura
			ALTURA, 
			// Linha final de caracteres
			i, 
			// Caracter desejado ACSII
			'#');
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

    // o LARGURA - 4, garante uma folga no tela para evitar desenhar fora da tela as entidades
    // as entidades tem um limite de 4 caracteres para o tamanho
     
    // essas linhas são inicializadores dos structs definidos das entidades
    Player submarino = {10, ALTURA / 2}; // posição inicial do jogador. Eixo X e Eixo Y respectivamente
    Inimigo tubarao = {LARGURA - 4, 8, 1, -1}; // posição inicial X e Y, o (1) velocidade/direção para a direita e o (-1) altura no eixo Y.
    Mergulador humano = {2, 14, 1, 1}; // posição inicial X e Y, o (1) velocidade/direção para direita e o (1) direção no eixo y para baixo
    Inimigo sub_enemy = {LARGURA - 2, 5, 1, -1};

    // variáveis do jogo
    int pontuacao = 0;
    int mergulhadores_salvos = 0;
    int oxigenio = 100;
    int game_over = 0;
    int loop_count = 0;

    while (!game_over) {
        clear();
        desenhar_borda();
        
        // Interface de Status
        //mvprintw(0, 2, " Pontos: %d | Mergulhadores: %d | Oxigenio: %d%% ", pontuacao, mergulhadores_salvos, oxigenio);
        mvprintw(0, 25, "Pontos: %d | Mergulhadores: %d | Oxigenio: ", pontuacao, mergulhadores_salvos);
                
        // Define a cor com base no nível de oxigênio
        int par_cor = 1; // Verde padrão
        if (oxigenio <= 20) {
            par_cor = 3; // Vermelho crítico
        } else if (oxigenio <= 50) {
            par_cor = 2; // Amarelo alerta
        }

        // Ativa a cor escolhida
        attron(COLOR_PAIR(par_cor));

        // Desenha a barra visual (com tamanho máximo de 10 caracteres)
        // Cada caractere 'X' ou '#' representa 10% de oxigênio
        int tamanho_barra = oxigenio / 10;
        mvprintw(0, 65, "[");
        for (int b = 0; b < 10; b++) {
            if (b < tamanho_barra) {
                printw("#"); // Parte cheia da barra (pode usar 'X' ou 'O' se preferir)
            } else {
                printw(" "); // Parte vazia da barra
            }
        }
        printw("] %d%%", oxigenio);

        // Desativa a cor para não pintar o resto do jogo
        attroff(COLOR_PAIR(par_cor));
        // bloco de definição da barra de oxigênio
        
        // Linha da superfície da água
        mvprintw(3, 1, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

        // Desenha o Jogador (Submarino)
        mvprintw(submarino.y, submarino.x, "=0=");

        // Movimenta e Desenha o Tubarão
        if (tubarao.ativo) {
            // sprite do tubarão
            // defini qual tubarão vai ser spawnado
            if(tubarao.direcao == -1){
                mvprintw(tubarao.y, tubarao.x, "<><"); // da direita para esquerda
            } else {
                mvprintw(tubarao.y, tubarao.x, "><>"); // da esquerda para direita
            }
            // Velocidade do tubarão
            // loop de movimentação do personagem, a variável conta os frames do jogo e divide por 2
            // se o resto da divisão for diferente de 0, o personagem fica parado
            // se o resto da divisão for igual a 0, o personagem dá um 1 passo
            if (loop_count % 2 == 0) { 
                // muda a direção que ele tá andando automaticamente
                tubarao.x += tubarao.direcao;
            }
            // verifica a direção do tubarão
            if(tubarao.x <= 1 || tubarao.x >= LARGURA - 4){
                // sorteia a sua direção
                // moeda aleátoria que o jogo tira quando o tubarão sai de tela
                tubarao.direcao = (rand() % 2 == 0) ? -1 : 1;
                // reposiciona o tubarão na tela
                if(tubarao.direcao == -1){
                    // spawna na direita
                    tubarao.x = LARGURA - 4;
                } else {
                    // spawna na esquerda
                    tubarao.x = 2;
                }
                // altura aleatória do tubarão
                tubarao.y = (rand() % (ALTURA - 6)) + 4;
            }
        } // fim do bloco de código do tubarão

        // Cria o submarino inimigo
        if (sub_enemy.ativo) {
            // sprite do submarino inimigo
            // defini qual submarino inimigo vai ser spawnado
            if(sub_enemy.direcao == -1){
                mvprintw(sub_enemy.y, sub_enemy.x, "=0+"); // da direita para esquerda
            } else {
                mvprintw(sub_enemy.y, sub_enemy.x, "+0="); // da esquerda para direita
            }
            // Velocidade do submarino inimigo
            // loop de movimentação do personagem, a variável conta os frames do jogo e divide por 2
            // se o resto da divisão for diferente de 0, o personagem fica parado
            // se o resto da divisão for igual a 0, o personagem dá um 1 passo
            if (loop_count % 3 == 0) { 
                // muda a direção que ele tá andando automaticamente
                sub_enemy.x += sub_enemy.direcao;
            }
            // verifica a direção do submarino inimigo
            if(sub_enemy.x <= 1 || sub_enemy.x >= LARGURA - 4){
                // sorteia a sua direção
                // moeda aleátoria que o jogo tira quando o tubarão sai de tela
                sub_enemy.direcao = (rand() % 2 == 0) ? -1 : 1;
                // reposiciona o submarino inimigo na tela
                if(sub_enemy.direcao == -1){
                    // spawna na direita
                    sub_enemy.x = LARGURA - 4;
                } else {
                    // spawna na esquerda
                    sub_enemy.x = 2;
                }
                // altura aleatória do submarino inimigo
                sub_enemy.y = (rand() % (ALTURA - 6)) + 4;
            }
        } // fim do bloco de código do submarino inimigo
    
        // Movimenta e Desenha o Mergulhador humano
        if (humano.ativo) {
            // sprite do mergulhador
            mvprintw(humano.y, humano.x, "oOo");
            // loop de movimentação do personagem, a variável conta os frames do jogo e divide por 3
            // se o resto da divisão for diferente de 0, o personagem fica parado
            // se o resto da divisão for igual a 0, o personagem dá um 1 passo
            if (loop_count % 3 == 0) { 
                humano.x += humano.direcao;
            }
            // verifica se ele já saiu da tela
            if (humano.x <= 1 || humano.x >= LARGURA - 2) {
                // moeda de troca
                // sorteia a posição que o mergulhador irá surgi na tela
                humano.direcao = (rand() % 2 == 0) ? -1 : 1;
                // reposiciona o humano na tela
                if(humano.direcao == -1){
                    humano.x = LARGURA - 2; // spawna na direita
                } else{
                  humano.x = 2; // spawna na esquerda  
                }
                humano.y = (rand() % (ALTURA - 6)) + 4; // determina o spawn aleátorio do humano
            }
        } // fim do bloco de código do humano

        // Sistema de colisão com base no tamanho dos sprites das entidades
        // Submarino pega mergulhador
        if (humano.ativo && submarino.y == humano.y && (submarino.x + 2 >= humano.x && submarino.x <= humano.x + 2)) {
            mergulhadores_salvos++;
            pontuacao += 100;
            // Respawna o mergulhador em um lugar aleatório
            humano.direcao = (rand() % 2 ==0) ? -1 : 1;
            humano.x = (humano.direcao == -1) ? LARGURA -2 : 2;
            humano.y = (rand() % (ALTURA - 6)) + 4;
        }

        // Submarino bate no tubarão (Game Over)
        if (tubarao.ativo && submarino.y == tubarao.y && (submarino.x + 2 >= tubarao.x && submarino.x <= tubarao.x + 2)) {
            game_over = 1;
        }

        if (sub_enemy.ativo && submarino.y == sub_enemy.y && (submarino.x + 2 >= sub_enemy.x && submarino.x <= sub_enemy.x + 2)) {
            game_over = 1;
        }
        // fim do sistema de colisão 

        // Submarino volta à superfície
        if (submarino.y <= 3) {
            if (mergulhadores_salvos > 0) {
                pontuacao += mergulhadores_salvos * 200;
                mergulhadores_salvos = 0;
            }
            // preenche o oxigênio
            if (oxigenio < 100) oxigenio += 1;
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

        // Controles
        int ch;
        while ((ch = getch()) != ERR) { //ERR é o valor que o getchar retorna quando nao há mais nada precionado, entao o while so funciona caso o botao seja precionado
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
                case 'q':
                    game_over = 1;
                    break;
            }
        } // fim do while que le o teclado

    }

    // Tela de Game Over temporária antes de voltar ao menu
    // (agora roda so 1 vez, fora do loop do jogo, depois que game_over vira 1)
    nodelay(stdscr, FALSE);
    clear();
    desenhar_borda();
    // definições
    mvprintw(
        // Eixo Y
        ALTURA / 2 - 1, 
        // Eixo X centralizado (metade do tamanho da string)
        (LARGURA / 2) - 5, "GAME OVER");
    mvprintw(ALTURA / 2 + 1, (LARGURA / 2) - 9, "Pontuacao Final: %d", pontuacao);
    mvprintw(ALTURA / 2 + 3, (LARGURA / 2) - 14, "Pressione qualquer tecla");
    refresh();
    getch();

}

// Função principal main
int main(){
	initscr(); // Iniciar a tela
    resize_term(ALTURA + 2, LARGURA + 2);  // garante que a janela seja grande o suficiente
	cbreak(); // ativa o modo (quebra de linha)
	noecho(); // Não imprime as teclas digitadas pelo usuário
	keypad(stdscr, TRUE); // Ativa as teclas
	curs_set(0); // Esconde o cursor do terminal
	srand(time(NULL)); // faz os números gerados aleatoriamente sejam sempre diferentes

	// bloco de código para as cores
	if (has_colors()) {
        start_color();
        // Par 1: Verde para Oxigênio Alto (acima de 50%)
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        // Par 2: Amarelo para Oxigênio Médio (entre 21% e 50%)
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        // Par 3: Vermelho para Oxigênio Crítico (20% ou menos)
        init_pair(3, COLOR_RED, COLOR_BLACK);
    } // fim do bloco de código
	
	menu(); // abre o menu do jogo
	endwin(); // fecha a tela do jogo
	return 0;
} // fim da função principal