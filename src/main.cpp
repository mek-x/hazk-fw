#include <Arduino.h>

// This function is called by the Arduino Core before setup()
// We force it to use the Internal High Speed oscillator (HSI)
// to avoid the "External Crystal Hang" we encountered.
extern "C" void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  // 1. Setup HSI and Main PLL
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

  // On F105, HSI is always divided by 2 before reaching the PLL
  // 8MHz / 2 = 4MHz.
  // 4MHz * 9 = 36MHz (Let's start safe at 36MHz)
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    while(1); // Hang here if config fails
  }

  // 2. Configure Clock Tree
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2; // APB1 must be <= 36MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    while(1);
  }
}

// Map Serial1 to PA9/PA10 explicitly
HardwareSerial Serial1(PA10, PA9);

void setup() {
  Serial1.begin(115200);
  delay(1000);
  Serial1.println("Arduino Framework FIX: SUCCESS");

  // Prepare PC6 for testing
  pinMode(PC6, OUTPUT);
}

void loop() {
  Serial1.println("Ping...");
  digitalWrite(PC6, !digitalRead(PC6));
  delay(500);
}
