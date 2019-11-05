const int read = A0; //Sensor AO pin to Arduino pin A0
int value;          //Variable to store the incomming data

void setup()
{
  //Begin serial communication
  Serial.begin(9600);
  
}

void loop()
{
  Serial.println(analogRead(read)); 
  delay(1000); // Check for new value every 5 sec
}
