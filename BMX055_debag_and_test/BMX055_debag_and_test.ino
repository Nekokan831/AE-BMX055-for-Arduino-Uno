#include <Wire.h>

#define Addr_Gyro 0x69  // BMX055 ジャイロセンサーのI2Cアドレス

// 角速度の値を保存するグローバル変数
float xGyro = 0.00;
float yGyro = 0.00;
float zGyro = 0.00;

// 前回の角速度を保存するグローバル変数
float previousGyroX = 0.00;
float previousGyroY = 0.00;
float previousGyroZ = 0.00;

// 角度計算用の変数
float angleX = 0.00;
float angleY = 0.00;
float angleZ = 0.00;

// タイムスタンプ用の変数
unsigned long previousTime = 0;
unsigned long resetTime = 0; // 角度リセット用
unsigned long printTime = 0; // シリアルプリント用

// シリアルプリントの間隔 (ミリ秒単位)
unsigned long printInterval = 100; // ここで出力間隔を設定 (自由に変更可能)

// データリセットの間隔 (ミリ秒単位)
unsigned long resetInterval = 100000; // ここでデータリセット間隔を設定 (自由に変更可能)

void setup() {
  Wire.begin();
  Serial.begin(9600);
  BMX055_Gyro_Init();
  delay(300);
  previousTime = millis();
  resetTime = millis();
  printTime = millis();
}

void loop() {
  // ジャイロの読み取り
  BMX055_Gyro();

  // 角度の計算
  calculateAngles();

  // シリアルプリントの間隔をチェックして角度の表示
  if (millis() - printTime >= printInterval) {
    Serial.print("Rall:");
    Serial.print(angleY);
    Serial.print(",");
    Serial.print("Pitch:");
    Serial.print(angleX);
    Serial.print(",");
    Serial.print("Yaw:");
    Serial.print(angleZ);
//    Serial.println(",");
//    Serial.print("1:");
//    Serial.print(1);
//    Serial.println(",");
//    Serial.print("-1:");
//    Serial.print(-1);
    Serial.println("");
    printTime = millis(); // タイムスタンプをリセット
  }

  // リセット間隔をチェックして角度のリセット
  if (millis() - resetTime >= resetInterval) {
    angleX = 0.00;
    angleY = 0.00;
    angleZ = 0.00;
    resetTime = millis(); // タイムスタンプをリセット
  }
}

void BMX055_Gyro_Init() {
  Wire.beginTransmission(Addr_Gyro);
  Wire.write(0x0F);  // レンジレジスタの選択
  Wire.write(0x04);  // フルスケール = +/- 125 degree/s
  Wire.endTransmission();
  delay(100);

  Wire.beginTransmission(Addr_Gyro);
  Wire.write(0x10);  // バンド幅レジスタの選択
  Wire.write(0x07);  // 出力データレート = 100 Hz
  Wire.endTransmission();
  delay(100);

  Wire.beginTransmission(Addr_Gyro);
  Wire.write(0x11);  // 低電力モードレジスタの選択
  Wire.write(0x00);  // 通常モード、スリープ時間 = 2ms
  Wire.endTransmission();
  delay(100);
}

void BMX055_Gyro() {
  unsigned int data[6];
  for (int i = 0; i < 6; i++) {
    Wire.beginTransmission(Addr_Gyro);
    Wire.write((2 + i));    // データレジスタの選択
    Wire.endTransmission();
    Wire.requestFrom(Addr_Gyro, 1);    // データを1バイト要求
    if (Wire.available() == 1)
      data[i] = Wire.read();
  }
  xGyro = (data[1] * 256) + data[0];
  if (xGyro > 32767)  xGyro -= 65536;
  yGyro = (data[3] * 256) + data[2];
  if (yGyro > 32767)  yGyro -= 65536;
  zGyro = (data[5] * 256) + data[4];
  if (zGyro > 32767)  zGyro -= 65536;

  xGyro = xGyro * 0.0038; // 角速度の計算 (単位: 度/秒)
  yGyro = yGyro * 0.0038;
  zGyro = zGyro * 0.0038;
}

void calculateAngles() {
  unsigned long currentTime = millis();
  float elapsedTime = (currentTime - previousTime) / 1000.0; // 経過時間を秒で計算
  previousTime = currentTime;

  // 台形則による角度計算
  angleX += ((previousGyroX + xGyro) / 2.0) * elapsedTime+0.00021; // X軸の角度
  angleY += ((previousGyroY + yGyro) / 2.0) * elapsedTime+0.00052; // Y軸の角度
  angleZ += ((previousGyroZ + zGyro) / 2.0) * elapsedTime-0.00035; // Z軸の角度

  // 現在のジャイロ値を次回のために保存
  previousGyroX = xGyro;
  previousGyroY = yGyro;
  previousGyroZ = zGyro;
}
