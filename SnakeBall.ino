#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IRremote.hpp>
#include "Net.h"
#define SERVER
// OLED 显示参数
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCL_PIN 23
#define SDA_PIN 19
#define IR_PIN 18

// 初始化 OLED 屏幕对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 游戏参数
const int CELL_SIZE = 6;                     // 每节蛇和食物的尺寸变大了（由 4 改为 6）
const int MAX_LENGTH = 128;                  // 蛇的最大节数
int snakeX[MAX_LENGTH], snakeY[MAX_LENGTH];  // 蛇每一节的坐标数组
int snakeLength = 3;                         // 当前蛇的长度
int foodX = 40, foodY = 20;                  // 食物的初始位置
int dx = CELL_SIZE, dy = 0;                  // 初始运动方向为右

unsigned long lastMove = 0;
const int moveInterval = 1000;  // 蛇的移动时间间隔（单位 ms）
int gameTime = 0;
bool ballWin = false;
bool gameOver = false;
// 初始化 OLED 和红外遥控
void setup() {
  Serial.begin(9600);
  setup_net();
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);  // 初始化红外接收
}


// void timer() {
//   delay(1000);
//   gameTime++;
// }

uint8_t snakeHeadX, snakeHeadY;
uint8_t foodHeadX=foodX, foodHeadY=foodY;
void restrictFood() {
  // 穿墙逻辑
  if (foodX < 0) foodX = SCREEN_WIDTH - CELL_SIZE;
  if (foodX >= SCREEN_WIDTH) foodX = 0;
  if (foodY < 0) foodY = SCREEN_HEIGHT - CELL_SIZE;
  if (foodY >= SCREEN_HEIGHT) foodY = 0;
}
void moveSnake() {
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }
}

void stepSnake() {
  snakeX[0] += dx;
  snakeY[0] += dy;
}
void restrictSnake() {
  // 穿墙逻辑
  if (snakeX[0] < 0) snakeX[0] = SCREEN_WIDTH - CELL_SIZE;
  if (snakeX[0] >= SCREEN_WIDTH) snakeX[0] = 0;
  if (snakeY[0] < 0) snakeY[0] = SCREEN_HEIGHT - CELL_SIZE;
  if (snakeY[0] >= SCREEN_HEIGHT) snakeY[0] = 0;
}
void update() {
  try_receive();
  bool shouldUpdate;
  if (millis() - lastMove >= moveInterval) {
    lastMove = millis();
    shouldUpdate = true;
  } else shouldUpdate = false;


#ifdef SERVER
  //Serial.println("ServerUpdate");
  if (shouldUpdate) {
    Serial.print("ShouldUpdate: ");
    Serial.println(shouldUpdate);
    moveSnake();
    stepSnake();
    restrictSnake();
    packet[0] = snakeX[0];
    packet[1] = snakeY[0];
    Serial.print("SnakeX: ");
    Serial.println(snakeX[0]);
    Serial.print("SnakeY: ");
    Serial.println(snakeX[1]);
    //delay(1000);
  }

  if (valid_digit(4, 5)) {
    foodX = packet[4];
    foodY = packet[5];
    restrictFood();
    packet[3] = foodX;
    packet[4] = foodY;
  }
#else
  //Serial.println("ClientUpdate");
  if (valid_digit(0, 1) && (packet[0] != snakeX[0] || packet[1] != snakeY[0])) {
    moveSnake();
    snakeX[0] = packet[0];
    snakeY[0] = packet[1];
    Serial.print("SnakeX: ");
    Serial.println(snakeX[0]);
    Serial.print("SnakeY: ");
    Serial.println(snakeY[0]);
    Serial.println("updateSnakePos");
    //delay(1000);
  }
  if (valid_digit(2, 3)) {
    foodX = packet[2];
    foodY = packet[3];
    Serial.print("foodX: ");
    Serial.println(foodX);
    Serial.print("foodY: ");
    Serial.println(foodY);
    Serial.println("updateFoodPos");
    //delay(1000);
  }
  if (foodHeadX != foodX || foodHeadY != foodY) {
    packet[4] = foodHeadX;
    packet[5] = foodHeadY;
    Serial.print("foodX: ");
    Serial.println(foodHeadX);
    Serial.print("foodY: ");
    Serial.println(foodHeadY);
    Serial.println("uploadFoodPos");
    //delay(3000);
  }
#endif


#ifdef SERVER
  send_to_client();
#else
  send_to_server();
#endif
}

void drawSnake() {
  for (int i = 0; i < snakeLength; i++) {
    display.fillRect(snakeX[i], snakeY[i], CELL_SIZE, CELL_SIZE, WHITE);
  }
}

// // 蛇移动，每节跟随前一节
// void resetGame() {
//   //snakeLength = 3;
//   //dx = CELL_SIZE;
//   //dy = 0;
//   //or (int i = 0; i < snakeLength; i++) {
//   // snakeX[i] = 64 - i * CELL_SIZE;
//   //snakeY[i] = 32;
//   //}
//   // 随机生成一个食物位置（以 CELL_SIZE 对齐）
//   foodX = (rand() % (SCREEN_WIDTH / CELL_SIZE)) * CELL_SIZE;
//   foodY = (rand() % (SCREEN_HEIGHT / CELL_SIZE)) * CELL_SIZE;
//   gameOver = false;
//   gameTime = 0;
// }
// 绘制食物
void drawFood() {
  display.fillRect(foodX, foodY, CELL_SIZE, CELL_SIZE, WHITE);
}




// 蛇吃到食物后的处理
void checkFood() {
  int dx = snakeX[0] - foodX;
  int dy = snakeY[0] - foodY;
  if (dx * dx + dy * dy < CELL_SIZE * CELL_SIZE) {
    // 吃到食物，身体增长
    if (snakeLength < MAX_LENGTH) {
      snakeLength++;
    }
    // 生成新食物位置（CELL_SIZE 对齐）
    foodX = (rand() % (SCREEN_WIDTH / CELL_SIZE)) * CELL_SIZE;
    foodY = (rand() % (SCREEN_HEIGHT / CELL_SIZE)) * CELL_SIZE;

    packet[3] = foodX;
    packet[4] = foodY;
  }
}


void drawTime() {

  // ...其他游戏逻辑（如蛇移动、碰撞检测等）

  // 更新屏幕显示
  // display.clearDisplay();  // 清空屏幕

  // 显示gameTime（右上角）
  display.setTextSize(1);                   // 字体大小
  display.setTextColor(WHITE);              // 颜色
  display.setCursor(SCREEN_WIDTH - 30, 0);  // 位置：X=屏幕宽度-30像素，Y=0
  display.print("Time:");
  display.print(gameTime);

  // display.display();  // 刷新屏幕
}

// void check1() {
//   if (gameTime == 20) {
//     gameOver = true;
//     ballWin = true;
//   }
// }
// // 显示“GAME OVER”并等待 5 秒后重新开始游戏
// void showGameOver() {
//   display.clearDisplay();
//   display.setTextSize(2);
//   display.setTextColor(WHITE);
//   display.setCursor(20, 24);  // 居中大约显示
//   if (ballWin) {
//     display.println("BALL WIN");
//     display.display();
//     delay(5000);  // 停留 5 秒
//     resetGame();
//   }
// }

// 接收红外遥控指令并设置方向（不能向相反方向转弯）
void handleIR() {
  if (!IrReceiver.decode()) return;
  uint8_t cmd = IrReceiver.decodedIRData.command;
  Serial.println(cmd, HEX);

#ifdef SERVER
  if (cmd == 0x18 && dy == 0) {
    dx = 0;
    dy = -CELL_SIZE;
  }  // 上
  if (cmd == 0x52 && dy == 0) {
    dx = 0;
    dy = CELL_SIZE;
  }  // 下
  if (cmd == 0x08 && dx == 0) {
    dx = -CELL_SIZE;
    dy = 0;
  }  // 左
  if (cmd == 0x5A && dx == 0) {
    dx = CELL_SIZE;
    dy = 0;
  }  // 右

#else
  if (cmd == 0x18 && foodY < SCREEN_HEIGHT) {
    foodHeadX = foodX;
    foodHeadY = foodY - CELL_SIZE;
  }  // 上
  if (cmd == 0x52 && foodY > 0) {
    foodHeadX = foodX;
    foodHeadY = foodY + CELL_SIZE;
  }  // 下
  if (cmd == 0x08 && foodX > 0) {
    foodHeadX = foodX - CELL_SIZE;
    foodHeadY = foodY;
  }  // 左
  if (cmd == 0x5A && foodX < SCREEN_WIDTH) {
    foodHeadX = foodX + CELL_SIZE;
    foodHeadY = foodY;
  }  // 右
     // 穿墙逻辑
  if (foodHeadX < 0) foodHeadX = SCREEN_WIDTH - CELL_SIZE;
  if (foodHeadX >= SCREEN_WIDTH) foodHeadX = 0;
  if (foodHeadY < 0) foodHeadY = SCREEN_HEIGHT - CELL_SIZE;
  if (foodHeadY >= SCREEN_HEIGHT) foodHeadY = 0;
#endif
  IrReceiver.resume();
}

// 主循环
void loop() {
  // timer();
  // drawTime();
  handleIR();  // 处理红外遥控指令

  // if (gameOver) {
  //   showGameOver();  // 撞到自己后显示 GAME OVER
  //   return;
  // }

  // 控制蛇的移动节奏


  update();
  // check1();
  // checkFood();

  display.clearDisplay();
  drawSnake();
  drawFood();
  display.display();
}
