#include <stdio.h>
// inclui somente o header criado
#include "my_OS.h"

int main(){
	printf("Carregando o sistema...\n");

	// Roda nos dois OS com suas funções definidas (WINDOWS = Sleep ou LINUX = usleep)
	esperar_ms(20);

	// Roda nos dois OS com suas funções definidas (WINDOWS = cls ou LINUX = clear)
	limpar_tela();

	printf("Pronto! Este código rodou sem bugs aqui.\n");
	return 0;
}