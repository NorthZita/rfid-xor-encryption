#include <SPI.h>

#include <MFRC522.h>
#include <WiFi.h>
#define BUZZER 16
#define SS_PIN 5
#define RST_PIN 33
#include <HTTPClient.h>
#define WIFI_SSID "and"
#define WIFI_PASSWORD "qwerty12345"

const char* serverName = "http://192.168.43.185/esp/register.php";
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
String sharekey[] = {"ABC123", "BCD234", "FG56", "FG56", "FG56", "FG56", "FG56", "FG56"};
String input;
String vname = "";
String vnim = "";
MFRC522 mfrc522(SS_PIN, RST_PIN);

bool failsignup = false;
bool signupok = true;

unsigned long dataMillis = 0;
int count = 0;
bool signupOK = false;

String email = "";
String password = "";
String mykey = ""; //Inisiasi variable untuk key enkripsi

//fungsi untuk generate untuk secret key kartu
String getSecret(){
  char Alpha[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N',
                  'O','P','Q','R','S','T','U','P','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9'};
  int panjang = 16;
  int i = 0;
  String ranKeys = "";
  for(i = 0;i < panjang; i++){
    ranKeys = ranKeys + Alpha[random (0, 62)];
  }

  return ranKeys;
}

//fungsi untuk mengambil uid pada kartu
String dump_byte_array(byte *buffer, byte bufferSize) {
    String uid = "";
    for (byte i = 0; i < bufferSize; i++) {
      if (buffer[i] < 0x10){
        Serial.print(" 0");
        uid = uid+String("0");
      } else {
        Serial.print(" ");
      }
      Serial.print(buffer[i], HEX);
      uid = uid+String(buffer[i], HEX);
    }
    return uid;
}

//fungsi untuk verifikasi format email
bool verifikasiEmail(String email){
  
  if(email.indexOf("@gmail.com") > 0){
    String cekemail = email.substring(email.length()-10, email.length());
    if(cekemail == "@gmail.com"){
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
  
}

void setup() {

    Serial.begin(9600);
    digitalWrite(16, HIGH); 
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    randomSeed(millis());//agar fungsi random berubah sesuai waktu
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.println("------------------------------------");
    Serial.println("Sign up new user...");

    Serial.println("Inputkan email senagai akun integrasi aplikasi!");
    Serial.println(email);
    Serial.flush();
    while (!Serial.available()){
          
    }

    Serial.println("");
    email = Serial.readStringUntil('\n');
    Serial.println("Masukan password email akun integrasi aplikasi! (Minimal password adalah 6 karakter)");
    Serial.flush();//output serial dibuat kosong
    while (!Serial.available()){
    }

    Serial.println("");
    password = Serial.readStringUntil('\n');
    if (verifikasiEmail(email)){
      if (password.length() < 6){
        Serial.println("Format password salah, Minimal password adalah 6 karakter");
        Serial.println("");
        signupok = false;
      }
    } else {
      Serial.println("format email yang diinputkan tidak sesuai (@gmail.com)");
      Serial.println("");
      signupok = false;
    }
    
    Serial.println(email);
    Serial.println(password);
    /* Sign up */
    
    if (signupok){
      
    SPI.begin();                                              
    mfrc522.PCD_Init();                                              
    Serial.println("Dekatkan RFID untuk menginputkan data");
    Serial.println();
    }
    
}
//Fungsi enkripsi XOR, sama dengan yang ada di PHP
String encrypt(String plainText, String key) { 
  String output = ""; 
  int keyPos = 0;
  int keyLen = key.length(); 
  for (int p = 0; p < plainText.length(); p++) { 
    // Wrap around key position if necessary 
    if (keyPos >= keyLen) { keyPos = 0; } 
    // XOR operation between characters 
    char charToEncrypt = plainText[p]; 
    char keyChar = key[keyPos]; 
    char encryptedChar = charToEncrypt ^ keyChar; // Convert encrypted character to hexadecimal 
    String hexStr = String(encryptedChar, HEX); 
    if (hexStr.length() == 1) { 
      hexStr = "0" + hexStr; // Pad with leading zero if necessary 
      } // Append to output and move to the next key position 
      output += hexStr; keyPos++; } 
      output.toUpperCase(); // Convert output to uppercase 
      return output; 
      }

//Fungsi dekripsi XOR, sama dengan yang ada di PHP
String decrypt(String encryptedText, String key) { 
  String output = ""; 
  int keyPos = 0;
  int keyLen = key.length(); // Remove spaces from encrypted text 
  encryptedText.replace(" ", ""); // Iterate through each pair of hex characters in the encrypted text 
  for (int p = 0; p < encryptedText.length(); p += 2) { 
    // Extract two characters from the encrypted text 
    String hexStr = encryptedText.substring(p, p + 2); // Convert hexadecimal string to integer 
    int charValue = strtol(hexStr.c_str(), NULL, 16); // Convert integer to character and apply XOR with the key character 
    char decryptedChar = charValue ^ key[keyPos]; // Append the decrypted character to the output 
    output += decryptedChar; // Move to the next key position
    keyPos++; 
    if (keyPos >= keyLen) { 
      keyPos = 0; // Wrap around the key position if necessary 
      } 
    } 
    return output; 
  }


void loop()
{
    MFRC522::MIFARE_Key key;
    String uid = "";
    String vsecret = "";
    // set key sesuai pabrikan ke FFFFFFFFFFFFh
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
    // jika tag baru tidak terdeteksi, reset loop
    if ( !mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
  
    if ( !mfrc522.PICC_ReadCardSerial()) {
      return;
    }
  
  
    // path = "/data/rfid/";
  
    // menampilkan UID tag
    Serial.print(F("UID Tag : "));
    uid = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  
    Serial.print(F(" PICC type: "));   // menampilkan PICC Type
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));       
    Serial.flush();
    vname = "";
    int key_index = random (0, 8);
    Serial.println(key_index);
    mykey = sharekey[key_index];
    Serial.println(mykey);
    Serial.println(vname.length());
    if (vname.length() <= 0){
          Serial.println("Masukan nama lengkap anda!");
          while (Serial.available() == 0){
            
          }
          vname = Serial.readStringUntil('\n');

          Serial.println(vname.length());

        if (vname.length() > 50){
            Serial.println("Panjang max nama 50 karakter");
            delay(500);
            vname = "";
            Serial.println(' ');
            mfrc522.PICC_HaltA(); // Halt PICC
            mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
        } else {
          Serial.println("Masukan nim anda!");

          Serial.flush();
          while (!Serial.available()){
            
          }
          vnim = Serial.readStringUntil('\n');
          
          if (vnim.length() > 10){
            Serial.println("Panjang max nim 10 karakter");
            Serial.println(' ');  
            Serial.println("Masukan ulang nim anda");
            delay(500);
            Serial.println("");
            
          } else {
            Serial.println("Name : "+vname);
            Serial.println("NIM  : "+vnim);
  
            Serial.println("------------------------------------");
            Serial.println("Sign up new user...");
            
                        
            /* Sign up */
            
  
              byte buffer[50];
              byte block;
              MFRC522::StatusCode status;
        
              String secretKeys = "";
              Serial.println("Menulis nama pada tag RFID : "+vname);
              
              block = 1;
              vname.getBytes(buffer, 50);
              for (byte i = vname.length(); i < 50; i++) buffer[i] = '!';
            
              status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
              if (status != MFRC522::STATUS_OK) {
                Serial.print(F("PCD_Authenticate() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
              }
              else Serial.println(F("PCD_Authenticate() success: "));
            
              status = mfrc522.MIFARE_Write(block, buffer, 16);
              if (status != MFRC522::STATUS_OK) {
                Serial.print(F("MIFARE_Write() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
              }
              else Serial.println(F("MIFARE_Write() success: "));
            
              block = 2;
              //Serial.println(F("Authenticating using key A..."));
              status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
              if (status != MFRC522::STATUS_OK) {
                Serial.print(F("PCD_Authenticate() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
              }
            
              // Write block
              status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
              if (status != MFRC522::STATUS_OK) {
                Serial.print(F("MIFARE_Write() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
              }
              else Serial.println(F("MIFARE_Write() success: "));
            
              block = 4;
              //Serial.println(F("Authenticating using key A..."));
              status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
              if (status != MFRC522::STATUS_OK) {
                Serial.print(F("PCD_Authenticate() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
              }
            
              // Write block
              status = mfrc522.MIFARE_Write(block, &buffer[32], 16);
              if (status != MFRC522::STATUS_OK) {
                Serial.print(F("MIFARE_Write() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
              }
              else Serial.println(F("MIFARE_Write() success: "));
        
              delay(2000);
        
              Serial.println("Menulis nim pada tag RFID : "+vnim);
              byte buffer2[20];
              vnim.getBytes(buffer2, 20);
              for (byte i = vnim.length(); i < 20; i++) buffer2[i] = '!';     // pad with spaces
            
              block = 5;
              //Serial.println(F("Authenticating using key A..."));
              status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
              if (status != MFRC522::STATUS_OK) {
                Serial.print(F("PCD_Authenticate() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
              }
            
              // Write block
              status = mfrc522.MIFARE_Write(block, buffer2, 16);
              if (status != MFRC522::STATUS_OK) {
                Serial.print(F("MIFARE_Write() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
              }
              else Serial.println(F("MIFARE_Write() success: "));
        
              delay(2000);
        
              
            
              secretKeys = getSecret();
              Serial.println("Secret Keys : "+secretKeys);
              byte buffer3[secretKeys.length()];
            
              secretKeys.getBytes(buffer3, sizeof(buffer3));
        
        
              Serial.println("Menulis secret keys pada tag RFID : "+secretKeys);
              block = 6;
              //Serial.println(F("Authenticating using key A..."));
              status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
              if (status != MFRC522::STATUS_OK) {
                }
            
              // Write block
              status = mfrc522.MIFARE_Write(block, buffer3, 16);
              if (status != MFRC522::STATUS_OK) {
                Serial.print(F("MIFARE_Write() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
              }
              else 
              {
                Serial.println(F("MIFARE_Write() success: "));
                WiFiClient client;
              HTTPClient http;
              http.begin(client, serverName);
              http.addHeader("Content-Type", "application/x-www-form-urlencoded");

              //Encrypt data
              String uid_enc = encrypt(uid, mykey);
              String email_enc = encrypt(email, mykey);
              String password_enc = encrypt(password, mykey);
              String vnim_enc = encrypt(vnim, mykey);
              String secretKeys_enc = encrypt(secretKeys, mykey);

              //data yang telah di enkripsi lalu dikirm oleh ESP ke server
              String httpRequestData = "uid=" + uid_enc + "&username=" + email_enc + "&password=" + password_enc + "&nim=" + vnim_enc + "&secret_key=" + secretKeys_enc + "&sharedkey=" + key_index;
              Serial.println(httpRequestData);
              int httpResponseCode = http.POST(httpRequestData);//eksekusi metode POST ke server
              if (httpResponseCode ==200) { //Respon dari server terhadap request dari ESP
                Serial.println(http.getString());
                Serial.println("OK"); 
                } else {
                  Serial.println(http.getString());
                  }
                  http.end();
              }

              // secretKeys = secretKeys.substring(0,10);
            
              Serial.println(" ");
              mfrc522.PICC_HaltA(); // Halt PICC                 Serial.print(F("PCD_Authenticate() failed: "));
              Serial.println(mfrc522.GetStatusCodeName(status));
              ESP.restart();
              return;
                
              }
            }      
         }
}
  
