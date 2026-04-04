# AOA 功能移植修改文档

## 一、新增文件（7个）

| 文件路径 | 说明 |
|---------|------|
| `drivers/aoa/aoa.h` | AOA 设备驱动基础定义 |
| `drivers/aoa/aoa_virtual.h` | AOA 虚拟设备驱动头文件 |
| `drivers/aoa/aoa_virtual.c` | AOA 虚拟设备驱动实现 |
| `sensors/aoa.h` | AOA 传感器核心定义（含控制配置） |
| `sensors/aoa.c` | AOA 传感器核心实现（含控制逻辑） |
| `io/aoa.h` | AOA MSP 接口头文件 |
| `io/aoa_msp.c` | AOA MSP 接口实现 |

## 二、修改文件（21个）

| 文件路径 | 修改内容 |
|---------|---------|
| `msp/msp_protocol_v2_sensor.h` | 新增 `MSP2_SENSOR_AOA` 命令定义 |
| `msp/msp_protocol_v2_sensor_msg.h` | 新增 `mspSensorAoaDataMessage_t` 结构体 |
| `config/parameter_group_ids.h` | 新增 `PG_AOA_CONFIG`、`PG_AOA_CONTROL_CONFIG` |
| `sensors/sensors.h` | 新增 `SENSOR_AOA` 枚举 |
| `sensors/initialisation.c` | 新增 `aoaInit()` 调用 |
| `sensors/diagnostics.c` | 新增 `getHwAoaStatus()` 函数 |
| `sensors/diagnostics.h` | 新增 `getHwAoaStatus()` 声明 |
| `fc/fc_tasks.c` | 新增 AOA 任务处理 |
| `fc/fc_msp.c` | 新增 AOA 硬件状态读写、MSP 消息处理 |
| `fc/settings.yaml` | 新增 AOA 配置参数定义 |
| `fc/cli.c` | 新增 `table_aoa_hardware` 引用、`debugModeNames` 中添加 "AOA" |
| `io/osd.c` | 新增 `OSD_AOA` 显示逻辑 |
| `io/osd.h` | 新增 `OSD_AOA` 枚举 |
| `drivers/osd_symbols.h` | 新增 `SYM_AOA`、`SYM_AOA_UP`、`SYM_AOA_DOWN` 符号 |
| `build/debug.h` | 新增 `DEBUG_AOA` 调试类型 |
| `scheduler/scheduler.h` | 新增 `TASK_AOA` 任务 |
| `flight/servos.c` | 新增 AOA 控制输出到 GVAR |
| `programming/logic_condition.c` | 新增 `LOGIC_CONDITION_OPERAND_FLIGHT_AOA` 支持 |
| `programming/logic_condition.h` | 新增 `LOGIC_CONDITION_OPERAND_FLIGHT_AOA` 枚举 |
| `target/common.h` | 新增 `USE_AOA`、`USE_AOA_MSP` 宏 |
| `CMakeLists.txt` | 新增 AOA 相关源文件 |

## 三、核心功能说明

### 1. AOA 传感器配置

```c
typedef struct aoaConfig_s {
    uint8_t aoa_hardware;      // 硬件类型 (NONE/MSP/FAKE)
    int16_t aoa_offset;        // 传感器偏移校准 (度)
    int16_t aoa_max_angle;     // 最大测量角度 (度)
    int16_t aoa_min_angle;     // 最小测量角度 (度)
} aoaConfig_t;
```

### 2. AOA 控制配置

```c
typedef struct aoaControlConfig_s {
    int8_t fw_aoa_control_channel;    // 使能通道 (-1默认启用)
    int8_t fw_aoa_gvar_index;         // GVAR 输出索引 (-1禁用)
    uint8_t fw_aoa_deg2pwm;           // 角度转PWM系数
    int8_t fw_aoa_trim_angle;         // 配平攻角 (度)
    int8_t fw_aoa_upper_limit_angle;  // 上限角度 (度)
    int8_t fw_aoa_lower_limit_angle;  // 下限角度 (度)
    uint8_t fw_aoa_aircraft_type;     // 布局类型 (常规/鸭翼)
    uint8_t fw_aoa_kp;                // P增益百分比
} aoaControlConfig_t;
```

### 3. CLI 命令

```
aoa_hardware              - AOA 硬件选择
aoa_offset                - 传感器偏移
aoa_max_angle             - 最大角度
aoa_min_angle             - 最小角度
fw_aoa_control_channel    - 使能通道
fw_aoa_gvar_index         - GVAR索引
fw_aoa_deg2pwm            - 角度转PWM
fw_aoa_trim_angle         - 配平角度
fw_aoa_upper_limit_angle  - 上限
fw_aoa_lower_limit_angle  - 下限
fw_aoa_aircraft_type      - 布局类型
fw_aoa_kp                 - P增益
```

### 4. OSD 显示

- `OSD_AOA` - 显示当前攻角值，带上下箭头指示

### 5. MSP 协议

- `MSP2_SENSOR_AOA` (0x1F08) - 接收外部 AOA 传感器数据

## 四、删除内容

| 文件 | 删除内容 | 原因 |
|------|---------|------|
| `common/maths.h` | `scaleRange2()` 声明 | 未使用 |
| `common/maths.c` | `scaleRange2()` 实现 | 未使用 |

## 五、命名规范调整

| 原始命名 | INAV 规范命名 |
|---------|--------------|
| `pidAoaControlConfig_t` | `aoaControlConfig_t` |
| `PG_PID_AOA_CONTROL_CONFIG` | `PG_AOA_CONTROL_CONFIG` |
