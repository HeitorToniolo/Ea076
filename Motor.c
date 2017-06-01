/*
Heitor Toniolo - RA: 155700
Leonardo Fill  - RA: 156205
 No experimento 3 escolhemos realizar um controle da rotação
de um motor em malha fechada. Inicialmente escolhemos 
um circuito de driver baseado em um único transistor, mas 
decidimos utilizar um circuito de ponte H já que era possível 
obter uma potência maior.
 Implementamos um controle PD, onde realizamos um ajuste na potência
do motor(de modo a controlar a rotação do mesmo) limitando-a através
do tempo em que os transistores ficam ativos. O controle PD é baseado
em realizar um ajuste do sistema utilizando uma constante proporcional
ao erro e outra proporcional a derivada do erro, deste modo procuramos
obter um erro de regime aceitável, através do ajuste da constante
proporcional, e melhorar o transitorio ajustando a constante proporcional
à derivada.
 Como realizamos um controle de malha fechada, então foi necessário utilizar
uma forma de sensoriamento, a opção escolhida foi um par emissor transimissor IR
já que era uma opção simples e barata, além de que critério como precisão não
eram restritivos no projeto, o funcionamento se dava pela interrupção do feixe
IR entre o par pela hélice do motor, que gerava uma interrupção externa no MCU,
utilizando isso contavamos o número de interrupções, considerando um intervalo
de tempo definido por um timer, foi possível determinar uma medida de frequência
das rotações no motor.
 De forma a validar a ideia, utilizamos dois valores de frequências pré definidos, que
podiam ser selecionados através do monitor serial com 'F' e 'f' (é inicializada em 30.0),
e verificamos se o motor alcançava tais valores, além disso introduzimos um torque no 
sistema (segurando o eixo) e notamos que, passando o transitorio, que o sistema seguia 
a frequência definida.
*/

#include "TimerOne.h"
int motor = 4; // Porta para controle do motor
int counts_motor = 0; // Variável que conta quantas vezes o motor interrompeu o feixe IR
float freq_desejada = 0.0; // Valor desejado de frequência
float erro = 0; // Variável para armazenar o valor de erro
float passo = 0.02; // Passo do controlador proporcional, determinado por métodos heurísticos
float passo_derivada = 0.05; // Passo do controlador derivada, determinado por métodos heurísticos
int tensao_base = 0; // Valor que será utilizado no PWM (de 0 até 255)
float tensao_base_real = 0; // Valor de tensão real, ajuda a determinar os limites de operação do sistema
float erro_antigo = 0; // variável para armazenar o erro do passo anterior, para ser utilizado na estimativa da derivada
// https://forum.arduino.cc/index.php?topic=410287.0  -> Link para quais portas de PWM são alteradas pela biblioteca TimerOne
void setup(){
  pinMode(motor, OUTPUT); 
  attachInterrupt(1, interrupcao_IR, FALLING); // int 1 define interrupção externo no pino 3
  Serial.begin(9600); // Inicializa a comunicação serial
  Timer1.initialize(1000000); // Interrupção de 1s
  Timer1.attachInterrupt(timer_interrupt_handler); 
}
void timer_interrupt_handler(){
  float freq = (float)counts_motor; // Obtem a frequencia do motor -> counts/tempo_da_interrupção
  Serial.println(freq); 
  erro_antigo = erro; // Salva o antigo valor do erro
  erro = freq_desejada - freq; // Novo valor do erro 
  float derivada = erro - erro_antigo; // Estimativa da derivada -> diferença dos erros/intervalo de tempo 
  //Serial.println(erro);
  // As sequências de if a seguir avaliam a condição do sistema, de forma a evitar que a tensão seja saturada
  if((tensao_base_real + passo*erro + passo_derivada*derivada >= 5.0 && erro >= 0)){
    // Não realizamos ajuste
  }
  else if((tensao_base_real + passo*erro  + passo_derivada*derivada < 0 && erro <= 0)){
    // Não realizamos ajuste
  }
  else{
    tensao_base_real += (passo*erro  + passo_derivada*derivada); // Passo de ajuste
  }
  tensao_base = (int)(255.0*tensao_base_real/5.0); // Conversão de Volts para Int (para o PWM)
  Serial.print(tensao_base_real);
  Serial.println("V");
  //Serial.println(tensao_base);
  counts_motor = 0; // Reseta a contagem dos ciclos
}
void interrupcao_IR(){
  counts_motor++; // Contagem das interrupções do feixe IR
}

void loop(){
  if(Serial.available()){ // Avaliação para a frequência desejada
   char char_terminal = Serial.read();
   if(char_terminal == 'F'){ 
     freq_desejada = 30.0;
   }
   else if(char_terminal == 'f'){
      freq_desejada = 15.0;
    } 
  }
  analogWrite(motor, tensao_base); // Atuação no motor
}
