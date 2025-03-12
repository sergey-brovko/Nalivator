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

byte mode = 0;
bool volume_setting = false;
const int time_step = 202; // время работы помпы на объем 5мл
int pump_timer = 6*time_step; // время работы помпы начально на объем 30мл
const int max_pump_timer = 10*time_step, min_pump_timer = 2*time_step; // время работы помпы максимальный объем 50мл, минимальный объем 10мл


int polozenie = 0; // для определения угла сервы

//---------------------------------------------------------------------
void LCD_print(String s1, String s2){
    LCD.setCursor(0, 0); 
    LCD.print(s1); 
    LCD.setCursor(0, 1); 
    LCD.print(s2); 
}

//---------------------------------------------------------------------
class Glass{
public:
  bool filled;
  int pin;
  Glass(int g_pin, int g_num,  int g_position_angle){
    pin = g_pin;
    num = g_num;
    filled = false;
    position_angle = g_position_angle;
    amount = 0;
    }

  bool pour_shot(int v)  //Функция налива стопки
  {
    if (digitalRead(pin) && !filled) // Если поставили рюмку и она не была налита то нальем ее
    {
      turn_to_position(); // Вызываем функцию разлива
      delay (500);
      pump(v);
      strip.setPixelColor(num, 0, 255, 0);
      strip.show();
      delay(1000);
      amount ++; // Для статистики
      filled = true;
      return true;
    }
    return false;
  }
  void LED_glass_stands_on() // Функция включения света если рюмка стоит пустая
  {
    if (digitalRead(pin) && !filled)
    {
      strip.setPixelColor(num, 0, 0, 255);
      strip.show();
    }
  }
  void LED_glass_stands_off() // Функция выключения света если рюмка не стоит
  {
    if (!digitalRead(pin))
    {
      filled = false;
      strip.setPixelColor(num, 0, 0, 0);
      strip.show();
    }
  }

  static void servoparking ()// Функция ПАРКОВКИ
  {
    myservo.attach(12);
    for (int pos = myservo.read(); pos >= 0; pos -= 1)
    {
      myservo.write(pos);
      delay (8);
    }
    myservo.detach();
  }
  
private:  
  int amount;
  int num;
  int position_angle;
  
  void turn_to_position()
  {
    int current_pos = myservo.read();
    myservo.attach(12);
    int pos = current_pos;
    while ( pos != position_angle)
    {
      if (current_pos < position_angle) pos += 1;
      else pos -= 1;
      myservo.write(pos);
      delay (8);
    }
    myservo.detach();
  }
  
  void pump (int v) // Функция для работы помпы
  {
    digitalWrite (5, 1);
    strip.setPixelColor(num, 255, 0, 0);
    strip.show();
    for (int i = 0; i <= v; i += 1)
    {
      if (!digitalRead(pin)) break;
      delay (1);
    }
    digitalWrite (5, 0);
  }   
};
//---------------------------------------------------------------------



class Desk{
  Glass shots[6]= {
  Glass(7, 0, 22),
  Glass(8, 1, 50),
  Glass(9, 2, 83),
  Glass(10, 3, 115),
  Glass(11, 4, 143),
  Glass(14, 5, 172), 
};
  public:

  void manual_pouring(){
    byte mp3_rundom = random (0, 11); // ПРисваеваем рандом
    LED_all_shots_stands_on(); // Включаем синий цвет при наличии неналитой рюмки
    LED_all_shots_stands_off(); // Выключаем свет рюмки если она не стоит
    byte ml = (map((pump_timer), (0), (max_pump_timer), (0), (50))); // Маштабируем дозу в миллилитры
    LCD_print("  Ручной режим  ", "     "+String(ml)+ " мл.     "); //выводим на экран

    
    if (!digitalRead(15))
    {
      bool flaggg = true; 
      while (flaggg){
        flaggg = pouring();
        LED_all_shots_stands_on();
        LED_all_shots_stands_off();
        }
    Glass::servoparking();
    delay(200);
    mp3_play (mp3_rundom);
    delay(8000);
    mp3_stop ();
    delay(100);
      
    }
  } 

  void switchback(){
    LED_all_shots_stands_on(); // Включаем синий цвет при наличии неналитой рюмки
    LED_all_shots_stands_off(); // Выключаем свет рюмки если она не стоит
    byte ml = (map((pump_timer), (0), (max_pump_timer), (0), (50))); // Маштабируем дозу в миллилитры
    LCD_print("Весёлые горки    ", "                       "); //выводим на экран

    
    if (!digitalRead(15)) // Нажили кнопку 
    {
      delay(250);
      mp3_play (53);
      delay(100);
      int total = sizeof(shots) / sizeof(shots[0]);
      for (byte i = 0; i < total; i++)
      {
        int r = random(min_pump_timer, max_pump_timer);
        ml = (map((r), (0), (max_pump_timer), (0), (50))); // Маштабируем дозу в миллилитры
        LCD_print("Весёлые горки    ", String(ml) + " мл. "); //выводим на экран
        shots[i].pour_shot(r);
      }
      Glass::servoparking ();
      delay (100);
      mp3_stop ();
    }
  } 

  void auto_pouring(){
    LED_all_shots_stands_on();// Включаем синий цвет при наличии неналитой рюмки
    LED_all_shots_stands_off();// Выключаем свет рюмки если она не стоит
    byte ml = (map((pump_timer), (0), (max_pump_timer), (0), (50))); // Маштабируем дозу в миллилитры
    LCD_print("   Авто режим   ", "     "+String(ml)+ " мл.     "); //выводим на экран

    bool flaggg = true; 
      while (flaggg){
        flaggg = pouring();
        LED_all_shots_stands_on();
        LED_all_shots_stands_off();
        }
    Glass::servoparking ();
  }

  void russian_roulette(){
    int igroki = 0;
    int total = sizeof(shots) / sizeof(shots[0]);
    int gamers_RUS [total];
    for (int i = 0; i < total; i += 1)
    {
      if (digitalRead(shots[i].pin)) {
        gamers_RUS[igroki] = i; 
        igroki++;
      }
    }
    byte rus_rul = gamers_RUS[random (0, igroki)]; //Присваиваем переменной рандомное число
    LED_all_shots_stands_on();
    LED_all_shots_stands_off();
    //--------------------------------------------------------------------------------------
    if ((igroki > 1)){
      LCD_print("Русская Рулетка  ", "Игроки - "+String(igroki)+ "          "); //выводим на экран
    }
    else{
      LCD_print("Русская Рулетка  ", "Мало игроков    "); //выводим на экран
      }
    if (!digitalRead(15) && (igroki > 1)) // если нажали кнопку и больше 1 игрока.
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

      shots[rus_rul].pour_shot(pump_timer);
      Glass::servoparking ();
      delay (100);
      mp3_stop ();
    }
}

    void american_roulette(){
    int igroki = 0;
    int total = sizeof(shots) / sizeof(shots[0]);
    int gamers_USA [total];
    for (int i = 0; i < total; i ++)
    {
      if (digitalRead(shots[i].pin)) {
        gamers_USA[igroki] = i; 
        igroki++;
      }
    }
    byte usa_rul = gamers_USA[random (0, igroki)]; //Присваиваем переменной рандомное число
    LED_all_shots_stands_on();
    LED_all_shots_stands_off();
    //--------------------------------------------------------------------------------------
    if ((igroki > 1)){
      LCD_print("Америк. Рулетка  ", "Игроки - "+String(igroki)+ "          "); //выводим на экран
    }
    else{
      LCD_print("Америк. Рулетка  ", "Мало игроков    "); //выводим на экран
      }
    if (!digitalRead(15) && (igroki > 1)) // если нажали кнопку и больше 1 игрока.
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
      for (int i = 0; i < total; i ++){
      if (i != usa_rul) shots[i].pour_shot(pump_timer);
      }
      Glass::servoparking ();
      delay (100);
      mp3_stop ();
    }
    }    
  
private:
  void LED_all_shots_stands_on () // Функция включения света всех рюмок которые рюмок стоят пустые
  {
    int total = sizeof(shots) / sizeof(shots[0]);
    for (int i = 0; i < total; i ++)
    {
      shots[i].LED_glass_stands_on();
    }
  }
  
  void LED_all_shots_stands_off () // Функция выключения света всех рюмок которые не стоят
  {
    int total = sizeof(shots) / sizeof(shots[0]);
    for (int i = 0; i < total; i ++)
    {
      shots[i].LED_glass_stands_off();
    }
  }
  
  bool pouring() //Функция для разлива по стопкам в ручном и автоматическом режимах
  {
    bool result = false;
    int total = sizeof(shots) / sizeof(shots[0]);
    for (int i = 0; i < total; i ++)
    {
      if (shots[i].pour_shot(pump_timer)) result = true;
    }
    return result;
  }
};

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

Desk my_desk;

//---------------------------------------------------------------------------------------------
void setup()
{
  Init();// инициализация

  myservo.attach(12);
  myservo.write(0);
  delay (1000);
  myservo.detach();

  LCD_print("     ЛёхБух     ", "      2025      ");

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


void loop()
{
  strip.setBrightness(30);// яркость светодиодов по умолчанию  на 30
  
  //---------------------------------------------------------------------
  if (!volume_setting) // РАБОЧИЙ
  {
    if (enc1.isRight()) mode++;
    if (enc1.isLeft())mode--;
    if (mode > 4) mode = 0; // Бегаем по РЕЖИМАМ
    //---------------------------------------------------------------------
    switch (mode) // Выбор режима РАБОТЫ
    {
      //---------------------------------------------------------------------
      case 0: // РУчной РЕЖИМ
        //---------------------------------------------------------------------
        if (enc1.isHold()) volume_setting = !volume_setting; // Нажили кнопку ЭНКОДЕРА
        my_desk.manual_pouring();
      break;

      //--------------------------------------------------------------------------------------
      case 1: // Режим РУССКАЯ РУЛЕТКА
        my_desk.russian_roulette();
      break;
      //--------------------------------------------------------------------------------------
      case 2: // Режим АМЕРИКАНСКАЯ РУЛЕТКА
        my_desk.american_roulette();
      break;
      //--------------------------------------------------------------------------------------------------
      case 3: // Режим ВЕСЕЛЫЕК ГОРКИ
        my_desk.switchback();  
      break;

      //------------------------------------------------------------------------------------
      case 4: // Автоматический РЕЖИМ
        if (enc1.isHold()) volume_setting = !volume_setting; // Нажили кнопку ЭНКОДЕРА
        my_desk.auto_pouring();
      break;
    }

  }
  //----------------------------------------------------------------------------------
  
  else
  {
    if (enc1.isRight())pump_timer += time_step;
    if (enc1.isLeft())pump_timer -= time_step;
    if (pump_timer >= max_pump_timer) pump_timer = max_pump_timer;
    if (pump_timer <= min_pump_timer) pump_timer = min_pump_timer;

    byte ml = (map((pump_timer), (0), (max_pump_timer), (0), (50))); // Маштабируем дозу в миллилитры
    LCD_print(" <Дозировка Мл.> ", "       " + String(ml) + "       ");
    
    if (enc1.isHold()) {
      volume_setting = !volume_setting; // Нажили кнопку ЭНКОДЕРА
      delay (300);
    }
  }
}
