#include <IRremote.h>
#include <SoftwareSerial.h>
#include <SimpleStack.h>

const int triggerPin = 8;
SoftwareSerial scanner(2, 3); // RX, TX
const int greenPin = 6;
const int redPin = 4;
const int buzzer = 5;
const int startBut = A0;
const int endBut = A1;
const int sendIR = A2;
const int IRPin = 7;
IRsend irsend(IRPin);

int buttonState = 1;  // 按鈕狀態
const int maxSize = 30;
int binArray[maxSize];  // 訊號陣列
int judgeArray[maxSize];  // 邏輯陣列

void setup() {
  pinMode(triggerPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(startBut, INPUT);
  pinMode(endBut, INPUT);
  pinMode(sendIR, INPUT);
  scanner.begin(9600);
  resetLight();
}

void loop() {
  buttonState = digitalRead(startBut);   // 讀取按鈕狀態

  if (buttonState == LOW) {
    resetLight();
    while (digitalRead(endBut) == HIGH) {
      Barcode_Scanner();
    }
    if (binArray[0] != 0) {
      preJudge();  // 根據 binArray 新增相對的數字至 judgeArray
      if (doJudge()) {
        while (digitalRead(sendIR) == HIGH) {
          delay(100);
        }
        IRsend();
        resetArray();
        resetLight();
      } else {
        resetArray();
      }
    } else {
      wrongLight();
    }
  }
}

void Barcode_Scanner() {
  digitalWrite(triggerPin, LOW);  // 開始掃描
  delay(500);
  String data = "";
  int firstScanData = 1;
  while (scanner.available()) {
    char c = scanner.read();
    data += c;
  } int intdata = data.toInt();

  if (intdata != 0) {
    if (firstScanData) {
      tone(buzzer, 1000);
      delay(500);
      noTone(buzzer); // 停止之前的音調
      for (int i = 0; i < maxSize; i++) {  // 加入 binArray
        if (binArray[i] == 0) {
          binArray[i] = intdata;
          break;
        }
      } firstScanData = 0;
    }
  }
  digitalWrite(triggerPin, HIGH);  // 掃描結束
  delay(500);
}

void preJudge() {
  int judgeNum = 0;
  for(int i = 0; i < sizeof(binArray) / sizeof(binArray[0]); i++) {
    int num = binArray[i];
    if(num == 0) {
      break;
    } else if(num == 1) {
      judgeNum = 1;
    } else if(num == 2) {
      judgeNum = 10;
    } else if(num == 3) {
      judgeNum = 2;
    } else if(num == 4) {
      judgeNum = 11;
    } else if(num == 5) {
      judgeNum = 3;
    } else if(num == 6) {
      judgeNum = 12;
    } else if(num == 7 || num == 8 || num == 9 || num == 10 || num == 11 || num == 12 || num == 24 || num == 28 || num == 29 || num == 30 || num == 31 || num == 32 || num == 36 || num == 37 || num == 42 || num == 43) {
      judgeNum = 6;
    } else if(num == 13 || num == 14 || num == 15 || num == 16 || num == 17 || num == 18 || num == 19 || num == 20 || num == 21 || num == 22 || num == 27 || num == 35 || num == 38 || num == 39) {
      judgeNum = 7;
    } else if(num == 23 || num == 40 || num == 41) {
      judgeNum = 8;
    } else if(num == 25) {
      judgeNum = 4;
    } else if(num == 26) {
      judgeNum = 13;
    } else if(num == 33) {
      judgeNum = 5;
    } else if(num == 34) {
      judgeNum = 14;
    } else if(num == 44) {
      judgeNum = 9;
    }
    judgeArray[i] = judgeNum;
  }
  for(int i = 0; i < maxSize; i++) {
    if(judgeArray[i] == 0) {
      break;
    }
  }
}

bool doJudge() {
  int ifNotCount = 0;
  int beginCount = 0;
  int endCount = 0;
  int lastNum;
  SimpleStack<int> myStack(maxSize);
  if(judgeArray[0] != 1 && judgeArray[0] != 4 && judgeArray[0] != 5) {  // 第一個不是 begin 或 sub begin
    wrongLight();
    return 0;
  }
  for (int i = 0; i < sizeof(judgeArray) / sizeof(judgeArray[0]); i++) {
    if(judgeArray[i] == 0) {
      break;
    } else if (judgeArray[i] <= 3) {
      myStack.push(judgeArray[i]);
      if(judgeArray[i] == 1) {  // begin
        beginCount += 1;
        if(beginCount > 1) {  // begin 只能有一個
          wrongLight();
          return 0;
        }
      } else if(judgeArray[i] == 2 && judgeArray[i+1] != 7) {  // repeat 後面不是 repeat 的條件
        wrongLight();
        return 0;
      } else if(judgeArray[i] == 3 && judgeArray[i+1] != 8) {  // if 後面不是 if 的條件
        wrongLight();
        return 0;
      }
    } else if (judgeArray[i] == 4 || judgeArray[i] == 5) {  // begin sub
      myStack.peek(&lastNum);
      if(judgeArray[i-1] == 10) {  // 在 end 之後
        myStack.push(judgeArray[i]);
      } else if(myStack.isEmpty()) {  // 在 begin 之前
        myStack.push(judgeArray[i]);
      } else {  // begin sub 只能在 end 後或 begin 前
        wrongLight();
        return 0;
      }
    } else if (judgeArray[i] >= 10) {
      myStack.pop(&lastNum);
      if (judgeArray[i] == 12) {  // end if
        if (ifNotCount > 0) {  // 有 if not 的情況
          int secondNum;
          myStack.pop(&secondNum);
          ifNotCount -= 1;
          if (lastNum != 9 || secondNum != 3) {  // end if 前兩個不為 if 和 if not
            wrongLight();
            return 0;
          }
        } else {
          if (lastNum != 3) {  // end if 前不為 if
            wrongLight();
            return 0;
          }
        }
      } else if((judgeArray[i] == 10 && lastNum != 1) || (judgeArray[i] == 11 && lastNum !=2) || (judgeArray[i] == 13 && lastNum != 4) || (judgeArray[i] == 14 && lastNum != 5)) {  // end, end repeat, end if, end sub 配對錯誤
        wrongLight();
        return 0;
      } else if(judgeArray[i] == 10) {  // end
        endCount += 1;
        if(endCount > 1) {  // end 只能有一個
          wrongLight();
          return 0;
        } 
        else if (judgeArray[i+1] != 0) {    // end 後面只能是 beginSub
          if (judgeArray[i+1] != 4 || judgeArray[i+1] != 5) {
            wrongLight();
            return 0;
          }
        }
      }
    } else if (judgeArray[i] == 7) {  // repeat 的條件
      if (judgeArray[i-1] != 2) {  // repeat 的條件只能在 repeat 後
        wrongLight();
        return 0;
      }
    } else if (judgeArray[i] == 8) {  // if 的條件
      if (judgeArray[i-1] != 3) {  // if 的條件只能在 if 後
        wrongLight();
        return 0;
      }
    } else if (judgeArray[i] == 9) {  // if not
      ifNotCount += 1;
      myStack.push(judgeArray[i]);
    }
  }
  if (myStack.isEmpty()) {  // Stack 為空
    rightLight();
    return 1;
  } else {
    wrongLight();
    return 0;
  }  
}

void IRsend() {
  unsigned long combinedNumber = 0;
  for (int i = 0; i < sizeof(binArray) / sizeof(binArray[0]); i++) {
    if (binArray[i] == 0) {
      break;
    }
    analogWrite(redPin, 0);
    delay(100);
    analogWrite(redPin, 255);
    delay(100);
    combinedNumber = (combinedNumber << 8) | binArray[i];  // 先左移8位再做 OR
    if ((i + 1) % 4 == 0) {  // 每四個訊號(4Byte)傳送一次
      irsend.sendNEC(combinedNumber, 32);
      delay(50);
      combinedNumber = 0; // 重置 combinedNumber
    }
  }
  combinedNumber = (combinedNumber << 8) | 255;  // 加入255並傳出剩餘的訊號(不足4個)
  irsend.sendNEC(combinedNumber, 32);
}

void resetArray() {
  for (int i = 0; i < maxSize; i++) {  // 初始化 binArray & judgeArray
    if (binArray[i] == 0) {
      break;
    }
    binArray[i] = 0;
    judgeArray[i] = 0;
  }
}

void rightLight() {
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
}

void wrongLight() {
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
}

void resetLight() {
  analogWrite(redPin, 255);
  analogWrite(greenPin, 255);
}