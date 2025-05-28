#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
 const char* serverName = "http://192.168.43.185/esp/tapping.php"; //Akses ke URL server tapping
 const char* updating = "http://192.168.43.185/esp/updatekey.php";//Link utnuk updaye key

#define RST_PIN         33
#define SS_PIN          5


#define WIFI_SSID "and"
#define WIFI_PASSWORD "qwerty12345"
String sharekey[] = {"ABC123", "BCD234", "FG56", "FG56", "FG56", "FG56", "FG56", "FG56"};
String input;
String dbsc = ""; //inisiasi database secret key

MFRC522 mfrc522(SS_PIN, RST_PIN);

//*****************************************************************************************//
void setup() {
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  randomSeed(millis());  
  delay(2000);
}

// String getSecret(int randomNumber){
//   char Alpha[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N',
//                   'O','P','Q','R','S','T','U','P','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9'};
//   int panjang = 16;
//   int i = 0;
//   String ranKeys = "";
//   for(i = 0;i < panjang; i++){
//     ranKeys = ranKeys + Alpha[random (0, randomNumber+i)];
//   }

//   return ranKeys;
// }

//Fungsi untuk generate secret key
String generateRandomString(int length) {
  String result = "";
  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  for (int i = 0; i < length; i++) {
    int index = random(0, sizeof(charset) - 1);
    result += charset[index];
  }
  return result;
}
//fungsi untuk membaca UID
String dump_byte_array(byte *buffer, byte bufferSize) {
    String uid = "";
    for (byte i = 0; i < bufferSize; i++) {
      if (buffer[i] < 0x10){
        uid = uid+String("0");
      }
      uid = uid+String(buffer[i], HEX);
    }
    return uid;
}
//Fungsi untuk enkrip XOR
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
//Fungsi untuk Dekrip XOR
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


//*****************************************************************************************//
void loop() {

  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  byte block;
  byte len;
  String vname = "";
  String vnim = "";
  String vsecret = "";
  MFRC522::StatusCode status;
  String uid = "";
  String newsecret = "";
  uid = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size); //baca uid

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // path = "/data/rfid/";

  Serial.println(F("**Card Detected:**"));
  //memilih secara acak key untuk enkrip dan dekrip
  int key_index = random (0, 8);
  Serial.println(key_index);
  String mykey = sharekey[key_index];
  Serial.println(mykey); //

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card
  uid = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------

  // path = path+uid+"/";

  Serial.println(" ");
  byte buffer1[18];

  block = 6;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 6, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32 || buffer1[i] != 33)
    {
      Serial.print((char)buffer1[i]);
      vsecret = vsecret + (char)buffer1[i]; //membaca secret key di kartu
    }
  }

  Serial.println();

  //---------------------------------------- GET NIM

  byte buffer2[18];

  block = 5;
  len = 18;
  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 5, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT FIRST NAME
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer2[i] != 33)
    {
      vnim = vnim + (char)buffer2[i]; //membaca nim dari kartu
    }
  }
  

  //---------------------------------------- GET NAME

  byte buffer3[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer3, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 17; i++) {
    if (buffer3[i] != 33){
      vname = vname + (char)buffer3[i]; //membaca nama yang ada di kartu
    }
  }

  byte buffer4[18];
  block = 2;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 2, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer4, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
    if (buffer4[i] != 33){
      vname = vname + (char)buffer4[i];
    }
  }

  byte buffer5[18];
  block = 4;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer5, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
    if (buffer5[i] != 33){
      vname = vname + (char)buffer5[i];
    }
  }

  Serial.println("Name : "+String(vname)); //hasil membaca di blok 4
  Serial.println("NIM : "+String(vnim)); //hasil membaca di blok 5
  Serial.println("Secret Keys : "+String(vsecret)); //hasil membaca di blok 6


  //----------------------------------------

  //Serial.println(F("\n**End Reading**\n"));

  delay(1000); //change value if you want to read cards faster

  Serial.println(' ');
  //membuat koneksi ke server
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);
  String vnim_enc = encrypt(vnim, mykey);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = "nim=" + vnim_enc + "&sharedkey=" + key_index;
  Serial.println(httpRequestData);
  int httpResponseCode = http.POST(httpRequestData);
  if (httpResponseCode ==200) {
    String dbsc_enc = http.getString(); //untuk memperoleh secret key dari database
    dbsc = decrypt(dbsc_enc, mykey);
    vsecret = vsecret.substring(0,15);
    dbsc = dbsc.substring(0,15);
    //cek jika secret key database dengan secret di kartu sama, maka generate secret key baru
    if (vsecret == dbsc) { 
          Serial.println("Authentication OK");
          Serial.println("Generating New Key .... ");
          // int randomNumber = random(0, 62);
          newsecret = generateRandomString(16);
          // newsecret = getSecret(randomNumber);
          Serial.println(newsecret);
          byte buffer3[newsecret.length()];
          newsecret.getBytes(buffer3, sizeof(buffer3));
          Serial.println("Menulis secret keys baru pada tag RFID : "+newsecret);
          block = 6; //menulis secret key baru ke blok 6
          status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
          if (status != MFRC522::STATUS_OK) {
            Serial.print(F("PCD_Authenticate() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
          }
          
          status = mfrc522.MIFARE_Write(block, buffer3, 16);
          if (status != MFRC522::STATUS_OK) {
            Serial.print(F("MIFARE_Write() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
          } else {
            http.end();
            http.begin(client, updating);
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            
            String uid_enc = encrypt(uid, mykey);
            String newsecret_enc = encrypt(newsecret, mykey);
            //mengirim data uid, nim, new secret key, dan key enkripsi ke server untuk update table tapping dan registrasi (update secret key
            String httpRequestData = "uid=" + uid_enc + "&nim=" + vnim_enc + "&new_secret_key=" + newsecret_enc + "&sharedkey=" + key_index;
                  Serial.println(httpRequestData);
                  int httpResponseCode = http.POST(httpRequestData);
                  if (httpResponseCode ==200) {
                    Serial.println(http.getString());
                    Serial.println("OK");
                    delay(1000);
                    ESP.restart();
                    } else {
                      Serial.println(http.getString());
                      }
                      http.end();
                  }
          }
          else {
            Serial.println("Kartu telah terblokir"); 
            ESP.restart();  
            delay(5000);   
          }
    } else {
      Serial.println(http.getString());
      }
  http.end();


  // path = "/data/rfid/"+uid;

  // if (Firebase.get(firebaseData, path)){
  //   Serial.print("Get variant data success, type = ");
  //   Serial.println(firebaseData.dataType());
  //   Serial.println(path);
  //   if (firebaseData.dataType() != "null"){

  //     Serial.println("");
  //     Serial.println("UID tag rfid ditemukan");
      
  //     Firebase.getString(firebaseData, path+"/count");
  //     jmlItem = firebaseData.intData();


  //     Firebase.getString(firebaseData, path+"/secret");
  //     dbsc = firebaseData.stringData();

  //     Serial.println("Secret Key di database = "+String(dbsc));
  //     Serial.println("Secret Key pada tag RFID = "+String(vsecret));
  //     Serial.println("============================================");

  //     if (dbsc == vsecret){
  //         Serial.println("Secret Key Sesuai.");
        
  //         String newsecret = getSecret();

  //         Serial.println(newsecret);
    
  //         byte buffer3[newsecret.length()];
            
  //         newsecret.getBytes(buffer3, sizeof(buffer3));
          
  //         Serial.println("Menulis secret keys baru pada tag RFID : "+newsecret);
  //         block = 6;
  //         status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  //         if (status != MFRC522::STATUS_OK) {
  //           Serial.print(F("PCD_Authenticate() failed: "));
  //           Serial.println(mfrc522.GetStatusCodeName(status));
  //           return;
  //         }
          
  //         status = mfrc522.MIFARE_Write(block, buffer3, 16);
  //         if (status != MFRC522::STATUS_OK) {
  //           Serial.print(F("MIFARE_Write() failed: "));
  //           Serial.println(mfrc522.GetStatusCodeName(status));
  //           return;
  //         } else {
    
  //           newsecret = newsecret.substring(0,10);
    
  //           Serial.println(newsecret);
    
  //           timeClient.update();
  //           Serial.println(F("MIFARE_Write() success: "));
  //           Serial.println(F("Updating data on server"));
  //           unsigned long epochTime = timeClient.getEpochTime();
  //           String formattedTime = timeClient.getFormattedTime();
  //           int currentHour = timeClient.getHours();
  //           int currentMinute = timeClient.getMinutes();
  //           int currentSecond = timeClient.getSeconds();
  //           String weekDay = weekDays[timeClient.getDay()];
            
  //           struct tm *ptm = gmtime ((time_t *)&epochTime); 
  //           int monthDay = ptm->tm_mday;
  //           int currentMonth = ptm->tm_mon+1;
  //           String currentMonthName = months[currentMonth-1];
          
  //           int currentYear = ptm->tm_year+1900;
  //           jmlItem = jmlItem + 1;
          
  //           String lastupdate = String(monthDay) +"-"+ String(currentMonthName.substring(0,3)) + "-" + String(currentYear) + "-" + String(currentHour)+":"+String(currentMinute)+":"+String(currentSecond);
  //           String newitem = String(monthDay)+String(currentMonth) + String(currentYear) +String(jmlItem);
    
  //           if (Firebase.get(firebaseData, path+"/historyTap")){
    
  //               Serial.println(firebaseData.dataType());
  //               Serial.println(path);
  //               if (firebaseData.dataType() != "null"){
  //                 Serial.print("Sudah pernah tapping");
  //                 Firebase.setString(firebaseData, path+"/historyTap/"+newitem+"/date", lastupdate);
  //                 Firebase.setString(firebaseData, path+"/historyTap/"+newitem+"/prevsecret", vsecret);
  //                 Firebase.setString(firebaseData, path+"/historyTap/"+newitem+"/secret", newsecret);
  //                 Firebase.setString(firebaseData, path+"/updated", "true");
  //                 Firebase.setString(firebaseData, path+"/secret", newsecret);
  //                 Firebase.setInt(firebaseData, path+"/count", jmlItem);
  //               } else {
  //                 Serial.print("Belum pernah tapping");
  //                 Firebase.setString(firebaseData, path+"/historyTap/"+newitem+"/date", lastupdate);
  //                 Firebase.setString(firebaseData, path+"/historyTap/"+newitem+"/prevsecret", newsecret);
  //                 Firebase.setString(firebaseData, path+"/historyTap/"+newitem+"/secret", newsecret);
  //                 Firebase.setString(firebaseData, path+"/updated", "true");
  //                 Firebase.setString(firebaseData, path+"/secret", newsecret);
  //                 Firebase.setInt(firebaseData, path+"/count", jmlItem);
  //               }
  //           }
      
            
  //           delay(1000);
  //         }
  //         Serial.println(" ");
  //         Serial.println("Success, proses tapping selesai");
    
  //         mfrc522.PICC_HaltA(); // Halt PICC
  //         mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  //     } else {
  //         Serial.println(" ");
  //         Serial.println("secet key tidak sesuai, apakah itu benar kartu anda?");

  //         mfrc522.PICC_HaltA(); // Halt PICC
  //         mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  //     }


       
  //   } else if (firebaseData.dataType() == "null"){
  //     Serial.println("UID tag rfid tidak ditemukan");
  //     mfrc522.PICC_HaltA();
  //     mfrc522.PCD_StopCrypto1();
  //   }
  // }
}


