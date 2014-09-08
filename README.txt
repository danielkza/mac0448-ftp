Universidade de São Paulo
Intituto de Matemática e Estatística

Exercício Programa 1
MAC-0448/Programação para Redes de Computadores
Professor: Daniel Macêdo Batista
Setembro de 2014

Daniel Quadros de Miranda - Nº USP: 7577406
Rafael Marinaro Verona    - Nº USP: 7577323

== Sobre o programa ==

	O programa descrito por este arquivo implementa , em linguagem C, a interpretação e o processamento dos principais comandos de um servidor FTP.

	- Comandos:

	Os seguintes comandos foram implementados e são reconhecidos pelo programa:
	
	-> USER : Recebe e define um usuário.
	-> PASS : Recebe uma senha correspondente ao usuário previamente definido.
	-> PASV : Não recebe parâmetros. Solicita que o servidor entre em modo passivo, isto é, demanda que o servidor "escute" a porta de dados e espere uma conexão. Devolve, quando bem sucedido, o endereço do host e da porta.
	-> TYPE : Define o tipo de representação de dados a ser utilizado nas operações do servidor, recebe um parâmetro denotado por uma caracter.
	-> MODE : Define o modo de transferencia de dados a ser utilizado nas operações do servidor, recebe um parâmetro denotado por uma caracter.
	-> STRU : Define a estrutura do arquivo a ser manipulado nas operações do servidor, recebe um parâmetro denotado por uma caracter.
	-> QUIT : Desconecta o usuário e fecha a conexão.
	-> NOOP : Requisita uma resposta "OK" do servidor.
	-> RETR : Requisita ao servidor que transfira para o requerente uma cópia do arquivo especificado pelo caminho passado como parâmetro.
	-> STOR : Requisita ao servidor que armazene uma cópia do arquivo especificado pelo caminho passado como parâmetro.

	- Arquivos de código fonte:

	Os seguintes arquivos contém o código deste projeto:
	
	/* SOURCES */
	
	- picoftpd.c : Laço principal do programa, função main, tratamento da captura da entrada (comandos) e inicialização dos sockets.
	- state.c : Definição do estado atual.
	- commands.c : Declaração e tratamento dos comandos reconhecidos pelo servidor.
	- util.c : Re-implementação das funções 'malloc' e 'strdup' com o devido tratamento de erros.
	
	/* HEADERS */
	
	- state.h : Definição da struct 'ftp_state_t', que armazena o estado do servidor, e cabeçalho das funções implementadas em 'state.h'.
	- commands.h : Definição de struct 'ftp_command_t', que armazena o nome de um comando e um ponteiro para a função que o executa.
	- util.h: Contém o cabeçalho das funções implementadas em 'util.c'.
	
	- Observações:

	1. Não é verificada a autenticidade do usuário e da senha inseridas pela sequência de comandos USER <nome_de_usuário> e PASS <senha>.
	2. O comando TYPE tem como representeção padrão o formato 'ASCII', porém este servidor aceita apenas o tipo 'Image' e por isso não reconhece dois parâmetros.
	3. O comando MODE tem como padrão o modo 'Stream', que é o único suportado por este servidor.
	4. O comando STRU tem como padrão a estrutura 'File', que é a única suportada por este servidor.
	5. A implementação foi desenvolvida de modo a ser auto-explicativa, com nomes de funções e variáveis intuitivos.
	6. O prgrama foi compilado e testado utilazando as mesmas configurações disponíveis aqui em conjunto com o compilador GCC 4.7.2.
	
== Compilação ==

	-Programas necessários: autoconf, automake, compilador de C
	
	-Comandos para compilação:
	$ autoreconf -i
	$ ./configure
	$ make

	O executável será criado como `src/picoftpd` na pasta atual no momento da
execução do `configure`.

== Instruções de uso ==

	Para iniciar o servidor basta rodar o executável pela linha de comando passando dois parâmetros: 
	1º uma porta e 2º um IP (no formato IPv4) que serão atribuídos ao servidor. A seguir o servidor estará pronto para receber conexões e comandos de um usuário remoto.
