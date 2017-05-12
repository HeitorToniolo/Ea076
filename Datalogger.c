/*    Heitor Toniolo          Ra: 155700
      Leonardo Fill Cardoso   RA: 156205
EA076 - Experimento 2 - Datalogger - 1s2017
Nesse experimento coletamos dados do ambiente 
através de um LDR (divisor de tensão) e também por um
sensor temperatura (NTC). Os dados coletados do LDR
podem ser salvos em uma EEPROM 24C16N para serem 
consultados depois, a comunicação com este CI foi feita
através da biblioteca AH_24Cxx.h, fundamentada na biblioteca
Wire.h para realizar a comunicação I2C entre o microcontrolador
e a EEPROM. A biblioteca AH_24Cxx.h implementa o protocolo de
comunicação do CI dessa família com o microcontrolador.
Para controle do datalogger utilizamos duas alternativas.
A primeira é um teclado matricial, cuja leitura é feita 
colocando sinais nas linhas e lendo as colunas de forma 
sequencial, onde existem 4 comandos, iniciados por # e 
terminados por *, onde podemos acender um LED de teste,
gravar uma medição, iniciar ou interromper medições automáticas,
que consistem em gravar uma leitura por segundo na memória e 
imprimir o valor da temperatura estimada.
A segunda alternativa é baseada na comunicação serial 
do computador com o microcontrolador. Esta alternativa apresenta
comandos como o "PING", que testa o sistema retornando um "PONG",
o "ID", que identifica o grupo, o "MEASURE", que realiza uma
medicao do LDR, o "MEMSTATUS", que retorna quantas posições de
memória estão ocupadas (número de leituras salvas), o "RESET", que
reseta a EEPROM, o "RECORD", que grava uma medição na memória e o
"GET N", que retorna a medição na n-ésima posição de memória, se 
existir algum dado nesta posição.
                                                */
//Bibliotecas utilizadas
#include "Wire.h"          
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "TimerOne.h"
#include <AH_24Cxx.h>
/* Pode-se encontrar a biblioteca AH_24Cxx.h em:
https://codebender.cc/sketch:275439#EEPROM_SWITCH.ino */

//Parte relacionada a EEPROM
#define AT24C16 4
AH_24Cxx ic_eeprom = AH_24Cxx(AT24C16, 0);

//Buffers para armazenamento de caracteres vindos do computador ou do teclado matricial
char buffer[100];
char buffer_teclado[10];

/* Variável associada a EEPROM, escolhido valor nulo por simplicidade,
poderia ser qualquer valor arbitrário entre 0 e 7 */
int eeprom_adress = 0; 

//Variáveis de estado do sistema, assumem valor 0 ou 1
int pisca_led = 0;
int led_state = 0;
int medicao_auto = 0;
int ready_to_read = 0;

/*Variável referente ao teclado matricial, assume diferentes valores que  
dependem de qual botão está pressionado*/
int leitura_teclado = -1;

/*Variáveis auxiliares para contagem.
  As variáveis de indice de buffer nos dão a posição correta a se trabalhar
  nos mesmos, visto que em nenhum momento do código, apagamos qualquer iformação
  armazenada neles. */
int indice_buffer_teclado = 0;
int buffer_indice = 0;
int n_measures = 0; 

// variáveis para NTC
/* Para o cáculo da temperatura pelo ntc, utilizamos o método do fator Beta,
onde nosso ponto de referência era a temperatura corporal de um dos membros
da dupla, a qual também supomos ser em torno de 36,5ºC.*/
float Rext = 1000.0;
float R0 = (float)(Rext + 1588.0); // Resistor em série com o NTC 222
/* O valor de Beta a seguir foi escolhido pela dupla após alguns testes
práticos, sendo que o fornecido pelo datasheet não era conveniente */
float Beta = 1300.0; 
float T0 = 36.5 + 273; // Temperatura de referência em Kelvin
// Link para verificação do método: https://www.ametherm.com/thermistor/ntc-thermistor-beta

/* Atribuição dos pinos do teclado matricial, LED, e sensores */
int linha0 = 2, linha1 = 3, linha2 = 4, linha3 = 5; 
int coluna0 = 6, coluna1 = 7, coluna2 = 8, coluna3 = 9;
int led_teste = 11;
int sensor_NTC = 4;
int sensor_LDR  = 3; 

char* string_de_identificacao = "EA076 - Heitor e Fill - Experimento 2"; 


/* Começo da seção para lidar com a comunicação entre microcontrolador e memória */

/*Função para lidar com a string do tipo GET N
  Aqui, verificamos se o comando utilizado pelo usuário realmente começa
  com "GET ", para assim, evitarmos erros de digitação. */
int is_get_n(){
  char buffer_aux[100];
  int i = 0; 
  while(buffer[i] != '\0'){
    buffer_aux[i] = buffer[i];
    i++;
    if(i == 4){
    buffer_aux[i] = '\0';
      if(strcmp(buffer_aux, "GET ") == 0){
        return 0;
      }
      else{
        return -1;
      }
    }
  }
  return -1;
}

/* Função que recebe o número dado por GET N e o transforma em um inteiro utilizável.
*/
int get_number(){ 
  char number[10];
  int i = 4;
  while(buffer[i] != '\0'){
    if(buffer[i] <= 57 && buffer[i] >= 48){ // Digitos de 0-9 estão entre dec(48) e dec(57) na tabela ascii
      number[i - 4] = buffer[i];
    }
    else{
      return -1;
    }
    i++;
  }
  return atoi(number); // Faz a conversão da string para um inteiro equivalente
}

/*A função a seguir verifica se a ação digitada pelo usuário no computador 
está de fato correta, caso esteja, realiza a atividade solicitada.*/
void matching_string(){
  int i;
  if(strcmp("PING", buffer) == 0){ //Retorna "PONG", para teste da comunicação
    Serial.println("PONG");
  }
  else if(strcmp("ID", buffer) == 0){ //Retorna a identificação da dupla
    Serial.println(string_de_identificacao);
  }
  else if(strcmp("MEASURE", buffer) == 0){//Realiza uma medida, mas não grava na memória
    Serial.println(analogRead(sensor_LDR));
  }
  else if(strcmp("MEMSTATUS", buffer) == 0){//Nos dá o número de medidas gravadas na memória
    Serial.println(n_measures);
  }
  else if(strcmp("RESET", buffer) == 0){//Reseta o número de medidas da memória 
    n_measures = 0;
  }
  else if(strcmp("RECORD", buffer) == 0){//Realiza uma medição e grava o valor na memória
    n_measures++;
    byte leitura_sensor = (byte)(analogRead(sensor_LDR ) >> 2);
    ic_eeprom.write_byte(n_measures, leitura_sensor);
  }
  else if(is_get_n() == 0){  //Verifica se o comando está correto, recebe o valor da memória e o printa, caso seja um valor válido.
    int N = get_number(); 
    if(N > 0 && N < n_measures + 1){ 
      Serial.println(ic_eeprom.read_byte(N)); 
    }
    else{
      Serial.println("Posição inválida da memória, tente novamente"); 
    }
  }
  buffer_indice = 0;
}

/* Começo da seção para lidar com o teclado */

/*Função responsável pela varredura dos pinos do teclado.
  Aqui, mandamos um sinal linha por linha do teclado e verificamos se alguma das 
  colunas recebe o mesmo, então podemos saber qual dos botões está pressionado,
  após isso, a função varredura() recebe um valor diferente para cada botão ou 
  um valor -1 caso essa condição não se satisfaça. */
int varredura(){
  int i,j; // i representará linhas e j colunas
  // nosso teclado é 4x4 e vamos variar i e j entre 0 e 3 (incluso) para representar os indices
  digitalWrite(linha0, LOW);
  digitalWrite(linha1, LOW);
  digitalWrite(linha2, LOW);
  digitalWrite(linha3, LOW);
  for(i = 0; i < 4; i++){
    if(i == 0){
      digitalWrite(linha0, HIGH);
      digitalWrite(linha1, LOW);
      digitalWrite(linha2, LOW);
      digitalWrite(linha3, LOW);
    }
    else if(i == 1){
      digitalWrite(linha0, LOW);
      digitalWrite(linha1, HIGH);
      digitalWrite(linha2, LOW);
      digitalWrite(linha3, LOW);
    }
    else if(i == 2){
      digitalWrite(linha0, LOW);
      digitalWrite(linha1, LOW);
      digitalWrite(linha2, HIGH);
      digitalWrite(linha3, LOW);
    }
    else if(i == 3){
      digitalWrite(linha0, LOW);
      digitalWrite(linha1, LOW);
      digitalWrite(linha2, LOW);
      digitalWrite(linha3, HIGH);
    }
    for(j = 0; j < 4; j++){
      if(i == 0){
        if(j == 0){
          if(digitalRead(coluna0) == HIGH){
            return 1;
          }
        }
        else if(j == 1){
          if(digitalRead(coluna1) == HIGH){
            return 2;
          }
        }
        else if(j == 2){
          if(digitalRead(coluna2) == HIGH){
            return 3;
          }
        }
        else if(j == 3){
          if(digitalRead(coluna3) == HIGH){
            return 12;
          }
        }
      }
      else if(i == 1){
        if(j == 0){
          if(digitalRead(coluna0) == HIGH){
            return 4;
          }
        }
        else if(j == 1){
          if(digitalRead(coluna1) == HIGH){
            return 5;
          }
        }
        else if(j == 2){
          if(digitalRead(coluna2) == HIGH){
            return 6;
          }
        }
        else if(j == 3){
          if(digitalRead(coluna3) == HIGH){
            return 13;
          }
        }
      }
      else if(i == 2){
        if(j == 0){
          if(digitalRead(coluna0) == HIGH){
            return 7;
          }
        }
        else if(j == 1){
          if(digitalRead(coluna1) == HIGH){
            return 8;
          }
        }
        else if(j == 2){
          if(digitalRead(coluna2) == HIGH){
            return 9;
          }
        }
        else if(j == 3){
          if(digitalRead(coluna3) == HIGH){
            return 14;
          }
        }
      }
      else if(i == 3){
        if(j == 0){
          if(digitalRead(coluna0) == HIGH){
            return 10;
          }
        }
        else if(j == 1){
          if(digitalRead(coluna1) == HIGH){
            return 0;
          }
        }
        else if(j == 2){
          if(digitalRead(coluna2) == HIGH){
            return 11;
          }
        }
        else if(j == 3){
          if(digitalRead(coluna3) == HIGH){
            return 15;
          }
        }
      }
    }
  }
  return -1;
}

/*Função que verifica através de comparações de strings, se o comando utilizado no teclado matricial
é de fato válido, caso seja, realiza a tarefa atribuida ao mesmo.
*/
void validacao_mensagem(){
  buffer_teclado[indice_buffer_teclado] = '\0'; // Caractere adicionado para comparação com outra string
  if(strcmp(buffer_teclado, "#1*") == 0){ //Pisca um led apenas para teste
    pisca_led = 1;    
  }
  else if(strcmp(buffer_teclado, "#2*") == 0){//Realiza uma medição com o sensor LDR
    n_measures++;
    byte leitura_sensor = (byte)(analogRead(sensor_LDR ) >> 2);
    ic_eeprom.write_byte(n_measures, leitura_sensor);
  }
  else if(strcmp(buffer_teclado, "#3*") == 0){//Inicia medição automática
    medicao_auto = 1;
  }
  else if(strcmp(buffer_teclado, "#4*") == 0){//Pausa medição automática
    medicao_auto = 0;
  }
  else{
    Serial.println("Comando do teclado inválido"); //Caso não seja um comando válido
  }
  indice_buffer_teclado = 0;
}

/*Função de interrupção, responsável pelo piscar do LED, bem como pela verificação
da medição automática.*/
void timer_interrupt_handler(){
  if(pisca_led == 1){
    if(led_state == 0){ // Está apagado
      digitalWrite(led_teste, LOW);
      led_state = 1;
    }
    else if(led_state == 1){ // Está aceso
      digitalWrite(led_teste, HIGH);
      led_state = 0;
    }
  }
  /*Caso o usuário acione a medição automática, a interrupção muda a variável que 
  é responsável por uma gravação e leitura de medida*/
  if(medicao_auto == 1){
      ready_to_read = 1; 
  }
}

void setup(){
  //Para controle do LED, que inicia apagado
  pinMode(led_teste, OUTPUT); /
  digitalWrite(led_teste, HIGH);
  /*Inicialização dos pinos do teclado matricial*/
  pinMode(linha0, OUTPUT);
  pinMode(linha1, OUTPUT);
  pinMode(linha2, OUTPUT);
  pinMode(linha3, OUTPUT);
  pinMode(coluna0, INPUT);
  pinMode(coluna1, INPUT);
  pinMode(coluna2, INPUT);
  pinMode(coluna3, INPUT);
  Serial.begin(9600); //Inicia porta serial, e seta o data rate para 9600bps
  Wire.begin(); //Inicializa biblioteca Wire
   /* Inicialização do timer para 1s e ligado a função de tratamento de interrupção timer_interrupt_handler que progamamos*/
  Timer1.initialize(1000000); // interrupção de 1s
  Timer1.attachInterrupt(timer_interrupt_handler);
}

void loop(){
  while(Serial.available() > 0){ //Recebe os caracteres do usuário e verifica se podem ser executados
    buffer[buffer_indice] = Serial.read();
    buffer_indice++;
    if(buffer[buffer_indice - 1] == '\n'){
      buffer[buffer_indice - 1] = '\0'; //caractere adicionado para comparação das strings
      matching_string();
    }
  }
  /*Faz a varredura do teclado e toma diferentes decisões conforme o resultado*/
  leitura_teclado = varredura();
  if(leitura_teclado != -1){ 
    // Caso seja válido, nas próximas linhas de códigos fazemos os mapeamentos dos números fornecidos em leitura nos símbolos do teclado
    if(leitura_teclado == 1){
      while(varredura() == leitura_teclado);
      buffer_teclado[indice_buffer_teclado] = '1';
      indice_buffer_teclado++; 
    }
    else if(leitura_teclado == 2){
      while(varredura() == leitura_teclado);
      buffer_teclado[indice_buffer_teclado] = '2';
      indice_buffer_teclado++; 
    }
    else if(leitura_teclado == 3){
      while(varredura() == leitura_teclado);
      buffer_teclado[indice_buffer_teclado] = '3';
      indice_buffer_teclado++; 
    }
    else if(leitura_teclado == 4){
      while(varredura() == leitura_teclado);
      buffer_teclado[indice_buffer_teclado] = '4';
      indice_buffer_teclado++; 
    }
    else if(leitura_teclado == 10){
      while(varredura() == leitura_teclado);
      buffer_teclado[indice_buffer_teclado] = '*';
      indice_buffer_teclado++; 
    }
    else if(leitura_teclado == 11){
      while(varredura() == leitura_teclado);
      buffer_teclado[indice_buffer_teclado] = '#';
      indice_buffer_teclado++; 
    }
    else{
      while(varredura() == leitura_teclado); // Esperamos o botao do teclado ser solto
      indice_buffer_teclado = 0;
    }
  }

  //Quando o usuário pressiona 3 botões do teclado matricial seguidos, o programa verifica se é um comando válido
  if(indice_buffer_teclado == 3){//Quando o usuário pressiona 3 botões do teclado matricial seguidos, 
    validacao_mensagem();
  }
  
  /*Caso esteja pronto, realiza uma medida de temperatura e grava a mesma na memória, e como parte extre do projeto,
  imprime a cada um segundo, o valor da temperatura do NTC.*/  
  if(ready_to_read == 1){
    ready_to_read = 0;
    if(n_measures < 2047){ //Para não ultrapassarmos o limite da EEPROM
        n_measures++;
        /*A leitura do sensor LDR pelo microcontrolador é em 10 bits, porém a EEPROM é em 8 bits, por isso é necessária 
        a conversão.*/
        byte leitura_sensor = (byte)(analogRead(sensor_LDR) >> 2); 
        ic_eeprom.write_byte(n_measures, leitura_sensor);
      }
      else{
        Serial.println("Memória estorou!!!");
      }
      // Parte extra -> Fazemos leitura de um sensor de temperatura e imprimimos o valor da temperatura estimado
      int leitura_NTC = analogRead(sensor_NTC);
      //Os calculos a seguir são do Método do fator Beta, para cálculo da temperatura com termistores
      float R = ((float)(1023 - leitura_NTC))/((float)(leitura_NTC)/Rext);
      R += Rext; // considera a resistencia em serie com o NTC
      float T = (float)(1.0/(1.0/T0 + (1.0/Beta)*(float)log(R/R0)));
      Serial.println(T - 273.0); //Printa a temperatura em graus Celsius
  }
}


