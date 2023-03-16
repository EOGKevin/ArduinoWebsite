# 1 "C:\\Users\\nikl3009\\vscode arduino\\Project_Uge_2\\Project_Uge_2.ino"
# 2 "C:\\Users\\nikl3009\\vscode arduino\\Project_Uge_2\\Project_Uge_2.ino" 2
# 3 "C:\\Users\\nikl3009\\vscode arduino\\Project_Uge_2\\Project_Uge_2.ino" 2
# 4 "C:\\Users\\nikl3009\\vscode arduino\\Project_Uge_2\\Project_Uge_2.ino" 2
# 5 "C:\\Users\\nikl3009\\vscode arduino\\Project_Uge_2\\Project_Uge_2.ino" 2
# 6 "C:\\Users\\nikl3009\\vscode arduino\\Project_Uge_2\\Project_Uge_2.ino" 2
# 7 "C:\\Users\\nikl3009\\vscode arduino\\Project_Uge_2\\Project_Uge_2.ino" 2

#define RED_LED 15 /* the pin the red LED is connected to*/
#define GREEN_LED 2 /* the pin the green LED is connected to*/
#define BUZZER 4
#define RST_PIN 22
#define SS_Pin 21
#define D7 32
#define D6 33
#define D5 25
#define D4 26
#define RS 14
#define EN 27
const String WIFI_SSID = apistuff::WiFi_SSID();
const String API_KEY = apistuff::API_KEY();
const String DATABASE_URL = apistuff::RTDB_URL();

RfidDictionaryView rfidDictionary(21, 22);
LiquidCrystal lcd(14, 27, 26, 25, 33, 32);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long dataMillis = 0;

void setup() {
    pinMode(15 /* the pin the red LED is connected to*/, 0x02); // Declare the red LED as an output
    pinMode(2 /* the pin the green LED is connected to*/, 0x02); // Declare the green LED as an output
    pinMode(4, 0x02);
    Serial.begin(115200); // Initialize Serial Monitor with 9600 Baud rate

    WiFi.begin(WIFI_SSID.c_str());
    Serial.print("Connecting to Wi-Fi");
    while (WiFiClass::status())
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", "4.3.8");

    /* Assign the API key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    Firebase.reconnectWiFi(true);

    SPI.begin();
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Read personal data on a MIFARE PICC:")))));
}

void loop() {
    print("set mode", "e: edit else log");
    while (Serial.available() == 0);

    String res = Serial.readStringUntil('\n');
    res.trim();

    if (millis() - dataMillis > 5000)
    {
        dataMillis = millis();
        lcd.clear();
        print("No card read");
        while (!rfidDictionary.detectTag()) delay(1000);

        String email = rfidDictionary.get("Email");

        String pass = rfidDictionary.get("Password");

        if(res.startsWith("e")) edit();
        else logon(email, pass);

        rfidDictionary.disconnectTag();
        delay(2000);
    }
}

void edit(){
    print("Input new email");
    while (Serial.available() == 0);

    String email = Serial.readStringUntil('\n');
    email.trim();

 print("Input new password");
 while (Serial.available() == 0);

 String pass = Serial.readStringUntil('\n');
 pass.trim();

 rfidDictionary.set("Email", email);
 rfidDictionary.set("Password", pass);

    logon(email, pass);
}

void logon(String email, String pass){
    signIn(email, pass);

    String namePath = auth.token.uid.c_str();
    String loggedinPath = namePath;
    namePath += "/Name";
    loggedinPath += "/Loggedin";

    String name = "";

    if (Firebase.getString(fbdo, namePath))
    {
        name = fbdo.stringData().c_str();
        Serial.printf("Name: %s\n", name.c_str());
    }
    else
    {
        Serial.printf("Get Name... %s\n", fbdo.errorReason().c_str());
        name = "Unknown";
    }

    bool loggedin = false;

    if (Firebase.getBool(fbdo, loggedinPath))
    {
        loggedin = !fbdo.boolData();
        Serial.printf("Loggedin: %s\n", loggedin ? "True" : "False");
    }
    else
    {
        Serial.printf("Failed loggedin get with error: %s\n", fbdo.errorReason().c_str());
    }


    Serial.println("Reading data from MIFARE PICC");

    if (!loggedin)
    {
        print("Forvel", name);
        digitalWrite(15 /* the pin the red LED is connected to*/, 0x1);
        digitalWrite(4, 0x1);
        delay(100);
        digitalWrite(4, 0x0);
        delay(100);
        digitalWrite(4, 0x1);
        delay(200);
        digitalWrite(15 /* the pin the red LED is connected to*/, 0x0);
        digitalWrite(4, 0x0);
    }
    else{
        print("Hej", name);
        digitalWrite(2 /* the pin the green LED is connected to*/, 0x1);
        digitalWrite(4, 0x1);
        delay(200);
        digitalWrite(2 /* the pin the green LED is connected to*/, 0x0);
        digitalWrite(4, 0x0);
    }

    Serial.printf("Set Loggedin... %s\n", Firebase.setBool(fbdo, loggedinPath, loggedin) ? "ok" : fbdo.errorReason().c_str());
}

void signIn(const String email, const String password){
    /* Assign the user sign in credentials */
    auth.user.email = email;
    auth.user.password = password;

    /* Reset stored authen and config */
    Firebase.reset(&config);

    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);
}

void print(String line1, String line2){
    print(line1);
    lcd.setCursor(0, 1);
    print(line2);
}

void print(String text){
    for (size_t i = 0; i < 16; i++)
    {
        lcd.print(text.length() > i ? text[i] : ' ');
    }
    Serial.println(text);
    lcd.setCursor(0, 0);
}
