#define MR 6 //Motor Direita MD -  D6 - GPIO12
#define ML 5 //Motor Esquerda ME - D8 - GPIO15

//ESQUERDA
#define TRIG_PIN_2 8
#define ECHO_PIN_2 9
//DIREITA
#define TRIG_PIN_3 10
#define ECHO_PIN_3 11

#define KP 6.2
#define KI 0.33

//Variáveis Globais
long prevTime=0;
float i=0;

void setup(){
  Serial.begin(9600);
  pinMode(MR,OUTPUT);
  pinMode(ML,OUTPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(TRIG_PIN_3, OUTPUT);
  pinMode(ECHO_PIN_3, INPUT);
}

long sensor(int id){
  int trigger, echo;
  long distance;

  switch(id){
    case(1)://Esquerda
      trigger=TRIG_PIN_2;
      echo=ECHO_PIN_2;
    break;
    case(2)://Direita
      trigger=TRIG_PIN_3;
      echo=ECHO_PIN_3;

    break;
    default:
      return 0;
    break;
  }
  
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  distance = pulseIn(echo, HIGH);
  distance = (distance/2) / 29.1;

  return distance;
}

float truncate(float element){
  if(element > 255) element = 255;
  if(element < -255) element = -255;
  
  return element;

}

float controller(long error){
  float p=0, pid=0;
  float varTime=millis()-prevTime;

  prevTime = millis();
  varTime = varTime/1000;

  p = error * KP;
  p = truncate(p);
  
  i = i + (error * KI * varTime);
  i = truncate(i);
  
  pid = p + i;
  pid = truncate(pid);

  return pid;
}

int inercia(int i_pid) {

  float new_pid = (120+(100*((float)i_pid/255)));
  new_pid = truncate(new_pid);

  return (int)new_pid;
}

void actuator(float pid){
  int i_pid = (int)pid;
  if(i_pid>0){//curva para esquerda
    analogWrite(MR,inercia(255-i_pid));
    analogWrite(ML,255);
    // Serial.println("curva para esquerda: ");
    // Serial.print("MR: ");
    // Serial.println(inercia(255-i_pid));
    // Serial.println("ML: 255");
  }else{//curva para direita
    analogWrite(ML,inercia(255+i_pid));
    analogWrite(MR,255);
    // Serial.println("curva para direita: ");
    // Serial.println("MR: 255");
    // Serial.print("ML: ");
    // Serial.println(inercia(255-i_pid));
  }
}

void loop(){
  long distance[2], error;
  float pid;

  //Passo 1: Medição
  distance[0]=sensor(1);//Esquerda
  delayMicroseconds(20);
  distance[1]=sensor(2);//Direita


  //Passo 2: Calculo do erro
    /*sensor da direita menos o sensor da esquerda
    distance[0] => esquerda
    distance[1] => direita*/
  error = distance[1]-distance[0];

  //Passo 3: Controle
  pid=controller(error);

  //Passo 4: Força nos motores!
  actuator(pid);


}