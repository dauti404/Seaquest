#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// inclui somente o header criado
#include "my_OS.h"

// option
#define HEIGHT 20
#define WIDTH 50
#define SEA "🌊"

#define PEOPLE "🤸"
#define FISH "🦈"
#define SUBMARINE
#define ENEMY_SUBMARINE

// entidade submarino
//tyṕedef struct{
	
//} Submarine;

int main(){
	// Para dispositivos WINDOWS
	#ifdef _WIN32
		screen_windows();

	// Para dispositivos LINUX
	#else
		// função do HEADER que desenha a tela
		screen_linux();

		// função que inicia a tela
		initscr();

		// cria a tela do jogo
		for(int i = 0; i < HEIGHT; i++){
			for(int j = 0; j < WIDTH; j++){
				printf(SEA);
			}
			printf("\n");
		}

		// fecha a tela
		endwin();
	#endif

	return 0;
}