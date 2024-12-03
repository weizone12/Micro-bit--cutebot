// 若有 bug，可能是因為 preJudge 最下方的 for 被註解掉了

#include <IRremote.h>              // 引入 IRremote 函式庫，用於紅外線傳輸
#include <SoftwareSerial.h>        // 引入 SoftwareSerial 函式庫，用於串列通訊
#include <SimpleStack.h>           // 引入 SimpleStack 函式庫，使用堆疊結構

const int triggerPin = 8;          // 掃描觸發腳位
SoftwareSerial scanner(2, 3);      // 掃描器通訊腳位 (RX, TX)
const int greenPin = 6;            // 綠色 LED 腳位
const int redPin = 4;              // 紅色 LED 腳位
const int buzzer = 5;              // 蜂鳴器腳位
const int startBut = A0;           // 傳送按鈕腳位
const int endBut = A1;             // 判斷按鈕腳位
const int sendIR = A2;             // 傳送按鈕腳位
const int IRPin = 7;               // 紅外線發送腳位
IRsend irsend(IRPin);              // 建立 IRsend 物件來發送紅外線訊號

const int maxSize = 100;           // 最大陣列大小
int scanSuccess = 0;               // 掃描是否成功
int binArrayHasElement = 0;        // binArray 中是否存在資料
int intdata;                       // 掃描的條碼資料
int binArray[maxSize];             // 掃描到的資料陣列
int judgeArray[maxSize];           // 邏輯編號陣列

void setup() {
  pinMode(triggerPin, OUTPUT);     // 掃描觸發腳位為輸出
  digitalWrite(triggerPin, HIGH);  // 掃描觸發腳位為高電位(關閉)
  pinMode(greenPin, OUTPUT);       // 綠色 LED 腳位為輸出
  pinMode(redPin, OUTPUT);         // 紅色 LED 腳位為輸出
  analogWrite(greenPin, 255);      // 綠色 LED 亮度為 255(暗)
  analogWrite(redPin, 255);        // 紅色 LED 亮度為 255(暗)
  pinMode(buzzer, OUTPUT);         // 蜂鳴器腳位為輸出
  pinMode(startBut, INPUT);        // 掃描按鈕為輸入
  pinMode(endBut, INPUT);          // 判斷按鈕為輸入
  pinMode(sendIR, INPUT);          // 傳送按鈕為輸入
  scanner.begin(9600);             // 開啟掃描器的串列通訊
  resetLight();                    // 重設LED燈號
}

void Barcode_Scanner() {
  scanSuccess = 0;                       // bool, 是否成功掃描
  digitalWrite(triggerPin, LOW);         // 開始掃描
  delay(500);                            // 掃描期間
  String data = "";                      // 儲存掃描到的資料
  while (scanner.available()) {          // 有掃到資料時
    char c = scanner.read();             // 讀取掃描器資料
    data += c;                           // 儲存掃描資料
  }
  intdata = data.toInt();                // 將掃描到的資料轉為整數

  if (intdata != 0) {                    // 如果掃描到的資料不為0
    tone(buzzer, 1000);                  // 播放蜂鳴器
    delay(300);
    noTone(buzzer);                      // 停止蜂鳴器
    for (int i = 0; i < maxSize; i++) {  // 將資料加入binArray陣列
      if (binArray[i] == 0) {            // 如果binArray位置為空
        binArray[i] = intdata;           // 將資料存入
        break;
      }
    }
    scanSuccess = 1;                     // 設定掃描成功
  }
  digitalWrite(triggerPin, HIGH);        // 結束掃描
  delay(300);
}

void preJudge() {  // 將各資料的邏輯編號存入 judgeArray
  int judgeNum = 0;
  for(int i = 0; i < sizeof(binArray) / sizeof(binArray[0]); i++) {  // 遍歷 binArray
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
    } else if(num == 7 || num == 8 || num == 9 || num == 10 || num == 11 || num == 12 || num == 24 || num == 28 || num == 29 || num == 30 || num == 31 || num == 32 || num == 36 || num == 37 || num == 42 || num == 43 || num == 47) {
      judgeNum = 6;
    } else if(num == 13 || num == 14 || num == 15 || num == 16 || num == 17 || num == 18 || num == 19 || num == 20 || num == 21 || num == 22 || num == 27 || num == 35 || num == 38 || num == 39 || num == 46) {
      judgeNum = 7;
    } else if(num == 23 || num == 40 || num == 41 || num == 45) {
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
  // for(int i = 0; i < maxSize; i++) {
  //   if(judgeArray[i] == 0) {
  //     break;
  //   }
  // }
}

bool doJudge() {
  int ifNotCount = 0;                 // if not 的出現次數
  int beginCount = 0;                 // begin 的出現次數
  int endCount = 0;                   // end 的出現次數
  int lastNum;                        // 用來記錄最近彈出堆疊的數字
  SimpleStack<int> myStack(maxSize);  // 宣告一個 Stack
  if(judgeArray[0] != 1 && judgeArray[0] != 4 && judgeArray[0] != 5) {    // 第一個不是 begin 或 sub begin
    wrongLight();
    return 0;
  }
  for (int i = 0; i < sizeof(judgeArray) / sizeof(judgeArray[0]); i++) {  // 遍歷 judgeArray
    if(judgeArray[i] == 0) {          // 如果當前元素為 0，break
      break;
    } else if (judgeArray[i] <= 3) {  // 如果元素是 begin 或 repeat 或 if
      myStack.push(judgeArray[i]);    // push 進 Stack
      if(judgeArray[i] == 1) {        // 如果是 begin
        beginCount += 1;              // begin 個數 + 1
        if(beginCount > 1) {          // 如果 begin 個數超過 1
          wrongLight();
          return 0;
        }
      } else if(judgeArray[i] == 2 && judgeArray[i+1] != 7) {  // 如果 repeat 後面不是 repeat 的條件
        wrongLight();
        return 0;
      } else if(judgeArray[i] == 3 && judgeArray[i+1] != 8) {  // 如果 if 後面不是 if 的條件
        wrongLight();
        return 0;
      }
    } else if (judgeArray[i] == 4 || judgeArray[i] == 5) {  // 如果是 begin sub
      myStack.peek(&lastNum);                               // Stack 最後的元素存入 lastNum
      if(judgeArray[i-1] == 10) {                           // 如果前一個元素是 end
        myStack.push(judgeArray[i]);                        // push 進 Stack
      } else if(myStack.isEmpty()) {                        // 如果在 begin 之前
        myStack.push(judgeArray[i]);                        // push 進 Stack
      } else {                                              // 如果能在 begin 和 end 之間
        wrongLight();
        return 0;
      }
    } else if (judgeArray[i] >= 10) {            // 如果是 end 或 end repeat 或 end if 或 end sub
      myStack.pop(&lastNum);                     // pop 掉 Stack 最後的元素，存入 lastNum
      if (judgeArray[i] == 12) {                 // 如果是 end if
        if (ifNotCount > 0) {                    // 有 if not 的情況
          int secondNum;
          myStack.pop(&secondNum);               // pop 掉倒數第二個元素，存入 secondNum
          ifNotCount -= 1;                       // 減少 if not 的計數
          if (lastNum != 9 || secondNum != 3) {  // 如果 end if 前兩個不為 if 和 if not
            wrongLight();
            return 0;
          }
        } else {                                 // 沒有 if not 的情況
          if (lastNum != 3) {                    // 如果 end if 前不為 if
            wrongLight();
            return 0;
          }
        }
      } else if((judgeArray[i] == 10 && lastNum != 1) || (judgeArray[i] == 11 && lastNum !=2) || 
                (judgeArray[i] == 13 && lastNum != 4) || (judgeArray[i] == 14 && lastNum != 5)) {  // end 或 end repeat 或 end sub 配對錯誤
        wrongLight();
        return 0;
      } else if(judgeArray[i] == 10) {  // 如果是 end
        endCount += 1;                  // end 個數 + 1
        if(endCount > 1) {              // 如果 end 個數超過 1
          wrongLight();
          return 0;
        } 
        else if (judgeArray[i+1] != 0) {                       // 如果 end 後還有元素
          if (judgeArray[i+1] != 4 && judgeArray[i+1] != 5) {  // 如果 end 後面不是 beginSub
            wrongLight();
            return 0;
          }
        }
      }
    } else if (judgeArray[i] == 7) {  // 如果是 repeat 的條件
      if (judgeArray[i-1] != 2) {     // 如果 repeat 的條件前不是 repeat
        wrongLight();
        return 0;
      }
    } else if (judgeArray[i] == 8) {  // 如果是 if 的條件
      if (judgeArray[i-1] != 3) {     // 如果 if 的條件前不是 if
        wrongLight();
        return 0;
      }
    } else if (judgeArray[i] == 9) {  // 如果是 if not
      ifNotCount += 1;                // if not 個數 + 1
      myStack.push(judgeArray[i]);    // push 進 Stack
    }
  }
  if (myStack.isEmpty()) {  // 如果最後 Stack 為空，表示結構正確
    rightLight();
    return 1;
  } else {
    wrongLight();
    return 0;
  }  
}

void IRsend() {
  unsigned long combinedNumber = 0;                                   // 合併後的訊號數值
  for (int i = 0; i < sizeof(binArray) / sizeof(binArray[0]); i++) {  // 遍歷 binArray 陣列
    if (binArray[i] == 0) {                                           // 如果當前元素為 0，break
      break;
    }
    analogWrite(redPin, 0);                                           // 燈號閃爍
    delay(100);
    analogWrite(redPin, 255);
    delay(100);
    combinedNumber = (combinedNumber << 8) | binArray[i];             // 先左移 8 位再做 OR 合併訊號(一個訊號 8bits, 以 16 進位表示) ex: 0xFF -> 11111111
    if ((i + 1) % 4 == 0) {                                           // 每 4 個訊號傳送一次
      irsend.sendNEC(combinedNumber, 32);                             // 傳送 32 位的 NEC 編碼訊號(4 個訊號共 32bits)
      delay(50);
      combinedNumber = 0;                                             // 重置 combinedNumber, 準備傳送下一批訊號
    }
  }
  combinedNumber = (combinedNumber << 8) | 255;                       // 加入 255(0xFF) 並傳出剩餘的訊號(不足 4 個)
  irsend.sendNEC(combinedNumber, 32);
}

void resetArray() {  // 重製 binArray & judgeArray
  for (int i = 0; i < maxSize; i++) {
    if (binArray[i] == 0) {
      break;
    }
    binArray[i] = 0;
    judgeArray[i] = 0;
  }
}

void rightLight() {  // 正確燈號
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
}

void wrongLight() {  // 錯誤燈號
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
}

void resetLight() {  // 重製燈號
  analogWrite(redPin, 255);
  analogWrite(greenPin, 255);
}

void loop() {
  while (digitalRead(startBut) == LOW) {  // 掃碼按鈕被按下
    resetLight();                         // 燈號重製
    Barcode_Scanner();                    // 條碼掃描
    if (scanSuccess) {                    // 成功掃到條碼
      delay(300);
      binArrayHasElement = 1;             // binArray 中有資料
    }
  } 
  if (binArrayHasElement && digitalRead(endBut) == LOW) {  // binArray 中有資料 and 判斷按鈕被按下
    preJudge();                                            // 根據 binArray 新增相對的數字至 judgeArray
    if (doJudge()) {                                       // 判斷掃描順序是否正確
      int startButPress = 0;
      while (digitalRead(startBut) == HIGH) {              // 如果掃描按鈕沒被按下
        while (digitalRead(sendIR) == HIGH) {              // 如果傳送按鈕沒被按下
          if (digitalRead(startBut) == LOW) {              // 如果掃描按鈕按下，break 出外層 while loop
            startButPress = 1;
            break;
          } 
          delay(100);
        } if (startButPress) {
          break;
        }
        IRsend();                                         // 傳送按鈕被按下，送出訊號
      }
      resetArray();                                       // 掃描按鈕被按下，重製 array 和燈號
      resetLight();
      binArrayHasElement = 0;
    } else {
      resetArray();                                       // 字卡排序錯誤，重製 array
    }
  }
}