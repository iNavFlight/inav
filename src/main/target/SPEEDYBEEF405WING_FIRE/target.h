#pragma once

#define TARGET_BOARD_IDENTIFIER "S405"
#define TARGET_MANUFACTURER_IDENTIFIER "SPBE"

#define TARGET_NAME "SPEEDYBEEF405WING_FIRE"  // новое имя таргета

#include "SPEEDYBEEF405WING/target.h"  // наследуем всё от оригинала

// Переопределяем только то, что нужно
#undef TARGET
#define TARGET "SPEEDYBEEF405WING_FIRE"