#include <Arduino.h>

#define MODE_MATCH    0X01
#define MODE_ENROLL   0X02
#define MODE_DELETE   0X03

#include <SX1508.h>
SX1508 sx;

#include <FPM.h>
HardwareSerial fserial(2);
FPM finger(&fserial);
FPM_System_Params params;

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int lineNow = 0;

#define BUTTONOK digitalRead(34)
#define BUTTONA digitalRead(35)

uint8_t readNumber(){
  uint num = 0;
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

bool passwordCheck(){
    unsigned long millchk = 0;

    while(BUTTONA == HIGH || BUTTONOK == HIGH) delay(50);
    Serial.println("Start pushed");
    millchk = millis();
    delay(1000);
    while(BUTTONA == LOW || BUTTONOK == LOW) delay(50);
    Serial.println("Done pushed");

    if (millis() - millchk > 10000){
        return true;    
    }
    return false;
}

void matchMode();
void enrollMode();
void deleteMode();

void customDisplay(String str, int line);
uint8_t homeMenu();

void setup()
{
    Serial.begin(115200);
    pinMode(34, INPUT_PULLUP);
    pinMode(35, INPUT_PULLUP);

    Wire.begin();
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
        Serial.println(F("Failed init display"));
        while(1) delay(10); // Don't proceed, loop forever
    }

    customDisplay("Haii..", 0);
    bool pass = passwordCheck();
    if (!pass){
        Serial.println("Failed to pass");
        customDisplay("Who are you?", 1);
    }
    Serial.println("Okay to pass");
    customDisplay("Tayo", 1);

    sx.begin();

    fserial.begin(57600, SERIAL_8N1, 14, 12);
    if (finger.begin()) {
        finger.readParams(&params);
        Serial.println("Found fingerprint sensor!");
        Serial.print("Capacity: "); Serial.println(params.capacity);
        Serial.print("Packet length: "); Serial.println(FPM::packet_lengths[params.packet_len]);
    } else {
        Serial.println("Did not find fingerprint sensor :(");
        while (1) yield();
    }

    // uint8_t mode = readNumber();
    customDisplay("Choose mode", 2);
    delay(2000);
    uint8_t mode = homeMenu();
    switch(mode){
        case 0: matchMode();  break;
        case 1: enrollMode(); break;
        case 2: deleteMode(); break;
    }
}

void loop()
{
  yield();
}

void displayMenu(int page){
    display.clearDisplay();

    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(10,10);             // Start at top-left corner

    if (page == 0){
        display.println(F("Matching"));
    } else if (page == 1) {
        display.println(F("Enrolling"));
    } else {
        display.println(F("Deleting"));
    }
    display.display();
}

uint8_t homeMenu(){
    int sw = 0;

    while(BUTTONOK == HIGH){
        delay(1000);
        sw += 1;
        if (sw > 2) sw = 0;
        displayMenu(sw);
        while(BUTTONA == HIGH && BUTTONOK == HIGH) delay(50);
        Serial.println(sw);
    }
    Serial.print("page"); Serial.println(sw);
    return sw;
}

void validFingerDisplay(bool valid){
    display.clearDisplay();

    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,10);             // Start at top-left corner
    if (valid){
        display.println("Jari cocok");
    } else {
        display.println("Coba lagi");
    }
    display.display();
}

void customDisplay(String str, int line){

    if (line <= lineNow || line > 3){
        display.clearDisplay();
        display.setTextSize(1);             // Normal 1:1 pixel scale
        display.setTextColor(WHITE);        // Draw white text
        display.setCursor(0,0);             // Start at top-left corner

        lineNow = 0;
    } else {
        lineNow = line;
    }
    display.println(str);
    display.display();
}

int search_database(uint16_t &fid) {
    int16_t p = -1;

    /* first get the finger image */
    Serial.println("Waiting for valid finger");
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");
                break;
            case FPM_NOFINGER:
                Serial.print(".");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FPM_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                break;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                break;
            default:
                Serial.println("Unknown error");
                break;
        }
        yield();
    }

    /* convert it */
    p = finger.image2Tz();
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            return p;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    p = 0;
    // while (p != FPM_NOFINGER) {
    //     p = finger.getImage();
    //     yield();
    // }
    // Serial.println();

    /* search the database for the converted print */
    uint16_t score;
    p = finger.fingerFastSearch(&fid, &score);
    if (p == FPM_OK) {
        Serial.println("Found a print match!");
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_NOTFOUND) {
        Serial.println("Did not find a match");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    // found a match!
    Serial.print("Found ID #"); Serial.print(fid);
    Serial.print(" with confidence of "); Serial.println(score);
    return p;
}


void matchMode(){
  Serial.println("Mathing mode");
  customDisplay("Matching Mode", 0);
  delay(2000);
  while(1){
    // Serial.println("Send any character to search for a print...");
    // while (Serial.available() == 0) yield();
    delay(100);
    uint16_t fid = -1;
    if (search_database(fid) == FPM_OK){
        validFingerDisplay(true);
        sx.SendMsg(MODE_MATCH, fid);
        delay(2000);
        customDisplay("Waiting finger..", 0);
    } else {
        validFingerDisplay(false);
        delay(2000);
        customDisplay("Waiting finger..", 0);
    }
  }
}

bool get_free_id(int16_t * fid) {
    int16_t p = -1;
    for (int page = 0; page < (params.capacity / FPM_TEMPLATES_PER_PAGE) + 1; page++) {
        p = finger.getFreeIndex(page, fid);
        switch (p) {
            case FPM_OK:
                if (*fid != FPM_NOFREEINDEX) {
                    Serial.print("Free slot at ID ");
                    Serial.println(*fid);
                    return true;
                }
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error!");
                return false;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                return false;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                return false;
            default:
                Serial.println("Unknown error!");
                return false;
        }
        yield();
    }
}

int16_t enroll_finger(int16_t fid) {
    int16_t p = -1;
    Serial.println("Waiting for valid finger to enroll");
    customDisplay("Waiting finger to enroll", 0);
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");
                customDisplay("Image taken", 1);
                break;
            case FPM_NOFINGER:
                Serial.print(".");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FPM_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                break;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                break;
            default:
                Serial.println("Unknown error");
                break;
        }
        yield();
    }
    // OK success!

    p = finger.image2Tz(1);
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");
            customDisplay("Image Converted", 2);
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            return p;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    Serial.println("Remove finger");
    customDisplay("Remove Finger", 3);
    delay(2000);
    p = 0;
    while (p != FPM_NOFINGER) {
        p = finger.getImage();
        yield();
    }

    p = -1;
    Serial.println("Place same finger again");
    customDisplay("Place same finger", 0);
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");
                customDisplay("Image taken", 1);
                break;
            case FPM_NOFINGER:
                Serial.print(".");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FPM_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                break;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                break;
            default:
                Serial.println("Unknown error");
                break;
        }
        yield();
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");
            customDisplay("Image Converted", 2);
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            return false;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            return false;
        default:
            Serial.println("Unknown error");
            return p;
    }


    // OK converted!
    p = finger.createModel();
    if (p == FPM_OK) {
        Serial.println("Prints matched!");
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_ENROLLMISMATCH) {
        Serial.println("Fingerprints did not match");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("ID "); Serial.println(fid);
    p = finger.storeModel(fid);
    if (p == FPM_OK) {
        Serial.println("Stored!");
        return FPM_OK;
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_BADLOCATION) {
        Serial.println("Could not store in that location");
        return p;
    } else if (p == FPM_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }
}

void enrollMode(){
  Serial.println("Enroll Mode");
  customDisplay("Enroll Mode", 0);
  delay(2000);

  while(1){
    customDisplay("Wait to start", 0);
    while (BUTTONOK == HIGH) delay(50);

    customDisplay("Search for id..", 1);
    Serial.println("Searching for a free slot to store the template...");
    int16_t fid = -1;
    if (get_free_id(&fid)){
        if (enroll_finger(fid) == FPM_OK){
            customDisplay("Enroll success", 0);
            String str = "ID: " + String(fid);
            customDisplay(str, 1);
            sx.SendMsg(MODE_ENROLL, fid);
            delay(5000);
        } else {
            customDisplay("Failed enroll", 0);
            delay(2000);
        }
    }
    else {
        Serial.println("No free slot in flash library!");
        customDisplay("Database full", 2);
    }
  }
}

int deleteFingerprint(int fid) {
    int p = -1;

    p = finger.deleteModel(fid);

    if (p == FPM_OK) {
        Serial.println("Deleted!");
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
    } else if (p == FPM_BADLOCATION) {
        Serial.println("Could not delete in that location");
    } else if (p == FPM_FLASHERR) {
        Serial.println("Error writing to flash");
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
    } else {
        Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    }
    return p;
}

uint8_t pickId(){
    int maxid = 130;
    int pickedid = 0;

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10,10);
    display.println("0");
    display.display();

    while (BUTTONOK == HIGH){
        if (BUTTONA == LOW){
            pickedid += 1;
            if (pickedid > maxid) {
                pickedid = 0;
            }
            Serial.print("pickedid: ");
            Serial.println(pickedid);
            String str = String(pickedid);

            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(10,10);
            display.println(str);
            display.display();

            delay(400);
        }
        delay(50);
    }
    return pickedid;
}

void deleteMode(){
  Serial.println("Delete Mode");
  customDisplay("Delete Mode", 0);
  delay(2000);

  while(1){
    Serial.println("Enter the finger ID # you want to delete...");
    customDisplay("Pick id after this text hidden", 0);
    delay(2000);

    int fid = pickId();
    String str = String(fid);
    customDisplay("Deleting ID: ", 0);
    customDisplay(str, 1);
    Serial.print("Deleting ID #");
    Serial.println(fid);


    if (deleteFingerprint(fid) == FPM_OK){
        customDisplay("Success", 2);
        sx.SendMsg(MODE_DELETE, fid);
    }
    else {
        customDisplay("Failed", 2);
    }
    delay(4000);
  }
}