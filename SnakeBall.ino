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
const int moveInterval = 150;  // 蛇的移动时间间隔（单位 ms）

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
int gameTime = 0;
bool gameOver = false;
bool ballWin = false; 
void timer() {
  delay (1000);
  gameTime++;
}

//void drawSnake() {
//for (int i = 0; i < snakeLength; i++) {
//display.fillRect(snakeX[i], snakeY[i], CELL_SIZE, CELL_SIZE, WHITE);
//}
//}

// 蛇移动，每节跟随前一节
void resetGame() {
  //snakeLength = 3;
  //dx = CELL_SIZE;
  //dy = 0;
  //or (int i = 0; i < snakeLength; i++) {
  // snakeX[i] = 64 - i * CELL_SIZE;
  //snakeY[i] = 32;
  //}
  // 随机生成一个食物位置（以 CELL_SIZE 对齐）
  foodX = (rand() % (SCREEN_WIDTH / CELL_SIZE)) * CELL_SIZE;
  foodY = (rand() % (SCREEN_HEIGHT / CELL_SIZE)) * CELL_SIZE;
  gameOver = false;
  gameTime = 0;
  
}
//void moveSnake() {
  //for (int i = snakeLength - 1; i > 0; i--) {
  //snakeX[i] = snakeX[i - 1];
  //snakeY[i] = snakeY[i - 1];
  //}
  //snakeX[0] += dx;
  //snakeY[0] += dy;

  // 穿墙逻辑
  //if (foodX < 0) foodX = SCREEN_WIDTH - CELL_SIZE;
  //if (foodX >= SCREEN_WIDTH) foodX = 0;
  //if (foodY < 0) foodY = SCREEN_HEIGHT - CELL_SIZE;
  //if (foodY >= SCREEN_HEIGHT) foodY = 0;
  //}

  // 绘制食物
  void drawFood() {
    display.fillRect(foodX, foodY, CELL_SIZE, CELL_SIZE, WHITE);
  }

  // 蛇吃到食物后的处理
  //void checkFood() {
  //int dx = snakeX[0] - foodX;
  //int dy = snakeY[0] - foodY;
  //if (dx * dx + dy * dy < CELL_SIZE * CELL_SIZE) {
  // 吃到食物，身体增长
  //if (snakeLength < MAX_LENGTH) {
  //snakeLength++;
  //}
  // 生成新食物位置（CELL_SIZE 对齐）
  //foodX = (rand() % (SCREEN_WIDTH / CELL_SIZE)) * CELL_SIZE;
  //foodY = (rand() % (SCREEN_HEIGHT / CELL_SIZE)) * CELL_SIZE;
  //}
  //}
void drawTime() {
  // ...其他游戏逻辑（如蛇移动、碰撞检测等）

  // 更新屏幕显示
  display.clearDisplay(); // 清空屏幕
  
  // 显示gameTime（右上角）
  display.setTextSize(1);        // 字体大小
  display.setTextColor(WHITE);   // 颜色
  display.setCursor(SCREEN_WIDTH - 30, 0); // 位置：X=屏幕宽度-30像素，Y=0
  display.print("Time:");
  display.print(gameTime);

  // ...绘制其他元素（如蛇、球等）
  
  display.display(); // 刷新屏幕
}
  // 检测蛇是否撞到自己身体
  void check1() {
    if (gameTime == 20) {
      gameOver = true;
      ballWin = true;
    }
  }
    // 显示“GAME OVER”并等待 5 秒后重新开始游戏
    void showGameOver() {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(20, 24);  // 居中大约显示
      if (ballWin) {
        display.println("BALL WIN");
        display.display();
        delay(5000);  // 停留 5 秒
        resetGame();
      }
    }

    // 接收红外遥控指令并设置方向（不能向相反方向转弯）
    void handleIR() {
      if (!IrReceiver.decode()) return;

      uint8_t cmd = IrReceiver.decodedIRData.command;
      Serial.println(cmd, HEX);

      if (cmd == 0x18 && foodY < SCREEN_HEIGHT) {
        foodX += 0;
        foodY += -CELL_SIZE;
      }  // 上
      if (cmd == 0x52 && foodY > 0) {
        foodX += 0;
        foodY += CELL_SIZE;
      }  // 下
      if (cmd == 0x08 && foodX > 0) {
        foodX += -CELL_SIZE;
        foodY += 0;
      }  // 左
      if (cmd == 0x5A && foodX < SCREEN_WIDTH) {
        foodX += CELL_SIZE;
        foodY += 0;
      }  // 右
         // 穿墙逻辑
      if (foodX < 0) foodX = SCREEN_WIDTH - CELL_SIZE;
      if (foodX >= SCREEN_WIDTH) foodX = 0;
      if (foodY < 0) foodY = SCREEN_HEIGHT - CELL_SIZE;
      if (foodY >= SCREEN_HEIGHT) foodY = 0;
      IrReceiver.resume();
    }

    // 主循环
    void loop() {
      timer();
      drawTime();
      handleIR();  // 处理红外遥控指令

      if (gameOver) {
        showGameOver();  // 撞到自己后显示 GAME OVER
        return;
      }

      // 控制蛇的移动节奏
      //if (millis() - lastMove >= moveInterval) {
      // lastMove = millis();

      //moveSnake();
      check1();
      // checkFood();

      display.clearDisplay();
      //drawSnake();
      drawFood();
      display.display();
    }

uint8_t snakeHeadX, snakeHeadY;
uint8_t void1, void2;
void updateSnake() {
#ifdef SEREVR
  moveSnake();
  packet[0]=snakeX[0];
  packet[1]=snakeY[0];
#else
  if (try_receive(snakeHeadX, snakeHeadY, void1, void2)
      && (snakeHeadX != snakeX[0] || snakeHeadY != snakeY[0])) {
    moveSnake();
  }
#endif
}
void updateFood(){
  
}
