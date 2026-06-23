#ifndef MY_OS_H
#define MY_OS_H

// bloco para bibliotecas do Windows
#ifdef _WIN32
	// bibliotecas do windows para usar no código se necessário
	#include <windows.h>

	// define funções do windows
	// define - nome da função - função da biblioteca
	#define limpar_tela() system("cls")
	#define esperar_ms(ms) Sleep(ms)

// bloco de código para funções linux
#elif defined(__linux__)
	// bibliotecas do linux (POSIX) para uso no programa
	#include <unistd.h>
	#include <stdlib.h>

	// define funções do linux
	#define limpar_tela() system("clear")
	#define esperar_ms(ms) sleep((ms) * 1000)

// caso o OS não seja reconhecido
#else
	#error "Sistema Operacional NÂO suportado!"
#endif // fim do bloco de código

#endif // MY_OS_H