/*
Projeto final EA076 - Prof. Dr. Tiago Tavares
Data : 19/06/2017 - FEEC, Campinas.
Alunos: Heitor Toniolo           RA: 155700
        Leonardo Fill Cardoso        156205
 Esse projeto consistiu em duas partes relativamente
independentes, na primeira montamos uma estrutura de
um braço mecânico com três graus de liberdade, o 
primeiro grau permite o braço ir para frente\trás,
o segundo para cima\baixo e o terceiro permite que 
a cesta posicionada no fim do braço gire entre 0 e 180
graus, ou seja, podemos "derrubar" algo que foi deixado
anteriormente na cesta.
 A segunda parte do projeto foi utilizar o braço em uma 
aplicação de engenharia simplificada devidamente selecionada,
uma das aplicações considerada foi utilizar o braço como um
atuador na seleção de objetos identificados por um sensor em
uma esteira, onde remetemos a ideia de remover produtos com
defeito de uma linha de produção, sendo que o defeito é 
detecetado pelo sensor e o braço removeria o produto defeituoso.
 Essa ideia não a escolhida porque o projeto da esteira acabou por
tomar muito tempo dos alunos e também alguns dos materiais necessários
não foram encontrados a tempo para apresentação do projeto. Ao invés 
disso optamos por um projeto mais simples, do ponto de vista de software,
onde fizemos um guindaste controlado por 6 push buttons, onde cada par
de botões controlavam um dos graus de liberdade do sistema. 
 Algumas ideias que poderiam ser implementadas para tornar o projeto mais
sofisticado seriam, por exemplo, realizar um controle analógico dos graus 
de liberdade do guindaste (utilizando potênciometros, por exemplo), ou
utilizar o braço para deixar algo em uma certa posição pré determinada
quando uma leitura de um sensor indicasse alguma uma situação que gostariamos
de atuar. Um exemplo: um sensor de humidade indica que plantas precisam ser
regadas, quando isso acontecer podemos deslocar o braço para a posição das 
plantas, que já são conhecida a priori, utilizar o terceiro grau de liberdade
para derrubarmos a água. Feito isso podemos retornar o braço a posição inicial,
reabastecer a cesta com água e continuar o processo. Essa ideia parece atraente
quando temos uma grande quantidade de plantas para regarmos e\ou consideramos
expandir o jantar no futuro (já que adicionar uma nova planta irá requerer apenas
um sensor de humidade e uma ligeira alteração no software).
 Por fim, citamos que os motores utilizados forneciam torque relativamente baixos,
fato que restrigiu algumas possibilidades de estrutura, limitando o alcançe do 
sistema (alcançe está diretamente relacionado com o braço e braço está diretamente
relacionado com o torque) e fazendo com os alunos "otimizarem" a estrutura (claro 
que nossa estrutura está longe de ser a melhor possível - se é que é possível 
encontrar a estrutura ótima - mas foi necessário realizar diversos testes e ajustes
às ideias iniciais para que o projeto se tornasse bom o suficiente). Outro problema
foi que, mesmo os motores não fornecendo grande torque, a alimentação USB poderia não
conseguir fornecer a corrente consumida pelos motores, por isso os alunos utilizaram
uma fonte de tensão 12V - 1A, conectada a um regulador (colocar o tipo) que fornecia 5V
na saída e utilizamos essa saída como alimentação dos servos, curto circuitando as 
referências do MCU (arduino) e do regulador (e da fonte externa).
*/

#include <Servo.h>

Servo servo1;  // servo horizontal 
Servo servo2;  // servo vertical
Servo servo3;  // roda cesta

int angle1; 
int angle2;
int angle3;
int hor_rate = 5;
int vert_rate = 5;
int rotate_rate = 5;
int push1 = 2, push2 = 3, push3 = 4, push4 = 5, push5 = 6, push6 = 7, push7 = 8;
int mov_led = 12;
int leitura;

void setup() {
  servo1.attach(9); // attaches the servo on pin 9 to the servo object 
  servo2.attach(10);
  servo3.attach(11);
  Serial.begin(9600); // open a serial connection to your computer
  angle1 = 90;
  angle2 = 90;
  angle3 = 165;
  servo1.write(angle1);
  delay(100);
  servo2.write(angle2);
  delay(100);
  servo3.write(angle3);
  delay(100);
  pinMode(push1, INPUT);
  pinMode(push2, INPUT);
  pinMode(push3, INPUT);
  pinMode(push4, INPUT);
  pinMode(push5, INPUT);
  pinMode(push6, INPUT);
  pinMode(push7, INPUT);
  pinMode(mov_led, OUTPUT);
  digitalWrite(mov_led, LOW); // led desliga já que é ativo alto
}
int varredura(){
  if(digitalRead(push1) == HIGH){
    return 1;
  }
  else if(digitalRead(push2) == HIGH){
    return 2;
  }
  else if(digitalRead(push3) == HIGH){
    return 3;
  }
  else if(digitalRead(push4) == HIGH){
    return 4;
  }
  else if(digitalRead(push5) == HIGH){
    return 5;
  }
  else if(digitalRead(push6) == HIGH){
    return 6;
  }
  else if(digitalRead(push7) == HIGH){
    return 7;
  }
  else{
    return -1;
  }
}
void loop() {
  leitura = varredura();
  if(leitura != -1){
    while(varredura() == leitura);
    digitalWrite(mov_led, HIGH);
    if(leitura == 1){
      if(angle1 <= 120 - hor_rate){
        angle1 += hor_rate;
      }
    }
    else if(leitura == 2){
      if(angle1 >= 60 + hor_rate){
        angle1 -= hor_rate;
      }
    }
    if(leitura == 3){
      if(angle2 <= 120 - vert_rate){
        angle2 += vert_rate;
      }
    }
    if(leitura == 4){
      if(angle2 >= 60 + vert_rate){
        angle2 -= vert_rate;
      }
    }
    if(leitura == 5){
      if(angle3 == 165){
        for(;;){
          angle3 -= rotate_rate;
          servo3.write(angle3);
          delay(100);
          if(angle3 == 15){
            digitalWrite(mov_led, LOW);
            break;
          }     
        }
      }
    }
    if(leitura == 6){
      if(angle3 == 15){
        for(;;){
          angle3 += rotate_rate;
          servo3.write(angle3);
          delay(100);
          if(angle3 == 165){
            digitalWrite(mov_led, LOW);
            break;
          }     
        }
      }
    }
    if(leitura == 7){
      angle1 = 90;
      angle2 = 90;
      angle3 = 165;
    }
    servo1.write(angle1);
    delay(100);
    servo2.write(angle2);
    delay(100);
    servo3.write(angle3);
    delay(100);
    digitalWrite(mov_led, LOW);
  }
}

