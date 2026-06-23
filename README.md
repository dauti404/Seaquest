# Seaquest

Jogo de Atari feito em linguagem C como componente avaliativo da disciplinas de Programação I

## Comandos

### Linux

No terminal do linux, para compilar o código basta digitar o seguinte comando:
```
~/local_onde_tá_salvo_o_jogo$  make
```
Para executar o jogo:
```
~/local_onde_tá_salvo_o_jogo$ ./bin/main
```
Obs.:Caso queira testar o jogo com WINE para rodar o arquivo .exe, primeria limpa a pasta do jogo com:
```
~/local_onde_tá_salvo_o_jogo$ make clean
```
E compila novamente com o comando:
```
~/local_onde_tá_salvo_o_jogo$ make windows=1
```
E executa com o WINE:
```
~/local_onde_tá_salvo_o_jogo$ wine main.exe
```

### Windows

Na IDE ou no cmd, para compilar o código, basta digitar o seguinte comando:
```
C:\local_onde_tá_salvo_o_jogo\ make windows=1
```
Para executar o jogo é só clicar no arquivo .exe ou pelo próprio cmd:
```
C:\local_onde_tá_salvo_o_jogo\main.exe
```
Para limpar a pasta do jogo basta digitar o comando:
```
C:\local_onde_tá_salvo_o_jogo\ make clean
```
