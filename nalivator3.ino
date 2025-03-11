
#include <LCD_1602_RUS.h> // подключаем библиотеку LCD_1602_RUS
#include <Servo.h>
#include <SoftwareSerial.h>
#include <GyverEncoder.h>
#include <GyverTimers.h>
#include <DFPlayer_Mini_Mp3.h>
#include <Adafruit_NeoPixel.h>


//---------------------------------------------------------------------
LCD_1602_RUS LCD(0x27, 16, 2); // присваиваем имя LCD для дисплея
Servo myservo;
Encoder enc1(2, 3, 4);  // для работы c кнопкой

Adafruit_NeoPixel strip = Adafruit_NeoPixel(6, 6, NEO_GRB + NEO_KHZ800);

//---------------------------------------------------------------------
int ugol[6] = {22, 50, 83, 115, 143, 172}; // Угол поворота серво
int pin_stopki[6] = {7, 8, 9, 10, 11, 14};
int statistika [6] = {0}, gamers_RUS [6] = {0}, gamers_USA [6] = {0};
bool  nalil [6] = {0};
byte rezim = 0, vibor_meny = 0;
int ml = 0, igroki_RUS = 0, igroki_USA = 0;
bool flag_meny = 0, flag_nalich_rum = 0, trig = 0, trig1 = 0, vozvrat = 0, volume_setting = 0;
const int time_step = 202; // время работы помпы на объем 5мл
int pump_timer = 6*time_step; // время работы помпы начально на объем 30мл
const int max_pump_timer = 10*time_step, min_pump_timer = 2*time_step; // время работы помпы максимальный объем 50мл, минимальный объем 10мл
int rus_rul = 0, USA_rul = 0, mp3_rundom = 0, LED_yarkost = 153, LED_yarkost2 = 0, LED_yarkost3 = 153, volume = 25, volume2 = 0, statistika_per = 0;

int polozenie = 0; // для определения угла сервы
//---------------------------------------------------------------------
ISR (TIMER2_A)
{
  enc1.tick();     // Обрабатываем енкодер с помощью таймера 2
}
//---------------------------------------------------------------------
void Init (void) // функция инициализации
{
  LCD.init(); // инициализация LCD дисплея
  LCD.backlight(); // включение подсветки дисплея
  enc1.setType(TYPE2); // тип энкодера
  pinMode (4, INPUT_PULLUP );
  pinMode (7, INPUT_PULLUP );
  pinMode (8, INPUT_PULLUP );
  pinMode (9, INPUT_PULLUP );
  pinMode (10, INPUT_PULLUP );
  pinMode (11, INPUT_PULLUP );
  pinMode (14, INPUT_PULLUP );
  pinMode (5, OUTPUT);
  digitalWrite (5, 0);
  Timer2.setPeriod (1000);
  Timer2.enableISR (CHANNEL_A);
  strip.begin(); //Инициализируем ленту.
  strip.show();
  randomSeed (analogRead(3)); // Для РАНДОМА
  Serial.begin(9600);
  mp3_set_serial(Serial); // включаем передачу данных с DFPlayer mini mp3
  delay(1);               // задержка в 1ms для установки громкости
  mp3_set_EQ(0);
  pinMode (15, INPUT_PULLUP );
}
//---------------------------------------------------------------------
void pump (int v, int n) // Функция для работы помпы
{
  digitalWrite (5, 1);
  led_naliv(n);
  for (int i = 0; i <= v; i += 1)
  {
    if (!digitalRead(pin_stopki[n])) break;
    delay (1);
  }
  digitalWrite (5, 0);
}
//---------------------------------------------------------------------
void razliv (int rumka) // Функция для разлива
{
  int current_pos = myservo.read();
  myservo.attach(12);
  int pos = current_pos;
  while ( pos != ugol[rumka])
  {
    if (current_pos < ugol[rumka]) pos += 1;
    else pos -= 1;
    myservo.write(pos);
    delay (8);
  }
  myservo.detach();
  vozvrat = 1;
}
//---------------------------------------------------------------------
void servoparking ()// Функция ПАРКОВКИ
{
  myservo.attach(12);
  for (int pos = myservo.read(); pos >= 0; pos -= 1)
  {
    myservo.write(pos);
    delay (8);
  }
  myservo.detach();
}
//---------------------------------------------------------------------
void sost_rumka (void) // Функция состояния рюмки
{
  for (int i = 0; i <= 5; i += 1)
  {
    if (!digitalRead(pin_stopki[i])) nalil[i] = 0; // если была налита рюмка то в нее больше не наливаем
  }
}
//---------------------------------------------------------------------
int number_of_glasses() // Функция количества рюмок
{
  int res = 0;
  for (int i = 0; i <= 5; i += 1)
  {
    if (digitalRead(pin_stopki[i]) && !nalil[i]) res++; 
  }
  return res;
}
//---------------------------------------------------------------------
void list_of_gamers_rus(int n) // Функция определяющая адреса стопок учавствющих в русской рулетке
{
  int j = 0;
  for (int i = 0; i <= 5; i += 1)
  {
    if (digitalRead(pin_stopki[i])) {
      gamers_RUS[j] = i; 
      j++;
    }
  
  }
}
//---------------------------------------------------------------------
void list_of_gamers_usa(int n) // Функция определяющая адреса стопок учавствющих в американской рулетке
{
  int j = 0;
  for (int i = 0; i <= 5; i += 1)
  {
    if (digitalRead(pin_stopki[i])) {
      gamers_USA[j] = i; 
      j++;
    }
  
  }
}
//---------------------------------------------------------------------
void rumka_svet_on (int n) // Функция включения света если рюмка стоит
{
  if (digitalRead(pin_stopki[n]) && !nalil[n])
  {
    strip.setPixelColor(n, 0, 0, 255);
    strip.show();
  }
}
//---------------------------------------------------------------------
void all_rumka_svet_on () // Функция включения света всех рюмок которые рюмок стоят
{
  for (int i = 0; i <= 5; i += 1)
  {
    rumka_svet_on(i);
  }
}
//---------------------------------------------------------------------
void rumka_svet_off (int n) // Функция выключения света конкретной рюмки

{
  if (!digitalRead(pin_stopki[n]))
  {
    strip.setPixelColor(n, 0, 0, 0);
    strip.show();
  }
}
//---------------------------------------------------------------------
void all_rumka_svet_off () // Функция выключения света всех рюмок

{
  for (int i = 0; i <= 5; i += 1)
  {
    rumka_svet_off(i);
  }
}
//---------------------------------------------------------------------
void led_naliv (char l) // Подсвечивает во время налива
{
  strip.setPixelColor(l, 255, 0, 0);
  strip.show();
}
//---------------------------------------------------------------------

void nalich_rum() // Функция проверяющая наличие хоть одной рюмки
{
  flag_nalich_rum = 0;
  for (int i = 0; i <= 5; i += 1)
  {
    if (digitalRead(pin_stopki[i])) flag_nalich_rum = 1; 
  }
  
}
//---------------------------------------------------------------------------------------------------
void LED_statistika (byte x) // Для статистики
{
  for (int i = 0; i <= 5; i += 1)
  {
    if (x == i){
      strip.setPixelColor(i, 0, 0, 245);
      strip.show();
    }
    else{
      strip.setPixelColor(i, 0, 0, 0);
      strip.show();
    }
  }
}
//---------------------------------------------------------------------------------------------
void setup()
{
  Init();// инициализация

  myservo.attach(12);
  myservo.write(0);
  delay (1000);
  myservo.detach();


  LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
  LCD.print("    ЛёхБух)     "); // печатаем символ на первой строке
  LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
  LCD.print("     2025г"        ); // печатаем символ на первой строке

  mp3_set_volume(20);
  delay(250);
  mp3_play (54);

  strip.setBrightness(255); // Максимальная яркость при включении
  for (byte i = 0; i < 7; i++)
  {
    strip.setPixelColor(i, random (0, 255), random (0, 255), random (0, 255) );
    strip.show();
    delay(300);
  }


  for (byte i = 7; i > 0; i--)
  {
    strip.setPixelColor(i, random (0, 255), random (0, 255), random (0, 255) );
    strip.show();
    delay(300);
  }
  delay(500);
  mp3_stop ();
  strip.clear();
}

void vistrel(int n, int v)  //Функция выстрела в рулетке
{
  if (digitalRead(pin_stopki[n]) && nalil[n] == 0) // Если поставили рюмку и она не была налита то нальем ее
  {
    razliv (n); // Вызываем функцию разлива
    delay (500);
    pump(v, n);
    strip.setPixelColor(n, 0, 255, 0);
    strip.show();
    delay(1000);
    statistika [n] ++; // Для статистики
    nalil[n] = 1;
  }
}

bool ruchnoy_naliv() //Функция для разлива по стопкам в ручном и автоматическом режимах
{
  bool result = false;
  for (int n = 0; n <= 5; n += 1){
    if (digitalRead(pin_stopki[n]) && nalil[n] == 0) // Если поставили рюмку и она не была налита то нальем ее
    {
      razliv (n); // Вызываем функцию разлива
      delay (500);
      pump(pump_timer, n);
      strip.setPixelColor(n, 0, 255, 0);
      strip.show(); // Присваем свою подсветку рюмке после налития
      delay(1000);
      statistika [n] ++; // Для статистики
      nalil[n] = 1; // Эта рюмка была налита
      result = true;
    }
  }
  return result;
}

void loop()
{
  strip.setBrightness(LED_yarkost);// яркость светодиодов по умолчанию  на 30
  
  //---------------------------------------------------------------------
  ml = (map((pump_timer), (0), (time_step*12), (0), (60))); // Маштабируем дозу в миллилитры
  LED_yarkost2 = (map((LED_yarkost), (0), (255), (0), (50))); // Маштабируем яркость
  volume2 = (map((volume), (0), (30), (0), (6))); // Маштабируем яркость
  //---------------------------------------------------------------------
  if (enc1.isHolded()) flag_meny = !flag_meny; // Определяем либо МЕНЮ либо РАБОЧИЙ

  //---------------------------------------------------------------------
  if (!flag_meny && !volume_setting) // РАБОЧИЙ
  {
    if (enc1.isRight()) rezim++;
    if (enc1.isLeft())rezim--;
    if (rezim > 5) rezim = 0; // Бегаем по РЕЖИМАМ
    //---------------------------------------------------------------------
    switch (rezim) // Выбор режима РАБОТЫ
    {
      //---------------------------------------------------------------------
      case 0: // РУчной РЕЖИМ
        //---------------------------------------------------------------------
        if (enc1.isHold()) volume_setting = !volume_setting; // Нажили кнопку ЭНКОДЕРА
        
        sost_rumka(); // Состояние рюмки
        mp3_rundom = random (0, 11); // ПРисваеваем рандом
        all_rumka_svet_on(); // Включаем красный цвет при наличии неналитой рюмки
        //---------------------------------------------------------------------
        all_rumka_svet_off (); // Выключаем свет рюмки если она не стоит
        //---------------------------------------------------------------------

        LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
        LCD.print("    <Ручной>         "); // печатаем символ на первой строке
        LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
        LCD.print("      "); // печатаем символ на первой строке
        LCD.setCursor(6, 1);// ставим курсор на 3 символ первой строки
        LCD.print(ml); // печатаем символ на первой строке
        LCD.setCursor(8, 1); // ставим курсор на 3 символ первой строки
        LCD.print(" Мл.      "); // печатаем символ на первой строке

        
        if (!digitalRead(15))
        {
          bool flaggg = true; 
          while (flaggg){
            flaggg = ruchnoy_naliv();
            all_rumka_svet_on();
            all_rumka_svet_off ();
            }
        if (vozvrat)
        {
          servoparking ();
          vozvrat = 0;
          delay(200);
          mp3_play (mp3_rundom);
          delay(8000);
          mp3_stop ();
          delay(100);

        }
      }
        
      break;

      //--------------------------------------------------------------------------------------
      case 1: // Режим РУССКАЯ РУЛЕТКА
        igroki_RUS = number_of_glasses();
        //--------------------------------------------------------------------------------------
        sost_rumka();
        list_of_gamers_rus(igroki_RUS);
        rus_rul = gamers_RUS[random (0, igroki_RUS)]; //Присваиваем переменной рандомное число
        all_rumka_svet_on ();
        //--------------------------------------------------------------------------------------
        all_rumka_svet_off ();
        //--------------------------------------------------------------------------------------
        if ((igroki_RUS > 1)){
          LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
          LCD.print("Русская Рулетка  "); // печатаем символ на первой строке
          LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
          LCD.print("Игроки - "); // печатаем символ на первой строке
          LCD.setCursor(9, 1); // ставим курсор на 3 символ первой строки
          LCD.print(igroki_RUS); // печатаем символ на первой строке
          LCD.setCursor(10, 1); // ставим курсор на 3 символ первой строки
          LCD.print("          "); // печатаем символ на первой строке
        }
        else{
          LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
          LCD.print("Русская Рулетка  "); // печатаем символ на первой строке
          LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
          LCD.print("Мало игроков!!!"); // печатаем символ на первой строке
          LCD.setCursor(16, 1); // ставим курсор на 3 символ первой строки
          LCD.print("          "); // печатаем символ на первой строке
          }
        nalich_rum();
        if (!digitalRead(15) && (igroki_RUS > 1)) // если нажали кнопку и больше 1 игрока.
        {
          delay(250);
          mp3_play (51);
          delay(100);

          for (byte r = 0; r < 6; r++) // Обыгрываем режи "Русская рулетка"
          {
            strip.setPixelColor(r, 0, 0, 245);
            strip.show();
            delay (1000);
          }

          vistrel(rus_rul, max_pump_timer);
          servoparking ();
          delay (100);
          mp3_stop ();
        }
        
        break;
      //--------------------------------------------------------------------------------------
      case 2: // Режим АМЕРИКАНСКАЯ РУЛЕТКА

        igroki_USA = number_of_glasses();
        sost_rumka();
        list_of_gamers_usa(igroki_USA);
        USA_rul = gamers_USA[random (0, igroki_USA)]; //Присваиваем переменной рандомное число
        all_rumka_svet_on ();
        //--------------------------------------------------------------------------------------
        all_rumka_svet_off ();
        //--------------------------------------------------------------------------------------

        LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
        LCD.print("Америк. Рулетка"    ); // печатаем символ на первой строке
        if (igroki_USA > 1){
          LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
          LCD.print("Игроки - "); // печатаем символ на первой строке
          LCD.setCursor(9, 1); // ставим курсор на 3 символ первой строки
          LCD.print(igroki_USA); // печатаем символ на первой строке
          LCD.setCursor(10, 1); // ставим курсор на 3 символ первой строки
          LCD.print("         "); // печатаем символ на первой строке
        }
        else{
          LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
          LCD.print("Мало игроков!!!"); // печатаем символ на первой строке
          LCD.setCursor(16, 1); // ставим курсор на 3 символ первой строки
          LCD.print("          "); // печатаем символ на первой строке
          }
        nalich_rum();
        if (!digitalRead(15) && flag_nalich_rum) // если нажали кнопку  и стоят рюмки.
        {
          delay(250);
          mp3_play (52);
          delay(100);
          for (byte r = 0; r < 6; r++) // Обыгрываем режи "Американская рулетка"
          {
            strip.setPixelColor(r, 0, 255, 196);
            strip.show();
            delay (1000);
          }

          for (byte i = 0; i < 6; i++)
          {
            if (i != USA_rul) vistrel(i, max_pump_timer);

          }
          servoparking ();
          delay (100);
          mp3_stop ();
        }



        break;
      //--------------------------------------------------------------------------------------------------
      case 3: // Режим ВЕСЕЛЫЕК ГОРКИ

        sost_rumka();
        all_rumka_svet_on ();
        //--------------------------------------------------------------------------------------
        all_rumka_svet_off ();
        //--------------------------------------------------------------------------------------

        LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
        LCD.print("<Веселые Горки>  "); // печатаем символ на первой строке
        LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
        LCD.print("                 "); // печатаем символ на первой строке

        if (!digitalRead(15)) // Нажили кнопку 
        {

          delay(250);
          mp3_play (53);
          delay(100);

          for (byte i = 0; i < 6; i++)
          {
            vistrel(i, random(min_pump_timer, max_pump_timer));
          }

          servoparking ();
          delay (100);
          mp3_stop ();
        }
        
        break;

      //------------------------------------------------------------------------------------
      case 4: // Автоматический РЕЖИМ
        if (enc1.isHold()) volume_setting = !volume_setting; // Нажили кнопку ЭНКОДЕРА
        sost_rumka();
        all_rumka_svet_on ();
        all_rumka_svet_off ();

        
        LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
        LCD.print("<Автоматический>"); // печатаем символ на первой строке
        LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
        LCD.print("      "); // печатаем символ на первой строке
        LCD.setCursor(6, 1);// ставим курсор на 3 символ первой строки
        LCD.print(ml); // печатаем символ на первой строке
        LCD.setCursor(8, 1); // ставим курсор на 3 символ первой строки
        LCD.print(" Мл.      "); // печатаем символ на первой строке

        bool flaggg = true; 
          while (flaggg){
            flaggg = ruchnoy_naliv();
            all_rumka_svet_on();
            all_rumka_svet_off ();
            sost_rumka();
            }
        if (vozvrat)
        {
          servoparking ();
          vozvrat = 0;
        }
              
        break;


    }

  }
  //----------------------------------------------------------------------------------
  else if (flag_meny && !volume_setting) //МЕНЮ
  {
    if (enc1.isRight())vibor_meny++ ;
    if (enc1.isLeft())vibor_meny--;
    if (vibor_meny > 5) vibor_meny = 0;
    if (vibor_meny < 0) vibor_meny = 0;

    switch (vibor_meny)
    {

      case 0:

        LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
        LCD.print("     <МЕНЮ>       "); // печатаем символ на первой строке
        LCD.setCursor(0, 1); // ставим курсор на 3 символ второй строки
        LCD.print("                 "); // печатаем символ на второй строке
        break;
      //----------------------------------------------------------------------------------
      case 1: // Промывка помпы
        rumka_svet_on (0);
        rumka_svet_off (0);
        LCD.setCursor(0, 0); // ставим курсор на 0 символ первой строки
        LCD.print(" Промывка помпы "); // печатаем символ на первой строке
        LCD.setCursor(0, 1); // ставим курсор на 3 символ второй строки
        LCD.print(" 50 мл, рюмка 1 "); // печатаем символ на второй строке

        if (digitalRead(7) && !digitalRead(15))
        {
          razliv (0);
          delay (500);
          pump(max_pump_timer, 0); // включаем помпу на 50 мл
          delay (1000);
          servoparking ();

        }

        break;

      //----------------------------------------------------------------------------------

      case 2: //ЯРКОСТЬ Светодиодов

        if (enc1.isRightH()) LED_yarkost += 51;
        if (enc1.isLeftH()) LED_yarkost -= 51;
        if (LED_yarkost >= 255) LED_yarkost = 255;
        if (LED_yarkost <= 10) LED_yarkost = 10;

        strip.setPixelColor(2, 245, 245, 245);
        strip.show();

        LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
        LCD.print("   <Яркость>      "); // печатаем символ на первой строке
        LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
        LCD.print(LED_yarkost2); // печатаем символ на первой строке
        LCD.setCursor(3, 1); // ставим курсор на 3 символ первой строки
        LCD.print("                "); // печатаем символ на первой строке
        break;
      //--------------------------------------------------------------------------------
      case 3: // Громкость
        if (enc1.isRightH()) volume += 5;
        if (enc1.isLeftH()) volume -= 5;
        if (volume > 30) volume = 30;
        if (volume <= 0) volume = 0;

        LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
        LCD.print("  <Громкость>    "); // печатаем символ на первой строке
        LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
        LCD.print(volume2); // печатаем символ на первой строке
        LCD.setCursor(1, 1); // ставим курсор на 3 символ первой строки
        LCD.print("                  "); // печатаем символ на первой строке

        strip.setPixelColor(2, 0, 0, 0);
        strip.show();

        break;
      //-----------------------------------------------------------------------------

      case 4: // СТАТИСТИКА

        if (enc1.isHold())
        {
          if (!trig)
          {
            trig = 1;
            statistika_per ++;
          }
        }
        else
        {
          trig = 0;
        }

        if (statistika_per >= 6) statistika_per = 0;

        LED_statistika (statistika_per);



        LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
        LCD.print("  <Статистика>    "); // печатаем символ на первой строке
        LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
        LCD.print("Стопка- "); // печатаем символ на первой строке
        LCD.setCursor(8, 1); // ставим курсор на 3 символ первой строки
        LCD.print(statistika_per + 1 ); // печатаем символ на первой строке
        LCD.setCursor(9, 1); // ставим курсор на 3 символ первой строки
        LCD.print("   "); // печатаем символ на первой строке
        LCD.setCursor(12, 1); // ставим курсор на 3 символ первой строки
        LCD.print(statistika [statistika_per]); // печатаем символ на первой строк

        break;



    }
  }
  else if (volume_setting)
        {
          if (enc1.isRight())pump_timer += time_step;
          if (enc1.isLeft())pump_timer -= time_step;
          if (pump_timer >= max_pump_timer) pump_timer = max_pump_timer;
          if (pump_timer <= min_pump_timer) pump_timer = min_pump_timer;
  
          LCD.setCursor(0, 0); // ставим курсор на 3 символ первой строки
          LCD.print("<Дозировка Мл.>  "); // печатаем символ на первой строке
          LCD.setCursor(0, 1); // ставим курсор на 3 символ первой строки
          LCD.print(ml); // печатаем символ на первой строке
          LCD.setCursor(2, 1); // ставим курсор на 3 символ первой строки
          LCD.print("                "); // печатаем символ на первой строке
          strip.setPixelColor(2, 0, 0, 0);
          strip.show();
          if (enc1.isHold()) {
            volume_setting = !volume_setting; // Нажили кнопку ЭНКОДЕРА
            delay (300);
          }
        }
}
