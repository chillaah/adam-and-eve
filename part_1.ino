#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <math.h> 
#include <stdio.h> 
#include "Wire.h"

const int MPU_ADDR = 0x68;
const int gyroscopes = 3;
int gyro_order[gyroscopes];

float temp;
int reg = 0;
int on = 0;

float acc_x [gyroscopes], acc_y [gyroscopes], acc_z [gyroscopes]; // variables for accelerometer raw data
float gyro_x [gyroscopes], gyro_y [gyroscopes], gyro_z [gyroscopes]; // variables for gyro raw data
float angle_z[gyroscopes], angle_x [gyroscopes], angle_y [gyroscopes];
float acc_error_z[gyroscopes], acc_error_x[gyroscopes], acc_error_y[gyroscopes], gyro_error_x[gyroscopes], gyro_error_y[gyroscopes], gyro_error_z[gyroscopes];

void gyro_setup(){
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);  
  Serial.println("gyro setup done");
}

void setup() {
  Serial.begin(57600);
  //LOW is READ, HIGH is not READ
  delay(1000);
  for(int x = 0; x < gyroscopes; x++){pinMode(5+x, OUTPUT);}
  
  for(int x = 0; x < gyroscopes; x++){
  switch_gryo_register();
  check_state(true);
    
  for(int i = 0; i < gyroscopes; i++){
  Serial.print(gyro_order[i]);
    }
    Serial.println();
  }  
}

void read_sensor(int i){
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
    
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  acc_x[i] = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  acc_y[i] = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  acc_z[i] = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)

  temp = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x[i] = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y[i] = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z[i] = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
}
/*
void angle_z_angle_x_angle_y(){
  
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds
  previousTime = currentTime;        // Previous time is stored before the actual time read
  
  angle_z =  angle_z + gyro_z * elapsedTime;
  
  if(acc_y < 200 && acc_y > -200){
    angle_x =  0.95*(angle_x + gyro_x * elapsedTime) + 0.05 * -acc_y;
  } else{
    angle_x =  angle_x + gyro_x * elapsedTime;  
  }
  if(acc_x < 200 && acc_x > -200){
    angle_y =  0.95*(angle_y + gyro_y * elapsedTime) + 0.05 * acc_x;
  } else{
    angle_y =  angle_y + gyro_y * elapsedTime;  
  }
}
*/
void print_data(){      // print out data
  //for(int i = 0; i < gyroscopes; i++){
  //Serial.print("~aX1: "); 
  Serial.print(acc_x[0]);
  Serial.print(" ");  
  //Serial.print("~aX2: "); 
  Serial.print(acc_x[1]);
  Serial.print(" ");  
  //Serial.print("~aX3: "); 
  Serial.print(acc_x[2]); 
  //+-16000
  //Serial.print(",~aY1: "); Serial.print(-acc_y[i]);
  //Serial.print(",~aY2:"); Serial.print(-acc_y[1]);
  //Serial.print(",~aY3:"); Serial.print(-acc_y[2]);
  //+-16000
  //Serial.print(",~aZ:"); Serial.print(acc_z);
  
  //Serial.print(",~gX:"); Serial.print(gyro_x[i]);  //+-32000  
  //Serial.print(",~gY:"); Serial.print(gyro_y);  //+-36000
  //Serial.print(",~gZ:"); Serial.print(gyro_z);

  //Serial.print(",acc_error_x:");  Serial.print(acc_error_x);
  //Serial.print(",acc_error_y:");  Serial.print(acc_error_y);
  //Serial.print(",gyro_errorX:");  Serial.print(gyro_error_x);
  //Serial.print(",gyro_errorY:");  Serial.print(gyro_error_y);
  //Serial.print(",gyro_errorZ:");  Serial.println(gyro_error_z);  
  
  Serial.println();
  //}
}

void switch_gryo_register(){
  for(int x = 0; x < gyroscopes; x++){gyro_order[x] = 0;} // clearing all values in array
  gyro_order[reg] = 1; //changing value at reg index
  reg++; //incrementing index
}

void check_state(bool setup)
{
  for(int x = 0; x < gyroscopes; x++){ 
    
  if(gyro_order[x] == 0){digitalWrite(5+x, HIGH);} //changing register to high to not read
  if(gyro_order[x] == 1)
  {
    digitalWrite(5+x, LOW);
    if(setup == true){gyro_setup();} //only true to turn on all gyros
    else{read_sensor(x);} // read sensor value
  }
 }
}


void loop() {
  if(reg == gyroscopes){reg = 0;}
  
  switch_gryo_register();
  check_state(false);

  print_data();

}
