# fixwing PID衰减因子优化实施记录

**日期**: 2026-04-06

**目标**: 优化 `/home/sherlock/inav/inav/src/main/flight/pid.c` 的 fixwing 衰减逻辑

---

## 一、修改内容

### 1. fwPidAttenuation_t 结构体扩展

**位置**: [pid.c:71-81](file:///home/sherlock/inav/inav/src/main/flight/pid.c#L71-L81)

**变更**: 增加三个字段用于回弹检测

```c
typedef struct {
    float aP;
    float aI;
    float aD;
    float aFF;
    timeMs_t targetOverThresholdTimeMs;
    float lastRateTarget;        // 新增
    timeMs_t stickReturnTimeMs;   // 新增
    int8_t lastErrorSign;        // 新增
} fwPidAttenuation_t;
```

---

### 2. iTermLockApply 函数重写

**位置**: [pid.c:819-855](file:///home/sherlock/inav/inav/src/main/flight/pid.c#L819-L855)

**变更**:
- aI 计算改为 `MAX(dampingFactor, 0.5f)`，最低保留50%
- aP 和 aD 始终设为 1.0f
- 增加 sign change 检测用于回弹检测
- 回弹时反转并减半 I 项: `errorGyroIf *= -0.2f`

**核心逻辑**:
```c
if (signChanged) {
    uint32_t timeSinceLastChange = millis() - pidState->attenuation.targetOverThresholdTimeMs;
    if (timeSinceLastChange > ATTENUATION_SIGN_CHANGE_MIN_INTERVAL_MS) {
        pidState->errorGyroIf *= -0.2f;
        pidState->attenuation.targetOverThresholdTimeMs = millis();
        pidState->attenuation.aI = 1.0f;
    }
}
```

---

### 3. 新增常量定义

**位置**: [pid.c:168](file:///home/sherlock/inav/inav/src/main/flight/pid.c#L168)

```c
#define ATTENUATION_SIGN_CHANGE_MIN_INTERVAL_MS 500
```

**作用**: sign change 触发间隔限制 ≥500ms，避免噪声频繁触发

---

### 4. calculateFixedWingAirspeedTPAFactor 重写

**位置**: [pid.c:453-474](file:///home/sherlock/inav/inav/src/main/flight/pid.c#L453-L474)

**变更**:
- 新增 `AIRSPEED_MIN_VALUE 980` (约35km/h最小有效空速)
- 使用更平滑的曲线: `0.5f + 1.0f / (1.0f + ratio * ratio)`
- 增加防除零保护: `deltaReferenceAirspeed = MAX(deltaReferenceAirspeed, 0.001f)`
- 约束范围从 `[0.3f, 2.0f]`
- 函数签名改为接收 `float airspeed` 参数

**新公式**:
```c
static float calculateFixedWingAirspeedTPAFactor(float airspeed)
{
    float tpaFactor = 1.0f;

    if (currentControlProfile->throttle.dynPID != 0 &&
        !FLIGHT_MODE(AUTO_TUNE) && ARMING_FLAG(ARMED))
    {
        if (airspeed > AIRSPEED_MIN_VALUE) {
            float deltaAirspeed = airspeed - AIRSPEED_MIN_VALUE;
            float deltaReferenceAirspeed = pidProfile()->fixedWingReferenceAirspeed - AIRSPEED_MIN_VALUE;
            deltaReferenceAirspeed = MAX(deltaReferenceAirspeed, 0.001f);
            float ratio = deltaAirspeed / deltaReferenceAirspeed;
            tpaFactor = 0.5f + 1.0f / (1.0f + ratio * ratio);
            tpaFactor = constrainf(tpaFactor, 0.5f, 1.5f);
        } else {
            tpaFactor = 1.5f;
        }
        tpaFactor = 1.0f + (tpaFactor - 1.0f) * (currentControlProfile->throttle.dynPID / 100.0f);
    }
    return tpaFactor;
}
```

---

### 5. 删除 calculateFixedWingAirspeedITermFactor

**变更**: 删除独立的 I 项缩放函数，统一使用 tpaFactor

---

### 6. updatePIDCoefficients 简化

**位置**: [pid.c:559-587](file:///home/sherlock/inav/inav/src/main/flight/pid.c#L559-L587)

**变更**:
- 条件从 `apa_pow>0` 改为 `dynPID>0`
- 删除 `iTermFactor` 变量
- P/I/D/FF 全部使用相同 `tpaFactor`

**简化后的逻辑**:
```c
float tpaFactor=1.0f;
if(usedPidControllerType == PID_TYPE_PIFF){
    if(currentControlProfile->throttle.dynPID>0 && pitotValidForAirspeed()){
        tpaFactor = calculateFixedWingAirspeedTPAFactor(getAirspeedEstimate());
    } else {
        tpaFactor = calculateFixedWingTPAFactor(calculateTPAThtrottle());
    }
} else {
    tpaFactor = calculateMultirotorTPAFactor(calculateTPAThtrottle());
}
```

---

## 二、编译验证

**目标板**: SPEEDYBEEF405WING

**编译结果**:
- FLASH: 622792 B / 896 KB (67.88%)
- RAM: 108616 B / 128 KB (82.87%)
- CCM: 25276 B / 64 KB (38.57%)

**状态**: ✅ 编译成功

---

## 三、原始参考

优化来源: `/home/sherlock/inav-8.01-main-aoa/main/flight/pid.c`

对比差异后选择性移植了以下优化:
1. sign change 回弹检测逻辑
2. 平滑TPA曲线公式
3. I项最低50%保留
