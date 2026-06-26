#ifndef MY_OS_H
#define MY_OS_H

// bloco para bibliotecas do Windows
#ifdef _WIN32
	// biblioteca para criação de telas no WINDOWS
	#include <windows.h>
	void screen_windows(){
		// pega o identificador da saída padrão (tela)
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		// altera a cor do texto para Verde (10)
		SetConsoleTextAttribute(hConsole, 10);
		printf("Esse texto esta em verde!\n");

		// Regra para o branco padrão (7)
		SetConsoleTextAttribute(hConsole, 7);
	}


// bloco de código para funções linux
#elif defined(__linux__)
	// biblioteca para criaçã de telas no LINUX
	#include <ncurses.h>
	// função para criação da tela do jogo
	void screen_linux(){
		initscr();
		keypad(stdscr, TRUE);
		// desativa o buffer da linha de terminal. Não é necesário clicar 'ENTER' para ler a tecla
		cbreak();
		// não mostra as teclas digitadas na tela
		noecho();
		refresh();
		endwin();
	}

// caso o OS não seja reconhecido
#else
	#error "Sistema Operacional NÂO suportado!"
#endif // fim do bloco de código

#endif // MY_OS_H