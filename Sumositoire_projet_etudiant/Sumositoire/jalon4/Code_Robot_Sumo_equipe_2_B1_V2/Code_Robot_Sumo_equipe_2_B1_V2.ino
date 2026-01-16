/* -------------------------------------------------------------
 			Auteur : Val
 			DUT : GEII.
 			Date : 14/10/2015 / 23h09.
 --------------------------------------------------------------*/
//sharpe
#define pin1 A5  //sharp arrier
#define pin2 A4 //sharp gauche
#define pin3 A7 //sharp droite
#define sueil_infra_arierre 5
#define sueil_infra_droite 5
#define sueil_infra_gauche 5
#define SHAR 16
#define SHG 32
#define SHD 64

#define get_sharpBack  get_IR1
#define get_sharpLeft  get_IR2
#define get_sharpRight get_IR3

//Ligne var
#define input_ligne_ar A2
#define AR 1

#define input_ligne_av_G A1
#define AVG 2

#define input_ligne_av_D A0
#define AVD 4


//US var
#define trig  13
#define echo  12

#define sueil_ultrason 40
#define US 8

uint16_t seuil_ir_ligne = 0;

// Motors
const byte moteur_gauche[] = {5, 7}; //Bin2_-_Bin1
const byte moteur_droite[] = {6, 8}; //Ain2_-_Ain1
enum { TOUT_DROIT = 1, MARCHE_ARRIERE, GAUCHE, DROITE, STOP};

//led
const byte led_g = 11, led_b = 9;
//button
const byte button = 2;

byte arret = 0;


void setup() {
  Serial.begin(9600);
  //LED Setup
  pinMode(led_b, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  //Ligne Setup
  pinMode(input_ligne_ar, INPUT);
  pinMode(input_ligne_av_G, INPUT);
  pinMode(input_ligne_av_D, INPUT);

  //US Setup
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  pinMode(echo, INPUT);

  //sharp
  pinMode(pin1, INPUT);
  pinMode(pin2, INPUT);
  pinMode(pin3, INPUT);

  //Motor Setup
  int i;
  for (i = 0; i < 2; i++) {
    pinMode(moteur_gauche[i], OUTPUT);           // moteur gauche.
    pinMode(moteur_droite[i], OUTPUT);           // moteur droit.

  }
}

byte Checkcapte()
{
  byte var = 0;
  byte sharpEtat = sharp();
  if ((uint16_t)analogRead(input_ligne_ar) < seuil_ir_ligne)
  {
    var |= AR;
  }
  if ((uint16_t)analogRead(input_ligne_av_G) < seuil_ir_ligne)
  {
    var |= AVG;
  }
  if ((uint16_t)analogRead(input_ligne_av_D) < seuil_ir_ligne)
  {
    var |= AVD;
  }
  if (ultrason(false) == 1)
  {
    var |= US;
  }
  if (sharpEtat == 1)
  {
    var |= SHAR;
  }
  if (sharpEtat == 2)
  {
    var |= SHG;
  }
  if (sharpEtat == 4)
  {
    var |= SHD;
  }
  return (var);
}

byte Priocapte(byte var)
{
  if (var & AVG && var & AVD)
    return (AVG | AVD);

  if (var & AVG)
  {
    return (AVG);
  }
  if (var & AVD)
  {
    return (AVD);
  }
  if (var & AR)
  {
    return (AR);
  }
  if (var &  US && var & SHG)
  {
    return (SHG);
  }
  if (var &  US && var & SHD)
  {
    return (SHD);
  }
  if (var &  US && var & SHAR)
  {
    return (SHAR);
  }
  if (var & SHG)
  {
    return (SHG);
  }
  if (var & SHD)
  {
    return (SHD);
  }
  if (var & SHAR)
  {
    return (SHAR);
  }
  if (var & US)
  {
    return (US);
  }
  return (var);
}

byte mouvement(int direction, byte ancienetat = 0, int temps = 0)
{
  switch (direction)
  {
    case TOUT_DROIT: // Tout droit.
      //Avant();
      analogWrite(moteur_gauche[0], 255);
      digitalWrite(moteur_gauche[1], HIGH);

      analogWrite(moteur_droite[0], 255);
      digitalWrite(moteur_droite[1], HIGH);
      break;

    case MARCHE_ARRIERE: // Marche-arrière.
      analogWrite(moteur_gauche[0], 255);
      digitalWrite(moteur_gauche[1], LOW);

      analogWrite(moteur_droite[0], 255);
      digitalWrite(moteur_droite[1], LOW);
      break;

    case GAUCHE: // Gauche.
      analogWrite(moteur_gauche[0], 255);
      digitalWrite(moteur_gauche[1], LOW);

      analogWrite(moteur_droite[0], 255);
      digitalWrite(moteur_droite[1], HIGH);
      break;

    case DROITE: // Droite.
      analogWrite(moteur_gauche[0], 255);
      digitalWrite(moteur_gauche[1], HIGH);

      analogWrite(moteur_droite[0], 255);
      digitalWrite(moteur_droite[1], LOW);
      break;

    case STOP: // Stop
      analogWrite(moteur_gauche[0], 0);
      digitalWrite(moteur_gauche[1], LOW);

      analogWrite(moteur_droite[0], 0);
      digitalWrite(moteur_droite[1], LOW);
      break;
  }
  if (temps != 0)
  {
    temps /= 5;
    int compteur = 0;
    byte nouveletat = ancienetat;
    while (nouveletat == ancienetat && compteur < temps)
    {
      delay(5);
      compteur++;
      nouveletat = Priocapte(Priocapte(Checkcapte()) | ancienetat); // SOME AWESOME MAGIC
    }
    if (compteur != temps)
      return (1);
  }
  return (0);
}

byte action ()
{
  byte etat = Priocapte(Checkcapte());

  //Serial.println(etat, BIN);
  switch (etat)
  {
    case 0:
      digitalWrite(led_b, LOW);
      digitalWrite(led_g, LOW);
      return (0);

    case AR: // Capteur Line Arriere Actif
      digitalWrite(led_g, LOW);
      digitalWrite(led_b, HIGH);
      mouvement(TOUT_DROIT);
      break;

    case (AVG|AVD): // Capteur Line Gauche et Droit Actif
      digitalWrite(led_g, HIGH);
      digitalWrite(led_b, HIGH);
      if (mouvement(MARCHE_ARRIERE, etat, 450) == 1)
        return (1);
      if (mouvement(DROITE, etat, 200) == 1)
        return (1);
      mouvement(TOUT_DROIT);
      break;

    case AVG: // Capteur Line Gauche Actif
      digitalWrite(led_g, HIGH);
      digitalWrite(led_b, LOW);
      if (mouvement(MARCHE_ARRIERE, etat, 300) == 1)
        return (1);
      if (mouvement(DROITE, etat, 200) == 1)
        return (1);
      mouvement(TOUT_DROIT);
      break;

    case AVD: // Capteur Line Droit Actif
      digitalWrite(led_g, HIGH);
      digitalWrite(led_b, LOW);
      if (mouvement(MARCHE_ARRIERE, etat, 300) == 1)
        return (1);
      if (mouvement(GAUCHE, etat, 200) == 1)
        return (1);
      mouvement(TOUT_DROIT);
      break;

    case US: // Capteur Ultrason
      mouvement(TOUT_DROIT);
      break;

    case SHG: // Capteur Sharp Gauche Actif
      if (mouvement(GAUCHE, etat, 200) == 1)
        return (1);
      mouvement(TOUT_DROIT);
      break;

    case SHD: // Capteur Sharp Droit Actif
      if (mouvement(DROITE, etat, 200) == 1)
        return (1);
      mouvement(TOUT_DROIT);
      break;

    case SHAR: // Capteur Sharp Arriere Actif
      mouvement(MARCHE_ARRIERE);
      break;

    default:
      break;
  }
  return (0);
}


byte ultrason(boolean rst_val)
{
  static byte ct = 0;
  static byte ancieneco = 0;
  if (rst_val == true)
  {
    ct = 0;
    ancieneco = 0;
    return (0);
  }
  
  if (ct == 20)
  {
    ct = 0;
    ancieneco = aquisition();
  }
  ct ++;
  return (ancieneco);
}


byte aquisition()
{
  long lecture_echo;
  long cm;
  
  digitalWrite(trig, HIGH);
  digitalWrite(trig, LOW);
  lecture_echo = pulseIn(echo, HIGH, 10000);
  cm = lecture_echo / 58;
  if (cm < 1)
  {
    cm = 150;
  }
  if (cm <= sueil_ultrason)
  {
    return (1);
  }
  return (0);
}


byte sharp()
{
  double distance;
  //sharp arriere
  distance = get_IR1 ();
  if (distance <= sueil_infra_arierre)
  {
    return (1);
  }

  //sharp droit
  distance = get_IR2 ();
  if (distance <= sueil_infra_droite)
  {
    return (2);
  }

  //sharp gauche
  distance = get_IR3 ();
  if (distance <= sueil_infra_gauche)
  {
    return (4);
  }
  return (0);
}


double get_IR1 () {
  double value;
  value = analogRead (pin1); //sharp arriere
  if (value < 16)  value = 16;
  return 2076.0 / (value - 11.0);
}


double get_IR2 () {
  double value;
  value = analogRead (pin2); //sharpe gauche
  if (value < 16)  value = 16;
  return 2076.0 / (value - 11.0);
}


double get_IR3 () {
  double value;
  value = analogRead (pin3); //sharpe droite
  if (value < 16)  value = 16;
  return 2076.0 / (value - 11.0);
}

void loop() {
  //Ligne Loop
  mouvement(STOP);
  arret = 0;
  ultrason (true);
  digitalWrite(led_g, HIGH);
  while (digitalRead(button) == HIGH); // attente de l'appui sur le bouton
  while (digitalRead(button) == LOW);  // attente du relachement du bouton
  digitalWrite(led_g, LOW);
  digitalWrite(led_b, HIGH);
  delay(4985);  //5 second d'attente

  seuil_ir_ligne = (analogRead(input_ligne_ar) + analogRead(input_ligne_av_G) + analogRead(input_ligne_av_D)) / 6;
  
  digitalWrite(led_b, LOW);

  double distL = get_sharpLeft();
  double distR = get_sharpRight();
  if (distL < distR)
   mouvement (GAUCHE, 0, 120);
   //mouvement (GAUCHE);
  else
    mouvement (DROITE, 0, 120);
    //mouvement (DROITE);
    

  while (arret == 0)
  {
    action ();
    if (digitalRead(button) == LOW) {
      arret = 1;
    }
  }
  while (digitalRead(button) == LOW); // attente du relachement du bouton
}

