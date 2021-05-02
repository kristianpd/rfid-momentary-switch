/**
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from more than one PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 *
 * Example sketch/program showing how to read data from more than one PICC (that is: a RFID Tag or Card) using a
 * MFRC522 based RFID Reader on the Arduino SPI interface.
 *
 * Warning: This may not work! Multiple devices at one SPI are difficult and cause many trouble!! Engineering skill
 *          and knowledge are required!
 *
 * @license Released into the public domain.
 *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS 1    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
 * SPI SS 2    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 */

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#define PROG_PIN D0
#define STATUS_PIN_1 D1
#define STATUS_PIN_2 D2
#define SS_1_PIN D3
#define RST_PIN D4
// RFID_X USED BY MFRC522 LIB - DO NOT USE
#define RFID_SCK_PIN D5
#define RFID_MISO_PIN D6
#define RFID_MOSI_PIN D7
#define SS_2_PIN D8

#define DEBUG 1

#define NR_OF_READERS 2
#define EEPROM_RFID_UID_1_ADDR 0
#define EEPROM_RFID_UID_2_ADDR 4
#define RFID_SIZE 4

byte ssPins[] = {SS_1_PIN, SS_2_PIN};

byte rfid1Target[4] = {0xFF, 0xFF, 0xFF, 0xFF};
byte rfid2Target[4] = {0xFF, 0xFF, 0xFF, 0xFF};

#define RFID_1_TARGET 0
#define RFID_2_TARGET 1

MFRC522 mfrc522[NR_OF_READERS];

bool rfid1Status = false;
bool rfid2Status = false;
bool isProgramming = false;

void clearStatuses()
{
  rfid1Status = false;
  rfid2Status = false;
}

void readTargetRFIDs()
{
  EEPROM.get(EEPROM_RFID_UID_1_ADDR, rfid1Target);
  EEPROM.get(EEPROM_RFID_UID_2_ADDR, rfid2Target);

  if (DEBUG)
  {
    Serial.print("RFID_1_TARGET: ");
    dump_byte_array(rfid1Target, RFID_SIZE);
    Serial.print("RFID_2_TARGET: ");
    dump_byte_array(rfid2Target, RFID_SIZE);
  }
}

/**
 * Initialize.
 */
void setup()
{

  Serial.begin(9600); // Initialize serial communications with the PC

  // while (!Serial)
  //   ;

  readTargetRFIDs();

  clearStatuses();
  pinMode(STATUS_PIN_1, OUTPUT);
  pinMode(STATUS_PIN_2, OUTPUT);
  pinMode(PROG_PIN, INPUT);

  SPI.begin(); // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
  {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    if (DEBUG)
    {
      Serial.print(F("Reader "));
      Serial.print(reader);
      Serial.print(F(": "));
      mfrc522[reader].PCD_DumpVersionToSerial();
    }
  }
}

void toggleRFID(uint8_t rfidIndex, bool status)
{
  // TODO: check against programmed RFIDs
  if (rfidIndex == RFID_1_TARGET)
  {
    rfid1Status = status;
  }
  else
  {
    rfid2Status = status;
  }
}

void setTargetUID(byte target, byte *uid)
{
  if (target == RFID_1_TARGET)
  {
    setUID(rfid1Target, uid);
    EEPROM.put(EEPROM_RFID_UID_1_ADDR, rfid1Target);
  }
  else if (target == RFID_2_TARGET)
  {
    setUID(rfid2Target, uid);
    EEPROM.put(EEPROM_RFID_UID_2_ADDR, rfid2Target);
  }
}

void setUID(byte *target, byte *uid)
{
  for (int i = 0; i < RFID_SIZE; i++)
  {
    target[i] = uid[i];
  }
}

bool rfidValid(byte *target, byte *uid)
{
  bool valid = true;
  for (int i = 0; i < RFID_SIZE; i++)
  {
    Serial.println(valid && ((target[i] | uid[i]) == target[i]));
    valid = valid && ((target[i] | uid[i]) == target[i]);
  }
  return valid;
}

void checkRFIDs()
{
  for (uint8_t i = 0; i < NR_OF_READERS; i++)
  {
    if (mfrc522[i].PICC_IsNewCardPresent())
    {
      Serial.println("card present");
      if (isProgramming)
      {
        setTargetUID(i, mfrc522[i].uid.uidByte);
      }
      else if (i == RFID_1_TARGET && rfidValid(rfid1Target, mfrc522[i].uid.uidByte) || i == RFID_2_TARGET && rfidValid(rfid2Target, mfrc522[i].uid.uidByte))
      {
        toggleRFID(i, true);
      }

      // Halt PICC
      mfrc522[i].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[i].PCD_StopCrypto1();
    }
    else
    {
      toggleRFID(i, false);
    }
  }
}

void setStatusPins()
{
  digitalWrite(STATUS_PIN_1, rfid1Status ? HIGH : LOW);
  digitalWrite(STATUS_PIN_2, rfid2Status ? HIGH : LOW);
}

/**
 * Main loop.
 */
void loop()
{
  checkRFIDs();
  setStatusPins();
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
