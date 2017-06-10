/* Marvin Piezo Vibration Sensor examples - bd
   This basically sends a LoRA packet whenever the piezo sensor gets hit.
   There's no sleep yet, nothing fancy, but it works.
*/
#include "CayenneLPP.h"

// set max size
int     MAX_SIZE = 52;
CayenneLPP lpp(MAX_SIZE);

// Port to assign the type of lora data (any port can be used between 1 and 223)
int     set_port  = 1;

// Some standard ports that depend on the layout of the Marvin
long    defaultBaudRate = 57600;
int     reset_port = 5;
int     RN2483_power_port = 6;
int     led_port = 13;
const int th_hold = 350;

//*** Set parameters here BEGIN ---->
// NOTE: Devices joining the network via ABP, what we're doing for the hackathon,
// only require DevADDR, NWKSKey, and APPSKey. DevEUI will be assigned by Network Server.
String set_nwkskey = "5a88096bed02837e0c824befa0cbecc4";
String set_appskey = "22c75fa397b15e61b65e0a8c95df4747";
String set_devaddr = "04001E1F";

const int analogInPin2 = A2; // Analog input pin that the potentiometer is attached to
const int analogInPin3 = A3; // Analog output pin that the LED is attached to
//*** <---- END Set parameters here

// Some global items
String reader = "";

// counter
int   count = 1;

void setup() {
  InitializeSerials(defaultBaudRate);
  initializeRN2483(RN2483_power_port, reset_port);
  Serial.println("STARTUP..." );

  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
}

void loop() {
    int analog2 = analogRead(analogInPin2);
    int analog3 = analogRead(analogInPin3);

    Serial.println(analog2); 

    if (analog2 > th_hold)
    {
        Serial.println("Gunshot Detected nearby");
        char payload[MAX_SIZE] = "";

        lpp.reset();

        // Stub out payloads if you want.
        lpp.addAnalogInput(1, analog2);
        lpp.addAnalogInput(2, analog3);

        uint8_t buff = *lpp.getBuffer();

        for (int i = 0; i < lpp.getSize(); i++) {
        char tmp[16];

        sprintf(tmp, "%.2X",(lpp.getBuffer())[i]);
        strcat(payload, tmp);
        }

        send_LoRa_data(set_port, payload);
        read_data_from_LoRa_Mod();
    }
    delay(100);

} // end loop()


void InitializeSerials(long baudrate) {
  Serial.begin(baudrate);
  Serial1.begin(baudrate);
  delay(1000);
  print_to_console("Serial ports initialised");
}


void initializeRN2483(int pwr_port, int rst_port) {
  //Enable power to the RN2483
  pinMode(pwr_port, OUTPUT);
  digitalWrite(pwr_port, HIGH);
  print_to_console("RN2483 Powered up");
  delay(1000);

  //Disable reset pin
  pinMode(rst_port, OUTPUT);
  digitalWrite(rst_port, HIGH);

  //Configure LoRa module
  send_LoRa_Command("sys reset");
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac set nwkskey " + set_nwkskey);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac set appskey " + set_appskey);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac set devaddr " + set_devaddr);
  read_data_from_LoRa_Mod();

  //For this commands some extra delay is needed.
  send_LoRa_Command("mac set adr on");
  //send_LoRa_Command("mac set dr 0"); //uncomment this line to fix the RN2483 on SF12 (dr=DataRate)
  delay(1000);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac save");
  delay(1000);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac join abp");
  delay(1000);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("radio set crc off");
  delay(1000);
  read_data_from_LoRa_Mod();
}

void print_to_console(String message) {
  Serial.println(message);
}

void read_data_from_LoRa_Mod() {
  if (Serial1.available()) {
    String inByte = Serial1.readString();
    Serial.println(inByte);
  }

}

void send_LoRa_Command(String cmd) {
    print_to_console("Sending: " + cmd);
  Serial1.println(cmd);
  delay(500);
}

void send_LoRa_data(int tx_port, String rawdata) {
  // send_LoRa_Command("mac tx uncnf " + String(tx_port) + String(" ") + rawdata);
  send_LoRa_Command("mac tx cnf " + String(tx_port) + String(" ") + rawdata);
}

