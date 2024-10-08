#include <Adafruit_Fingerprint.h>
#define BTN1 D5
#define BTN2 D6

#define LEDG D1
#define RELY D4
#define LEDR D7

#include <tuple>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(D2, D3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id = 1;
uint8_t mode = 0;
bool is_btn_down = false;
uint64_t time_btn_down = 0;
bool is_ready_for_next = true;
bool is_relay_on = false;
bool is_led_blink = false;
uint64_t time_led = 0;

void setup()
{
  pinMode(RELY, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);

  Serial.begin(9600);
  while (!Serial)
    ; // DON'T KNOW BUT IT'S IMPORTANT!
  delay(100);

  Serial.println("\nFINGERPRINT PROJECT");
  finger.begin(57600);

  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1)
    {
      delay(1);
    }
  }

  // FINGERPRINT SENSOR CHECK
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);

  pinMode(BTN1, INPUT);
}

// =================== USER FUNCTIONSSSSSSSS ===================================================>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
std::tuple<bool, uint8_t> getFingerprintEnroll()
{

  int p = -1;
  if (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Fingerprint terdeteksi");
      break;
    case FINGERPRINT_NOFINGER:
      return std::make_tuple(false, p);
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return std::make_tuple(false, p);
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return std::make_tuple(false, p);
    default:
      Serial.println("Unknown error");
      return std::make_tuple(false, p);
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Fingerprint dikonvesi (ntah bagaimana maksudnya)");
    break;
  case FINGERPRINT_IMAGEMESS:
    return std::make_tuple(false, p);
  case FINGERPRINT_PACKETRECIEVEERR:
    return std::make_tuple(false, p);
  case FINGERPRINT_FEATUREFAIL:
    return std::make_tuple(false, p);
  case FINGERPRINT_INVALIDIMAGE:
    return std::make_tuple(false, p);
  default:
    return std::make_tuple(false, p);
  }

  Serial.println("Angkat jari pls..");

  while (finger.getImage() != FINGERPRINT_NOFINGER)
    ;

  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("letakkan jari yang sama");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Fingerprint terdeteksi lagi..");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      return std::make_tuple(false, p);
    case FINGERPRINT_IMAGEFAIL:
      return std::make_tuple(false, p);
    default:
      return std::make_tuple(false, p);
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Fingerprint dikonvesi lagi");
    break;
  case FINGERPRINT_IMAGEMESS:
    return std::make_tuple(false, p);
  case FINGERPRINT_PACKETRECIEVEERR:
    return std::make_tuple(false, p);
  case FINGERPRINT_FEATUREFAIL:
    return std::make_tuple(false, p);
  case FINGERPRINT_INVALIDIMAGE:
    return std::make_tuple(false, p);
  default:
    return std::make_tuple(false, p);
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Nice!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    return std::make_tuple(false, p);
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Jari nya ga sama!");
    return std::make_tuple(false, p);
  }
  else
  {
    return std::make_tuple(false, p);
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Tersimpan!");
    return std::make_tuple(true, id);
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    return std::make_tuple(false, p);
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    return std::make_tuple(false, p);
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    return std::make_tuple(false, p);
  }
  else
  {
    return std::make_tuple(false, p);
  }

  id += 1;
  return std::make_tuple(true, (uint8_t)(id - 1));
}

std::tuple<bool, uint8_t> getFingerprintID()
{
  uint8_t p = finger.getImage();
  switch (p)
  {
  case FINGERPRINT_OK:
    break;
  case FINGERPRINT_NOFINGER:
    return std::make_tuple(false, p);
  case FINGERPRINT_PACKETRECIEVEERR:
    return std::make_tuple(false, p);
  case FINGERPRINT_IMAGEFAIL:
    return std::make_tuple(false, p);
  default:
    return std::make_tuple(false, p);
  }

  // OK success!

  p = finger.image2Tz();
  switch (p)
  {
  case FINGERPRINT_OK:
    break;
  case FINGERPRINT_IMAGEMESS:
    return std::make_tuple(false, p);
  case FINGERPRINT_PACKETRECIEVEERR:
    return std::make_tuple(false, p);
  case FINGERPRINT_FEATUREFAIL:
    return std::make_tuple(false, p);
  case FINGERPRINT_INVALIDIMAGE:
    return std::make_tuple(false, p);
  default:
    return std::make_tuple(false, p);
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
    // found a match!
    return std::make_tuple(true, finger.fingerID);
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    return std::make_tuple(false, p);
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    return std::make_tuple(false, p);
  }
  else
  {
    return std::make_tuple(false, p);
  }
}

std::tuple<bool, uint8_t> deleteFingerprint(uint8_t id)
{
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK)
  {
    return std::make_tuple(true, id);
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    return std::make_tuple(false, p);
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    return std::make_tuple(false, p);
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    return std::make_tuple(false, p);
  }
  else
  {
    return std::make_tuple(false, p);
  }
}

void wait_for_fingerprint_to_clear()
{
  while (finger.getImage() != FINGERPRINT_NOFINGER)
    ;
}

// ====================================== LOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOPPPP =========================================== //
void loop()
{
  if (mode == 0)
  {
    std::tuple<bool, uint8_t> result = getFingerprintEnroll();
    // Check if the fingerprint detects a finger
    if (std::get<1>(result) != FINGERPRINT_NOFINGER)
    {
      wait_for_fingerprint_to_clear();
    }
    else if (millis() > time_led)
    {
      is_led_blink = !is_led_blink;
      digitalWrite(LEDG, is_led_blink);
      time_led = millis() + 1000;
    }
  }
  else if (mode == 1)
  {
    std::tuple<bool, uint8_t> result = getFingerprintID();
    if (std::get<0>(result))
    {
      Serial.print("Fingerprint terdaftar dengan ID #");
      Serial.println(finger.fingerID);
      Serial.print("Tingkat kesamaan : ");
      Serial.print(finger.confidence);
      Serial.println("%");

      is_relay_on = !is_relay_on;
      digitalWrite(RELY, is_relay_on);
      Serial.print("is_relay_on :");
      Serial.println(is_relay_on);
      wait_for_fingerprint_to_clear();
    }
    else if (std::get<1>(result) == FINGERPRINT_NOTFOUND)
    {
      Serial.println("Fingerprint tidak terdaftar");
      wait_for_fingerprint_to_clear();
    }
    else if (millis() > time_led)
    {
      is_led_blink = !is_led_blink;
      digitalWrite(LEDR, is_led_blink);
      time_led = millis() + 1000;
    }
  }
  else if (mode == 2)
  {
    std::tuple<bool, uint8_t> result = getFingerprintID();
    if (std::get<0>(result))
    {
      uint8_t grabbed_id = std::get<1>(result);
      deleteFingerprint(grabbed_id);
      Serial.print("Fingerprint dengan id #");
      Serial.print(grabbed_id);
      Serial.println(" dihapus!");
      wait_for_fingerprint_to_clear();
    }
    else
    {
      digitalWrite(LEDR, HIGH);
    }
  }

  if (digitalRead(BTN1))
  {
    if (!is_btn_down && millis() > time_btn_down)
    {
      Serial.print("CHANGE TO ");
      if (mode == 0)
      {
        Serial.println("CHECKING..");
        Serial.println("Ready to check a fingerprint!");
        mode = 1;
      }
      else if (mode == 1)
      {
        Serial.println("DELETING..");
        Serial.println("Ready to delete a fingerprint!");
        mode = 2;
      }
      else if (mode == 2)
      {
        Serial.println("ENROLLING..");
        Serial.println("Ready to enroll a fingerprint!");
        mode = 0;
      }
      is_btn_down = true;
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDR, LOW);
    }
  }
  if (digitalRead(BTN2))
  {
    if (!is_btn_down && millis() > time_btn_down)
    {
      Serial.print("CHANGE TO ");
      if (mode == 0)
      {
        Serial.println("CHECKING..");
        Serial.println("Ready to check a fingerprint!");
        mode = 1;
      }
      else if (mode == 1)
      {
        Serial.println("DELETING..");
        Serial.println("Ready to delete a fingerprint!");
        mode = 2;
      }
      else if (mode == 2)
      {
        Serial.println("ENROLLING..");
        Serial.println("Ready to enroll a fingerprint!");
        mode = 0;
      }
      is_btn_down = true;
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDR, LOW);
    }
  }
  else
  {
    if (is_btn_down)
    {
      time_btn_down = millis() + 100;
    }
    is_btn_down = false;
  }
}