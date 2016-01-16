#include <LiquidCrystal.h>
#include <dht.h>

int interval = 5000;
unsigned long interval_buff = 0;
int interval_up = 1000;
unsigned long interval_up_buff = 0;

char get_buff[4];
int utm[3];
int (&u)[3] = utm;

DHT sens = DHT();
int t = 0;
int h = 0;

static const uint8_t PROGMEM dscrc_table[] = {
  0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
  157, 195, 33, 127, 252, 162, 64, 30, 95,  1, 227, 189, 62, 96, 130, 220,
  35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93,  3, 128, 222, 60, 98,
  190, 224,  2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
  70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89,  7,
  219, 133, 103, 57, 186, 228,  6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
  101, 59, 217, 135,  4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
  248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91,  5, 231, 185,
  140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
  17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
  175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
  50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
  202, 148, 118, 40, 171, 245, 23, 73,  8, 86, 180, 234, 105, 55, 213, 139,
  87,  9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
  233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
  116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

LiquidCrystal lcd(4, 5, 10, 11, 12, 13);

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 4);
  sens.attach(A0);
  print_lcd();
}

void loop() {
  if (Serial.available()) { //Передаем
    Serial.readBytes(get_buff, 3);
    if (strcmp(get_buff, "GET") == 0) {
      snd_data(t, h);
      memset(utm, 0, sizeof(utm));
    }
    memset(get_buff, 0, sizeof(get_buff));
  }
  else { //Автономно работаем
    if (millis() - interval_buff > interval) {
      upd_sens(&t, &h);
      print_temp(t);
      print_hum(h);
      interval_buff = millis();
    }
    if (millis() - interval_up_buff > interval_up) {
      uptime_data(5, u);
      interval_up_buff = millis();
    }
  }
}

void upd_sens(int *temp, int *hum) {
  sens.update();
  if (sens.getLastError() == DHT_ERROR_OK) {
    *temp = sens.getTemperatureInt();
    *hum = sens.getHumidityInt();
  }
  print_status(sens.getLastError());
}

void snd_data(int temp, int hum) {
  char buffer[32];
  char buf_crc[8];
  char crc;
  sprintf(buffer, "T:%d:H:%d", temp, hum);
  crc = crc8(buffer, strlen(buffer));
  sprintf(buf_crc, ":%d", crc);
  strcat(buffer, buf_crc);
  Serial.println(buffer);
}

void print_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Te\xbc\xbe""epa\xbf""ypa:");
  lcd.setCursor(0, 1);
  lcd.print("B\xbb""a\xb6\xbd""oc\xbf\xc4"":");
  lcd.setCursor(0, 2);
  lcd.print("C\xbf""a\xbf""yc:");
  lcd.setCursor(0, 3);
  lcd.print("Last:");
}

void print_temp(int temp) {
  int tb = 0;

  if (temp >= 0 && temp <= 9) tb = 3;
  else if (temp >= -9 && temp <= -1 || temp >= 10 && temp <= 99) tb = 2;
  else if (temp >= -99 && temp <= -10 || temp >= 100 && temp <= 999) tb = 1;

  lcd.setCursor(12, 0);
  for (int i = 0; i < tb; i++) {
    lcd.print(" ");
  }
  lcd.print(temp);
}

void print_hum(int hum) {
  int th = 0;

  if (hum >= 0 && hum <= 9) th = 4;
  else if (hum >= -9 && hum <= -1 || hum >= 10 && hum <= 99) th = 3;
  else if (hum >= -99 && hum <= -10 || hum >= 100 && hum <= 999) th = 2;

  lcd.setCursor(10, 1);
  for (int i = 0; i < th; i++) {
    lcd.print(" ");
  }
  lcd.print(hum);
  lcd.print("%");
}

void uptime_data(int offset, int t[]) {
  t[2]++;
  if (t[2] >= 60) {
    t[2] = 0;
    t[1]++;
  }
  if (t[1] >= 60) {
    t[1] = 0;
    t[0]++;
  }
  lcd.setCursor(offset, 3);

  if (t[0] < 10) {
    for (int i = offset; i < 9; i++) lcd.print(" ");
    lcd.print(t[0]);
  }
  else if (t[0] >= 10 && t[0] < 100) {
    for (int i = offset; i < 8; i++) lcd.print(" ");
    lcd.print(t[0]);
  }
  else if (t[0] >= 100 && t[0] < 1000) {
    for (int i = offset; i < 7; i++) lcd.print(" ");
    lcd.print("\xd9"" ");
  }
  else if (t[0] >= 1000 && t[0] < 10000) {
    for (int i = offset; i < 6; i++) lcd.print(" ");
    lcd.print(t[0]);
  }
  lcd.print(":");
  if (t[1] < 10) {
    lcd.print("0");
    lcd.print(t[1]);
  }
  else {
    lcd.print(t[1]);
  }
  lcd.print(":");
  if (t[2] < 10) {
    lcd.print("0");
    lcd.print(t[2]);
  }
  else {
    lcd.print(t[2]);
  }
}

void print_status(int err) {
  lcd.setCursor(7, 2);
  if (err == DHT_ERROR_OK) {
    for (int i = 7; i < 14; i++) lcd.print(" ");
    lcd.print("OK");
  }
  else if (err == DHT_ERROR_START_FAILED_1) {
    for (int i = 7; i < 11; i++) lcd.print(" ");
    lcd.print("E:SF1");
  }
  else if (err == DHT_ERROR_START_FAILED_2) {
    for (int i = 7; i < 11; i++) lcd.print(" ");
    lcd.print("E:SF2");
  }
  else if (err == DHT_ERROR_READ_TIMEOUT) {
    for (int i = 7; i < 11; i++) lcd.print(" ");
    lcd.print("E:RTO");
  }
  else if (err == DHT_ERROR_CHECKSUM_FAILURE) {
    for (int i = 7; i < 11; i++) lcd.print(" ");
    lcd.print("E:CSF");
  }
}

void cl_lcd(int n, int s, int e) {
  lcd.setCursor(s, n - 1);
  for (int i = 0; i <= e; i++) lcd.print(" ");
}

uint8_t crc8( char *addr, uint8_t len)
{
  uint8_t crc = 0;
  while (len--) {
    crc = pgm_read_byte(dscrc_table + (crc ^ *addr++));
  }
  return crc;
}
