//!!!!!!!!!!!!!PALAVRAS!!!!!!!!!!!!!!!
//    sorteve       |      programar
//    stroeve       |      rrogramap
//    steoevr       |      rragramop 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>

#define QTD_PALAVRAS 2

#define DISPLAY_1    1
#define DISPLAY_2    2
#define DISPLAY_3    3
#define DISPLAY_4    4
#define SWITCHES     5
#define BUTTONS      6
#define GREENLEDS    7
#define REDLEDS      8

int flag = 0;

int Vetor_Switches_valor[18] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072};

unsigned char hexdigit[] = {0x3F, 0x06, 0x5B, 0x4F,
                            0x66, 0x6D, 0x7D, 0x07, 
                            0x7F, 0x6F, 0x77, 0x7C,
                            0x39, 0x5E, 0x79, 0x71};


int display_1, display_2, display_3, display_4;

int Vetor_Indices[QTD_PALAVRAS] = {-1};

int palavras_jogadas = 0;

typedef struct{
  char data[50]; //palavra
  int plays;
  int tam;
}Word;

char Palavras_Embaralhadas[QTD_PALAVRAS][50] = { 
                                                  {"steoevr\0"},
                                                  {"rragramop\0"}
                                                };

char Palavras_Corretas[QTD_PALAVRAS][50] =     {
                                                  {"sorvete\0"},
                                                  {"programar\0"}
                                               };

void write_twice(int dev, uint32_t *data, int codigo){
  if(codigo == GREENLEDS){ 
    write(dev, data, codigo);
    if(flag==0){
      write(dev, data, codigo);
      flag=1;
    }
  }else{
    write(dev, data, codigo); 
    write(dev, data, codigo);
  }
}
   
void Apresentation_Menu(int dev){
  uint32_t leitura = 0;
  int retorno;
  
  printf("Ola, bem vindo a nossa aplicacao!\n");
  printf("Venha se divertir e treinar o quao bom (ou nao) voce eh em enigmas e criptografias\n");
  printf("Aperte o primeiro botao para comecar!\n");

  do{
    retorno = read(dev, &leitura, BUTTONS);
  }while(leitura != 7);
}


void Zerar_displays(int dev){
  int zero = ~hexdigit[0];

  write_twice(dev, &zero, DISPLAY_1);

  write_twice(dev, &zero, DISPLAY_2);
  
  write_twice(dev, &zero, DISPLAY_3);

  write_twice(dev, &zero, DISPLAY_4);
}

void pontuacao(int dev, char *strParcial, char *strOk, int tam){
    int i;
    double score = 0;
    double parcial;
    int led0 = 0b0, led1 = 0b00000001, led2 = 0b00000011, led3 = 0b00000111, led4 = 0b00001111, led5 = 0b00011111, led6 = 0b00111111, led7 = 0b01111111, led8 = 0b11111111;
    
    for(i=0;i<tam;i++){
        if(strParcial[i] == strOk[i]){
            score++;
        }
    }
 
    parcial = (double) (score/tam)*100;

    if(parcial > 0 && parcial <= 12.5){
        write_twice(dev, &led1, GREENLEDS);
    }else if(parcial > 12.5 && parcial <= 25){
        write_twice(dev, &led2, GREENLEDS);
    }else if(parcial > 25 && parcial <= 37.5){
        write_twice(dev, &led3, GREENLEDS);
    }else if(parcial > 37.5 && parcial <= 50){
        write_twice(dev, &led4, GREENLEDS);
    }else if(parcial > 50 && parcial <= 62.5){
        write_twice(dev, &led5, GREENLEDS);
    }else if(parcial > 62.5 && parcial <= 75){
        write_twice(dev, &led6, GREENLEDS);
    }else if(parcial > 75 && parcial < 100){
        write_twice(dev, &led7, GREENLEDS);
    }else if(parcial == 100){
        write_twice(dev, &led8, GREENLEDS);
    }else{
        write_twice(dev, &led0, GREENLEDS);
    }
}


void Leitura_opt(int *opt, int dev){
  uint32_t leitura = 0;
  int retorno;

  printf("Digite uma das seguintes opcoes abaixo:\n");
  printf("----------- OPERACAO ------------\n");
  printf("Botao 1 para Play!\n");
  printf("Botao 2 para Regras do jogo\n");
  printf("Botao 3 para Sair\n");
  printf("OPCAO: ");

  do{
    retorno = read(dev, &leitura, BUTTONS);
  }while(leitura != 7 && leitura != 11 && leitura != 13);

  switch(leitura){
    case 7:
      (*opt) = 1;
      break;
    case 11:
      (*opt) = 2;
      break;
    case 13:
      (*opt) = 3;
      break;
  }
}

void show_rules(){
  printf("As regras do jogo sao as seguintes:\n");
  printf("Voce recebera uma palavra ou frase criptografada\n");
  printf("Seu objetivo sera, trocando as posicoes das letras de forma gradativa, encontrar a mensagem original\n");
  printf("Para isso, voce tera um numero limite de trocas de posicao (entre as letras) que poderao ser feitas\n");
  printf("Esse numero estara aparecendo no 7 segmentos da placa. A cada jogada, esse numero diminuira em uma unidade\n");
  printf("Para informar as posicoes das letras a serem trocadas, com os switches informe primeiro a posicao da primeira letra\n");
  printf("Em seguida, aperte o primeiro botao (para informar que a primeira posicao ja foi selecionada) e informe a segunda posicao, e aperte o segundo botao\n");
  printf("Automaticamente, as letras serao trocadas nas posicoes que voce informou.\n");
  printf("A cada vez que voce chegar mais perto da palavra original, os leds irao acendendo gradativamente, informando sua situacao\n");
  printf("Caso o numero de jogadas acabe (e mesmo que voce saiba qual a mensagem), mas a frase nao estiver montada: Game Over!\n");
  printf("Enjoy!\n\n");
}

void Acertou(int indice){
  printf("Parabens, voce descobriu a palavra correta!\n");
  printf("---------- %s ----------\n", Palavras_Corretas[indice]);
}

void Errou(int indice){
  printf("Infelizmente voce nao descobriu a palavra correta! =(\n");
  printf("---------- %s ----------\n", Palavras_Corretas[indice]);
}

void ContinuaJogando(int dev, int *end){
  uint32_t leitura = 0;
  int retorno;

  printf("Voce deseja continuar jogando?\n");
  printf("Botao 1 - Sim\n");
  printf("Botao 2 - Nao\n");
  printf("Opcao: ");

  do{
    retorno = read(dev, &leitura, BUTTONS);
  }while(leitura != 7 && leitura != 11);

  if(leitura == 7) (*end) = 1;
  else{
    if(leitura == 11) (*end) = 0;
  }

  printf("\n");
}


int translate_position(int leitura){
  int pos = 0;

  switch(leitura){
    case 1:
      pos = 17;
      break;
    case 2:
      pos = 16;
      break;
    case 4:
      pos = 15;
      break;
    case 8:
      pos = 14;
      break;
    case 16:
      pos = 13;
      break;
    case 32:
      pos = 12;
      break;
    case 64:
      pos = 11;
      break;
    case 128:
      pos = 10;
      break;
    case 256:
      pos = 9;
      break;
    case 512:
      pos = 8;
      break;  
    case 1024:
      pos = 7;
      break;
    case 2048:
      pos = 6;
      break;
    case 4096:
      pos = 5;
      break;
    case 8192:
      pos = 4;
      break;
    case 16384:
      pos = 3;
      break;
    case 32768:
      pos = 2;
      break;
    case 65536:
      pos = 1;
      break;
    case 131072:
      pos = 0;
      break;
    default: 
      pos = -1;
      break;
  }
  return pos;
}


int Leitura_posicao(int dev, int tam){

  uint32_t leitura = 0, pos = 0;
  int retorno;

  do{
    retorno = read(dev, &leitura, SWITCHES); //lendo valor dos switches
  }while(leitura <= 0 || leitura > 131072);

  pos = translate_position(leitura);

  if(pos < 0 || pos > tam){
    printf("Posicao invalida! Tente novamente\n");
    do{
      //leitura dos valores
      do{
        pos = read(dev, &leitura, SWITCHES); //lendo valor dos switches
      }while(leitura <= 0 || leitura > 131072);
      
      pos = translate_position(leitura);

    }while(pos < 0 || pos > tam); //condicao do valor retornado seguir a mesma coisa da condional de cima
  }
  return pos;
}


void leitura_botao(int dev){
  uint32_t leitura = 0;
  int retorno;

  do{
    retorno = read(dev, &leitura, BUTTONS);
  }while(leitura != 7 && leitura != 11); //sai daqui caso aperte o primeiro ou o segundo botao
}


void Faz_troca(int pos1, int pos2, char *string){
  char aux_troca;

  aux_troca = string[pos1];
  string[pos1] = string[pos2];
  string[pos2] = aux_troca;
} 
 
int Get_indice(){
  int i = 0, end = 1, indice = 0;

  while(end){

    indice = rand() % QTD_PALAVRAS;

    for(i = 0; i<QTD_PALAVRAS; i++){
      if(indice == Vetor_Indices[i]){
        break;
      }
    }
    if(i == QTD_PALAVRAS){
      end = 0;
      Vetor_Indices[indice] = indice;
    }

  }
  return indice;
}

void Escreve_displays(int dev, int qtd){
  int resto, quociente;

  if(qtd < 10){

    display_1 = ~hexdigit[qtd];

    write_twice(dev, &display_1, DISPLAY_1);
    //write(dev, &display_1, DISPLAY_1);

    display_2 = ~hexdigit[0];

    write_twice(dev, &display_2, DISPLAY_2);
    //write(dev, &display_2, DISPLAY_2);

  }else{

    resto = qtd % 10; //resto da divisao por 10 esta em resto
    quociente = qtd / 10;

    display_1 = ~hexdigit[resto];
    
    write_twice(dev, &display_1, DISPLAY_1);
    //write(dev, &display_1, DISPLAY_1);

    display_2 = ~hexdigit[quociente];
    
    //write_twice(dev, &display_2, DISPLAY_2);
    write(dev, &display_2, DISPLAY_2);

  }
}


void set_leds(int dev, int tam){
  int i = 0, valor = 0;

  for(i = 17; i>17-tam; i--){
    valor += Vetor_Switches_valor[i];
  }

  write(dev, &valor, REDLEDS);

}


void Application(int dev){
  int pos1 = 0, pos2 = 0, indice, end = 1; //definir quantidade de jogadas
  Word message;

  while(end){

    indice = Get_indice();
    strcpy(message.data, Palavras_Embaralhadas[indice]); //copiando uma frase

    message.tam = strlen(message.data);
    message.plays = message.tam - 1;
    //message.plays = 12;


    set_leds(dev, message.tam);

    while(message.plays > 0){
      //envia esse valor de qtd de jogadas para o 7 segmentos
      Escreve_displays(dev, message.plays);
      pontuacao(dev, message.data, Palavras_Corretas[indice], message.tam);

      printf("Quantidade de jogadas: %d\n", message.plays);
      //seta os leds

      printf("MENSAGEM: ");
      printf("%s\n", message.data);

      printf("Digite a primeira posicao a ser lida:\n");
      //leitura do valor no switch

      leitura_botao(dev);
      pos1 = Leitura_posicao(dev, message.tam-1);
      printf("Posicao 1 lida\n");

      printf("Agora, digite o segundo valor a ser lido!\n");

      leitura_botao(dev);
      pos2 = Leitura_posicao(dev, message.tam-1);
      printf("posicao 2 lida\n");

      Faz_troca(pos1, pos2, message.data);

      message.plays--;
      printf("\n");

      if(strcmp(message.data, Palavras_Corretas[indice]) == 0){
        message.plays = 0;
        Acertou(indice);
      }
      if(message.plays == 0 && strcmp(message.data, Palavras_Corretas[indice]) != 0){
        Errou(indice);
      }
    }

    palavras_jogadas++;
    if(palavras_jogadas == QTD_PALAVRAS){
      end = 0;
    }
    else{
      ContinuaJogando(dev, &end);
    }
  }
}

int main(){
  int dev = open("/dev/de2i150_altera", O_RDWR);

  int opt = 0, end_menu = 1;

  Zerar_displays(dev);
  Apresentation_Menu(dev);

  //leitura dos botoes e atribuições dos valores lidos as respectivas condicionais
  while(end_menu != 0){

    Leitura_opt(&opt, dev);

    if(opt == 1){ //Play
      Application(dev);
    }else{
      if(opt == 2){
        show_rules();
      }
      else{
        if(opt == 3){
          end_menu = 0;
        }
        else{
          printf("Opcao invalida\n");
        }
      }
    }
  }
  close(dev);
  return 0;
}