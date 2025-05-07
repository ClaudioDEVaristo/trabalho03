O principal objetivo do projeto é simular um sistema de semáforo utilizando a ferramenta FreeRTOS que é um sistema operacional de tempo real que permite a criação de multitarefas em sistemas embarcados. O projeto simula o semáforo em dois modos diferentes, o modo normal e o modo noturno. No modo normal, os leds agem como um semáforo comum ligando as cores verde -> amarela -> vermelha de forma sequencial, com a inclusão de efeitos sonoros para acessibilidade de forma que seja possível distinguir cada estado do semáforo. Já no modo noturno os leds ficam piscando de forma intermitente no amarelo para indicar que é possível a passagem, mas com atenção, devido ao horário, mantendo também os efeitos sonoros para indicar o estado noturno.

*ATENÇÃO*
Verificar o caminho do FreeRTOS no CMakeLists
