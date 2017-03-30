#include "TimerOne.h"
int botao_apertado = 0; /* 0 -> Botão solto */
                        /* 1 -> Botão apertado  */
/* Variavéis de sincronização para fechar o semaforo */
/* Variavéis com total serão proporcionais ao tempo que algo vai acontecer (em dezenas de ms) */
/* Variavéis com cont são auxiliares para contagem */
int tempo_verde_total = 600;
int tempo_verde_cont  = 0;
int tempo_amarelo_total = 400;
int tempo_amarelo_cont  = 0;
int tempo_vermelho_total = 1000;
int tempo_vermelho_cont  = 0;
int tempo_amarelo_pisca_total = 50;
int tempo_amarelo_pisca_cont  = 0;
int tempo_vermelho_pisca_total = 10;
int tempo_vermelho_pisca_cont  = 0;

/* Variavéis de sincronização do LDR e do buzzer*/
int ja_e_noite = 0; /* flag que indica o estado do sistema (0 - dia e 1 - noite) */
int LDR_total = 300;
int LDR_cont  = 0;
int LDR_limite_inferior = 700; /* Valores da faixa de tensão que lemos do LDR pelo conversor A\D */
int LDR_limite_superior = 1023;
int tempo_buzzer_total = 20; 
int tempo_buzzer_cont = 0;

/* Atribuição dos pinos */
/* Os pinos dos semáforos e do botão devem ser ativos BAIXO -
no botão isso significa utilizar um resistor de pull up e nos LEDs conectar o cátodo(negativo) ao pino de controle*/
int semaforo_vermelho = 13;
int semaforo_amarelo = 12;
int semaforo_verde = 11;
int pedestre_verde = 10;
int pedestre_vermelho = 9;
int botao = 8;
int buzzer = 7;
/* os pinos data são utilizados no conversor BCD para o display de 7 segmentos */
int data0 = 6;
int data1 = 5;
int data2 = 4;
int data3 = 3;
/* O pino para o LDR é um pino com leitor A\D que tem numeração independente dos pinos digitais - 
por isso podemos utilizar o mesmo numero para o data3 e o LDR_input*/
int LDR_input = 3;

/* as funções de zero a nove modificam as variavéis data (ligadas ao conversor BCD) para que o número correspondente apareça do display */
void zero(){
  digitalWrite(data0, LOW);
  digitalWrite(data1, LOW);
  digitalWrite(data2, LOW);
  digitalWrite(data3, LOW);
}
void um(){
  digitalWrite(data0, HIGH);
  digitalWrite(data1, LOW);
  digitalWrite(data2, LOW);
  digitalWrite(data3, LOW);
}
void dois(){
  digitalWrite(data0, LOW);
  digitalWrite(data1, HIGH);
  digitalWrite(data2, LOW);
  digitalWrite(data3, LOW);
}
void tres(){
  digitalWrite(data0, HIGH);
  digitalWrite(data1, HIGH);
  digitalWrite(data2, LOW);
  digitalWrite(data3, LOW);
}
void quatro(){
  digitalWrite(data0, LOW);
  digitalWrite(data1, LOW);
  digitalWrite(data2, HIGH);
  digitalWrite(data3, LOW);
}
void cinco(){
  digitalWrite(data0, HIGH);
  digitalWrite(data1, LOW);
  digitalWrite(data2, HIGH);
  digitalWrite(data3, LOW);
}
void seis(){
  digitalWrite(data0, LOW);
  digitalWrite(data1, HIGH);
  digitalWrite(data2, HIGH);
  digitalWrite(data3, LOW);
}
void sete(){
  digitalWrite(data0, HIGH);
  digitalWrite(data1, HIGH);
  digitalWrite(data2, HIGH);
  digitalWrite(data3, LOW);
}
void oito(){
  digitalWrite(data0, LOW);
  digitalWrite(data1, LOW);
  digitalWrite(data2, LOW);
  digitalWrite(data3, HIGH);
}
void nove(){
  digitalWrite(data0, HIGH);
  digitalWrite(data1, LOW);
  digitalWrite(data2, LOW);
  digitalWrite(data3, HIGH);
}

/* Função de tratamento de interrupção do timer */
/* Aqui iremos realizar a maioria das tarefas do sistema */
/* 1-) Devemos identificar o estado do sistema - Dia vs Noite */
/* 2-) Devemos testar se o botão foi apertado ou não */
/* 3-) Vamos ler o estado do LDR através da função analogRead(pino_de_leitura) */
/* 4-) Tomar decisões relacionadas ao estado do sistema. Ex: Incrementar variavéis, apagar/acender LEDs, ...*/
void timer_interrupt_handler(){
  if(ja_e_noite == 0){ /* teste 1 - Dia vs Noite */
    if(botao_apertado == 1){ /* teste 2 - Botão apertado ou não */
      /* Contagem do sinal verde, após o botão ter sido pressionado */
      /* Troca semaforo de verde pra amarelo */
      if(tempo_verde_cont < tempo_verde_total){
        tempo_verde_cont++;
        if(tempo_verde_cont == tempo_verde_total){
          digitalWrite(semaforo_verde, HIGH);
          digitalWrite(semaforo_amarelo, LOW);
        }
      }
      else{
        /* Terminada a contagem do verde, inicia a contagem do amarelo */
        /* Troca semaforo de amarelo para vermelho e pedestre de vermelho para verde*/
        if(tempo_amarelo_cont < tempo_amarelo_total){
          tempo_amarelo_cont++;
          if(tempo_amarelo_cont == tempo_amarelo_total){
            digitalWrite(semaforo_amarelo, HIGH);
            digitalWrite(semaforo_vermelho, LOW);
            digitalWrite(pedestre_verde, LOW);
            digitalWrite(pedestre_vermelho, HIGH);
          }
        }
        else{
          /* Contagem do vermelho, alerta pedestre, display 7 segmentos e buzzer */
          /* Tempo total no vermelho foi definido em 10s */
          if(tempo_vermelho_cont < tempo_vermelho_total){
            tempo_vermelho_cont++;
            /* Contagem no display de 7 segmentos de 9 até 1, pela definição do tempo_vermelho_total -> se o tempo fosse alterado seria necessário alterar o código */
            if(tempo_vermelho_cont <= 1*tempo_vermelho_total/10){
              nove();
            }
            else if(tempo_vermelho_cont <= 2*tempo_vermelho_total/10){
              oito();
            }
            else if(tempo_vermelho_cont <= 3*tempo_vermelho_total/10){
              sete();
            }
            else if(tempo_vermelho_cont <= 4*tempo_vermelho_total/10){
              seis();
            }
            else if(tempo_vermelho_cont <= 5*tempo_vermelho_total/10){
              cinco();
            }
            else if(tempo_vermelho_cont <= 6*tempo_vermelho_total/10){
              quatro();
            }
            else if(tempo_vermelho_cont <= 7*tempo_vermelho_total/10){
              tres();
            }
            else if(tempo_vermelho_cont <= 8*tempo_vermelho_total/10){
              dois();
            }
            else if(tempo_vermelho_cont <= 9*tempo_vermelho_total/10){
              um();
            }
            if(tempo_vermelho_cont >= 2*tempo_vermelho_total/3){
              /* Aqui o semáforo dos pedestres irá fechar */
              /* Devemos tocar o buzzer sem intervalos e piscar o semáforo vermelho para avisar quem estiver atravessando */
              tempo_vermelho_pisca_cont++;
              digitalWrite(pedestre_verde, HIGH);
              digitalWrite(buzzer, HIGH);
              if(tempo_vermelho_pisca_cont < tempo_vermelho_pisca_total){
                digitalWrite(pedestre_vermelho, LOW);              
              }
              if(tempo_vermelho_pisca_cont >= tempo_vermelho_pisca_total){
                digitalWrite(pedestre_vermelho, HIGH);
                if(tempo_vermelho_pisca_cont == 2*tempo_vermelho_pisca_total){
                  tempo_vermelho_pisca_cont = 0;
                }
              }
            }
            else{
              /* Aqui o semáforo dos pedestres está no início */
              /* Vamos tocar o buzzer periodicamente para um sinal audível */
              tempo_buzzer_cont++;
              if(tempo_buzzer_cont <= tempo_buzzer_total){
                digitalWrite(buzzer, HIGH);
              }
              else if(tempo_buzzer_cont < 2*tempo_buzzer_total){
                digitalWrite(buzzer, LOW);
              }
              else if(tempo_buzzer_cont >= 2*tempo_buzzer_total){
                tempo_buzzer_cont = 0;
              }
            }
            if(tempo_vermelho_cont == tempo_vermelho_total){
              /* Chegando ao final do ciclo do botao devemos retornar ao estado inicial */
              digitalWrite(semaforo_vermelho, HIGH);
              digitalWrite(semaforo_verde, LOW);
              digitalWrite(pedestre_verde, HIGH);
              digitalWrite(pedestre_vermelho, LOW);
              digitalWrite(buzzer, LOW);
              zero();
              tempo_vermelho_cont = 0;
              tempo_amarelo_cont = 0;
              tempo_verde_cont = 0;
              botao_apertado = 0;
            }
          }
        }

      }
    }
  }
  else{
    /* Caso seja noite devemos piscar o semáforo dos carros em amarelo e dos pedestres em vermelho */
    /* Note que a leitura do botão não muda as decisões a noite */
    digitalWrite(semaforo_verde, HIGH);
    digitalWrite(semaforo_vermelho, HIGH);
    digitalWrite(pedestre_verde, HIGH);
    tempo_amarelo_pisca_cont++;
    if(tempo_amarelo_pisca_cont < tempo_amarelo_pisca_total){
      digitalWrite(semaforo_amarelo, LOW);
      digitalWrite(pedestre_vermelho, LOW);
    }
    if(tempo_amarelo_pisca_cont >= tempo_amarelo_pisca_total){
      digitalWrite(semaforo_amarelo, HIGH);
      digitalWrite(pedestre_vermelho, HIGH);
      if(tempo_amarelo_pisca_cont == 2*tempo_amarelo_pisca_total){
        tempo_amarelo_pisca_cont = 0;
      }
    }
  }
  /* Leitura do LDR */
  int LDR_read = analogRead(LDR_input);
  if(LDR_read >= LDR_limite_inferior && LDR_read <= LDR_limite_superior){
    if(LDR_cont == LDR_total){
      /* Caso a contagem tenha atingido o limite devemos mudar o estado da flag para indicar a noite */
      ja_e_noite = 1;
    }
    else{
      /* Caso a contagem não tenha atingido o limite então vamos incrementar a contagem */
      LDR_cont++;
    }  
  }
  else{
    if(LDR_cont == 0){
      /* Caso a contagem tenha sido zerada devemos voltar a flag para indicar o dia */
      ja_e_noite = 0;
    }
    else{
      /* Se a contagem ainda não estiver em zero vamos decrementar o contador */
      LDR_cont--;
      if(LDR_cont == 0){
        /* Reseta o sistema para o estado inicial */
        ja_e_noite = 0;
        botao_apertado = 0;
        zero();
        digitalWrite(buzzer, LOW);
        digitalWrite(semaforo_vermelho, HIGH);
        digitalWrite(semaforo_amarelo, HIGH);
        digitalWrite(semaforo_verde, LOW);
        digitalWrite(pedestre_vermelho, LOW);
        digitalWrite(pedestre_verde, HIGH);
      }
    }
  }
}


void setup(){
  int i;
  for(i = 13; i >= 9; i--){ /* Pinos de saída para os LEDs */
    pinMode(i, OUTPUT);
  }  
  pinMode(botao, INPUT); /* Pino de entrada para o botão */
  pinMode(buzzer, OUTPUT); /* Pino de saída para o buzzer */
  /* Pinos de saída para o conversor BCD -> Display de 7 segmentos */
  pinMode(data0, OUTPUT);
  pinMode(data1, OUTPUT);
  pinMode(data2, OUTPUT);
  pinMode(data3, OUTPUT);
  /* Estado inicial - Carros verde, Pedestres vermelho, display zerado e buzzer desligado*/
  zero(); 
  digitalWrite(buzzer, LOW);
  digitalWrite(semaforo_vermelho, HIGH);
  digitalWrite(semaforo_amarelo, HIGH);
  digitalWrite(semaforo_verde, LOW);
  digitalWrite(pedestre_vermelho, LOW);
  digitalWrite(pedestre_verde, HIGH);
  /* Inicialização do timer para 10000us(10ms) e ligado a função de tratamento de interrupção timer_interrupt_handler que progamamos*/
  Timer1.initialize(10000); 
  Timer1.attachInterrupt(timer_interrupt_handler);
}

void loop(){
  /* Devemos a cada iteração checar o estado do botao e, caso ele tenha sido apertado, alterar a flag*/
  if(digitalRead(botao) == LOW){ 
    botao_apertado = 1;
  }
}


