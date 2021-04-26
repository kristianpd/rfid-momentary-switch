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

#define RST_PIN D3  // Configurable, see typical pin layout above
#define SS_1_PIN D8 // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2
#define SS_2_PIN D2 // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 1

#define STATUS_PIN_1 D0
#define STATUS_PIN_2 D1

#define NR_OF_READERS 2

byte ssPins[] = {SS_1_PIN, SS_2_PIN};

MFRC522 mfrc522[NR_OF_READERS]; // Create MFRC522 instance.

bool rfid1Status = false;
bool rfid2Status = false;

void clearStatuses()
{
  rfid1Status = false;
  rfid2Status = false;
}

/**
 * Initialize.
 */
void setup()
{

  Serial.begin(9600); // Initialize serial communications with the PC

  clearStatuses();
  pinMode(STATUS_PIN_1, OUTPUT);
  pinMode(STATUS_PIN_2, OUTPUT);

  while (!Serial)
    ; // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  SPI.begin(); // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
  {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
}

void toggleRFID(uint8_t rfidIndex, bool status)
{
  // TODO: check against programmed RFIDs
  if (rfidIndex == 0)
  {
    rfid1Status = status;
  }
  else
  {
    rfid2Status = status;
  }
}

void updateRFIDStatuses()
{
  for (uint8_t i = 0; i < NR_OF_READERS; i++)
  {
    if (mfrc522[i].PICC_IsNewCardPresent())
    {
      toggleRFID(i, true);

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
  updateRFIDStatuses();
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
