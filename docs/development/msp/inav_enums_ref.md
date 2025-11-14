# Enumerations

**Auto-generated reference for MSP, refer to source for development, not this file, due to variations with #ifdefs which needs verification.**

## Table of contents

- [accelerationSensor_e](#enum-accelerationsensor_e)
- [accEvent_t](#enum-accevent_t)
- [adcChannel_e](#enum-adcchannel_e)
- [adcFunction_e](#enum-adcfunction_e)
- [adjustmentFunction_e](#enum-adjustmentfunction_e)
- [adjustmentMode_e](#enum-adjustmentmode_e)
- [afatfsAppendFreeClusterPhase_e](#enum-afatfsappendfreeclusterphase_e)
- [afatfsAppendSuperclusterPhase_e](#enum-afatfsappendsuperclusterphase_e)
- [afatfsCacheBlockState_e](#enum-afatfscacheblockstate_e)
- [afatfsClusterSearchCondition_e](#enum-afatfsclustersearchcondition_e)
- [afatfsDeleteFilePhase_e](#enum-afatfsdeletefilephase_e)
- [afatfsError_e](#enum-afatfserror_e)
- [afatfsExtendSubdirectoryPhase_e](#enum-afatfsextendsubdirectoryphase_e)
- [afatfsFATPattern_e](#enum-afatfsfatpattern_e)
- [afatfsFileOperation_e](#enum-afatfsfileoperation_e)
- [afatfsFilesystemState_e](#enum-afatfsfilesystemstate_e)
- [afatfsFileType_e](#enum-afatfsfiletype_e)
- [afatfsFindClusterStatus_e](#enum-afatfsfindclusterstatus_e)
- [afatfsFreeSpaceSearchPhase_e](#enum-afatfsfreespacesearchphase_e)
- [afatfsInitializationPhase_e](#enum-afatfsinitializationphase_e)
- [afatfsOperationStatus_e](#enum-afatfsoperationstatus_e)
- [afatfsSaveDirectoryEntryMode_e](#enum-afatfssavedirectoryentrymode_e)
- [afatfsSeek_e](#enum-afatfsseek_e)
- [afatfsTruncateFilePhase_e](#enum-afatfstruncatefilephase_e)
- [airmodeHandlingType_e](#enum-airmodehandlingtype_e)
- [angle_index_t](#enum-angle_index_t)
- [armingFlag_e](#enum-armingflag_e)
- [axis_e](#enum-axis_e)
- [barometerState_e](#enum-barometerstate_e)
- [baroSensor_e](#enum-barosensor_e)
- [batCapacityUnit_e](#enum-batcapacityunit_e)
- [batteryState_e](#enum-batterystate_e)
- [batVoltageSource_e](#enum-batvoltagesource_e)
- [baudRate_e](#enum-baudrate_e)
- [beeperMode_e](#enum-beepermode_e)
- [biquadFilterType_e](#enum-biquadfiltertype_e)
- [blackboxBufferReserveStatus_e](#enum-blackboxbufferreservestatus_e)
- [blackboxFeatureMask_e](#enum-blackboxfeaturemask_e)
- [bmi270Register_e](#enum-bmi270register_e)
- [bootLogEventCode_e](#enum-bootlogeventcode_e)
- [bootLogFlags_e](#enum-bootlogflags_e)
- [boxId_e](#enum-boxid_e)
- [busIndex_e](#enum-busindex_e)
- [busSpeed_e](#enum-busspeed_e)
- [busType_e](#enum-bustype_e)
- [channelType_t](#enum-channeltype_t)
- [climbRateToAltitudeControllerMode_e](#enum-climbratetoaltitudecontrollermode_e)
- [colorComponent_e](#enum-colorcomponent_e)
- [colorId_e](#enum-colorid_e)
- [crsfActiveAntenna_e](#enum-crsfactiveantenna_e)
- [crsfAddress_e](#enum-crsfaddress_e)
- [crsfFrameType_e](#enum-crsfframetype_e)
- [crsfFrameTypeIndex_e](#enum-crsfframetypeindex_e)
- [crsrRfMode_e](#enum-crsrrfmode_e)
- [crsrRfPower_e](#enum-crsrrfpower_e)
- [currentSensor_e](#enum-currentsensor_e)
- [devHardwareType_e](#enum-devhardwaretype_e)
- [deviceFlags_e](#enum-deviceflags_e)
- [displayCanvasBitmapOption_t](#enum-displaycanvasbitmapoption_t)
- [displayCanvasColor_e](#enum-displaycanvascolor_e)
- [displayCanvasOutlineType_e](#enum-displaycanvasoutlinetype_e)
- [displayportMspCommand_e](#enum-displayportmspcommand_e)
- [displayTransactionOption_e](#enum-displaytransactionoption_e)
- [displayWidgetType_e](#enum-displaywidgettype_e)
- [DjiCraftNameElements_t](#enum-djicraftnameelements_t)
- [dshotCommands_e](#enum-dshotcommands_e)
- [dumpFlags_e](#enum-dumpflags_e)
- [dynamicGyroNotchMode_e](#enum-dynamicgyronotchmode_e)
- [emergLandState_e](#enum-emerglandstate_e)
- [escSensorFrameStatus_t](#enum-escsensorframestatus_t)
- [escSensorState_t](#enum-escsensorstate_t)
- [failsafeChannelBehavior_e](#enum-failsafechannelbehavior_e)
- [failsafePhase_e](#enum-failsafephase_e)
- [failsafeProcedure_e](#enum-failsafeprocedure_e)
- [failsafeRxLinkState_e](#enum-failsaferxlinkstate_e)
- [failureMode_e](#enum-failuremode_e)
- [fatFilesystemType_e](#enum-fatfilesystemtype_e)
- [features_e](#enum-features_e)
- [filterType_e](#enum-filtertype_e)
- [fixedWingLaunchEvent_t](#enum-fixedwinglaunchevent_t)
- [fixedWingLaunchMessage_t](#enum-fixedwinglaunchmessage_t)
- [fixedWingLaunchState_t](#enum-fixedwinglaunchstate_t)
- [flashPartitionType_e](#enum-flashpartitiontype_e)
- [flashType_e](#enum-flashtype_e)
- [flight_dynamics_index_t](#enum-flight_dynamics_index_t)
- [flightModeFlags_e](#enum-flightmodeflags_e)
- [flightModeForTelemetry_e](#enum-flightmodefortelemetry_e)
- [flyingPlatformType_e](#enum-flyingplatformtype_e)
- [fport2_control_frame_type_e](#enum-fport2_control_frame_type_e)
- [frame_state_e](#enum-frame_state_e)
- [frame_type_e](#enum-frame_type_e)
- [frskyOSDColor_e](#enum-frskyosdcolor_e)
- [frskyOSDLineOutlineType_e](#enum-frskyosdlineoutlinetype_e)
- [frskyOSDRecvState_e](#enum-frskyosdrecvstate_e)
- [frskyOSDTransactionOptions_e](#enum-frskyosdtransactionoptions_e)
- [fw_autotune_rate_adjustment_e](#enum-fw_autotune_rate_adjustment_e)
- [fwAutolandApproachDirection_e](#enum-fwautolandapproachdirection_e)
- [fwAutolandState_t](#enum-fwautolandstate_t)
- [fwAutolandWaypoint_t](#enum-fwautolandwaypoint_t)
- [geoAltitudeConversionMode_e](#enum-geoaltitudeconversionmode_e)
- [geoAltitudeDatumFlag_e](#enum-geoaltitudedatumflag_e)
- [geoOriginResetMode_e](#enum-geooriginresetmode_e)
- [geozoneActionState_e](#enum-geozoneactionstate_e)
- [geozoneMessageState_e](#enum-geozonemessagestate_e)
- [ghstAddr_e](#enum-ghstaddr_e)
- [ghstDl_e](#enum-ghstdl_e)
- [ghstFrameTypeIndex_e](#enum-ghstframetypeindex_e)
- [ghstUl_e](#enum-ghstul_e)
- [gimbal_htk_mode_e](#enum-gimbal_htk_mode_e)
- [gimbalDevType_e](#enum-gimbaldevtype_e)
- [gimbalHeadtrackerState_e](#enum-gimbalheadtrackerstate_e)
- [gpsAutoBaud_e](#enum-gpsautobaud_e)
- [gpsAutoConfig_e](#enum-gpsautoconfig_e)
- [gpsBaudRate_e](#enum-gpsbaudrate_e)
- [gpsDynModel_e](#enum-gpsdynmodel_e)
- [gpsFixChar_e](#enum-gpsfixchar_e)
- [gpsFixType_e](#enum-gpsfixtype_e)
- [gpsProvider_e](#enum-gpsprovider_e)
- [gpsState_e](#enum-gpsstate_e)
- [gyroFilterMode_e](#enum-gyrofiltermode_e)
- [gyroHardwareLpf_e](#enum-gyrohardwarelpf_e)
- [gyroSensor_e](#enum-gyrosensor_e)
- [HardwareMotorTypes_e](#enum-hardwaremotortypes_e)
- [hardwareSensorStatus_e](#enum-hardwaresensorstatus_e)
- [headTrackerDevType_e](#enum-headtrackerdevtype_e)
- [hottEamAlarm1Flag_e](#enum-hotteamalarm1flag_e)
- [hottEamAlarm2Flag_e](#enum-hotteamalarm2flag_e)
- [hottState_e](#enum-hottstate_e)
- [hsvColorComponent_e](#enum-hsvcolorcomponent_e)
- [I2CSpeed](#enum-i2cspeed)
- [i2cState_t](#enum-i2cstate_t)
- [i2cTransferDirection_t](#enum-i2ctransferdirection_t)
- [ibusCommand_e](#enum-ibuscommand_e)
- [ibusSensorType1_e](#enum-ibussensortype1_e)
- [ibusSensorType_e](#enum-ibussensortype_e)
- [ibusSensorValue_e](#enum-ibussensorvalue_e)
- [inputSource_e](#enum-inputsource_e)
- [itermRelax_e](#enum-itermrelax_e)
- [led_pin_pwm_mode_e](#enum-led_pin_pwm_mode_e)
- [ledBaseFunctionId_e](#enum-ledbasefunctionid_e)
- [ledDirectionId_e](#enum-leddirectionid_e)
- [ledModeIndex_e](#enum-ledmodeindex_e)
- [ledOverlayId_e](#enum-ledoverlayid_e)
- [ledSpecialColorIds_e](#enum-ledspecialcolorids_e)
- [logicConditionFlags_e](#enum-logicconditionflags_e)
- [logicConditionsGlobalFlags_t](#enum-logicconditionsglobalflags_t)
- [logicFlightModeOperands_e](#enum-logicflightmodeoperands_e)
- [logicFlightOperands_e](#enum-logicflightoperands_e)
- [logicOperation_e](#enum-logicoperation_e)
- [logicWaypointOperands_e](#enum-logicwaypointoperands_e)
- [logTopic_e](#enum-logtopic_e)
- [lsm6dxxConfigMasks_e](#enum-lsm6dxxconfigmasks_e)
- [lsm6dxxConfigValues_e](#enum-lsm6dxxconfigvalues_e)
- [lsm6dxxRegister_e](#enum-lsm6dxxregister_e)
- [ltm_frame_e](#enum-ltm_frame_e)
- [ltm_modes_e](#enum-ltm_modes_e)
- [ltmUpdateRate_e](#enum-ltmupdaterate_e)
- [magSensor_e](#enum-magsensor_e)
- [mavlinkAutopilotType_e](#enum-mavlinkautopilottype_e)
- [mavlinkRadio_e](#enum-mavlinkradio_e)
- [measurementSteps_e](#enum-measurementsteps_e)
- [mixerProfileATRequest_e](#enum-mixerprofileatrequest_e)
- [mixerProfileATState_e](#enum-mixerprofileatstate_e)
- [modeActivationOperator_e](#enum-modeactivationoperator_e)
- [motorPwmProtocolTypes_e](#enum-motorpwmprotocoltypes_e)
- [motorStatus_e](#enum-motorstatus_e)
- [mpu9250CompassReadState_e](#enum-mpu9250compassreadstate_e)
- [mspFlashfsFlags_e](#enum-mspflashfsflags_e)
- [mspPassthroughType_e](#enum-msppassthroughtype_e)
- [mspSDCardFlags_e](#enum-mspsdcardflags_e)
- [mspSDCardState_e](#enum-mspsdcardstate_e)
- [multi_function_e](#enum-multi_function_e)
- [multiFunctionFlags_e](#enum-multifunctionflags_e)
- [nav_reset_type_e](#enum-nav_reset_type_e)
- [navAGLEstimateQuality_e](#enum-navaglestimatequality_e)
- [navArmingBlocker_e](#enum-navarmingblocker_e)
- [navDefaultAltitudeSensor_e](#enum-navdefaultaltitudesensor_e)
- [navExtraArmingSafety_e](#enum-navextraarmingsafety_e)
- [navFwLaunchStatus_e](#enum-navfwlaunchstatus_e)
- [navigationEstimateStatus_e](#enum-navigationestimatestatus_e)
- [navigationFSMEvent_t](#enum-navigationfsmevent_t)
- [navigationFSMState_t](#enum-navigationfsmstate_t)
- [navigationFSMStateFlags_t](#enum-navigationfsmstateflags_t)
- [navigationHomeFlags_t](#enum-navigationhomeflags_t)
- [navigationPersistentId_e](#enum-navigationpersistentid_e)
- [navMcAltHoldThrottle_e](#enum-navmcaltholdthrottle_e)
- [navMissionRestart_e](#enum-navmissionrestart_e)
- [navOverridesMotorStop_e](#enum-navoverridesmotorstop_e)
- [navPositionEstimationFlags_e](#enum-navpositionestimationflags_e)
- [navRTHAllowLanding_e](#enum-navrthallowlanding_e)
- [navRTHClimbFirst_e](#enum-navrthclimbfirst_e)
- [navSetWaypointFlags_t](#enum-navsetwaypointflags_t)
- [navSystemStatus_Error_e](#enum-navsystemstatus_error_e)
- [navSystemStatus_Flags_e](#enum-navsystemstatus_flags_e)
- [navSystemStatus_Mode_e](#enum-navsystemstatus_mode_e)
- [navSystemStatus_State_e](#enum-navsystemstatus_state_e)
- [navWaypointActions_e](#enum-navwaypointactions_e)
- [navWaypointFlags_e](#enum-navwaypointflags_e)
- [navWaypointHeadings_e](#enum-navwaypointheadings_e)
- [navWaypointP3Flags_e](#enum-navwaypointp3flags_e)
- [opflowQuality_e](#enum-opflowquality_e)
- [opticalFlowSensor_e](#enum-opticalflowsensor_e)
- [osd_adsb_warning_style_e](#enum-osd_adsb_warning_style_e)
- [osd_ahi_style_e](#enum-osd_ahi_style_e)
- [osd_alignment_e](#enum-osd_alignment_e)
- [osd_crosshairs_style_e](#enum-osd_crosshairs_style_e)
- [osd_crsf_lq_format_e](#enum-osd_crsf_lq_format_e)
- [osd_items_e](#enum-osd_items_e)
- [osd_sidebar_arrow_e](#enum-osd_sidebar_arrow_e)
- [osd_sidebar_scroll_e](#enum-osd_sidebar_scroll_e)
- [osd_SpeedTypes_e](#enum-osd_speedtypes_e)
- [osd_stats_energy_unit_e](#enum-osd_stats_energy_unit_e)
- [osd_unit_e](#enum-osd_unit_e)
- [osdCustomElementType_e](#enum-osdcustomelementtype_e)
- [osdCustomElementTypeVisibility_e](#enum-osdcustomelementtypevisibility_e)
- [osdDrawPointType_e](#enum-osddrawpointtype_e)
- [osdDriver_e](#enum-osddriver_e)
- [osdSpeedSource_e](#enum-osdspeedsource_e)
- [outputMode_e](#enum-outputmode_e)
- [pageId_e](#enum-pageid_e)
- [persistentObjectId_e](#enum-persistentobjectid_e)
- [pidAutotuneState_e](#enum-pidautotunestate_e)
- [pidControllerFlags_e](#enum-pidcontrollerflags_e)
- [pidIndex_e](#enum-pidindex_e)
- [pidType_e](#enum-pidtype_e)
- [pinLabel_e](#enum-pinlabel_e)
- [pitotSensor_e](#enum-pitotsensor_e)
- [pollType_e](#enum-polltype_e)
- [portSharing_e](#enum-portsharing_e)
- [pwmInitError_e](#enum-pwminiterror_e)
- [quadrant_e](#enum-quadrant_e)
- [QUADSPIClockDivider_e](#enum-quadspiclockdivider_e)
- [quadSpiMode_e](#enum-quadspimode_e)
- [rangefinderType_e](#enum-rangefindertype_e)
- [RCDEVICE_5key_connection_event_e](#enum-rcdevice_5key_connection_event_e)
- [rcdevice_5key_simulation_operation_e](#enum-rcdevice_5key_simulation_operation_e)
- [rcdevice_camera_control_opeation_e](#enum-rcdevice_camera_control_opeation_e)
- [rcdevice_features_e](#enum-rcdevice_features_e)
- [rcdevice_protocol_version_e](#enum-rcdevice_protocol_version_e)
- [rcdeviceCamSimulationKeyEvent_e](#enum-rcdevicecamsimulationkeyevent_e)
- [rcdeviceResponseStatus_e](#enum-rcdeviceresponsestatus_e)
- [resolutionType_e](#enum-resolutiontype_e)
- [resourceOwner_e](#enum-resourceowner_e)
- [resourceType_e](#enum-resourcetype_e)
- [reversibleMotorsThrottleState_e](#enum-reversiblemotorsthrottlestate_e)
- [rollPitchStatus_e](#enum-rollpitchstatus_e)
- [rssiSource_e](#enum-rssisource_e)
- [rthState_e](#enum-rthstate_e)
- [rthTargetMode_e](#enum-rthtargetmode_e)
- [rthTrackbackMode_e](#enum-rthtrackbackmode_e)
- [rxFrameState_e](#enum-rxframestate_e)
- [rxReceiverType_e](#enum-rxreceivertype_e)
- [rxSerialReceiverType_e](#enum-rxserialreceivertype_e)
- [safehomeUsageMode_e](#enum-safehomeusagemode_e)
- [sbasMode_e](#enum-sbasmode_e)
- [sbusDecoderState_e](#enum-sbusdecoderstate_e)
- [sdcardBlockOperation_e](#enum-sdcardblockoperation_e)
- [sdcardOperationStatus_e](#enum-sdcardoperationstatus_e)
- [sdcardReceiveBlockStatus_e](#enum-sdcardreceiveblockstatus_e)
- [sdcardReceiveBlockStatus_e](#enum-sdcardreceiveblockstatus_e)
- [sdcardState_e](#enum-sdcardstate_e)
- [SDIODevice](#enum-sdiodevice)
- [sensor_align_e](#enum-sensor_align_e)
- [sensorIndex_e](#enum-sensorindex_e)
- [sensors_e](#enum-sensors_e)
- [sensorTempCalState_e](#enum-sensortempcalstate_e)
- [serialPortFunction_e](#enum-serialportfunction_e)
- [serialPortIdentifier_e](#enum-serialportidentifier_e)
- [servoAutotrimState_e](#enum-servoautotrimstate_e)
- [servoIndex_e](#enum-servoindex_e)
- [servoProtocolType_e](#enum-servoprotocoltype_e)
- [setting_mode_e](#enum-setting_mode_e)
- [setting_section_e](#enum-setting_section_e)
- [setting_type_e](#enum-setting_type_e)
- [simATCommandState_e](#enum-simatcommandstate_e)
- [simModuleState_e](#enum-simmodulestate_e)
- [simReadState_e](#enum-simreadstate_e)
- [simTelemetryState_e](#enum-simtelemetrystate_e)
- [simTransmissionState_e](#enum-simtransmissionstate_e)
- [simTxFlags_e](#enum-simtxflags_e)
- [simulatorFlags_t](#enum-simulatorflags_t)
- [smartAudioVersion_e](#enum-smartaudioversion_e)
- [smartportFuelUnit_e](#enum-smartportfuelunit_e)
- [softSerialPortIndex_e](#enum-softserialportindex_e)
- [SPIClockSpeed_e](#enum-spiclockspeed_e)
- [Srxl2BindRequest](#enum-srxl2bindrequest)
- [Srxl2BindType](#enum-srxl2bindtype)
- [Srxl2ControlDataCommand](#enum-srxl2controldatacommand)
- [Srxl2DeviceId](#enum-srxl2deviceid)
- [Srxl2DeviceType](#enum-srxl2devicetype)
- [Srxl2PacketType](#enum-srxl2packettype)
- [Srxl2State](#enum-srxl2state)
- [stateFlags_t](#enum-stateflags_t)
- [stickPositions_e](#enum-stickpositions_e)
- [systemState_e](#enum-systemstate_e)
- [systemState_e](#enum-systemstate_e)
- [tchDmaState_e](#enum-tchdmastate_e)
- [tempSensorType_e](#enum-tempsensortype_e)
- [throttleStatus_e](#enum-throttlestatus_e)
- [throttleStatusType_e](#enum-throttlestatustype_e)
- [timerMode_e](#enum-timermode_e)
- [timerUsageFlag_e](#enum-timerusageflag_e)
- [timId_e](#enum-timid_e)
- [tristate_e](#enum-tristate_e)
- [tz_automatic_dst_e](#enum-tz_automatic_dst_e)
- [UARTDevice_e](#enum-uartdevice_e)
- [uartInverterLine_e](#enum-uartinverterline_e)
- [ublox_nav_sig_health_e](#enum-ublox_nav_sig_health_e)
- [ublox_nav_sig_quality](#enum-ublox_nav_sig_quality)
- [ubs_nav_fix_type_t](#enum-ubs_nav_fix_type_t)
- [ubx_ack_state_t](#enum-ubx_ack_state_t)
- [ubx_nav_status_bits_t](#enum-ubx_nav_status_bits_t)
- [ubx_protocol_bytes_t](#enum-ubx_protocol_bytes_t)
- [vcselPeriodType_e](#enum-vcselperiodtype_e)
- [videoSystem_e](#enum-videosystem_e)
- [voltageSensor_e](#enum-voltagesensor_e)
- [vs600Band_e](#enum-vs600band_e)
- [vs600Power_e](#enum-vs600power_e)
- [vtxDevType_e](#enum-vtxdevtype_e)
- [vtxFrequencyGroups_e](#enum-vtxfrequencygroups_e)
- [vtxLowerPowerDisarm_e](#enum-vtxlowerpowerdisarm_e)
- [vtxProtoResponseType_e](#enum-vtxprotoresponsetype_e)
- [vtxProtoState_e](#enum-vtxprotostate_e)
- [vtxScheduleParams_e](#enum-vtxscheduleparams_e)
- [warningFlags_e](#enum-warningflags_e)
- [warningLedState_e](#enum-warningledstate_e)
- [widgetAHIOptions_t](#enum-widgetahioptions_t)
- [widgetAHIStyle_e](#enum-widgetahistyle_e)
- [wpFwTurnSmoothing_e](#enum-wpfwturnsmoothing_e)
- [wpMissionPlannerStatus_e](#enum-wpmissionplannerstatus_e)
- [zeroCalibrationState_e](#enum-zerocalibrationstate_e)

---
## <a id="enum-accelerationsensor_e"></a>`accelerationSensor_e`

> Source: ../../../src/main/sensors/acceleration.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ACC_NONE` | 0 |  |
| `ACC_AUTODETECT` | 1 |  |
| `ACC_MPU6000` | 2 |  |
| `ACC_MPU6500` | 3 |  |
| `ACC_MPU9250` | 4 |  |
| `ACC_BMI160` | 5 |  |
| `ACC_ICM20689` | 6 |  |
| `ACC_BMI088` | 7 |  |
| `ACC_ICM42605` | 8 |  |
| `ACC_BMI270` | 9 |  |
| `ACC_LSM6DXX` | 10 |  |
| `ACC_FAKE` | 11 |  |
| `ACC_MAX` | ACC_FAKE |  |

---
## <a id="enum-accevent_t"></a>`accEvent_t`

> Source: ../../../src/main/telemetry/sim.c

| Enumerator | Value | Condition |
|---|---:|---|
| `ACC_EVENT_NONE` | 0 |  |
| `ACC_EVENT_HIGH` | 1 |  |
| `ACC_EVENT_LOW` | 2 |  |
| `ACC_EVENT_NEG_X` | 3 |  |

---
## <a id="enum-adcchannel_e"></a>`adcChannel_e`

> Source: ../../../src/main/drivers/adc.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ADC_CHN_NONE` | 0 |  |
| `ADC_CHN_1` | 1 |  |
| `ADC_CHN_2` | 2 |  |
| `ADC_CHN_3` | 3 |  |
| `ADC_CHN_4` | 4 |  |
| `ADC_CHN_5` | 5 |  |
| `ADC_CHN_6` | 6 |  |
| `ADC_CHN_MAX` | ADC_CHN_6 |  |
| `ADC_CHN_COUNT` |  |  |

---
## <a id="enum-adcfunction_e"></a>`adcFunction_e`

> Source: ../../../src/main/drivers/adc.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ADC_BATTERY` | 0 |  |
| `ADC_RSSI` | 1 |  |
| `ADC_CURRENT` | 2 |  |
| `ADC_AIRSPEED` | 3 |  |
| `ADC_FUNCTION_COUNT` | 4 |  |

---
## <a id="enum-adjustmentfunction_e"></a>`adjustmentFunction_e`

> Source: ../../../src/main/fc/rc_adjustments.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ADJUSTMENT_NONE` | 0 |  |
| `ADJUSTMENT_RC_RATE` | 1 |  |
| `ADJUSTMENT_RC_EXPO` | 2 |  |
| `ADJUSTMENT_THROTTLE_EXPO` | 3 |  |
| `ADJUSTMENT_PITCH_ROLL_RATE` | 4 |  |
| `ADJUSTMENT_YAW_RATE` | 5 |  |
| `ADJUSTMENT_PITCH_ROLL_P` | 6 |  |
| `ADJUSTMENT_PITCH_ROLL_I` | 7 |  |
| `ADJUSTMENT_PITCH_ROLL_D` | 8 |  |
| `ADJUSTMENT_PITCH_ROLL_FF` | 9 |  |
| `ADJUSTMENT_PITCH_P` | 10 |  |
| `ADJUSTMENT_PITCH_I` | 11 |  |
| `ADJUSTMENT_PITCH_D` | 12 |  |
| `ADJUSTMENT_PITCH_FF` | 13 |  |
| `ADJUSTMENT_ROLL_P` | 14 |  |
| `ADJUSTMENT_ROLL_I` | 15 |  |
| `ADJUSTMENT_ROLL_D` | 16 |  |
| `ADJUSTMENT_ROLL_FF` | 17 |  |
| `ADJUSTMENT_YAW_P` | 18 |  |
| `ADJUSTMENT_YAW_I` | 19 |  |
| `ADJUSTMENT_YAW_D` | 20 |  |
| `ADJUSTMENT_YAW_FF` | 21 |  |
| `ADJUSTMENT_RATE_PROFILE` | 22 |  |
| `ADJUSTMENT_PITCH_RATE` | 23 |  |
| `ADJUSTMENT_ROLL_RATE` | 24 |  |
| `ADJUSTMENT_RC_YAW_EXPO` | 25 |  |
| `ADJUSTMENT_MANUAL_RC_EXPO` | 26 |  |
| `ADJUSTMENT_MANUAL_RC_YAW_EXPO` | 27 |  |
| `ADJUSTMENT_MANUAL_PITCH_ROLL_RATE` | 28 |  |
| `ADJUSTMENT_MANUAL_ROLL_RATE` | 29 |  |
| `ADJUSTMENT_MANUAL_PITCH_RATE` | 30 |  |
| `ADJUSTMENT_MANUAL_YAW_RATE` | 31 |  |
| `ADJUSTMENT_NAV_FW_CRUISE_THR` | 32 |  |
| `ADJUSTMENT_NAV_FW_PITCH2THR` | 33 |  |
| `ADJUSTMENT_ROLL_BOARD_ALIGNMENT` | 34 |  |
| `ADJUSTMENT_PITCH_BOARD_ALIGNMENT` | 35 |  |
| `ADJUSTMENT_LEVEL_P` | 36 |  |
| `ADJUSTMENT_LEVEL_I` | 37 |  |
| `ADJUSTMENT_LEVEL_D` | 38 |  |
| `ADJUSTMENT_POS_XY_P` | 39 |  |
| `ADJUSTMENT_POS_XY_I` | 40 |  |
| `ADJUSTMENT_POS_XY_D` | 41 |  |
| `ADJUSTMENT_POS_Z_P` | 42 |  |
| `ADJUSTMENT_POS_Z_I` | 43 |  |
| `ADJUSTMENT_POS_Z_D` | 44 |  |
| `ADJUSTMENT_HEADING_P` | 45 |  |
| `ADJUSTMENT_VEL_XY_P` | 46 |  |
| `ADJUSTMENT_VEL_XY_I` | 47 |  |
| `ADJUSTMENT_VEL_XY_D` | 48 |  |
| `ADJUSTMENT_VEL_Z_P` | 49 |  |
| `ADJUSTMENT_VEL_Z_I` | 50 |  |
| `ADJUSTMENT_VEL_Z_D` | 51 |  |
| `ADJUSTMENT_FW_MIN_THROTTLE_DOWN_PITCH_ANGLE` | 52 |  |
| `ADJUSTMENT_VTX_POWER_LEVEL` | 53 |  |
| `ADJUSTMENT_TPA` | 54 |  |
| `ADJUSTMENT_TPA_BREAKPOINT` | 55 |  |
| `ADJUSTMENT_NAV_FW_CONTROL_SMOOTHNESS` | 56 |  |
| `ADJUSTMENT_FW_TPA_TIME_CONSTANT` | 57 |  |
| `ADJUSTMENT_FW_LEVEL_TRIM` | 58 |  |
| `ADJUSTMENT_NAV_WP_MULTI_MISSION_INDEX` | 59 |  |
| `ADJUSTMENT_NAV_FW_ALT_CONTROL_RESPONSE` | 60 |  |
| `ADJUSTMENT_FUNCTION_COUNT` | 61 |  |

---
## <a id="enum-adjustmentmode_e"></a>`adjustmentMode_e`

> Source: ../../../src/main/fc/rc_adjustments.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ADJUSTMENT_MODE_STEP` | 0 |  |
| `ADJUSTMENT_MODE_SELECT` | 1 |  |

---
## <a id="enum-afatfsappendfreeclusterphase_e"></a>`afatfsAppendFreeClusterPhase_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_APPEND_FREE_CLUSTER_PHASE_INITIAL` | 0 |  |
| `AFATFS_APPEND_FREE_CLUSTER_PHASE_FIND_FREESPACE` | 0 |  |
| `AFATFS_APPEND_FREE_CLUSTER_PHASE_UPDATE_FAT1` | 1 |  |
| `AFATFS_APPEND_FREE_CLUSTER_PHASE_UPDATE_FAT2` | 2 |  |
| `AFATFS_APPEND_FREE_CLUSTER_PHASE_UPDATE_FILE_DIRECTORY` | 3 |  |
| `AFATFS_APPEND_FREE_CLUSTER_PHASE_COMPLETE` | 4 |  |
| `AFATFS_APPEND_FREE_CLUSTER_PHASE_FAILURE` | 5 |  |

---
## <a id="enum-afatfsappendsuperclusterphase_e"></a>`afatfsAppendSuperclusterPhase_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_APPEND_SUPERCLUSTER_PHASE_INIT` | 0 |  |
| `AFATFS_APPEND_SUPERCLUSTER_PHASE_UPDATE_FREEFILE_DIRECTORY` | 1 |  |
| `AFATFS_APPEND_SUPERCLUSTER_PHASE_UPDATE_FAT` | 2 |  |
| `AFATFS_APPEND_SUPERCLUSTER_PHASE_UPDATE_FILE_DIRECTORY` | 3 |  |

---
## <a id="enum-afatfscacheblockstate_e"></a>`afatfsCacheBlockState_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_CACHE_STATE_EMPTY` | 0 |  |
| `AFATFS_CACHE_STATE_IN_SYNC` | 1 |  |
| `AFATFS_CACHE_STATE_READING` | 2 |  |
| `AFATFS_CACHE_STATE_WRITING` | 3 |  |
| `AFATFS_CACHE_STATE_DIRTY` | 4 |  |

---
## <a id="enum-afatfsclustersearchcondition_e"></a>`afatfsClusterSearchCondition_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `CLUSTER_SEARCH_FREE_AT_BEGINNING_OF_FAT_SECTOR` | 0 |  |
| `CLUSTER_SEARCH_FREE` | 1 |  |
| `CLUSTER_SEARCH_OCCUPIED` | 2 |  |

---
## <a id="enum-afatfsdeletefilephase_e"></a>`afatfsDeleteFilePhase_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_DELETE_FILE_DELETE_DIRECTORY_ENTRY` | 0 |  |
| `AFATFS_DELETE_FILE_DEALLOCATE_CLUSTERS` | 1 |  |

---
## <a id="enum-afatfserror_e"></a>`afatfsError_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.h

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_ERROR_NONE` | 0 |  |
| `AFATFS_ERROR_GENERIC` | 1 |  |
| `AFATFS_ERROR_BAD_MBR` | 2 |  |
| `AFATFS_ERROR_BAD_FILESYSTEM_HEADER` | 3 |  |

---
## <a id="enum-afatfsextendsubdirectoryphase_e"></a>`afatfsExtendSubdirectoryPhase_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_EXTEND_SUBDIRECTORY_PHASE_INITIAL` | 0 |  |
| `AFATFS_EXTEND_SUBDIRECTORY_PHASE_ADD_FREE_CLUSTER` | 0 |  |
| `AFATFS_EXTEND_SUBDIRECTORY_PHASE_WRITE_SECTORS` | 1 |  |
| `AFATFS_EXTEND_SUBDIRECTORY_PHASE_SUCCESS` | 2 |  |
| `AFATFS_EXTEND_SUBDIRECTORY_PHASE_FAILURE` | 3 |  |

---
## <a id="enum-afatfsfatpattern_e"></a>`afatfsFATPattern_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_FAT_PATTERN_UNTERMINATED_CHAIN` | 0 |  |
| `AFATFS_FAT_PATTERN_TERMINATED_CHAIN` | 1 |  |
| `AFATFS_FAT_PATTERN_FREE` | 2 |  |

---
## <a id="enum-afatfsfileoperation_e"></a>`afatfsFileOperation_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_FILE_OPERATION_NONE` | 0 |  |
| `AFATFS_FILE_OPERATION_CREATE_FILE` | 1 |  |
| `AFATFS_FILE_OPERATION_SEEK` | 2 |  |
| `AFATFS_FILE_OPERATION_CLOSE` | 3 |  |
| `AFATFS_FILE_OPERATION_TRUNCATE` | 4 |  |
| `AFATFS_FILE_OPERATION_UNLINK` | 5 |  |
| `AFATFS_FILE_OPERATION_APPEND_SUPERCLUSTER` | (6) | AFATFS_USE_FREEFILE |
| `AFATFS_FILE_OPERATION_LOCKED` | (7) | AFATFS_USE_FREEFILE |
| `AFATFS_FILE_OPERATION_APPEND_FREE_CLUSTER` | 8 |  |
| `AFATFS_FILE_OPERATION_EXTEND_SUBDIRECTORY` | 9 |  |

---
## <a id="enum-afatfsfilesystemstate_e"></a>`afatfsFilesystemState_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.h

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_FILESYSTEM_STATE_UNKNOWN` | 0 |  |
| `AFATFS_FILESYSTEM_STATE_FATAL` | 1 |  |
| `AFATFS_FILESYSTEM_STATE_INITIALIZATION` | 2 |  |
| `AFATFS_FILESYSTEM_STATE_READY` | 3 |  |

---
## <a id="enum-afatfsfiletype_e"></a>`afatfsFileType_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_FILE_TYPE_NONE` | 0 |  |
| `AFATFS_FILE_TYPE_NORMAL` | 1 |  |
| `AFATFS_FILE_TYPE_FAT16_ROOT_DIRECTORY` | 2 |  |
| `AFATFS_FILE_TYPE_DIRECTORY` | 3 |  |

---
## <a id="enum-afatfsfindclusterstatus_e"></a>`afatfsFindClusterStatus_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_FIND_CLUSTER_IN_PROGRESS` | 0 |  |
| `AFATFS_FIND_CLUSTER_FOUND` | 1 |  |
| `AFATFS_FIND_CLUSTER_FATAL` | 2 |  |
| `AFATFS_FIND_CLUSTER_NOT_FOUND` | 3 |  |

---
## <a id="enum-afatfsfreespacesearchphase_e"></a>`afatfsFreeSpaceSearchPhase_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_FREE_SPACE_SEARCH_PHASE_FIND_HOLE` | 0 |  |
| `AFATFS_FREE_SPACE_SEARCH_PHASE_GROW_HOLE` | 1 |  |

---
## <a id="enum-afatfsinitializationphase_e"></a>`afatfsInitializationPhase_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_INITIALIZATION_READ_MBR` | 0 |  |
| `AFATFS_INITIALIZATION_READ_VOLUME_ID` | 1 |  |
| `AFATFS_INITIALIZATION_FREEFILE_CREATE` | (2) | AFATFS_USE_FREEFILE |
| `AFATFS_INITIALIZATION_FREEFILE_CREATING` | (3) | AFATFS_USE_FREEFILE |
| `AFATFS_INITIALIZATION_FREEFILE_FAT_SEARCH` | (4) | AFATFS_USE_FREEFILE |
| `AFATFS_INITIALIZATION_FREEFILE_UPDATE_FAT` | (5) | AFATFS_USE_FREEFILE |
| `AFATFS_INITIALIZATION_FREEFILE_SAVE_DIR_ENTRY` | (6) | AFATFS_USE_FREEFILE |
| `AFATFS_INITIALIZATION_FREEFILE_LAST` | AFATFS_INITIALIZATION_FREEFILE_SAVE_DIR_ENTRY | AFATFS_USE_FREEFILE |
| `AFATFS_INITIALIZATION_DONE` |  |  |

---
## <a id="enum-afatfsoperationstatus_e"></a>`afatfsOperationStatus_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.h

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_OPERATION_IN_PROGRESS` | 0 |  |
| `AFATFS_OPERATION_SUCCESS` | 1 |  |
| `AFATFS_OPERATION_FAILURE` | 2 |  |

---
## <a id="enum-afatfssavedirectoryentrymode_e"></a>`afatfsSaveDirectoryEntryMode_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_SAVE_DIRECTORY_NORMAL` | 0 |  |
| `AFATFS_SAVE_DIRECTORY_FOR_CLOSE` | 1 |  |
| `AFATFS_SAVE_DIRECTORY_DELETED` | 2 |  |

---
## <a id="enum-afatfsseek_e"></a>`afatfsSeek_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.h

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_SEEK_SET` | 0 |  |
| `AFATFS_SEEK_CUR` | 1 |  |
| `AFATFS_SEEK_END` | 2 |  |

---
## <a id="enum-afatfstruncatefilephase_e"></a>`afatfsTruncateFilePhase_e`

> Source: ../../../src/main/io/asyncfatfs/asyncfatfs.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AFATFS_TRUNCATE_FILE_INITIAL` | 0 |  |
| `AFATFS_TRUNCATE_FILE_UPDATE_DIRECTORY` | 0 |  |
| `AFATFS_TRUNCATE_FILE_ERASE_FAT_CHAIN_NORMAL` | 1 |  |
| `AFATFS_TRUNCATE_FILE_ERASE_FAT_CHAIN_CONTIGUOUS` | (2) | AFATFS_USE_FREEFILE |
| `AFATFS_TRUNCATE_FILE_PREPEND_TO_FREEFILE` | (3) | AFATFS_USE_FREEFILE |
| `AFATFS_TRUNCATE_FILE_SUCCESS` | 4 |  |

---
## <a id="enum-airmodehandlingtype_e"></a>`airmodeHandlingType_e`

> Source: ../../../src/main/fc/rc_controls.h

| Enumerator | Value | Condition |
|---|---:|---|
| `STICK_CENTER` | 0 |  |
| `THROTTLE_THRESHOLD` | 1 |  |
| `STICK_CENTER_ONCE` | 2 |  |

---
## <a id="enum-angle_index_t"></a>`angle_index_t`

> Source: ../../../src/main/common/axis.h

| Enumerator | Value | Condition |
|---|---:|---|
| `AI_ROLL` | 0 |  |
| `AI_PITCH` | 1 |  |

---
## <a id="enum-armingflag_e"></a>`armingFlag_e`

> Source: ../../../src/main/fc/runtime_config.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ARMED` | (1 << 2) |  |
| `WAS_EVER_ARMED` | (1 << 3) |  |
| `SIMULATOR_MODE_HITL` | (1 << 4) |  |
| `SIMULATOR_MODE_SITL` | (1 << 5) |  |
| `ARMING_DISABLED_GEOZONE` | (1 << 6) |  |
| `ARMING_DISABLED_FAILSAFE_SYSTEM` | (1 << 7) |  |
| `ARMING_DISABLED_NOT_LEVEL` | (1 << 8) |  |
| `ARMING_DISABLED_SENSORS_CALIBRATING` | (1 << 9) |  |
| `ARMING_DISABLED_SYSTEM_OVERLOADED` | (1 << 10) |  |
| `ARMING_DISABLED_NAVIGATION_UNSAFE` | (1 << 11) |  |
| `ARMING_DISABLED_COMPASS_NOT_CALIBRATED` | (1 << 12) |  |
| `ARMING_DISABLED_ACCELEROMETER_NOT_CALIBRATED` | (1 << 13) |  |
| `ARMING_DISABLED_ARM_SWITCH` | (1 << 14) |  |
| `ARMING_DISABLED_HARDWARE_FAILURE` | (1 << 15) |  |
| `ARMING_DISABLED_BOXFAILSAFE` | (1 << 16) |  |
| `ARMING_DISABLED_RC_LINK` | (1 << 18) |  |
| `ARMING_DISABLED_THROTTLE` | (1 << 19) |  |
| `ARMING_DISABLED_CLI` | (1 << 20) |  |
| `ARMING_DISABLED_CMS_MENU` | (1 << 21) |  |
| `ARMING_DISABLED_OSD_MENU` | (1 << 22) |  |
| `ARMING_DISABLED_ROLLPITCH_NOT_CENTERED` | (1 << 23) |  |
| `ARMING_DISABLED_SERVO_AUTOTRIM` | (1 << 24) |  |
| `ARMING_DISABLED_OOM` | (1 << 25) |  |
| `ARMING_DISABLED_INVALID_SETTING` | (1 << 26) |  |
| `ARMING_DISABLED_PWM_OUTPUT_ERROR` | (1 << 27) |  |
| `ARMING_DISABLED_NO_PREARM` | (1 << 28) |  |
| `ARMING_DISABLED_DSHOT_BEEPER` | (1 << 29) |  |
| `ARMING_DISABLED_LANDING_DETECTED` | (1 << 30) |  |
| `ARMING_DISABLED_ALL_FLAGS` | (ARMING_DISABLED_GEOZONE | ARMING_DISABLED_FAILSAFE_SYSTEM | ARMING_DISABLED_NOT_LEVEL |                                                         ARMING_DISABLED_SENSORS_CALIBRATING | ARMING_DISABLED_SYSTEM_OVERLOADED | ARMING_DISABLED_NAVIGATION_UNSAFE |                                                        ARMING_DISABLED_COMPASS_NOT_CALIBRATED | ARMING_DISABLED_ACCELEROMETER_NOT_CALIBRATED |                                                        ARMING_DISABLED_ARM_SWITCH | ARMING_DISABLED_HARDWARE_FAILURE | ARMING_DISABLED_BOXFAILSAFE |                                                        ARMING_DISABLED_RC_LINK | ARMING_DISABLED_THROTTLE | ARMING_DISABLED_CLI |                                                        ARMING_DISABLED_CMS_MENU | ARMING_DISABLED_OSD_MENU | ARMING_DISABLED_ROLLPITCH_NOT_CENTERED |                                                        ARMING_DISABLED_SERVO_AUTOTRIM | ARMING_DISABLED_OOM | ARMING_DISABLED_INVALID_SETTING |                                                        ARMING_DISABLED_PWM_OUTPUT_ERROR | ARMING_DISABLED_NO_PREARM | ARMING_DISABLED_DSHOT_BEEPER |                                                        ARMING_DISABLED_LANDING_DETECTED) |  |

---
## <a id="enum-axis_e"></a>`axis_e`

> Source: ../../../src/main/common/axis.h

| Enumerator | Value | Condition |
|---|---:|---|
| `X` | 0 |  |
| `Y` | 1 |  |
| `Z` | 2 |  |

---
## <a id="enum-barometerstate_e"></a>`barometerState_e`

> Source: ../../../src/main/sensors/barometer.c

| Enumerator | Value | Condition |
|---|---:|---|
| `BAROMETER_NEEDS_SAMPLES` | 0 |  |
| `BAROMETER_NEEDS_CALCULATION` | 1 |  |

---
## <a id="enum-barosensor_e"></a>`baroSensor_e`

> Source: ../../../src/main/sensors/barometer.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BARO_NONE` | 0 |  |
| `BARO_AUTODETECT` | 1 |  |
| `BARO_BMP085` | 2 |  |
| `BARO_MS5611` | 3 |  |
| `BARO_BMP280` | 4 |  |
| `BARO_MS5607` | 5 |  |
| `BARO_LPS25H` | 6 |  |
| `BARO_SPL06` | 7 |  |
| `BARO_BMP388` | 8 |  |
| `BARO_DPS310` | 9 |  |
| `BARO_B2SMPB` | 10 |  |
| `BARO_MSP` | 11 |  |
| `BARO_FAKE` | 12 |  |
| `BARO_MAX` | BARO_FAKE |  |

---
## <a id="enum-batcapacityunit_e"></a>`batCapacityUnit_e`

> Source: ../../../src/main/sensors/battery_config_structs.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BAT_CAPACITY_UNIT_MAH` | 0 |  |
| `BAT_CAPACITY_UNIT_MWH` | 1 |  |

---
## <a id="enum-batterystate_e"></a>`batteryState_e`

> Source: ../../../src/main/sensors/battery.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BATTERY_OK` | 0 |  |
| `BATTERY_WARNING` | 1 |  |
| `BATTERY_CRITICAL` | 2 |  |
| `BATTERY_NOT_PRESENT` | 3 |  |

---
## <a id="enum-batvoltagesource_e"></a>`batVoltageSource_e`

> Source: ../../../src/main/sensors/battery_config_structs.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BAT_VOLTAGE_RAW` | 0 |  |
| `BAT_VOLTAGE_SAG_COMP` | 1 |  |

---
## <a id="enum-baudrate_e"></a>`baudRate_e`

> Source: ../../../src/main/io/serial.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BAUD_AUTO` | 0 |  |
| `BAUD_1200` | 1 |  |
| `BAUD_2400` | 2 |  |
| `BAUD_4800` | 3 |  |
| `BAUD_9600` | 4 |  |
| `BAUD_19200` | 5 |  |
| `BAUD_38400` | 6 |  |
| `BAUD_57600` | 7 |  |
| `BAUD_115200` | 8 |  |
| `BAUD_230400` | 9 |  |
| `BAUD_250000` | 10 |  |
| `BAUD_460800` | 11 |  |
| `BAUD_921600` | 12 |  |
| `BAUD_1000000` | 13 |  |
| `BAUD_1500000` | 14 |  |
| `BAUD_2000000` | 15 |  |
| `BAUD_2470000` | 16 |  |
| `BAUD_MIN` | BAUD_AUTO |  |
| `BAUD_MAX` | BAUD_2470000 |  |

---
## <a id="enum-beepermode_e"></a>`beeperMode_e`

> Source: ../../../src/main/io/beeper.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BEEPER_SILENCE` | 0 |  |
| `BEEPER_RUNTIME_CALIBRATION_DONE` | 1 |  |
| `BEEPER_HARDWARE_FAILURE` | 2 |  |
| `BEEPER_RX_LOST` | 3 |  |
| `BEEPER_RX_LOST_LANDING` | 4 |  |
| `BEEPER_DISARMING` | 5 |  |
| `BEEPER_ARMING` | 6 |  |
| `BEEPER_ARMING_GPS_FIX` | 7 |  |
| `BEEPER_BAT_CRIT_LOW` | 8 |  |
| `BEEPER_BAT_LOW` | 9 |  |
| `BEEPER_GPS_STATUS` | 10 |  |
| `BEEPER_RX_SET` | 11 |  |
| `BEEPER_ACTION_SUCCESS` | 12 |  |
| `BEEPER_ACTION_FAIL` | 13 |  |
| `BEEPER_READY_BEEP` | 14 |  |
| `BEEPER_MULTI_BEEPS` | 15 |  |
| `BEEPER_DISARM_REPEAT` | 16 |  |
| `BEEPER_ARMED` | 17 |  |
| `BEEPER_SYSTEM_INIT` | 18 |  |
| `BEEPER_USB` | 19 |  |
| `BEEPER_LAUNCH_MODE_ENABLED` | 20 |  |
| `BEEPER_LAUNCH_MODE_LOW_THROTTLE` | 21 |  |
| `BEEPER_LAUNCH_MODE_IDLE_START` | 22 |  |
| `BEEPER_CAM_CONNECTION_OPEN` | 23 |  |
| `BEEPER_CAM_CONNECTION_CLOSE` | 24 |  |
| `BEEPER_ALL` | 25 |  |
| `BEEPER_PREFERENCE` | 26 |  |

---
## <a id="enum-biquadfiltertype_e"></a>`biquadFilterType_e`

> Source: ../../../src/main/common/filter.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FILTER_LPF` | 0 |  |
| `FILTER_NOTCH` | 1 |  |

---
## <a id="enum-blackboxbufferreservestatus_e"></a>`blackboxBufferReserveStatus_e`

> Source: ../../../src/main/blackbox/blackbox_io.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BLACKBOX_RESERVE_SUCCESS` | 0 |  |
| `BLACKBOX_RESERVE_TEMPORARY_FAILURE` | 1 |  |
| `BLACKBOX_RESERVE_PERMANENT_FAILURE` | 2 |  |

---
## <a id="enum-blackboxfeaturemask_e"></a>`blackboxFeatureMask_e`

> Source: ../../../src/main/blackbox/blackbox.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BLACKBOX_FEATURE_NAV_ACC` | 1 << 0 |  |
| `BLACKBOX_FEATURE_NAV_POS` | 1 << 1 |  |
| `BLACKBOX_FEATURE_NAV_PID` | 1 << 2 |  |
| `BLACKBOX_FEATURE_MAG` | 1 << 3 |  |
| `BLACKBOX_FEATURE_ACC` | 1 << 4 |  |
| `BLACKBOX_FEATURE_ATTITUDE` | 1 << 5 |  |
| `BLACKBOX_FEATURE_RC_DATA` | 1 << 6 |  |
| `BLACKBOX_FEATURE_RC_COMMAND` | 1 << 7 |  |
| `BLACKBOX_FEATURE_MOTORS` | 1 << 8 |  |
| `BLACKBOX_FEATURE_GYRO_RAW` | 1 << 9 |  |
| `BLACKBOX_FEATURE_GYRO_PEAKS_ROLL` | 1 << 10 |  |
| `BLACKBOX_FEATURE_GYRO_PEAKS_PITCH` | 1 << 11 |  |
| `BLACKBOX_FEATURE_GYRO_PEAKS_YAW` | 1 << 12 |  |
| `BLACKBOX_FEATURE_SERVOS` | 1 << 13 |  |

---
## <a id="enum-bmi270register_e"></a>`bmi270Register_e`

> Source: ../../../src/main/drivers/accgyro/accgyro_bmi270.c

| Enumerator | Value | Condition |
|---|---:|---|
| `BMI270_REG_CHIP_ID` | 0 |  |
| `BMI270_REG_ERR_REG` | 2 |  |
| `BMI270_REG_STATUS` | 3 |  |
| `BMI270_REG_ACC_DATA_X_LSB` | 12 |  |
| `BMI270_REG_GYR_DATA_X_LSB` | 18 |  |
| `BMI270_REG_SENSORTIME_0` | 24 |  |
| `BMI270_REG_SENSORTIME_1` | 25 |  |
| `BMI270_REG_SENSORTIME_2` | 26 |  |
| `BMI270_REG_EVENT` | 27 |  |
| `BMI270_REG_INT_STATUS_0` | 28 |  |
| `BMI270_REG_INT_STATUS_1` | 29 |  |
| `BMI270_REG_INTERNAL_STATUS` | 33 |  |
| `BMI270_REG_TEMPERATURE_LSB` | 34 |  |
| `BMI270_REG_TEMPERATURE_MSB` | 35 |  |
| `BMI270_REG_FIFO_LENGTH_LSB` | 36 |  |
| `BMI270_REG_FIFO_LENGTH_MSB` | 37 |  |
| `BMI270_REG_FIFO_DATA` | 38 |  |
| `BMI270_REG_ACC_CONF` | 64 |  |
| `BMI270_REG_ACC_RANGE` | 65 |  |
| `BMI270_REG_GYRO_CONF` | 66 |  |
| `BMI270_REG_GYRO_RANGE` | 67 |  |
| `BMI270_REG_AUX_CONF` | 68 |  |
| `BMI270_REG_FIFO_DOWNS` | 69 |  |
| `BMI270_REG_FIFO_WTM_0` | 70 |  |
| `BMI270_REG_FIFO_WTM_1` | 71 |  |
| `BMI270_REG_FIFO_CONFIG_0` | 72 |  |
| `BMI270_REG_FIFO_CONFIG_1` | 73 |  |
| `BMI270_REG_SATURATION` | 74 |  |
| `BMI270_REG_INT1_IO_CTRL` | 83 |  |
| `BMI270_REG_INT2_IO_CTRL` | 84 |  |
| `BMI270_REG_INT_LATCH` | 85 |  |
| `BMI270_REG_INT1_MAP_FEAT` | 86 |  |
| `BMI270_REG_INT2_MAP_FEAT` | 87 |  |
| `BMI270_REG_INT_MAP_DATA` | 88 |  |
| `BMI270_REG_INIT_CTRL` | 89 |  |
| `BMI270_REG_INIT_DATA` | 94 |  |
| `BMI270_REG_ACC_SELF_TEST` | 109 |  |
| `BMI270_REG_GYR_SELF_TEST_AXES` | 110 |  |
| `BMI270_REG_PWR_CONF` | 124 |  |
| `BMI270_REG_PWR_CTRL` | 125 |  |
| `BMI270_REG_CMD` | 126 |  |

---
## <a id="enum-bootlogeventcode_e"></a>`bootLogEventCode_e`

> Source: ../../../src/main/drivers/logging_codes.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BOOT_EVENT_CONFIG_LOADED` | 0 |  |
| `BOOT_EVENT_SYSTEM_INIT_DONE` | 1 |  |
| `BOOT_EVENT_PWM_INIT_DONE` | 2 |  |
| `BOOT_EVENT_EXTRA_BOOT_DELAY` | 3 |  |
| `BOOT_EVENT_SENSOR_INIT_DONE` | 4 |  |
| `BOOT_EVENT_GPS_INIT_DONE` | 5 |  |
| `BOOT_EVENT_LEDSTRIP_INIT_DONE` | 6 |  |
| `BOOT_EVENT_TELEMETRY_INIT_DONE` | 7 |  |
| `BOOT_EVENT_SYSTEM_READY` | 8 |  |
| `BOOT_EVENT_GYRO_DETECTION` | 9 |  |
| `BOOT_EVENT_ACC_DETECTION` | 10 |  |
| `BOOT_EVENT_BARO_DETECTION` | 11 |  |
| `BOOT_EVENT_MAG_DETECTION` | 12 |  |
| `BOOT_EVENT_RANGEFINDER_DETECTION` | 13 |  |
| `BOOT_EVENT_MAG_INIT_FAILED` | 14 |  |
| `BOOT_EVENT_HMC5883L_READ_OK_COUNT` | 15 |  |
| `BOOT_EVENT_HMC5883L_READ_FAILED` | 16 |  |
| `BOOT_EVENT_HMC5883L_SATURATION` | 17 |  |
| `BOOT_EVENT_TIMER_CH_SKIPPED` | 18 |  |
| `BOOT_EVENT_TIMER_CH_MAPPED` | 19 |  |
| `BOOT_EVENT_PITOT_DETECTION` | 20 |  |
| `BOOT_EVENT_TEMP_SENSOR_DETECTION` | 21 |  |
| `BOOT_EVENT_1WIRE_DETECTION` | 22 |  |
| `BOOT_EVENT_HARDWARE_IO_CONFLICT` | 23 |  |
| `BOOT_EVENT_OPFLOW_DETECTION` | 24 |  |
| `BOOT_EVENT_CODE_COUNT` | 25 |  |

---
## <a id="enum-bootlogflags_e"></a>`bootLogFlags_e`

> Source: ../../../src/main/drivers/logging_codes.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BOOT_EVENT_FLAGS_NONE` | 0 |  |
| `BOOT_EVENT_FLAGS_WARNING` | 1 << 0 |  |
| `BOOT_EVENT_FLAGS_ERROR` | 1 << 1 |  |
| `BOOT_EVENT_FLAGS_PARAM16` | 1 << 14 |  |
| `BOOT_EVENT_FLAGS_PARAM32` | 1 << 15 |  |

---
## <a id="enum-boxid_e"></a>`boxId_e`

> Source: ../../../src/main/fc/rc_modes.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BOXARM` | 0 |  |
| `BOXANGLE` | 1 |  |
| `BOXHORIZON` | 2 |  |
| `BOXNAVALTHOLD` | 3 |  |
| `BOXHEADINGHOLD` | 4 |  |
| `BOXHEADFREE` | 5 |  |
| `BOXHEADADJ` | 6 |  |
| `BOXCAMSTAB` | 7 |  |
| `BOXNAVRTH` | 8 |  |
| `BOXNAVPOSHOLD` | 9 |  |
| `BOXMANUAL` | 10 |  |
| `BOXBEEPERON` | 11 |  |
| `BOXLEDLOW` | 12 |  |
| `BOXLIGHTS` | 13 |  |
| `BOXNAVLAUNCH` | 14 |  |
| `BOXOSD` | 15 |  |
| `BOXTELEMETRY` | 16 |  |
| `BOXBLACKBOX` | 17 |  |
| `BOXFAILSAFE` | 18 |  |
| `BOXNAVWP` | 19 |  |
| `BOXAIRMODE` | 20 |  |
| `BOXHOMERESET` | 21 |  |
| `BOXGCSNAV` | 22 |  |
| `BOXSURFACE` | 24 |  |
| `BOXFLAPERON` | 25 |  |
| `BOXTURNASSIST` | 26 |  |
| `BOXAUTOTRIM` | 27 |  |
| `BOXAUTOTUNE` | 28 |  |
| `BOXCAMERA1` | 29 |  |
| `BOXCAMERA2` | 30 |  |
| `BOXCAMERA3` | 31 |  |
| `BOXOSDALT1` | 32 |  |
| `BOXOSDALT2` | 33 |  |
| `BOXOSDALT3` | 34 |  |
| `BOXNAVCOURSEHOLD` | 35 |  |
| `BOXBRAKING` | 36 |  |
| `BOXUSER1` | 37 |  |
| `BOXUSER2` | 38 |  |
| `BOXFPVANGLEMIX` | 39 |  |
| `BOXLOITERDIRCHN` | 40 |  |
| `BOXMSPRCOVERRIDE` | 41 |  |
| `BOXPREARM` | 42 |  |
| `BOXTURTLE` | 43 |  |
| `BOXNAVCRUISE` | 44 |  |
| `BOXAUTOLEVEL` | 45 |  |
| `BOXPLANWPMISSION` | 46 |  |
| `BOXSOARING` | 47 |  |
| `BOXUSER3` | 48 |  |
| `BOXUSER4` | 49 |  |
| `BOXCHANGEMISSION` | 50 |  |
| `BOXBEEPERMUTE` | 51 |  |
| `BOXMULTIFUNCTION` | 52 |  |
| `BOXMIXERPROFILE` | 53 |  |
| `BOXMIXERTRANSITION` | 54 |  |
| `BOXANGLEHOLD` | 55 |  |
| `BOXGIMBALTLOCK` | 56 |  |
| `BOXGIMBALRLOCK` | 57 |  |
| `BOXGIMBALCENTER` | 58 |  |
| `BOXGIMBALHTRK` | 59 |  |
| `CHECKBOX_ITEM_COUNT` | 60 |  |

---
## <a id="enum-busindex_e"></a>`busIndex_e`

> Source: ../../../src/main/drivers/bus.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BUSINDEX_1` | 0 |  |
| `BUSINDEX_2` | 1 |  |
| `BUSINDEX_3` | 2 |  |
| `BUSINDEX_4` | 3 |  |

---
## <a id="enum-busspeed_e"></a>`busSpeed_e`

> Source: ../../../src/main/drivers/bus.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BUS_SPEED_INITIALIZATION` | 0 |  |
| `BUS_SPEED_SLOW` | 1 |  |
| `BUS_SPEED_STANDARD` | 2 |  |
| `BUS_SPEED_FAST` | 3 |  |
| `BUS_SPEED_ULTRAFAST` | 4 |  |

---
## <a id="enum-bustype_e"></a>`busType_e`

> Source: ../../../src/main/drivers/bus.h

| Enumerator | Value | Condition |
|---|---:|---|
| `BUSTYPE_ANY` | 0 |  |
| `BUSTYPE_NONE` | 0 |  |
| `BUSTYPE_I2C` | 1 |  |
| `BUSTYPE_SPI` | 2 |  |
| `BUSTYPE_SDIO` | 3 |  |

---
## <a id="enum-channeltype_t"></a>`channelType_t`

> Source: ../../../src/main/drivers/timer.h

| Enumerator | Value | Condition |
|---|---:|---|
| `TYPE_FREE` | 0 |  |
| `TYPE_PWMINPUT` | 1 |  |
| `TYPE_PPMINPUT` | 2 |  |
| `TYPE_PWMOUTPUT_MOTOR` | 3 |  |
| `TYPE_PWMOUTPUT_FAST` | 4 |  |
| `TYPE_PWMOUTPUT_SERVO` | 5 |  |
| `TYPE_SOFTSERIAL_RX` | 6 |  |
| `TYPE_SOFTSERIAL_TX` | 7 |  |
| `TYPE_SOFTSERIAL_RXTX` | 8 |  |
| `TYPE_SOFTSERIAL_AUXTIMER` | 9 |  |
| `TYPE_ADC` | 10 |  |
| `TYPE_SERIAL_RX` | 11 |  |
| `TYPE_SERIAL_TX` | 12 |  |
| `TYPE_SERIAL_RXTX` | 13 |  |
| `TYPE_TIMER` | 14 |  |

---
## <a id="enum-climbratetoaltitudecontrollermode_e"></a>`climbRateToAltitudeControllerMode_e`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ROC_TO_ALT_CURRENT` | 0 |  |
| `ROC_TO_ALT_CONSTANT` | 1 |  |
| `ROC_TO_ALT_TARGET` | 2 |  |

---
## <a id="enum-colorcomponent_e"></a>`colorComponent_e`

> Source: ../../../src/main/common/color.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RGB_RED` | 0 |  |
| `RGB_GREEN` | 1 |  |
| `RGB_BLUE` | 2 |  |

---
## <a id="enum-colorid_e"></a>`colorId_e`

> Source: ../../../src/main/io/ledstrip.h

| Enumerator | Value | Condition |
|---|---:|---|
| `COLOR_BLACK` | 0 |  |
| `COLOR_WHITE` | 1 |  |
| `COLOR_RED` | 2 |  |
| `COLOR_ORANGE` | 3 |  |
| `COLOR_YELLOW` | 4 |  |
| `COLOR_LIME_GREEN` | 5 |  |
| `COLOR_GREEN` | 6 |  |
| `COLOR_MINT_GREEN` | 7 |  |
| `COLOR_CYAN` | 8 |  |
| `COLOR_LIGHT_BLUE` | 9 |  |
| `COLOR_BLUE` | 10 |  |
| `COLOR_DARK_VIOLET` | 11 |  |
| `COLOR_MAGENTA` | 12 |  |
| `COLOR_DEEP_PINK` | 13 |  |

---
## <a id="enum-crsfactiveantenna_e"></a>`crsfActiveAntenna_e`

> Source: ../../../src/main/telemetry/crsf.c

| Enumerator | Value | Condition |
|---|---:|---|
| `CRSF_ACTIVE_ANTENNA1` | 0 |  |
| `CRSF_ACTIVE_ANTENNA2` | 1 |  |

---
## <a id="enum-crsfaddress_e"></a>`crsfAddress_e`

> Source: ../../../src/main/rx/crsf.h

| Enumerator | Value | Condition |
|---|---:|---|
| `CRSF_ADDRESS_BROADCAST` | 0 |  |
| `CRSF_ADDRESS_USB` | 16 |  |
| `CRSF_ADDRESS_TBS_CORE_PNP_PRO` | 128 |  |
| `CRSF_ADDRESS_RESERVED1` | 138 |  |
| `CRSF_ADDRESS_CURRENT_SENSOR` | 192 |  |
| `CRSF_ADDRESS_GPS` | 194 |  |
| `CRSF_ADDRESS_TBS_BLACKBOX` | 196 |  |
| `CRSF_ADDRESS_FLIGHT_CONTROLLER` | 200 |  |
| `CRSF_ADDRESS_RESERVED2` | 202 |  |
| `CRSF_ADDRESS_RACE_TAG` | 204 |  |
| `CRSF_ADDRESS_RADIO_TRANSMITTER` | 234 |  |
| `CRSF_ADDRESS_CRSF_RECEIVER` | 236 |  |
| `CRSF_ADDRESS_CRSF_TRANSMITTER` | 238 |  |

---
## <a id="enum-crsfframetype_e"></a>`crsfFrameType_e`

> Source: ../../../src/main/rx/crsf.h

| Enumerator | Value | Condition |
|---|---:|---|
| `CRSF_FRAMETYPE_GPS` | 2 |  |
| `CRSF_FRAMETYPE_VARIO_SENSOR` | 7 |  |
| `CRSF_FRAMETYPE_BATTERY_SENSOR` | 8 |  |
| `CRSF_FRAMETYPE_BAROMETER_ALTITUDE` | 9 |  |
| `CRSF_FRAMETYPE_LINK_STATISTICS` | 20 |  |
| `CRSF_FRAMETYPE_RC_CHANNELS_PACKED` | 22 |  |
| `CRSF_FRAMETYPE_ATTITUDE` | 30 |  |
| `CRSF_FRAMETYPE_FLIGHT_MODE` | 33 |  |
| `CRSF_FRAMETYPE_DEVICE_PING` | 40 |  |
| `CRSF_FRAMETYPE_DEVICE_INFO` | 41 |  |
| `CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY` | 43 |  |
| `CRSF_FRAMETYPE_PARAMETER_READ` | 44 |  |
| `CRSF_FRAMETYPE_PARAMETER_WRITE` | 45 |  |
| `CRSF_FRAMETYPE_COMMAND` | 50 |  |
| `CRSF_FRAMETYPE_MSP_REQ` | 122 |  |
| `CRSF_FRAMETYPE_MSP_RESP` | 123 |  |
| `CRSF_FRAMETYPE_MSP_WRITE` | 124 |  |
| `CRSF_FRAMETYPE_DISPLAYPORT_CMD` | 125 |  |

---
## <a id="enum-crsfframetypeindex_e"></a>`crsfFrameTypeIndex_e`

> Source: ../../../src/main/telemetry/crsf.c

| Enumerator | Value | Condition |
|---|---:|---|
| `CRSF_FRAME_START_INDEX` | 0 |  |
| `CRSF_FRAME_ATTITUDE_INDEX` | CRSF_FRAME_START_INDEX |  |
| `CRSF_FRAME_BATTERY_SENSOR_INDEX` |  |  |
| `CRSF_FRAME_FLIGHT_MODE_INDEX` |  |  |
| `CRSF_FRAME_GPS_INDEX` |  |  |
| `CRSF_FRAME_VARIO_SENSOR_INDEX` |  |  |
| `CRSF_FRAME_BAROMETER_ALTITUDE_INDEX` |  |  |
| `CRSF_SCHEDULE_COUNT_MAX` |  |  |

---
## <a id="enum-crsrrfmode_e"></a>`crsrRfMode_e`

> Source: ../../../src/main/telemetry/crsf.c

| Enumerator | Value | Condition |
|---|---:|---|
| `CRSF_RF_MODE_4_HZ` | 0 |  |
| `CRSF_RF_MODE_50_HZ` | 1 |  |
| `CRSF_RF_MODE_150_HZ` | 2 |  |

---
## <a id="enum-crsrrfpower_e"></a>`crsrRfPower_e`

> Source: ../../../src/main/telemetry/crsf.c

| Enumerator | Value | Condition |
|---|---:|---|
| `CRSF_RF_POWER_0_mW` | 0 |  |
| `CRSF_RF_POWER_10_mW` | 1 |  |
| `CRSF_RF_POWER_25_mW` | 2 |  |
| `CRSF_RF_POWER_100_mW` | 3 |  |
| `CRSF_RF_POWER_500_mW` | 4 |  |
| `CRSF_RF_POWER_1000_mW` | 5 |  |
| `CRSF_RF_POWER_2000_mW` | 6 |  |
| `CRSF_RF_POWER_250_mW` | 7 |  |

---
## <a id="enum-currentsensor_e"></a>`currentSensor_e`

> Source: ../../../src/main/sensors/battery_config_structs.h

| Enumerator | Value | Condition |
|---|---:|---|
| `CURRENT_SENSOR_NONE` | 0 |  |
| `CURRENT_SENSOR_ADC` | 1 |  |
| `CURRENT_SENSOR_VIRTUAL` | 2 |  |
| `CURRENT_SENSOR_FAKE` | 3 |  |
| `CURRENT_SENSOR_ESC` | 4 |  |
| `CURRENT_SENSOR_SMARTPORT` | 5 |  |
| `CURRENT_SENSOR_MAX` | CURRENT_SENSOR_SMARTPORT |  |

---
## <a id="enum-devhardwaretype_e"></a>`devHardwareType_e`

> Source: ../../../src/main/drivers/bus.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DEVHW_NONE` | 0 |  |
| `DEVHW_MPU6000` | 1 |  |
| `DEVHW_MPU6500` | 2 |  |
| `DEVHW_BMI160` | 3 |  |
| `DEVHW_BMI088_GYRO` | 4 |  |
| `DEVHW_BMI088_ACC` | 5 |  |
| `DEVHW_ICM20689` | 6 |  |
| `DEVHW_ICM42605` | 7 |  |
| `DEVHW_BMI270` | 8 |  |
| `DEVHW_LSM6D` | 9 |  |
| `DEVHW_MPU9250` | 10 |  |
| `DEVHW_BMP085` | 11 |  |
| `DEVHW_BMP280` | 12 |  |
| `DEVHW_MS5611` | 13 |  |
| `DEVHW_MS5607` | 14 |  |
| `DEVHW_LPS25H` | 15 |  |
| `DEVHW_SPL06` | 16 |  |
| `DEVHW_BMP388` | 17 |  |
| `DEVHW_DPS310` | 18 |  |
| `DEVHW_B2SMPB` | 19 |  |
| `DEVHW_HMC5883` | 20 |  |
| `DEVHW_AK8963` | 21 |  |
| `DEVHW_AK8975` | 22 |  |
| `DEVHW_IST8310_0` | 23 |  |
| `DEVHW_IST8310_1` | 24 |  |
| `DEVHW_IST8308` | 25 |  |
| `DEVHW_QMC5883` | 26 |  |
| `DEVHW_QMC5883P` | 27 |  |
| `DEVHW_MAG3110` | 28 |  |
| `DEVHW_LIS3MDL` | 29 |  |
| `DEVHW_RM3100` | 30 |  |
| `DEVHW_VCM5883` | 31 |  |
| `DEVHW_MLX90393` | 32 |  |
| `DEVHW_LM75_0` | 33 |  |
| `DEVHW_LM75_1` | 34 |  |
| `DEVHW_LM75_2` | 35 |  |
| `DEVHW_LM75_3` | 36 |  |
| `DEVHW_LM75_4` | 37 |  |
| `DEVHW_LM75_5` | 38 |  |
| `DEVHW_LM75_6` | 39 |  |
| `DEVHW_LM75_7` | 40 |  |
| `DEVHW_DS2482` | 41 |  |
| `DEVHW_MAX7456` | 42 |  |
| `DEVHW_SRF10` | 43 |  |
| `DEVHW_VL53L0X` | 44 |  |
| `DEVHW_VL53L1X` | 45 |  |
| `DEVHW_US42` | 46 |  |
| `DEVHW_TOF10120_I2C` | 47 |  |
| `DEVHW_TERARANGER_EVO_I2C` | 48 |  |
| `DEVHW_MS4525` | 49 |  |
| `DEVHW_DLVR` | 50 |  |
| `DEVHW_M25P16` | 51 |  |
| `DEVHW_W25N01G` | 52 |  |
| `DEVHW_UG2864` | 53 |  |
| `DEVHW_SDCARD` | 54 |  |
| `DEVHW_IRLOCK` | 55 |  |
| `DEVHW_PCF8574` | 56 |  |

---
## <a id="enum-deviceflags_e"></a>`deviceFlags_e`

> Source: ../../../src/main/drivers/bus.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DEVFLAGS_NONE` | 0 |  |
| `DEVFLAGS_USE_RAW_REGISTERS` | (1 << 0) |  |
| `DEVFLAGS_USE_MANUAL_DEVICE_SELECT` | (1 << 1) |  |
| `DEVFLAGS_SPI_MODE_0` | (1 << 2) |  |

---
## <a id="enum-displaycanvasbitmapoption_t"></a>`displayCanvasBitmapOption_t`

> Source: ../../../src/main/drivers/display_canvas.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DISPLAY_CANVAS_BITMAP_OPT_INVERT_COLORS` | 1 << 0 |  |
| `DISPLAY_CANVAS_BITMAP_OPT_SOLID_BACKGROUND` | 1 << 1 |  |
| `DISPLAY_CANVAS_BITMAP_OPT_ERASE_TRANSPARENT` | 1 << 2 |  |

---
## <a id="enum-displaycanvascolor_e"></a>`displayCanvasColor_e`

> Source: ../../../src/main/drivers/display_canvas.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DISPLAY_CANVAS_COLOR_BLACK` | 0 |  |
| `DISPLAY_CANVAS_COLOR_TRANSPARENT` | 1 |  |
| `DISPLAY_CANVAS_COLOR_WHITE` | 2 |  |
| `DISPLAY_CANVAS_COLOR_GRAY` | 3 |  |

---
## <a id="enum-displaycanvasoutlinetype_e"></a>`displayCanvasOutlineType_e`

> Source: ../../../src/main/drivers/display_canvas.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DISPLAY_CANVAS_OUTLINE_TYPE_NONE` | 0 |  |
| `DISPLAY_CANVAS_OUTLINE_TYPE_TOP` | 1 << 0 |  |
| `DISPLAY_CANVAS_OUTLINE_TYPE_RIGHT` | 1 << 1 |  |
| `DISPLAY_CANVAS_OUTLINE_TYPE_BOTTOM` | 1 << 2 |  |
| `DISPLAY_CANVAS_OUTLINE_TYPE_LEFT` | 1 << 3 |  |

---
## <a id="enum-displayportmspcommand_e"></a>`displayportMspCommand_e`

> Source: ../../../src/main/io/displayport_msp.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MSP_DP_HEARTBEAT` | 0 |  |
| `MSP_DP_RELEASE` | 1 |  |
| `MSP_DP_CLEAR_SCREEN` | 2 |  |
| `MSP_DP_WRITE_STRING` | 3 |  |
| `MSP_DP_DRAW_SCREEN` | 4 |  |
| `MSP_DP_OPTIONS` | 5 |  |
| `MSP_DP_SYS` | 6 |  |
| `MSP_DP_COUNT` | 7 |  |

---
## <a id="enum-displaytransactionoption_e"></a>`displayTransactionOption_e`

> Source: ../../../src/main/drivers/display.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DISPLAY_TRANSACTION_OPT_NONE` | 0 |  |
| `DISPLAY_TRANSACTION_OPT_PROFILED` | 1 << 0 |  |
| `DISPLAY_TRANSACTION_OPT_RESET_DRAWING` | 1 << 1 |  |

---
## <a id="enum-displaywidgettype_e"></a>`displayWidgetType_e`

> Source: ../../../src/main/drivers/display_widgets.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DISPLAY_WIDGET_TYPE_AHI` | 0 |  |
| `DISPLAY_WIDGET_TYPE_SIDEBAR` | 1 |  |

---
## <a id="enum-djicraftnameelements_t"></a>`DjiCraftNameElements_t`

> Source: ../../../src/main/io/osd_dji_hd.c

| Enumerator | Value | Condition |
|---|---:|---|
| `DJI_OSD_CN_MESSAGES` | 0 |  |
| `DJI_OSD_CN_THROTTLE` | 1 |  |
| `DJI_OSD_CN_THROTTLE_AUTO_THR` | 2 |  |
| `DJI_OSD_CN_AIR_SPEED` | 3 |  |
| `DJI_OSD_CN_EFFICIENCY` | 4 |  |
| `DJI_OSD_CN_DISTANCE` | 5 |  |
| `DJI_OSD_CN_ADJUSTEMNTS` | 6 |  |
| `DJI_OSD_CN_MAX_ELEMENTS` | 7 |  |

---
## <a id="enum-dshotcommands_e"></a>`dshotCommands_e`

> Source: ../../../src/main/drivers/pwm_output.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DSHOT_CMD_SPIN_DIRECTION_NORMAL` | 20 |  |
| `DSHOT_CMD_SPIN_DIRECTION_REVERSED` | 21 |  |

---
## <a id="enum-dumpflags_e"></a>`dumpFlags_e`

> Source: ../../../src/main/fc/cli.c

| Enumerator | Value | Condition |
|---|---:|---|
| `DUMP_MASTER` | (1 << 0) |  |
| `DUMP_CONTROL_PROFILE` | (1 << 1) |  |
| `DUMP_BATTERY_PROFILE` | (1 << 2) |  |
| `DUMP_MIXER_PROFILE` | (1 << 3) |  |
| `DUMP_ALL` | (1 << 4) |  |
| `DO_DIFF` | (1 << 5) |  |
| `SHOW_DEFAULTS` | (1 << 6) |  |
| `HIDE_UNUSED` | (1 << 7) |  |

---
## <a id="enum-dynamicgyronotchmode_e"></a>`dynamicGyroNotchMode_e`

> Source: ../../../src/main/sensors/gyro.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DYNAMIC_NOTCH_MODE_2D` | 0 |  |
| `DYNAMIC_NOTCH_MODE_3D` | 1 |  |

---
## <a id="enum-emerglandstate_e"></a>`emergLandState_e`

> Source: ../../../src/main/flight/failsafe.h

| Enumerator | Value | Condition |
|---|---:|---|
| `EMERG_LAND_IDLE` | 0 |  |
| `EMERG_LAND_IN_PROGRESS` | 1 |  |
| `EMERG_LAND_HAS_LANDED` | 2 |  |

---
## <a id="enum-escsensorframestatus_t"></a>`escSensorFrameStatus_t`

> Source: ../../../src/main/sensors/esc_sensor.c

| Enumerator | Value | Condition |
|---|---:|---|
| `ESC_SENSOR_FRAME_PENDING` | 0 |  |
| `ESC_SENSOR_FRAME_COMPLETE` | 1 |  |
| `ESC_SENSOR_FRAME_FAILED` | 2 |  |

---
## <a id="enum-escsensorstate_t"></a>`escSensorState_t`

> Source: ../../../src/main/sensors/esc_sensor.c

| Enumerator | Value | Condition |
|---|---:|---|
| `ESC_SENSOR_WAIT_STARTUP` | 0 |  |
| `ESC_SENSOR_READY` | 1 |  |
| `ESC_SENSOR_WAITING` | 2 |  |

---
## <a id="enum-failsafechannelbehavior_e"></a>`failsafeChannelBehavior_e`

> Source: ../../../src/main/flight/failsafe.c

| Enumerator | Value | Condition |
|---|---:|---|
| `FAILSAFE_CHANNEL_HOLD` | 0 |  |
| `FAILSAFE_CHANNEL_NEUTRAL` | 1 |  |

---
## <a id="enum-failsafephase_e"></a>`failsafePhase_e`

> Source: ../../../src/main/flight/failsafe.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FAILSAFE_IDLE` | 0 |  |
| `FAILSAFE_RX_LOSS_DETECTED` | 1 |  |
| `FAILSAFE_RX_LOSS_IDLE` | 2 |  |
| `FAILSAFE_RETURN_TO_HOME` | 3 |  |
| `FAILSAFE_LANDING` | 4 |  |
| `FAILSAFE_LANDED` | 5 |  |
| `FAILSAFE_RX_LOSS_MONITORING` | 6 |  |
| `FAILSAFE_RX_LOSS_RECOVERED` | 7 |  |

---
## <a id="enum-failsafeprocedure_e"></a>`failsafeProcedure_e`

> Source: ../../../src/main/flight/failsafe.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FAILSAFE_PROCEDURE_AUTO_LANDING` | 0 |  |
| `FAILSAFE_PROCEDURE_DROP_IT` | 1 |  |
| `FAILSAFE_PROCEDURE_RTH` | 2 |  |
| `FAILSAFE_PROCEDURE_NONE` | 3 |  |

---
## <a id="enum-failsaferxlinkstate_e"></a>`failsafeRxLinkState_e`

> Source: ../../../src/main/flight/failsafe.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FAILSAFE_RXLINK_DOWN` | 0 |  |
| `FAILSAFE_RXLINK_UP` | 1 |  |

---
## <a id="enum-failuremode_e"></a>`failureMode_e`

> Source: ../../../src/main/drivers/system.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FAILURE_DEVELOPER` | 0 |  |
| `FAILURE_MISSING_ACC` | 1 |  |
| `FAILURE_ACC_INIT` | 2 |  |
| `FAILURE_ACC_INCOMPATIBLE` | 3 |  |
| `FAILURE_INVALID_EEPROM_CONTENTS` | 4 |  |
| `FAILURE_FLASH_WRITE_FAILED` | 5 |  |
| `FAILURE_GYRO_INIT_FAILED` | 6 |  |
| `FAILURE_FLASH_READ_FAILED` | 7 |  |

---
## <a id="enum-fatfilesystemtype_e"></a>`fatFilesystemType_e`

> Source: ../../../src/main/io/asyncfatfs/fat_standard.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FAT_FILESYSTEM_TYPE_INVALID` | 0 |  |
| `FAT_FILESYSTEM_TYPE_FAT12` | 1 |  |
| `FAT_FILESYSTEM_TYPE_FAT16` | 2 |  |
| `FAT_FILESYSTEM_TYPE_FAT32` | 3 |  |

---
## <a id="enum-features_e"></a>`features_e`

> Source: ../../../src/main/fc/config.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FEATURE_THR_VBAT_COMP` | 1 << 0 |  |
| `FEATURE_VBAT` | 1 << 1 |  |
| `FEATURE_TX_PROF_SEL` | 1 << 2 |  |
| `FEATURE_BAT_PROFILE_AUTOSWITCH` | 1 << 3 |  |
| `FEATURE_GEOZONE` | 1 << 4 |  |
| `FEATURE_UNUSED_1` | 1 << 5 |  |
| `FEATURE_SOFTSERIAL` | 1 << 6 |  |
| `FEATURE_GPS` | 1 << 7 |  |
| `FEATURE_UNUSED_3` | 1 << 8 |  |
| `FEATURE_UNUSED_4` | 1 << 9 |  |
| `FEATURE_TELEMETRY` | 1 << 10 |  |
| `FEATURE_CURRENT_METER` | 1 << 11 |  |
| `FEATURE_REVERSIBLE_MOTORS` | 1 << 12 |  |
| `FEATURE_UNUSED_5` | 1 << 13 |  |
| `FEATURE_UNUSED_6` | 1 << 14 |  |
| `FEATURE_RSSI_ADC` | 1 << 15 |  |
| `FEATURE_LED_STRIP` | 1 << 16 |  |
| `FEATURE_DASHBOARD` | 1 << 17 |  |
| `FEATURE_UNUSED_7` | 1 << 18 |  |
| `FEATURE_BLACKBOX` | 1 << 19 |  |
| `FEATURE_UNUSED_10` | 1 << 20 |  |
| `FEATURE_TRANSPONDER` | 1 << 21 |  |
| `FEATURE_AIRMODE` | 1 << 22 |  |
| `FEATURE_SUPEREXPO_RATES` | 1 << 23 |  |
| `FEATURE_VTX` | 1 << 24 |  |
| `FEATURE_UNUSED_8` | 1 << 25 |  |
| `FEATURE_UNUSED_9` | 1 << 26 |  |
| `FEATURE_UNUSED_11` | 1 << 27 |  |
| `FEATURE_PWM_OUTPUT_ENABLE` | 1 << 28 |  |
| `FEATURE_OSD` | 1 << 29 |  |
| `FEATURE_FW_LAUNCH` | 1 << 30 |  |
| `FEATURE_FW_AUTOTRIM` | 1 << 31 |  |

---
## <a id="enum-filtertype_e"></a>`filterType_e`

> Source: ../../../src/main/common/filter.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FILTER_PT1` | 0 |  |
| `FILTER_BIQUAD` | 1 |  |
| `FILTER_PT2` | 2 |  |
| `FILTER_PT3` | 3 |  |
| `FILTER_LULU` | 4 |  |

---
## <a id="enum-fixedwinglaunchevent_t"></a>`fixedWingLaunchEvent_t`

> Source: ../../../src/main/navigation/navigation_fw_launch.c

| Enumerator | Value | Condition |
|---|---:|---|
| `FW_LAUNCH_EVENT_NONE` | 0 |  |
| `FW_LAUNCH_EVENT_SUCCESS` | 1 |  |
| `FW_LAUNCH_EVENT_GOTO_DETECTION` | 2 |  |
| `FW_LAUNCH_EVENT_ABORT` | 3 |  |
| `FW_LAUNCH_EVENT_THROTTLE_LOW` | 4 |  |
| `FW_LAUNCH_EVENT_COUNT` | 5 |  |

---
## <a id="enum-fixedwinglaunchmessage_t"></a>`fixedWingLaunchMessage_t`

> Source: ../../../src/main/navigation/navigation_fw_launch.c

| Enumerator | Value | Condition |
|---|---:|---|
| `FW_LAUNCH_MESSAGE_TYPE_NONE` | 0 |  |
| `FW_LAUNCH_MESSAGE_TYPE_WAIT_THROTTLE` | 1 |  |
| `FW_LAUNCH_MESSAGE_TYPE_WAIT_IDLE` | 2 |  |
| `FW_LAUNCH_MESSAGE_TYPE_WAIT_DETECTION` | 3 |  |
| `FW_LAUNCH_MESSAGE_TYPE_IN_PROGRESS` | 4 |  |
| `FW_LAUNCH_MESSAGE_TYPE_FINISHING` | 5 |  |

---
## <a id="enum-fixedwinglaunchstate_t"></a>`fixedWingLaunchState_t`

> Source: ../../../src/main/navigation/navigation_fw_launch.c

| Enumerator | Value | Condition |
|---|---:|---|
| `FW_LAUNCH_STATE_WAIT_THROTTLE` | 0 |  |
| `FW_LAUNCH_STATE_IDLE_WIGGLE_WAIT` | 1 |  |
| `FW_LAUNCH_STATE_IDLE_MOTOR_DELAY` | 2 |  |
| `FW_LAUNCH_STATE_MOTOR_IDLE` | 3 |  |
| `FW_LAUNCH_STATE_WAIT_DETECTION` | 4 |  |
| `FW_LAUNCH_STATE_DETECTED` | 5 |  |
| `FW_LAUNCH_STATE_MOTOR_DELAY` | 6 |  |
| `FW_LAUNCH_STATE_MOTOR_SPINUP` | 7 |  |
| `FW_LAUNCH_STATE_IN_PROGRESS` | 8 |  |
| `FW_LAUNCH_STATE_FINISH` | 9 |  |
| `FW_LAUNCH_STATE_ABORTED` | 10 |  |
| `FW_LAUNCH_STATE_FLYING` | 11 |  |
| `FW_LAUNCH_STATE_COUNT` | 12 |  |

---
## <a id="enum-flashpartitiontype_e"></a>`flashPartitionType_e`

> Source: ../../../src/main/drivers/flash.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FLASH_PARTITION_TYPE_UNKNOWN` | 0 |  |
| `FLASH_PARTITION_TYPE_PARTITION_TABLE` | 1 |  |
| `FLASH_PARTITION_TYPE_FLASHFS` | 2 |  |
| `FLASH_PARTITION_TYPE_BADBLOCK_MANAGEMENT` | 3 |  |
| `FLASH_PARTITION_TYPE_FIRMWARE` | 4 |  |
| `FLASH_PARTITION_TYPE_CONFIG` | 5 |  |
| `FLASH_PARTITION_TYPE_FULL_BACKUP` | 6 |  |
| `FLASH_PARTITION_TYPE_FIRMWARE_UPDATE_META` | 7 |  |
| `FLASH_PARTITION_TYPE_UPDATE_FIRMWARE` | 8 |  |
| `FLASH_MAX_PARTITIONS` | 9 |  |

---
## <a id="enum-flashtype_e"></a>`flashType_e`

> Source: ../../../src/main/drivers/flash.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FLASH_TYPE_NOR` | 0 |  |
| `FLASH_TYPE_NAND` | 1 |  |

---
## <a id="enum-flight_dynamics_index_t"></a>`flight_dynamics_index_t`

> Source: ../../../src/main/common/axis.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FD_ROLL` | 0 |  |
| `FD_PITCH` | 1 |  |
| `FD_YAW` | 2 |  |

---
## <a id="enum-flightmodeflags_e"></a>`flightModeFlags_e`

> Source: ../../../src/main/fc/runtime_config.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ANGLE_MODE` | (1 << 0) |  |
| `HORIZON_MODE` | (1 << 1) |  |
| `HEADING_MODE` | (1 << 2) |  |
| `NAV_ALTHOLD_MODE` | (1 << 3) |  |
| `NAV_RTH_MODE` | (1 << 4) |  |
| `NAV_POSHOLD_MODE` | (1 << 5) |  |
| `HEADFREE_MODE` | (1 << 6) |  |
| `NAV_LAUNCH_MODE` | (1 << 7) |  |
| `MANUAL_MODE` | (1 << 8) |  |
| `FAILSAFE_MODE` | (1 << 9) |  |
| `AUTO_TUNE` | (1 << 10) |  |
| `NAV_WP_MODE` | (1 << 11) |  |
| `NAV_COURSE_HOLD_MODE` | (1 << 12) |  |
| `FLAPERON` | (1 << 13) |  |
| `TURN_ASSISTANT` | (1 << 14) |  |
| `TURTLE_MODE` | (1 << 15) |  |
| `SOARING_MODE` | (1 << 16) |  |
| `ANGLEHOLD_MODE` | (1 << 17) |  |
| `NAV_FW_AUTOLAND` | (1 << 18) |  |
| `NAV_SEND_TO` | (1 << 19) |  |

---
## <a id="enum-flightmodefortelemetry_e"></a>`flightModeForTelemetry_e`

> Source: ../../../src/main/fc/runtime_config.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FLM_MANUAL` | 0 |  |
| `FLM_ACRO` | 1 |  |
| `FLM_ACRO_AIR` | 2 |  |
| `FLM_ANGLE` | 3 |  |
| `FLM_HORIZON` | 4 |  |
| `FLM_ALTITUDE_HOLD` | 5 |  |
| `FLM_POSITION_HOLD` | 6 |  |
| `FLM_RTH` | 7 |  |
| `FLM_MISSION` | 8 |  |
| `FLM_COURSE_HOLD` | 9 |  |
| `FLM_CRUISE` | 10 |  |
| `FLM_LAUNCH` | 11 |  |
| `FLM_FAILSAFE` | 12 |  |
| `FLM_ANGLEHOLD` | 13 |  |
| `FLM_COUNT` | 14 |  |

---
## <a id="enum-flyingplatformtype_e"></a>`flyingPlatformType_e`

> Source: ../../../src/main/flight/mixer.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PLATFORM_MULTIROTOR` | 0 |  |
| `PLATFORM_AIRPLANE` | 1 |  |
| `PLATFORM_HELICOPTER` | 2 |  |
| `PLATFORM_TRICOPTER` | 3 |  |
| `PLATFORM_ROVER` | 4 |  |
| `PLATFORM_BOAT` | 5 |  |

---
## <a id="enum-fport2_control_frame_type_e"></a>`fport2_control_frame_type_e`

> Source: ../../../src/main/rx/fport2.c

| Enumerator | Value | Condition |
|---|---:|---|
| `CFT_RC` | 255 |  |
| `CFT_OTA_START` | 240 |  |
| `CFT_OTA_DATA` | 241 |  |
| `CFT_OTA_STOP` | 242 |  |

---
## <a id="enum-frame_state_e"></a>`frame_state_e`

> Source: ../../../src/main/rx/fport2.c

| Enumerator | Value | Condition |
|---|---:|---|
| `FS_CONTROL_FRAME_START` | 0 |  |
| `FS_CONTROL_FRAME_TYPE` | 1 |  |
| `FS_CONTROL_FRAME_DATA` | 2 |  |
| `FS_DOWNLINK_FRAME_START` | 3 |  |
| `FS_DOWNLINK_FRAME_DATA` | 4 |  |

---
## <a id="enum-frame_type_e"></a>`frame_type_e`

> Source: ../../../src/main/rx/fport2.c

| Enumerator | Value | Condition |
|---|---:|---|
| `FT_CONTROL` | 0 |  |
| `FT_DOWNLINK` | 1 |  |

---
## <a id="enum-frskyosdcolor_e"></a>`frskyOSDColor_e`

> Source: ../../../src/main/io/frsky_osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FRSKY_OSD_COLOR_BLACK` | 0 |  |
| `FRSKY_OSD_COLOR_TRANSPARENT` | 1 |  |
| `FRSKY_OSD_COLOR_WHITE` | 2 |  |
| `FRSKY_OSD_COLOR_GRAY` | 3 |  |

---
## <a id="enum-frskyosdlineoutlinetype_e"></a>`frskyOSDLineOutlineType_e`

> Source: ../../../src/main/io/frsky_osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FRSKY_OSD_OUTLINE_TYPE_NONE` | 0 |  |
| `FRSKY_OSD_OUTLINE_TYPE_TOP` | 1 << 0 |  |
| `FRSKY_OSD_OUTLINE_TYPE_RIGHT` | 1 << 1 |  |
| `FRSKY_OSD_OUTLINE_TYPE_BOTTOM` | 1 << 2 |  |
| `FRSKY_OSD_OUTLINE_TYPE_LEFT` | 1 << 3 |  |

---
## <a id="enum-frskyosdrecvstate_e"></a>`frskyOSDRecvState_e`

> Source: ../../../src/main/io/frsky_osd.c

| Enumerator | Value | Condition |
|---|---:|---|
| `RECV_STATE_NONE` | 0 |  |
| `RECV_STATE_SYNC` | 1 |  |
| `RECV_STATE_LENGTH` | 2 |  |
| `RECV_STATE_DATA` | 3 |  |
| `RECV_STATE_CHECKSUM` | 4 |  |
| `RECV_STATE_DONE` | 5 |  |

---
## <a id="enum-frskyosdtransactionoptions_e"></a>`frskyOSDTransactionOptions_e`

> Source: ../../../src/main/io/frsky_osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FRSKY_OSD_TRANSACTION_OPT_PROFILED` | 1 << 0 |  |
| `FRSKY_OSD_TRANSACTION_OPT_RESET_DRAWING` | 1 << 1 |  |

---
## <a id="enum-fw_autotune_rate_adjustment_e"></a>`fw_autotune_rate_adjustment_e`

> Source: ../../../src/main/flight/pid.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FIXED` | 0 |  |
| `LIMIT` | 1 |  |
| `AUTO` | 2 |  |

---
## <a id="enum-fwautolandapproachdirection_e"></a>`fwAutolandApproachDirection_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FW_AUTOLAND_APPROACH_DIRECTION_LEFT` | 0 |  |
| `FW_AUTOLAND_APPROACH_DIRECTION_RIGHT` | 1 |  |

---
## <a id="enum-fwautolandstate_t"></a>`fwAutolandState_t`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FW_AUTOLAND_STATE_IDLE` | 0 |  |
| `FW_AUTOLAND_STATE_LOITER` | 1 |  |
| `FW_AUTOLAND_STATE_DOWNWIND` | 2 |  |
| `FW_AUTOLAND_STATE_BASE_LEG` | 3 |  |
| `FW_AUTOLAND_STATE_FINAL_APPROACH` | 4 |  |
| `FW_AUTOLAND_STATE_GLIDE` | 5 |  |
| `FW_AUTOLAND_STATE_FLARE` | 6 |  |

---
## <a id="enum-fwautolandwaypoint_t"></a>`fwAutolandWaypoint_t`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FW_AUTOLAND_WP_TURN` | 0 |  |
| `FW_AUTOLAND_WP_FINAL_APPROACH` | 1 |  |
| `FW_AUTOLAND_WP_LAND` | 2 |  |
| `FW_AUTOLAND_WP_COUNT` | 3 |  |

---
## <a id="enum-geoaltitudeconversionmode_e"></a>`geoAltitudeConversionMode_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GEO_ALT_ABSOLUTE` | 0 |  |
| `GEO_ALT_RELATIVE` | 1 |  |

---
## <a id="enum-geoaltitudedatumflag_e"></a>`geoAltitudeDatumFlag_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_WP_TAKEOFF_DATUM` | 0 |  |
| `NAV_WP_MSL_DATUM` | 1 |  |

---
## <a id="enum-geooriginresetmode_e"></a>`geoOriginResetMode_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GEO_ORIGIN_SET` | 0 |  |
| `GEO_ORIGIN_RESET_ALTITUDE` | 1 |  |

---
## <a id="enum-geozoneactionstate_e"></a>`geozoneActionState_e`

> Source: ../../../src/main/navigation/navigation_geozone.c

| Enumerator | Value | Condition |
|---|---:|---|
| `GEOZONE_ACTION_STATE_NONE` | 0 |  |
| `GEOZONE_ACTION_STATE_AVOIDING` | 1 |  |
| `GEOZONE_ACTION_STATE_AVOIDING_UPWARD` | 2 |  |
| `GEOZONE_ACTION_STATE_AVOIDING_ALTITUDE` | 3 |  |
| `GEOZONE_ACTION_STATE_RETURN_TO_FZ` | 4 |  |
| `GEOZONE_ACTION_STATE_FLYOUT_NFZ` | 5 |  |
| `GEOZONE_ACTION_STATE_POSHOLD` | 6 |  |
| `GEOZONE_ACTION_STATE_RTH` | 7 |  |

---
## <a id="enum-geozonemessagestate_e"></a>`geozoneMessageState_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GEOZONE_MESSAGE_STATE_NONE` | 0 |  |
| `GEOZONE_MESSAGE_STATE_NFZ` | 1 |  |
| `GEOZONE_MESSAGE_STATE_LEAVING_FZ` | 2 |  |
| `GEOZONE_MESSAGE_STATE_OUTSIDE_FZ` | 3 |  |
| `GEOZONE_MESSAGE_STATE_ENTERING_NFZ` | 4 |  |
| `GEOZONE_MESSAGE_STATE_AVOIDING_FB` | 5 |  |
| `GEOZONE_MESSAGE_STATE_RETURN_TO_ZONE` | 6 |  |
| `GEOZONE_MESSAGE_STATE_FLYOUT_NFZ` | 7 |  |
| `GEOZONE_MESSAGE_STATE_AVOIDING_ALTITUDE_BREACH` | 8 |  |
| `GEOZONE_MESSAGE_STATE_POS_HOLD` | 9 |  |

---
## <a id="enum-ghstaddr_e"></a>`ghstAddr_e`

> Source: ../../../src/main/rx/ghst_protocol.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GHST_ADDR_RADIO` | 128 |  |
| `GHST_ADDR_TX_MODULE_SYM` | 129 |  |
| `GHST_ADDR_TX_MODULE_ASYM` | 136 |  |
| `GHST_ADDR_FC` | 130 |  |
| `GHST_ADDR_GOGGLES` | 131 |  |
| `GHST_ADDR_QUANTUM_TEE1` | 132 |  |
| `GHST_ADDR_QUANTUM_TEE2` | 133 |  |
| `GHST_ADDR_QUANTUM_GW1` | 134 |  |
| `GHST_ADDR_5G_CLK` | 135 |  |
| `GHST_ADDR_RX` | 137 |  |

---
## <a id="enum-ghstdl_e"></a>`ghstDl_e`

> Source: ../../../src/main/rx/ghst_protocol.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GHST_DL_OPENTX_SYNC` | 32 |  |
| `GHST_DL_LINK_STAT` | 33 |  |
| `GHST_DL_VTX_STAT` | 34 |  |
| `GHST_DL_PACK_STAT` | 35 |  |
| `GHST_DL_GPS_PRIMARY` | 37 |  |
| `GHST_DL_GPS_SECONDARY` | 38 |  |

---
## <a id="enum-ghstframetypeindex_e"></a>`ghstFrameTypeIndex_e`

> Source: ../../../src/main/telemetry/ghst.c

| Enumerator | Value | Condition |
|---|---:|---|
| `GHST_FRAME_START_INDEX` | 0 |  |
| `GHST_FRAME_PACK_INDEX` | GHST_FRAME_START_INDEX |  |
| `GHST_FRAME_GPS_PRIMARY_INDEX` |  |  |
| `GHST_FRAME_GPS_SECONDARY_INDEX` |  |  |
| `GHST_SCHEDULE_COUNT_MAX` |  |  |

---
## <a id="enum-ghstul_e"></a>`ghstUl_e`

> Source: ../../../src/main/rx/ghst_protocol.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GHST_UL_RC_CHANS_HS4_FIRST` | 16 |  |
| `GHST_UL_RC_CHANS_HS4_5TO8` | 16 |  |
| `GHST_UL_RC_CHANS_HS4_9TO12` | 17 |  |
| `GHST_UL_RC_CHANS_HS4_13TO16` | 18 |  |
| `GHST_UL_RC_CHANS_HS4_RSSI` | 19 |  |
| `GHST_UL_RC_CHANS_HS4_LAST` | 31 |  |

---
## <a id="enum-gimbal_htk_mode_e"></a>`gimbal_htk_mode_e`

> Source: ../../../src/main/drivers/gimbal_common.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GIMBAL_MODE_FOLLOW` | 0 |  |
| `GIMBAL_MODE_TILT_LOCK` | (1<<0) |  |
| `GIMBAL_MODE_ROLL_LOCK` | (1<<1) |  |
| `GIMBAL_MODE_PAN_LOCK` | (1<<2) |  |

---
## <a id="enum-gimbaldevtype_e"></a>`gimbalDevType_e`

> Source: ../../../src/main/drivers/gimbal_common.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GIMBAL_DEV_UNSUPPORTED` | 0 |  |
| `GIMBAL_DEV_SERIAL` | 1 |  |
| `GIMBAL_DEV_UNKNOWN` | 255 |  |

---
## <a id="enum-gimbalheadtrackerstate_e"></a>`gimbalHeadtrackerState_e`

> Source: ../../../src/main/io/gimbal_serial.h

| Enumerator | Value | Condition |
|---|---:|---|
| `WAITING_HDR1` | 0 |  |
| `WAITING_HDR2` | 1 |  |
| `WAITING_PAYLOAD` | 2 |  |
| `WAITING_CRCH` | 3 |  |
| `WAITING_CRCL` | 4 |  |

---
## <a id="enum-gpsautobaud_e"></a>`gpsAutoBaud_e`

> Source: ../../../src/main/io/gps.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GPS_AUTOBAUD_OFF` | 0 |  |
| `GPS_AUTOBAUD_ON` | 1 |  |

---
## <a id="enum-gpsautoconfig_e"></a>`gpsAutoConfig_e`

> Source: ../../../src/main/io/gps.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GPS_AUTOCONFIG_OFF` | 0 |  |
| `GPS_AUTOCONFIG_ON` | 1 |  |

---
## <a id="enum-gpsbaudrate_e"></a>`gpsBaudRate_e`

> Source: ../../../src/main/io/gps.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GPS_BAUDRATE_115200` | 0 |  |
| `GPS_BAUDRATE_57600` | 1 |  |
| `GPS_BAUDRATE_38400` | 2 |  |
| `GPS_BAUDRATE_19200` | 3 |  |
| `GPS_BAUDRATE_9600` | 4 |  |
| `GPS_BAUDRATE_230400` | 5 |  |
| `GPS_BAUDRATE_460800` | 6 |  |
| `GPS_BAUDRATE_921600` | 7 |  |
| `GPS_BAUDRATE_COUNT` | 8 |  |

---
## <a id="enum-gpsdynmodel_e"></a>`gpsDynModel_e`

> Source: ../../../src/main/io/gps.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GPS_DYNMODEL_PEDESTRIAN` | 0 |  |
| `GPS_DYNMODEL_AUTOMOTIVE` | 1 |  |
| `GPS_DYNMODEL_AIR_1G` | 2 |  |
| `GPS_DYNMODEL_AIR_2G` | 3 |  |
| `GPS_DYNMODEL_AIR_4G` | 4 |  |
| `GPS_DYNMODEL_SEA` | 5 |  |
| `GPS_DYNMODEL_MOWER` | 6 |  |

---
## <a id="enum-gpsfixchar_e"></a>`gpsFixChar_e`

> Source: ../../../src/main/telemetry/hott.c

| Enumerator | Value | Condition |
|---|---:|---|
| `GPS_FIX_CHAR_NONE` | '-' |  |
| `GPS_FIX_CHAR_2D` | '2' |  |
| `GPS_FIX_CHAR_3D` | '3' |  |
| `GPS_FIX_CHAR_DGPS` | 'D' |  |

---
## <a id="enum-gpsfixtype_e"></a>`gpsFixType_e`

> Source: ../../../src/main/io/gps.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GPS_NO_FIX` | 0 |  |
| `GPS_FIX_2D` | 1 |  |
| `GPS_FIX_3D` | 2 |  |

---
## <a id="enum-gpsprovider_e"></a>`gpsProvider_e`

> Source: ../../../src/main/io/gps.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GPS_UBLOX` | 0 |  |
| `GPS_MSP` | 1 |  |
| `GPS_FAKE` | 2 |  |
| `GPS_PROVIDER_COUNT` | 3 |  |

---
## <a id="enum-gpsstate_e"></a>`gpsState_e`

> Source: ../../../src/main/io/gps_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GPS_UNKNOWN` | 0 |  |
| `GPS_INITIALIZING` | 1 |  |
| `GPS_RUNNING` | 2 |  |
| `GPS_LOST_COMMUNICATION` | 3 |  |

---
## <a id="enum-gyrofiltermode_e"></a>`gyroFilterMode_e`

> Source: ../../../src/main/sensors/gyro.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GYRO_FILTER_MODE_OFF` | 0 |  |
| `GYRO_FILTER_MODE_STATIC` | 1 |  |
| `GYRO_FILTER_MODE_DYNAMIC` | 2 |  |
| `GYRO_FILTER_MODE_ADAPTIVE` | 3 |  |

---
## <a id="enum-gyrohardwarelpf_e"></a>`gyroHardwareLpf_e`

> Source: ../../../src/main/drivers/accgyro/accgyro_lsm6dxx.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GYRO_HARDWARE_LPF_NORMAL` | 0 |  |
| `GYRO_HARDWARE_LPF_OPTION_1` | 1 |  |
| `GYRO_HARDWARE_LPF_OPTION_2` | 2 |  |
| `GYRO_HARDWARE_LPF_EXPERIMENTAL` | 3 |  |
| `GYRO_HARDWARE_LPF_COUNT` | 4 |  |

---
## <a id="enum-gyrosensor_e"></a>`gyroSensor_e`

> Source: ../../../src/main/sensors/gyro.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GYRO_NONE` | 0 |  |
| `GYRO_AUTODETECT` | 1 |  |
| `GYRO_MPU6000` | 2 |  |
| `GYRO_MPU6500` | 3 |  |
| `GYRO_MPU9250` | 4 |  |
| `GYRO_BMI160` | 5 |  |
| `GYRO_ICM20689` | 6 |  |
| `GYRO_BMI088` | 7 |  |
| `GYRO_ICM42605` | 8 |  |
| `GYRO_BMI270` | 9 |  |
| `GYRO_LSM6DXX` | 10 |  |
| `GYRO_FAKE` | 11 |  |

---
## <a id="enum-hardwaremotortypes_e"></a>`HardwareMotorTypes_e`

> Source: ../../../src/main/drivers/pwm_esc_detect.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MOTOR_UNKNOWN` | 0 |  |
| `MOTOR_BRUSHED` | 1 |  |
| `MOTOR_BRUSHLESS` | 2 |  |

---
## <a id="enum-hardwaresensorstatus_e"></a>`hardwareSensorStatus_e`

> Source: ../../../src/main/sensors/diagnostics.h

| Enumerator | Value | Condition |
|---|---:|---|
| `HW_SENSOR_NONE` | 0 |  |
| `HW_SENSOR_OK` | 1 |  |
| `HW_SENSOR_UNAVAILABLE` | 2 |  |
| `HW_SENSOR_UNHEALTHY` | 3 |  |

---
## <a id="enum-headtrackerdevtype_e"></a>`headTrackerDevType_e`

> Source: ../../../src/main/drivers/headtracker_common.h

| Enumerator | Value | Condition |
|---|---:|---|
| `HEADTRACKER_NONE` | 0 |  |
| `HEADTRACKER_SERIAL` | 1 |  |
| `HEADTRACKER_MSP` | 2 |  |
| `HEADTRACKER_UNKNOWN` | 255 |  |

---
## <a id="enum-hotteamalarm1flag_e"></a>`hottEamAlarm1Flag_e`

> Source: ../../../src/main/telemetry/hott.h

| Enumerator | Value | Condition |
|---|---:|---|
| `HOTT_EAM_ALARM1_FLAG_NONE` | 0 |  |
| `HOTT_EAM_ALARM1_FLAG_MAH` | (1 << 0) |  |
| `HOTT_EAM_ALARM1_FLAG_BATTERY_1` | (1 << 1) |  |
| `HOTT_EAM_ALARM1_FLAG_BATTERY_2` | (1 << 2) |  |
| `HOTT_EAM_ALARM1_FLAG_TEMPERATURE_1` | (1 << 3) |  |
| `HOTT_EAM_ALARM1_FLAG_TEMPERATURE_2` | (1 << 4) |  |
| `HOTT_EAM_ALARM1_FLAG_ALTITUDE` | (1 << 5) |  |
| `HOTT_EAM_ALARM1_FLAG_CURRENT` | (1 << 6) |  |
| `HOTT_EAM_ALARM1_FLAG_MAIN_VOLTAGE` | (1 << 7) |  |

---
## <a id="enum-hotteamalarm2flag_e"></a>`hottEamAlarm2Flag_e`

> Source: ../../../src/main/telemetry/hott.h

| Enumerator | Value | Condition |
|---|---:|---|
| `HOTT_EAM_ALARM2_FLAG_NONE` | 0 |  |
| `HOTT_EAM_ALARM2_FLAG_MS` | (1 << 0) |  |
| `HOTT_EAM_ALARM2_FLAG_M3S` | (1 << 1) |  |
| `HOTT_EAM_ALARM2_FLAG_ALTITUDE_DUPLICATE` | (1 << 2) |  |
| `HOTT_EAM_ALARM2_FLAG_MS_DUPLICATE` | (1 << 3) |  |
| `HOTT_EAM_ALARM2_FLAG_M3S_DUPLICATE` | (1 << 4) |  |
| `HOTT_EAM_ALARM2_FLAG_UNKNOWN_1` | (1 << 5) |  |
| `HOTT_EAM_ALARM2_FLAG_UNKNOWN_2` | (1 << 6) |  |
| `HOTT_EAM_ALARM2_FLAG_ON_SIGN_OR_TEXT_ACTIVE` | (1 << 7) |  |

---
## <a id="enum-hottstate_e"></a>`hottState_e`

> Source: ../../../src/main/telemetry/hott.c

| Enumerator | Value | Condition |
|---|---:|---|
| `HOTT_WAITING_FOR_REQUEST` | 0 |  |
| `HOTT_RECEIVING_REQUEST` | 1 |  |
| `HOTT_WAITING_FOR_TX_WINDOW` | 2 |  |
| `HOTT_TRANSMITTING` | 3 |  |
| `HOTT_ENDING_TRANSMISSION` | 4 |  |

---
## <a id="enum-hsvcolorcomponent_e"></a>`hsvColorComponent_e`

> Source: ../../../src/main/common/color.h

| Enumerator | Value | Condition |
|---|---:|---|
| `HSV_HUE` | 0 |  |
| `HSV_SATURATION` | 1 |  |
| `HSV_VALUE` | 2 |  |

---
## <a id="enum-i2cspeed"></a>`I2CSpeed`

> Source: ../../../src/main/drivers/bus_i2c.h

| Enumerator | Value | Condition |
|---|---:|---|
| `I2C_SPEED_100KHZ` | 2 |  |
| `I2C_SPEED_200KHZ` | 3 |  |
| `I2C_SPEED_400KHZ` | 0 |  |
| `I2C_SPEED_800KHZ` | 1 |  |

---
## <a id="enum-i2cstate_t"></a>`i2cState_t`

> Source: ../../../src/main/drivers/bus_i2c_stm32f40x.c

| Enumerator | Value | Condition |
|---|---:|---|
| `I2C_STATE_STOPPED` | 0 |  |
| `I2C_STATE_STOPPING` | 1 |  |
| `I2C_STATE_STARTING` | 2 |  |
| `I2C_STATE_STARTING_WAIT` | 3 |  |
| `I2C_STATE_R_ADDR` | 4 |  |
| `I2C_STATE_R_ADDR_WAIT` | 5 |  |
| `I2C_STATE_R_REGISTER` | 6 |  |
| `I2C_STATE_R_REGISTER_WAIT` | 7 |  |
| `I2C_STATE_R_RESTARTING` | 8 |  |
| `I2C_STATE_R_RESTARTING_WAIT` | 9 |  |
| `I2C_STATE_R_RESTART_ADDR` | 10 |  |
| `I2C_STATE_R_RESTART_ADDR_WAIT` | 11 |  |
| `I2C_STATE_R_TRANSFER_EQ1` | 12 |  |
| `I2C_STATE_R_TRANSFER_EQ2` | 13 |  |
| `I2C_STATE_R_TRANSFER_GE2` | 14 |  |
| `I2C_STATE_W_ADDR` | 15 |  |
| `I2C_STATE_W_ADDR_WAIT` | 16 |  |
| `I2C_STATE_W_REGISTER` | 17 |  |
| `I2C_STATE_W_TRANSFER_WAIT` | 18 |  |
| `I2C_STATE_W_TRANSFER` | 19 |  |
| `I2C_STATE_NACK` | 20 |  |
| `I2C_STATE_BUS_ERROR` | 21 |  |

---
## <a id="enum-i2ctransferdirection_t"></a>`i2cTransferDirection_t`

> Source: ../../../src/main/drivers/bus_i2c_stm32f40x.c

| Enumerator | Value | Condition |
|---|---:|---|
| `I2C_TXN_READ` | 0 |  |
| `I2C_TXN_WRITE` | 1 |  |

---
## <a id="enum-ibuscommand_e"></a>`ibusCommand_e`

> Source: ../../../src/main/telemetry/ibus_shared.c

| Enumerator | Value | Condition |
|---|---:|---|
| `IBUS_COMMAND_DISCOVER_SENSOR` | 128 |  |
| `IBUS_COMMAND_SENSOR_TYPE` | 144 |  |
| `IBUS_COMMAND_MEASUREMENT` | 160 |  |

---
## <a id="enum-ibussensortype1_e"></a>`ibusSensorType1_e`

> Source: ../../../src/main/telemetry/ibus_shared.h

| Enumerator | Value | Condition |
|---|---:|---|
| `IBUS_MEAS_TYPE1_INTV` | 0 |  |
| `IBUS_MEAS_TYPE1_TEM` | 1 |  |
| `IBUS_MEAS_TYPE1_MOT` | 2 |  |
| `IBUS_MEAS_TYPE1_EXTV` | 3 |  |
| `IBUS_MEAS_TYPE1_CELL` | 4 |  |
| `IBUS_MEAS_TYPE1_BAT_CURR` | 5 |  |
| `IBUS_MEAS_TYPE1_FUEL` | 6 |  |
| `IBUS_MEAS_TYPE1_RPM` | 7 |  |
| `IBUS_MEAS_TYPE1_CMP_HEAD` | 8 |  |
| `IBUS_MEAS_TYPE1_CLIMB_RATE` | 9 |  |
| `IBUS_MEAS_TYPE1_COG` | 10 |  |
| `IBUS_MEAS_TYPE1_GPS_STATUS` | 11 |  |
| `IBUS_MEAS_TYPE1_ACC_X` | 12 |  |
| `IBUS_MEAS_TYPE1_ACC_Y` | 13 |  |
| `IBUS_MEAS_TYPE1_ACC_Z` | 14 |  |
| `IBUS_MEAS_TYPE1_ROLL` | 15 |  |
| `IBUS_MEAS_TYPE1_PITCH` | 16 |  |
| `IBUS_MEAS_TYPE1_YAW` | 17 |  |
| `IBUS_MEAS_TYPE1_VERTICAL_SPEED` | 18 |  |
| `IBUS_MEAS_TYPE1_GROUND_SPEED` | 19 |  |
| `IBUS_MEAS_TYPE1_GPS_DIST` | 20 |  |
| `IBUS_MEAS_TYPE1_ARMED` | 21 |  |
| `IBUS_MEAS_TYPE1_FLIGHT_MODE` | 22 |  |
| `IBUS_MEAS_TYPE1_PRES` | 65 |  |
| `IBUS_MEAS_TYPE1_SPE` | 126 |  |
| `IBUS_MEAS_TYPE1_GPS_LAT` | 128 |  |
| `IBUS_MEAS_TYPE1_GPS_LON` | 129 |  |
| `IBUS_MEAS_TYPE1_GPS_ALT` | 130 |  |
| `IBUS_MEAS_TYPE1_ALT` | 131 |  |
| `IBUS_MEAS_TYPE1_S84` | 132 |  |
| `IBUS_MEAS_TYPE1_S85` | 133 |  |
| `IBUS_MEAS_TYPE1_S86` | 134 |  |
| `IBUS_MEAS_TYPE1_S87` | 135 |  |
| `IBUS_MEAS_TYPE1_S88` | 136 |  |
| `IBUS_MEAS_TYPE1_S89` | 137 |  |
| `IBUS_MEAS_TYPE1_S8a` | 138 |  |

---
## <a id="enum-ibussensortype_e"></a>`ibusSensorType_e`

> Source: ../../../src/main/telemetry/ibus_shared.h

| Enumerator | Value | Condition |
|---|---:|---|
| `IBUS_MEAS_TYPE_INTERNAL_VOLTAGE` | 0 |  |
| `IBUS_MEAS_TYPE_TEMPERATURE` | 1 |  |
| `IBUS_MEAS_TYPE_RPM` | 2 |  |
| `IBUS_MEAS_TYPE_EXTERNAL_VOLTAGE` | 3 |  |
| `IBUS_MEAS_TYPE_HEADING` | 4 |  |
| `IBUS_MEAS_TYPE_CURRENT` | 5 |  |
| `IBUS_MEAS_TYPE_CLIMB` | 6 |  |
| `IBUS_MEAS_TYPE_ACC_Z` | 7 |  |
| `IBUS_MEAS_TYPE_ACC_Y` | 8 |  |
| `IBUS_MEAS_TYPE_ACC_X` | 9 |  |
| `IBUS_MEAS_TYPE_VSPEED` | 10 |  |
| `IBUS_MEAS_TYPE_SPEED` | 11 |  |
| `IBUS_MEAS_TYPE_DIST` | 12 |  |
| `IBUS_MEAS_TYPE_ARMED` | 13 |  |
| `IBUS_MEAS_TYPE_MODE` | 14 |  |
| `IBUS_MEAS_TYPE_PRES` | 65 |  |
| `IBUS_MEAS_TYPE_SPE` | 126 |  |
| `IBUS_MEAS_TYPE_COG` | 128 |  |
| `IBUS_MEAS_TYPE_GPS_STATUS` | 129 |  |
| `IBUS_MEAS_TYPE_GPS_LON` | 130 |  |
| `IBUS_MEAS_TYPE_GPS_LAT` | 131 |  |
| `IBUS_MEAS_TYPE_ALT` | 132 |  |
| `IBUS_MEAS_TYPE_S85` | 133 |  |
| `IBUS_MEAS_TYPE_S86` | 134 |  |
| `IBUS_MEAS_TYPE_S87` | 135 |  |
| `IBUS_MEAS_TYPE_S88` | 136 |  |
| `IBUS_MEAS_TYPE_S89` | 137 |  |
| `IBUS_MEAS_TYPE_S8A` | 138 |  |
| `IBUS_MEAS_TYPE_GALT` | 249 |  |
| `IBUS_MEAS_TYPE_GPS` | 253 |  |

---
## <a id="enum-ibussensorvalue_e"></a>`ibusSensorValue_e`

> Source: ../../../src/main/telemetry/ibus_shared.h

| Enumerator | Value | Condition |
|---|---:|---|
| `IBUS_MEAS_VALUE_NONE` | 0 |  |
| `IBUS_MEAS_VALUE_TEMPERATURE` | 1 |  |
| `IBUS_MEAS_VALUE_MOT` | 2 |  |
| `IBUS_MEAS_VALUE_EXTERNAL_VOLTAGE` | 3 |  |
| `IBUS_MEAS_VALUE_CELL` | 4 |  |
| `IBUS_MEAS_VALUE_CURRENT` | 5 |  |
| `IBUS_MEAS_VALUE_FUEL` | 6 |  |
| `IBUS_MEAS_VALUE_RPM` | 7 |  |
| `IBUS_MEAS_VALUE_HEADING` | 8 |  |
| `IBUS_MEAS_VALUE_CLIMB` | 9 |  |
| `IBUS_MEAS_VALUE_COG` | 10 |  |
| `IBUS_MEAS_VALUE_GPS_STATUS` | 11 |  |
| `IBUS_MEAS_VALUE_ACC_X` | 12 |  |
| `IBUS_MEAS_VALUE_ACC_Y` | 13 |  |
| `IBUS_MEAS_VALUE_ACC_Z` | 14 |  |
| `IBUS_MEAS_VALUE_ROLL` | 15 |  |
| `IBUS_MEAS_VALUE_PITCH` | 16 |  |
| `IBUS_MEAS_VALUE_YAW` | 17 |  |
| `IBUS_MEAS_VALUE_VSPEED` | 18 |  |
| `IBUS_MEAS_VALUE_SPEED` | 19 |  |
| `IBUS_MEAS_VALUE_DIST` | 20 |  |
| `IBUS_MEAS_VALUE_ARMED` | 21 |  |
| `IBUS_MEAS_VALUE_MODE` | 22 |  |
| `IBUS_MEAS_VALUE_PRES` | 65 |  |
| `IBUS_MEAS_VALUE_SPE` | 126 |  |
| `IBUS_MEAS_VALUE_GPS_LAT` | 128 |  |
| `IBUS_MEAS_VALUE_GPS_LON` | 129 |  |
| `IBUS_MEAS_VALUE_GALT4` | 130 |  |
| `IBUS_MEAS_VALUE_ALT4` | 131 |  |
| `IBUS_MEAS_VALUE_GALT` | 132 |  |
| `IBUS_MEAS_VALUE_ALT` | 133 |  |
| `IBUS_MEAS_VALUE_STATUS` | 135 |  |
| `IBUS_MEAS_VALUE_GPS_LAT1` | 136 |  |
| `IBUS_MEAS_VALUE_GPS_LON1` | 137 |  |
| `IBUS_MEAS_VALUE_GPS_LAT2` | 144 |  |
| `IBUS_MEAS_VALUE_GPS_LON2` | 145 |  |
| `IBUS_MEAS_VALUE_GPS` | 253 |  |

---
## <a id="enum-inputsource_e"></a>`inputSource_e`

> Source: ../../../src/main/flight/servos.h

| Enumerator | Value | Condition |
|---|---:|---|
| `INPUT_STABILIZED_ROLL` | 0 |  |
| `INPUT_STABILIZED_PITCH` | 1 |  |
| `INPUT_STABILIZED_YAW` | 2 |  |
| `INPUT_STABILIZED_THROTTLE` | 3 |  |
| `INPUT_RC_ROLL` | 4 |  |
| `INPUT_RC_PITCH` | 5 |  |
| `INPUT_RC_YAW` | 6 |  |
| `INPUT_RC_THROTTLE` | 7 |  |
| `INPUT_RC_CH5` | 8 |  |
| `INPUT_RC_CH6` | 9 |  |
| `INPUT_RC_CH7` | 10 |  |
| `INPUT_RC_CH8` | 11 |  |
| `INPUT_GIMBAL_PITCH` | 12 |  |
| `INPUT_GIMBAL_ROLL` | 13 |  |
| `INPUT_FEATURE_FLAPS` | 14 |  |
| `INPUT_RC_CH9` | 15 |  |
| `INPUT_RC_CH10` | 16 |  |
| `INPUT_RC_CH11` | 17 |  |
| `INPUT_RC_CH12` | 18 |  |
| `INPUT_RC_CH13` | 19 |  |
| `INPUT_RC_CH14` | 20 |  |
| `INPUT_RC_CH15` | 21 |  |
| `INPUT_RC_CH16` | 22 |  |
| `INPUT_STABILIZED_ROLL_PLUS` | 23 |  |
| `INPUT_STABILIZED_ROLL_MINUS` | 24 |  |
| `INPUT_STABILIZED_PITCH_PLUS` | 25 |  |
| `INPUT_STABILIZED_PITCH_MINUS` | 26 |  |
| `INPUT_STABILIZED_YAW_PLUS` | 27 |  |
| `INPUT_STABILIZED_YAW_MINUS` | 28 |  |
| `INPUT_MAX` | 29 |  |
| `INPUT_GVAR_0` | 30 |  |
| `INPUT_GVAR_1` | 31 |  |
| `INPUT_GVAR_2` | 32 |  |
| `INPUT_GVAR_3` | 33 |  |
| `INPUT_GVAR_4` | 34 |  |
| `INPUT_GVAR_5` | 35 |  |
| `INPUT_GVAR_6` | 36 |  |
| `INPUT_GVAR_7` | 37 |  |
| `INPUT_MIXER_TRANSITION` | 38 |  |
| `INPUT_HEADTRACKER_PAN` | 39 |  |
| `INPUT_HEADTRACKER_TILT` | 40 |  |
| `INPUT_HEADTRACKER_ROLL` | 41 |  |
| `INPUT_RC_CH17` | 42 |  |
| `INPUT_RC_CH18` | 43 |  |
| `INPUT_RC_CH19` | 44 |  |
| `INPUT_RC_CH20` | 45 |  |
| `INPUT_RC_CH21` | 46 |  |
| `INPUT_RC_CH22` | 47 |  |
| `INPUT_RC_CH23` | 48 |  |
| `INPUT_RC_CH24` | 49 |  |
| `INPUT_RC_CH25` | 50 |  |
| `INPUT_RC_CH26` | 51 |  |
| `INPUT_RC_CH27` | 52 |  |
| `INPUT_RC_CH28` | 53 |  |
| `INPUT_RC_CH29` | 54 |  |
| `INPUT_RC_CH30` | 55 |  |
| `INPUT_RC_CH31` | 56 |  |
| `INPUT_RC_CH32` | 57 |  |
| `INPUT_RC_CH33` | 58 |  |
| `INPUT_RC_CH34` | 59 |  |
| `INPUT_MIXER_SWITCH_HELPER` | 60 |  |
| `INPUT_SOURCE_COUNT` | 61 |  |

---
## <a id="enum-itermrelax_e"></a>`itermRelax_e`

> Source: ../../../src/main/flight/pid.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ITERM_RELAX_OFF` | 0 |  |
| `ITERM_RELAX_RP` | 1 |  |
| `ITERM_RELAX_RPY` | 2 |  |

---
## <a id="enum-led_pin_pwm_mode_e"></a>`led_pin_pwm_mode_e`

> Source: ../../../src/main/drivers/light_ws2811strip.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LED_PIN_PWM_MODE_SHARED_LOW` | 0 |  |
| `LED_PIN_PWM_MODE_SHARED_HIGH` | 1 |  |
| `LED_PIN_PWM_MODE_LOW` | 2 |  |
| `LED_PIN_PWM_MODE_HIGH` | 3 |  |

---
## <a id="enum-ledbasefunctionid_e"></a>`ledBaseFunctionId_e`

> Source: ../../../src/main/io/ledstrip.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LED_FUNCTION_COLOR` | 0 |  |
| `LED_FUNCTION_FLIGHT_MODE` | 1 |  |
| `LED_FUNCTION_ARM_STATE` | 2 |  |
| `LED_FUNCTION_BATTERY` | 3 |  |
| `LED_FUNCTION_RSSI` | 4 |  |
| `LED_FUNCTION_GPS` | 5 |  |
| `LED_FUNCTION_THRUST_RING` | 6 |  |
| `LED_FUNCTION_CHANNEL` | 7 |  |

---
## <a id="enum-leddirectionid_e"></a>`ledDirectionId_e`

> Source: ../../../src/main/io/ledstrip.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LED_DIRECTION_NORTH` | 0 |  |
| `LED_DIRECTION_EAST` | 1 |  |
| `LED_DIRECTION_SOUTH` | 2 |  |
| `LED_DIRECTION_WEST` | 3 |  |
| `LED_DIRECTION_UP` | 4 |  |
| `LED_DIRECTION_DOWN` | 5 |  |

---
## <a id="enum-ledmodeindex_e"></a>`ledModeIndex_e`

> Source: ../../../src/main/io/ledstrip.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LED_MODE_ORIENTATION` | 0 |  |
| `LED_MODE_HEADFREE` | 1 |  |
| `LED_MODE_HORIZON` | 2 |  |
| `LED_MODE_ANGLE` | 3 |  |
| `LED_MODE_MAG` | 4 |  |
| `LED_MODE_BARO` | 5 |  |
| `LED_SPECIAL` | 6 |  |

---
## <a id="enum-ledoverlayid_e"></a>`ledOverlayId_e`

> Source: ../../../src/main/io/ledstrip.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LED_OVERLAY_THROTTLE` | 0 |  |
| `LED_OVERLAY_LARSON_SCANNER` | 1 |  |
| `LED_OVERLAY_BLINK` | 2 |  |
| `LED_OVERLAY_LANDING_FLASH` | 3 |  |
| `LED_OVERLAY_INDICATOR` | 4 |  |
| `LED_OVERLAY_WARNING` | 5 |  |
| `LED_OVERLAY_STROBE` | 6 |  |

---
## <a id="enum-ledspecialcolorids_e"></a>`ledSpecialColorIds_e`

> Source: ../../../src/main/io/ledstrip.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LED_SCOLOR_DISARMED` | 0 |  |
| `LED_SCOLOR_ARMED` | 1 |  |
| `LED_SCOLOR_ANIMATION` | 2 |  |
| `LED_SCOLOR_BACKGROUND` | 3 |  |
| `LED_SCOLOR_BLINKBACKGROUND` | 4 |  |
| `LED_SCOLOR_GPSNOSATS` | 5 |  |
| `LED_SCOLOR_GPSNOLOCK` | 6 |  |
| `LED_SCOLOR_GPSLOCKED` | 7 |  |
| `LED_SCOLOR_STROBE` | 8 |  |

---
## <a id="enum-logicconditionflags_e"></a>`logicConditionFlags_e`

> Source: ../../../src/main/programming/logic_condition.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LOGIC_CONDITION_FLAG_LATCH` | 1 << 0 |  |
| `LOGIC_CONDITION_FLAG_TIMEOUT_SATISFIED` | 1 << 1 |  |

---
## <a id="enum-logicconditionsglobalflags_t"></a>`logicConditionsGlobalFlags_t`

> Source: ../../../src/main/programming/logic_condition.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_ARMING_SAFETY` | (1 << 0) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE_SCALE` | (1 << 1) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW` | (1 << 2) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL` | (1 << 3) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_PITCH` | (1 << 4) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_YAW` | (1 << 5) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE` | (1 << 6) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_OSD_LAYOUT` | (1 << 7) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_RC_CHANNEL` | (1 << 8) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_LOITER_RADIUS` | (1 << 9) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_FLIGHT_AXIS` | (1 << 10) |  |
| `LOGIC_CONDITION_GLOBAL_FLAG_DISABLE_GPS_FIX` | (1 << 11) | USE_GPS_FIX_ESTIMATION |
| `LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_MIN_GROUND_SPEED` | (1 << 12) |  |

---
## <a id="enum-logicflightmodeoperands_e"></a>`logicFlightModeOperands_e`

> Source: ../../../src/main/programming/logic_condition.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_FAILSAFE` | 0 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_MANUAL` | 1 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_RTH` | 2 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_POSHOLD` | 3 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_CRUISE` | 4 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ALTHOLD` | 5 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ANGLE` | 6 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_HORIZON` | 7 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_AIR` | 8 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER1` | 9 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER2` | 10 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_COURSE_HOLD` | 11 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER3` | 12 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER4` | 13 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ACRO` | 14 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_WAYPOINT_MISSION` | 15 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ANGLEHOLD` | 16 |  |

---
## <a id="enum-logicflightoperands_e"></a>`logicFlightOperands_e`

> Source: ../../../src/main/programming/logic_condition.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LOGIC_CONDITION_OPERAND_FLIGHT_ARM_TIMER` | 0 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_HOME_DISTANCE` | 1 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_TRIP_DISTANCE` | 2 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_RSSI` | 3 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_VBAT` | 4 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_CELL_VOLTAGE` | 5 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_CURRENT` | 6 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MAH_DRAWN` | 7 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_GPS_SATS` | 8 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_GROUD_SPEED` | 9 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_3D_SPEED` | 10 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_AIR_SPEED` | 11 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_ALTITUDE` | 12 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_VERTICAL_SPEED` | 13 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_TROTTLE_POS` | 14 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_ROLL` | 15 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_PITCH` | 16 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_IS_ARMED` | 17 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_IS_AUTOLAUNCH` | 18 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_IS_ALTITUDE_CONTROL` | 19 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_IS_POSITION_CONTROL` | 20 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_IS_EMERGENCY_LANDING` | 21 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_IS_RTH` | 22 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_IS_LANDING` | 23 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_IS_FAILSAFE` | 24 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_ROLL` | 25 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_PITCH` | 26 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_YAW` | 27 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_3D_HOME_DISTANCE` | 28 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_LQ_UPLINK` | 29 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_SNR` | 30 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_GPS_VALID` | 31 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_LOITER_RADIUS` | 32 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_ACTIVE_PROFILE` | 33 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_BATT_CELLS` | 34 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_AGL_STATUS` | 35 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_AGL` | 36 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_RANGEFINDER_RAW` | 37 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_ACTIVE_MIXER_PROFILE` | 38 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MIXER_TRANSITION_ACTIVE` | 39 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_YAW` | 40 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_FW_LAND_STATE` | 41 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_BATT_PROFILE` | 42 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_FLOWN_LOITER_RADIUS` | 43 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_LQ_DOWNLINK` | 44 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_UPLINK_RSSI_DBM` | 45 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_MIN_GROUND_SPEED` | 46 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_HORIZONTAL_WIND_SPEED` | 47 |  |
| `LOGIC_CONDITION_OPERAND_FLIGHT_WIND_DIRECTION` | 48 |  |

---
## <a id="enum-logicoperation_e"></a>`logicOperation_e`

> Source: ../../../src/main/programming/logic_condition.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LOGIC_CONDITION_TRUE` | 0 |  |
| `LOGIC_CONDITION_EQUAL` | 1 |  |
| `LOGIC_CONDITION_GREATER_THAN` | 2 |  |
| `LOGIC_CONDITION_LOWER_THAN` | 3 |  |
| `LOGIC_CONDITION_LOW` | 4 |  |
| `LOGIC_CONDITION_MID` | 5 |  |
| `LOGIC_CONDITION_HIGH` | 6 |  |
| `LOGIC_CONDITION_AND` | 7 |  |
| `LOGIC_CONDITION_OR` | 8 |  |
| `LOGIC_CONDITION_XOR` | 9 |  |
| `LOGIC_CONDITION_NAND` | 10 |  |
| `LOGIC_CONDITION_NOR` | 11 |  |
| `LOGIC_CONDITION_NOT` | 12 |  |
| `LOGIC_CONDITION_STICKY` | 13 |  |
| `LOGIC_CONDITION_ADD` | 14 |  |
| `LOGIC_CONDITION_SUB` | 15 |  |
| `LOGIC_CONDITION_MUL` | 16 |  |
| `LOGIC_CONDITION_DIV` | 17 |  |
| `LOGIC_CONDITION_GVAR_SET` | 18 |  |
| `LOGIC_CONDITION_GVAR_INC` | 19 |  |
| `LOGIC_CONDITION_GVAR_DEC` | 20 |  |
| `LOGIC_CONDITION_PORT_SET` | 21 |  |
| `LOGIC_CONDITION_OVERRIDE_ARMING_SAFETY` | 22 |  |
| `LOGIC_CONDITION_OVERRIDE_THROTTLE_SCALE` | 23 |  |
| `LOGIC_CONDITION_SWAP_ROLL_YAW` | 24 |  |
| `LOGIC_CONDITION_SET_VTX_POWER_LEVEL` | 25 |  |
| `LOGIC_CONDITION_INVERT_ROLL` | 26 |  |
| `LOGIC_CONDITION_INVERT_PITCH` | 27 |  |
| `LOGIC_CONDITION_INVERT_YAW` | 28 |  |
| `LOGIC_CONDITION_OVERRIDE_THROTTLE` | 29 |  |
| `LOGIC_CONDITION_SET_VTX_BAND` | 30 |  |
| `LOGIC_CONDITION_SET_VTX_CHANNEL` | 31 |  |
| `LOGIC_CONDITION_SET_OSD_LAYOUT` | 32 |  |
| `LOGIC_CONDITION_SIN` | 33 |  |
| `LOGIC_CONDITION_COS` | 34 |  |
| `LOGIC_CONDITION_TAN` | 35 |  |
| `LOGIC_CONDITION_MAP_INPUT` | 36 |  |
| `LOGIC_CONDITION_MAP_OUTPUT` | 37 |  |
| `LOGIC_CONDITION_RC_CHANNEL_OVERRIDE` | 38 |  |
| `LOGIC_CONDITION_SET_HEADING_TARGET` | 39 |  |
| `LOGIC_CONDITION_MODULUS` | 40 |  |
| `LOGIC_CONDITION_LOITER_OVERRIDE` | 41 |  |
| `LOGIC_CONDITION_SET_PROFILE` | 42 |  |
| `LOGIC_CONDITION_MIN` | 43 |  |
| `LOGIC_CONDITION_MAX` | 44 |  |
| `LOGIC_CONDITION_FLIGHT_AXIS_ANGLE_OVERRIDE` | 45 |  |
| `LOGIC_CONDITION_FLIGHT_AXIS_RATE_OVERRIDE` | 46 |  |
| `LOGIC_CONDITION_EDGE` | 47 |  |
| `LOGIC_CONDITION_DELAY` | 48 |  |
| `LOGIC_CONDITION_TIMER` | 49 |  |
| `LOGIC_CONDITION_DELTA` | 50 |  |
| `LOGIC_CONDITION_APPROX_EQUAL` | 51 |  |
| `LOGIC_CONDITION_LED_PIN_PWM` | 52 |  |
| `LOGIC_CONDITION_DISABLE_GPS_FIX` | 53 |  |
| `LOGIC_CONDITION_RESET_MAG_CALIBRATION` | 54 |  |
| `LOGIC_CONDITION_SET_GIMBAL_SENSITIVITY` | 55 |  |
| `LOGIC_CONDITION_OVERRIDE_MIN_GROUND_SPEED` | 56 |  |
| `LOGIC_CONDITION_LAST` | 57 |  |

---
## <a id="enum-logicwaypointoperands_e"></a>`logicWaypointOperands_e`

> Source: ../../../src/main/programming/logic_condition.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_IS_WP` | 0 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_WAYPOINT_INDEX` | 1 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_WAYPOINT_ACTION` | 2 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_NEXT_WAYPOINT_ACTION` | 3 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_WAYPOINT_DISTANCE` | 4 |  |
| `LOGIC_CONDTIION_OPERAND_WAYPOINTS_DISTANCE_FROM_WAYPOINT` | 5 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_USER1_ACTION` | 6 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_USER2_ACTION` | 7 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_USER3_ACTION` | 8 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_USER4_ACTION` | 9 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_USER1_ACTION_NEXT_WP` | 10 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_USER2_ACTION_NEXT_WP` | 11 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_USER3_ACTION_NEXT_WP` | 12 |  |
| `LOGIC_CONDITION_OPERAND_WAYPOINTS_USER4_ACTION_NEXT_WP` | 13 |  |

---
## <a id="enum-logtopic_e"></a>`logTopic_e`

> Source: ../../../src/main/common/log.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LOG_TOPIC_SYSTEM` | 0 |  |
| `LOG_TOPIC_GYRO` | 1 |  |
| `LOG_TOPIC_BARO` | 2 |  |
| `LOG_TOPIC_PITOT` | 3 |  |
| `LOG_TOPIC_PWM` | 4 |  |
| `LOG_TOPIC_TIMER` | 5 |  |
| `LOG_TOPIC_IMU` | 6 |  |
| `LOG_TOPIC_TEMPERATURE` | 7 |  |
| `LOG_TOPIC_POS_ESTIMATOR` | 8 |  |
| `LOG_TOPIC_VTX` | 9 |  |
| `LOG_TOPIC_OSD` | 10 |  |
| `LOG_TOPIC_COUNT` | 11 |  |

---
## <a id="enum-lsm6dxxconfigmasks_e"></a>`lsm6dxxConfigMasks_e`

> Source: ../../../src/main/drivers/accgyro/accgyro_lsm6dxx.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LSM6DXX_MASK_COUNTER_BDR1` | 128 |  |
| `LSM6DXX_MASK_CTRL3_C` | 60 |  |
| `LSM6DXX_MASK_CTRL3_C_RESET` | BIT(0) |  |
| `LSM6DXX_MASK_CTRL4_C` | 14 |  |
| `LSM6DXX_MASK_CTRL6_C` | 23 |  |
| `LSM6DXX_MASK_CTRL7_G` | 112 |  |
| `LSM6DXX_MASK_CTRL9_XL` | 2 |  |
| `LSM6DSL_MASK_CTRL6_C` | 19 |  |

---
## <a id="enum-lsm6dxxconfigvalues_e"></a>`lsm6dxxConfigValues_e`

> Source: ../../../src/main/drivers/accgyro/accgyro_lsm6dxx.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LSM6DXX_VAL_COUNTER_BDR1_DDRY_PM` | BIT(7) |  |
| `LSM6DXX_VAL_INT1_CTRL` | 2 |  |
| `LSM6DXX_VAL_INT2_CTRL` | 0 |  |
| `LSM6DXX_VAL_CTRL1_XL_ODR833` | 7 |  |
| `LSM6DXX_VAL_CTRL1_XL_ODR1667` | 8 |  |
| `LSM6DXX_VAL_CTRL1_XL_ODR3332` | 9 |  |
| `LSM6DXX_VAL_CTRL1_XL_ODR3333` | 10 |  |
| `LSM6DXX_VAL_CTRL1_XL_8G` | 3 |  |
| `LSM6DXX_VAL_CTRL1_XL_16G` | 1 |  |
| `LSM6DXX_VAL_CTRL1_XL_LPF1` | 0 |  |
| `LSM6DXX_VAL_CTRL1_XL_LPF2` | 1 |  |
| `LSM6DXX_VAL_CTRL2_G_ODR6664` | 10 |  |
| `LSM6DXX_VAL_CTRL2_G_2000DPS` | 3 |  |
| `LSM6DXX_VAL_CTRL3_C_H_LACTIVE` | 0 |  |
| `LSM6DXX_VAL_CTRL3_C_PP_OD` | 0 |  |
| `LSM6DXX_VAL_CTRL3_C_SIM` | 0 |  |
| `LSM6DXX_VAL_CTRL3_C_IF_INC` | BIT(2) |  |
| `LSM6DXX_VAL_CTRL4_C_DRDY_MASK` | BIT(3) |  |
| `LSM6DXX_VAL_CTRL4_C_I2C_DISABLE` | BIT(2) |  |
| `LSM6DXX_VAL_CTRL4_C_LPF1_SEL_G` | BIT(1) |  |
| `LSM6DXX_VAL_CTRL6_C_XL_HM_MODE` | 0 |  |
| `LSM6DXX_VAL_CTRL6_C_FTYPE_300HZ` | 0 |  |
| `LSM6DXX_VAL_CTRL6_C_FTYPE_201HZ` | 1 |  |
| `LSM6DXX_VAL_CTRL6_C_FTYPE_102HZ` | 2 |  |
| `LSM6DXX_VAL_CTRL6_C_FTYPE_603HZ` | 3 |  |
| `LSM6DXX_VAL_CTRL7_G_HP_EN_G` | BIT(6) |  |
| `LSM6DXX_VAL_CTRL7_G_HPM_G_16` | 0 |  |
| `LSM6DXX_VAL_CTRL7_G_HPM_G_65` | 1 |  |
| `LSM6DXX_VAL_CTRL7_G_HPM_G_260` | 2 |  |
| `LSM6DXX_VAL_CTRL7_G_HPM_G_1040` | 3 |  |
| `LSM6DXX_VAL_CTRL9_XL_I3C_DISABLE` | BIT(1) |  |

---
## <a id="enum-lsm6dxxregister_e"></a>`lsm6dxxRegister_e`

> Source: ../../../src/main/drivers/accgyro/accgyro_lsm6dxx.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LSM6DXX_REG_COUNTER_BDR1` | 11 |  |
| `LSM6DXX_REG_INT1_CTRL` | 13 |  |
| `LSM6DXX_REG_INT2_CTRL` | 14 |  |
| `LSM6DXX_REG_WHO_AM_I` | 15 |  |
| `LSM6DXX_REG_CTRL1_XL` | 16 |  |
| `LSM6DXX_REG_CTRL2_G` | 17 |  |
| `LSM6DXX_REG_CTRL3_C` | 18 |  |
| `LSM6DXX_REG_CTRL4_C` | 19 |  |
| `LSM6DXX_REG_CTRL5_C` | 20 |  |
| `LSM6DXX_REG_CTRL6_C` | 21 |  |
| `LSM6DXX_REG_CTRL7_G` | 22 |  |
| `LSM6DXX_REG_CTRL8_XL` | 23 |  |
| `LSM6DXX_REG_CTRL9_XL` | 24 |  |
| `LSM6DXX_REG_CTRL10_C` | 25 |  |
| `LSM6DXX_REG_STATUS` | 30 |  |
| `LSM6DXX_REG_OUT_TEMP_L` | 32 |  |
| `LSM6DXX_REG_OUT_TEMP_H` | 33 |  |
| `LSM6DXX_REG_OUTX_L_G` | 34 |  |
| `LSM6DXX_REG_OUTX_H_G` | 35 |  |
| `LSM6DXX_REG_OUTY_L_G` | 36 |  |
| `LSM6DXX_REG_OUTY_H_G` | 37 |  |
| `LSM6DXX_REG_OUTZ_L_G` | 38 |  |
| `LSM6DXX_REG_OUTZ_H_G` | 39 |  |
| `LSM6DXX_REG_OUTX_L_A` | 40 |  |
| `LSM6DXX_REG_OUTX_H_A` | 41 |  |
| `LSM6DXX_REG_OUTY_L_A` | 42 |  |
| `LSM6DXX_REG_OUTY_H_A` | 43 |  |
| `LSM6DXX_REG_OUTZ_L_A` | 44 |  |
| `LSM6DXX_REG_OUTZ_H_A` | 45 |  |

---
## <a id="enum-ltm_frame_e"></a>`ltm_frame_e`

> Source: ../../../src/main/telemetry/ltm.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LTM_FRAME_START` | 0 |  |
| `LTM_AFRAME` | LTM_FRAME_START |  |
| `LTM_SFRAME` |  |  |
| `LTM_GFRAME` |  | USE_GPS |
| `LTM_OFRAME` |  | USE_GPS |
| `LTM_XFRAME` |  | USE_GPS |
| `LTM_NFRAME` |  |  |
| `LTM_FRAME_COUNT` |  |  |

---
## <a id="enum-ltm_modes_e"></a>`ltm_modes_e`

> Source: ../../../src/main/telemetry/ltm.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LTM_MODE_MANUAL` | 0 |  |
| `LTM_MODE_RATE` | 1 |  |
| `LTM_MODE_ANGLE` | 2 |  |
| `LTM_MODE_HORIZON` | 3 |  |
| `LTM_MODE_ACRO` | 4 |  |
| `LTM_MODE_STABALIZED1` | 5 |  |
| `LTM_MODE_STABALIZED2` | 6 |  |
| `LTM_MODE_STABILIZED3` | 7 |  |
| `LTM_MODE_ALTHOLD` | 8 |  |
| `LTM_MODE_GPSHOLD` | 9 |  |
| `LTM_MODE_WAYPOINTS` | 10 |  |
| `LTM_MODE_HEADHOLD` | 11 |  |
| `LTM_MODE_CIRCLE` | 12 |  |
| `LTM_MODE_RTH` | 13 |  |
| `LTM_MODE_FOLLOWWME` | 14 |  |
| `LTM_MODE_LAND` | 15 |  |
| `LTM_MODE_FLYBYWIRE1` | 16 |  |
| `LTM_MODE_FLYBYWIRE2` | 17 |  |
| `LTM_MODE_CRUISE` | 18 |  |
| `LTM_MODE_UNKNOWN` | 19 |  |
| `LTM_MODE_LAUNCH` | 20 |  |
| `LTM_MODE_AUTOTUNE` | 21 |  |

---
## <a id="enum-ltmupdaterate_e"></a>`ltmUpdateRate_e`

> Source: ../../../src/main/telemetry/telemetry.h

| Enumerator | Value | Condition |
|---|---:|---|
| `LTM_RATE_NORMAL` | 0 |  |
| `LTM_RATE_MEDIUM` | 1 |  |
| `LTM_RATE_SLOW` | 2 |  |

---
## <a id="enum-magsensor_e"></a>`magSensor_e`

> Source: ../../../src/main/sensors/compass.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MAG_NONE` | 0 |  |
| `MAG_AUTODETECT` | 1 |  |
| `MAG_HMC5883` | 2 |  |
| `MAG_AK8975` | 3 |  |
| `MAG_MAG3110` | 4 |  |
| `MAG_AK8963` | 5 |  |
| `MAG_IST8310` | 6 |  |
| `MAG_QMC5883` | 7 |  |
| `MAG_QMC5883P` | 8 |  |
| `MAG_MPU9250` | 9 |  |
| `MAG_IST8308` | 10 |  |
| `MAG_LIS3MDL` | 11 |  |
| `MAG_MSP` | 12 |  |
| `MAG_RM3100` | 13 |  |
| `MAG_VCM5883` | 14 |  |
| `MAG_MLX90393` | 15 |  |
| `MAG_FAKE` | 16 |  |
| `MAG_MAX` | MAG_FAKE |  |

---
## <a id="enum-mavlinkautopilottype_e"></a>`mavlinkAutopilotType_e`

> Source: ../../../src/main/telemetry/telemetry.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MAVLINK_AUTOPILOT_GENERIC` | 0 |  |
| `MAVLINK_AUTOPILOT_ARDUPILOT` | 1 |  |

---
## <a id="enum-mavlinkradio_e"></a>`mavlinkRadio_e`

> Source: ../../../src/main/telemetry/telemetry.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MAVLINK_RADIO_GENERIC` | 0 |  |
| `MAVLINK_RADIO_ELRS` | 1 |  |
| `MAVLINK_RADIO_SIK` | 2 |  |

---
## <a id="enum-measurementsteps_e"></a>`measurementSteps_e`

> Source: ../../../src/main/drivers/rangefinder/rangefinder_vl53l0x.c

| Enumerator | Value | Condition |
|---|---:|---|
| `MEASUREMENT_START` | 0 |  |
| `MEASUREMENT_WAIT` | 1 |  |
| `MEASUREMENT_READ` | 2 |  |

---
## <a id="enum-mixerprofileatrequest_e"></a>`mixerProfileATRequest_e`

> Source: ../../../src/main/flight/mixer_profile.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MIXERAT_REQUEST_NONE` | 0 |  |
| `MIXERAT_REQUEST_RTH` | 1 |  |
| `MIXERAT_REQUEST_LAND` | 2 |  |
| `MIXERAT_REQUEST_ABORT` | 3 |  |

---
## <a id="enum-mixerprofileatstate_e"></a>`mixerProfileATState_e`

> Source: ../../../src/main/flight/mixer_profile.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MIXERAT_PHASE_IDLE` | 0 |  |
| `MIXERAT_PHASE_TRANSITION_INITIALIZE` | 1 |  |
| `MIXERAT_PHASE_TRANSITIONING` | 2 |  |
| `MIXERAT_PHASE_DONE` | 3 |  |

---
## <a id="enum-modeactivationoperator_e"></a>`modeActivationOperator_e`

> Source: ../../../src/main/fc/rc_modes.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MODE_OPERATOR_OR` | 0 |  |
| `MODE_OPERATOR_AND` | 1 |  |

---
## <a id="enum-motorpwmprotocoltypes_e"></a>`motorPwmProtocolTypes_e`

> Source: ../../../src/main/drivers/pwm_mapping.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PWM_TYPE_STANDARD` | 0 |  |
| `PWM_TYPE_ONESHOT125` | 1 |  |
| `PWM_TYPE_MULTISHOT` | 2 |  |
| `PWM_TYPE_BRUSHED` | 3 |  |
| `PWM_TYPE_DSHOT150` | 4 |  |
| `PWM_TYPE_DSHOT300` | 5 |  |
| `PWM_TYPE_DSHOT600` | 6 |  |

---
## <a id="enum-motorstatus_e"></a>`motorStatus_e`

> Source: ../../../src/main/flight/mixer.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MOTOR_STOPPED_USER` | 0 |  |
| `MOTOR_STOPPED_AUTO` | 1 |  |
| `MOTOR_RUNNING` | 2 |  |

---
## <a id="enum-mpu9250compassreadstate_e"></a>`mpu9250CompassReadState_e`

> Source: ../../../src/main/drivers/compass/compass_mpu9250.c

| Enumerator | Value | Condition |
|---|---:|---|
| `CHECK_STATUS` | 0 |  |
| `WAITING_FOR_STATUS` | 1 |  |
| `WAITING_FOR_DATA` | 2 |  |

---
## <a id="enum-mspflashfsflags_e"></a>`mspFlashfsFlags_e`

> Source: ../../../src/main/fc/fc_msp.c

| Enumerator | Value | Condition |
|---|---:|---|
| `MSP_FLASHFS_BIT_READY` | 1 |  |
| `MSP_FLASHFS_BIT_SUPPORTED` | 2 |  |

---
## <a id="enum-msppassthroughtype_e"></a>`mspPassthroughType_e`

> Source: ../../../src/main/fc/fc_msp.c

| Enumerator | Value | Condition |
|---|---:|---|
| `MSP_PASSTHROUGH_SERIAL_ID` | 253 |  |
| `MSP_PASSTHROUGH_SERIAL_FUNCTION_ID` | 254 |  |
| `MSP_PASSTHROUGH_ESC_4WAY` | 255 |  |

---
## <a id="enum-mspsdcardflags_e"></a>`mspSDCardFlags_e`

> Source: ../../../src/main/fc/fc_msp.c

| Enumerator | Value | Condition |
|---|---:|---|
| `MSP_SDCARD_FLAG_SUPPORTTED` | 1 |  |

---
## <a id="enum-mspsdcardstate_e"></a>`mspSDCardState_e`

> Source: ../../../src/main/fc/fc_msp.c

| Enumerator | Value | Condition |
|---|---:|---|
| `MSP_SDCARD_STATE_NOT_PRESENT` | 0 |  |
| `MSP_SDCARD_STATE_FATAL` | 1 |  |
| `MSP_SDCARD_STATE_CARD_INIT` | 2 |  |
| `MSP_SDCARD_STATE_FS_INIT` | 3 |  |
| `MSP_SDCARD_STATE_READY` | 4 |  |

---
## <a id="enum-multi_function_e"></a>`multi_function_e`

> Source: ../../../src/main/fc/multifunction.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MULTI_FUNC_NONE` | 0 |  |
| `MULTI_FUNC_1` | 1 |  |
| `MULTI_FUNC_2` | 2 |  |
| `MULTI_FUNC_3` | 3 |  |
| `MULTI_FUNC_4` | 4 |  |
| `MULTI_FUNC_5` | 5 |  |
| `MULTI_FUNC_6` | 6 |  |
| `MULTI_FUNC_END` | 7 |  |

---
## <a id="enum-multifunctionflags_e"></a>`multiFunctionFlags_e`

> Source: ../../../src/main/fc/multifunction.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MF_SUSPEND_SAFEHOMES` | (1 << 0) |  |
| `MF_SUSPEND_TRACKBACK` | (1 << 1) |  |
| `MF_TURTLE_MODE` | (1 << 2) |  |

---
## <a id="enum-nav_reset_type_e"></a>`nav_reset_type_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_RESET_NEVER` | 0 |  |
| `NAV_RESET_ON_FIRST_ARM` | 1 |  |
| `NAV_RESET_ON_EACH_ARM` | 2 |  |

---
## <a id="enum-navaglestimatequality_e"></a>`navAGLEstimateQuality_e`

> Source: ../../../src/main/navigation/navigation_pos_estimator_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SURFACE_QUAL_LOW` | 0 |  |
| `SURFACE_QUAL_MID` | 1 |  |
| `SURFACE_QUAL_HIGH` | 2 |  |

---
## <a id="enum-navarmingblocker_e"></a>`navArmingBlocker_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_ARMING_BLOCKER_NONE` | 0 |  |
| `NAV_ARMING_BLOCKER_MISSING_GPS_FIX` | 1 |  |
| `NAV_ARMING_BLOCKER_NAV_IS_ALREADY_ACTIVE` | 2 |  |
| `NAV_ARMING_BLOCKER_FIRST_WAYPOINT_TOO_FAR` | 3 |  |
| `NAV_ARMING_BLOCKER_JUMP_WAYPOINT_ERROR` | 4 |  |

---
## <a id="enum-navdefaultaltitudesensor_e"></a>`navDefaultAltitudeSensor_e`

> Source: ../../../src/main/navigation/navigation_pos_estimator_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ALTITUDE_SOURCE_GPS` | 0 |  |
| `ALTITUDE_SOURCE_BARO` | 1 |  |
| `ALTITUDE_SOURCE_GPS_ONLY` | 2 |  |
| `ALTITUDE_SOURCE_BARO_ONLY` | 3 |  |

---
## <a id="enum-navextraarmingsafety_e"></a>`navExtraArmingSafety_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_EXTRA_ARMING_SAFETY_ON` | 0 |  |
| `NAV_EXTRA_ARMING_SAFETY_ALLOW_BYPASS` | 1 |  |

---
## <a id="enum-navfwlaunchstatus_e"></a>`navFwLaunchStatus_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FW_LAUNCH_DETECTED` | 5 |  |
| `FW_LAUNCH_ABORTED` | 10 |  |
| `FW_LAUNCH_FLYING` | 11 |  |

---
## <a id="enum-navigationestimatestatus_e"></a>`navigationEstimateStatus_e`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `EST_NONE` | 0 |  |
| `EST_USABLE` | 1 |  |
| `EST_TRUSTED` | 2 |  |

---
## <a id="enum-navigationfsmevent_t"></a>`navigationFSMEvent_t`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_FSM_EVENT_NONE` | 0 |  |
| `NAV_FSM_EVENT_TIMEOUT` | 1 |  |
| `NAV_FSM_EVENT_SUCCESS` | 2 |  |
| `NAV_FSM_EVENT_ERROR` | 3 |  |
| `NAV_FSM_EVENT_SWITCH_TO_IDLE` | 4 |  |
| `NAV_FSM_EVENT_SWITCH_TO_ALTHOLD` | 5 |  |
| `NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D` | 6 |  |
| `NAV_FSM_EVENT_SWITCH_TO_RTH` | 7 |  |
| `NAV_FSM_EVENT_SWITCH_TO_WAYPOINT` | 8 |  |
| `NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING` | 9 |  |
| `NAV_FSM_EVENT_SWITCH_TO_LAUNCH` | 10 |  |
| `NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD` | 11 |  |
| `NAV_FSM_EVENT_SWITCH_TO_CRUISE` | 12 |  |
| `NAV_FSM_EVENT_SWITCH_TO_COURSE_ADJ` | 13 |  |
| `NAV_FSM_EVENT_SWITCH_TO_MIXERAT` | 14 |  |
| `NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_FW_LANDING` | 15 |  |
| `NAV_FSM_EVENT_SWITCH_TO_SEND_TO` | 16 |  |
| `NAV_FSM_EVENT_STATE_SPECIFIC_1` | 17 |  |
| `NAV_FSM_EVENT_STATE_SPECIFIC_2` | 18 |  |
| `NAV_FSM_EVENT_STATE_SPECIFIC_3` | 19 |  |
| `NAV_FSM_EVENT_STATE_SPECIFIC_4` | 20 |  |
| `NAV_FSM_EVENT_STATE_SPECIFIC_5` | 21 |  |
| `NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_FW_LANDING_ABORT` | NAV_FSM_EVENT_STATE_SPECIFIC_1 |  |
| `NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_FW_LANDING_FINISHED` | NAV_FSM_EVENT_STATE_SPECIFIC_2 |  |
| `NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_HOLD_TIME` | NAV_FSM_EVENT_STATE_SPECIFIC_1 |  |
| `NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_RTH_LAND` | NAV_FSM_EVENT_STATE_SPECIFIC_2 |  |
| `NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_FINISHED` | NAV_FSM_EVENT_STATE_SPECIFIC_3 |  |
| `NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_RTH_INITIALIZE` | NAV_FSM_EVENT_STATE_SPECIFIC_1 |  |
| `NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_RTH_TRACKBACK` | NAV_FSM_EVENT_STATE_SPECIFIC_2 |  |
| `NAV_FSM_EVENT_SWITCH_TO_RTH_HEAD_HOME` | NAV_FSM_EVENT_STATE_SPECIFIC_3 |  |
| `NAV_FSM_EVENT_SWITCH_TO_RTH_LOITER_ABOVE_HOME` | NAV_FSM_EVENT_STATE_SPECIFIC_4 |  |
| `NAV_FSM_EVENT_SWITCH_TO_RTH_LANDING` | NAV_FSM_EVENT_STATE_SPECIFIC_5 |  |
| `NAV_FSM_EVENT_COUNT` |  |  |

---
## <a id="enum-navigationfsmstate_t"></a>`navigationFSMState_t`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_STATE_UNDEFINED` | 0 |  |
| `NAV_STATE_IDLE` | 1 |  |
| `NAV_STATE_ALTHOLD_INITIALIZE` | 2 |  |
| `NAV_STATE_ALTHOLD_IN_PROGRESS` | 3 |  |
| `NAV_STATE_POSHOLD_3D_INITIALIZE` | 4 |  |
| `NAV_STATE_POSHOLD_3D_IN_PROGRESS` | 5 |  |
| `NAV_STATE_RTH_INITIALIZE` | 6 |  |
| `NAV_STATE_RTH_CLIMB_TO_SAFE_ALT` | 7 |  |
| `NAV_STATE_RTH_TRACKBACK` | 8 |  |
| `NAV_STATE_RTH_HEAD_HOME` | 9 |  |
| `NAV_STATE_RTH_LOITER_PRIOR_TO_LANDING` | 10 |  |
| `NAV_STATE_RTH_LOITER_ABOVE_HOME` | 11 |  |
| `NAV_STATE_RTH_LANDING` | 12 |  |
| `NAV_STATE_RTH_FINISHING` | 13 |  |
| `NAV_STATE_RTH_FINISHED` | 14 |  |
| `NAV_STATE_WAYPOINT_INITIALIZE` | 15 |  |
| `NAV_STATE_WAYPOINT_PRE_ACTION` | 16 |  |
| `NAV_STATE_WAYPOINT_IN_PROGRESS` | 17 |  |
| `NAV_STATE_WAYPOINT_REACHED` | 18 |  |
| `NAV_STATE_WAYPOINT_HOLD_TIME` | 19 |  |
| `NAV_STATE_WAYPOINT_NEXT` | 20 |  |
| `NAV_STATE_WAYPOINT_FINISHED` | 21 |  |
| `NAV_STATE_WAYPOINT_RTH_LAND` | 22 |  |
| `NAV_STATE_EMERGENCY_LANDING_INITIALIZE` | 23 |  |
| `NAV_STATE_EMERGENCY_LANDING_IN_PROGRESS` | 24 |  |
| `NAV_STATE_EMERGENCY_LANDING_FINISHED` | 25 |  |
| `NAV_STATE_LAUNCH_INITIALIZE` | 26 |  |
| `NAV_STATE_LAUNCH_WAIT` | 27 |  |
| `NAV_STATE_LAUNCH_IN_PROGRESS` | 28 |  |
| `NAV_STATE_COURSE_HOLD_INITIALIZE` | 29 |  |
| `NAV_STATE_COURSE_HOLD_IN_PROGRESS` | 30 |  |
| `NAV_STATE_COURSE_HOLD_ADJUSTING` | 31 |  |
| `NAV_STATE_CRUISE_INITIALIZE` | 32 |  |
| `NAV_STATE_CRUISE_IN_PROGRESS` | 33 |  |
| `NAV_STATE_CRUISE_ADJUSTING` | 34 |  |
| `NAV_STATE_FW_LANDING_CLIMB_TO_LOITER` | 35 |  |
| `NAV_STATE_FW_LANDING_LOITER` | 36 |  |
| `NAV_STATE_FW_LANDING_APPROACH` | 37 |  |
| `NAV_STATE_FW_LANDING_GLIDE` | 38 |  |
| `NAV_STATE_FW_LANDING_FLARE` | 39 |  |
| `NAV_STATE_FW_LANDING_FINISHED` | 40 |  |
| `NAV_STATE_FW_LANDING_ABORT` | 41 |  |
| `NAV_STATE_MIXERAT_INITIALIZE` | 42 |  |
| `NAV_STATE_MIXERAT_IN_PROGRESS` | 43 |  |
| `NAV_STATE_MIXERAT_ABORT` | 44 |  |
| `NAV_STATE_SEND_TO_INITALIZE` | 45 |  |
| `NAV_STATE_SEND_TO_IN_PROGESS` | 46 |  |
| `NAV_STATE_SEND_TO_FINISHED` | 47 |  |
| `NAV_STATE_COUNT` | 48 |  |

---
## <a id="enum-navigationfsmstateflags_t"></a>`navigationFSMStateFlags_t`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_CTL_ALT` | (1 << 0) |  |
| `NAV_CTL_POS` | (1 << 1) |  |
| `NAV_CTL_YAW` | (1 << 2) |  |
| `NAV_CTL_EMERG` | (1 << 3) |  |
| `NAV_CTL_LAUNCH` | (1 << 4) |  |
| `NAV_REQUIRE_ANGLE` | (1 << 5) |  |
| `NAV_REQUIRE_ANGLE_FW` | (1 << 6) |  |
| `NAV_REQUIRE_MAGHOLD` | (1 << 7) |  |
| `NAV_REQUIRE_THRTILT` | (1 << 8) |  |
| `NAV_AUTO_RTH` | (1 << 9) |  |
| `NAV_AUTO_WP` | (1 << 10) |  |
| `NAV_RC_ALT` | (1 << 11) |  |
| `NAV_RC_POS` | (1 << 12) |  |
| `NAV_RC_YAW` | (1 << 13) |  |
| `NAV_CTL_LAND` | (1 << 14) |  |
| `NAV_AUTO_WP_DONE` | (1 << 15) |  |
| `NAV_MIXERAT` | (1 << 16) |  |
| `NAV_CTL_HOLD` | (1 << 17) |  |

---
## <a id="enum-navigationhomeflags_t"></a>`navigationHomeFlags_t`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_HOME_INVALID` | 0 |  |
| `NAV_HOME_VALID_XY` | 1 << 0 |  |
| `NAV_HOME_VALID_Z` | 1 << 1 |  |
| `NAV_HOME_VALID_HEADING` | 1 << 2 |  |
| `NAV_HOME_VALID_ALL` | NAV_HOME_VALID_XY | NAV_HOME_VALID_Z | NAV_HOME_VALID_HEADING |  |

---
## <a id="enum-navigationpersistentid_e"></a>`navigationPersistentId_e`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_PERSISTENT_ID_UNDEFINED` | 0 |  |
| `NAV_PERSISTENT_ID_IDLE` | 1 |  |
| `NAV_PERSISTENT_ID_ALTHOLD_INITIALIZE` | 2 |  |
| `NAV_PERSISTENT_ID_ALTHOLD_IN_PROGRESS` | 3 |  |
| `NAV_PERSISTENT_ID_UNUSED_1` | 4 |  |
| `NAV_PERSISTENT_ID_UNUSED_2` | 5 |  |
| `NAV_PERSISTENT_ID_POSHOLD_3D_INITIALIZE` | 6 |  |
| `NAV_PERSISTENT_ID_POSHOLD_3D_IN_PROGRESS` | 7 |  |
| `NAV_PERSISTENT_ID_RTH_INITIALIZE` | 8 |  |
| `NAV_PERSISTENT_ID_RTH_CLIMB_TO_SAFE_ALT` | 9 |  |
| `NAV_PERSISTENT_ID_RTH_HEAD_HOME` | 10 |  |
| `NAV_PERSISTENT_ID_RTH_LOITER_PRIOR_TO_LANDING` | 11 |  |
| `NAV_PERSISTENT_ID_RTH_LANDING` | 12 |  |
| `NAV_PERSISTENT_ID_RTH_FINISHING` | 13 |  |
| `NAV_PERSISTENT_ID_RTH_FINISHED` | 14 |  |
| `NAV_PERSISTENT_ID_WAYPOINT_INITIALIZE` | 15 |  |
| `NAV_PERSISTENT_ID_WAYPOINT_PRE_ACTION` | 16 |  |
| `NAV_PERSISTENT_ID_WAYPOINT_IN_PROGRESS` | 17 |  |
| `NAV_PERSISTENT_ID_WAYPOINT_REACHED` | 18 |  |
| `NAV_PERSISTENT_ID_WAYPOINT_NEXT` | 19 |  |
| `NAV_PERSISTENT_ID_WAYPOINT_FINISHED` | 20 |  |
| `NAV_PERSISTENT_ID_WAYPOINT_RTH_LAND` | 21 |  |
| `NAV_PERSISTENT_ID_EMERGENCY_LANDING_INITIALIZE` | 22 |  |
| `NAV_PERSISTENT_ID_EMERGENCY_LANDING_IN_PROGRESS` | 23 |  |
| `NAV_PERSISTENT_ID_EMERGENCY_LANDING_FINISHED` | 24 |  |
| `NAV_PERSISTENT_ID_LAUNCH_INITIALIZE` | 25 |  |
| `NAV_PERSISTENT_ID_LAUNCH_WAIT` | 26 |  |
| `NAV_PERSISTENT_ID_UNUSED_3` | 27 |  |
| `NAV_PERSISTENT_ID_LAUNCH_IN_PROGRESS` | 28 |  |
| `NAV_PERSISTENT_ID_COURSE_HOLD_INITIALIZE` | 29 |  |
| `NAV_PERSISTENT_ID_COURSE_HOLD_IN_PROGRESS` | 30 |  |
| `NAV_PERSISTENT_ID_COURSE_HOLD_ADJUSTING` | 31 |  |
| `NAV_PERSISTENT_ID_CRUISE_INITIALIZE` | 32 |  |
| `NAV_PERSISTENT_ID_CRUISE_IN_PROGRESS` | 33 |  |
| `NAV_PERSISTENT_ID_CRUISE_ADJUSTING` | 34 |  |
| `NAV_PERSISTENT_ID_WAYPOINT_HOLD_TIME` | 35 |  |
| `NAV_PERSISTENT_ID_RTH_LOITER_ABOVE_HOME` | 36 |  |
| `NAV_PERSISTENT_ID_UNUSED_4` | 37 |  |
| `NAV_PERSISTENT_ID_RTH_TRACKBACK` | 38 |  |
| `NAV_PERSISTENT_ID_MIXERAT_INITIALIZE` | 39 |  |
| `NAV_PERSISTENT_ID_MIXERAT_IN_PROGRESS` | 40 |  |
| `NAV_PERSISTENT_ID_MIXERAT_ABORT` | 41 |  |
| `NAV_PERSISTENT_ID_FW_LANDING_CLIMB_TO_LOITER` | 42 |  |
| `NAV_PERSISTENT_ID_FW_LANDING_LOITER` | 43 |  |
| `NAV_PERSISTENT_ID_FW_LANDING_APPROACH` | 44 |  |
| `NAV_PERSISTENT_ID_FW_LANDING_GLIDE` | 45 |  |
| `NAV_PERSISTENT_ID_FW_LANDING_FLARE` | 46 |  |
| `NAV_PERSISTENT_ID_FW_LANDING_ABORT` | 47 |  |
| `NAV_PERSISTENT_ID_FW_LANDING_FINISHED` | 48 |  |
| `NAV_PERSISTENT_ID_SEND_TO_INITALIZE` | 49 |  |
| `NAV_PERSISTENT_ID_SEND_TO_IN_PROGRES` | 50 |  |
| `NAV_PERSISTENT_ID_SEND_TO_FINISHED` | 51 |  |

---
## <a id="enum-navmcaltholdthrottle_e"></a>`navMcAltHoldThrottle_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MC_ALT_HOLD_STICK` | 0 |  |
| `MC_ALT_HOLD_MID` | 1 |  |
| `MC_ALT_HOLD_HOVER` | 2 |  |

---
## <a id="enum-navmissionrestart_e"></a>`navMissionRestart_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `WP_MISSION_START` | 0 |  |
| `WP_MISSION_RESUME` | 1 |  |
| `WP_MISSION_SWITCH` | 2 |  |

---
## <a id="enum-navoverridesmotorstop_e"></a>`navOverridesMotorStop_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NOMS_OFF_ALWAYS` | 0 |  |
| `NOMS_OFF` | 1 |  |
| `NOMS_AUTO_ONLY` | 2 |  |
| `NOMS_ALL_NAV` | 3 |  |

---
## <a id="enum-navpositionestimationflags_e"></a>`navPositionEstimationFlags_e`

> Source: ../../../src/main/navigation/navigation_pos_estimator_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `EST_GPS_XY_VALID` | (1 << 0) |  |
| `EST_GPS_Z_VALID` | (1 << 1) |  |
| `EST_BARO_VALID` | (1 << 2) |  |
| `EST_SURFACE_VALID` | (1 << 3) |  |
| `EST_FLOW_VALID` | (1 << 4) |  |
| `EST_XY_VALID` | (1 << 5) |  |
| `EST_Z_VALID` | (1 << 6) |  |

---
## <a id="enum-navrthallowlanding_e"></a>`navRTHAllowLanding_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_RTH_ALLOW_LANDING_NEVER` | 0 |  |
| `NAV_RTH_ALLOW_LANDING_ALWAYS` | 1 |  |
| `NAV_RTH_ALLOW_LANDING_FS_ONLY` | 2 |  |

---
## <a id="enum-navrthclimbfirst_e"></a>`navRTHClimbFirst_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RTH_CLIMB_OFF` | 0 |  |
| `RTH_CLIMB_ON` | 1 |  |
| `RTH_CLIMB_ON_FW_SPIRAL` | 2 |  |

---
## <a id="enum-navsetwaypointflags_t"></a>`navSetWaypointFlags_t`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_POS_UPDATE_NONE` | 0 |  |
| `NAV_POS_UPDATE_Z` | 1 << 1 |  |
| `NAV_POS_UPDATE_XY` | 1 << 0 |  |
| `NAV_POS_UPDATE_HEADING` | 1 << 2 |  |
| `NAV_POS_UPDATE_BEARING` | 1 << 3 |  |
| `NAV_POS_UPDATE_BEARING_TAIL_FIRST` | 1 << 4 |  |

---
## <a id="enum-navsystemstatus_error_e"></a>`navSystemStatus_Error_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MW_NAV_ERROR_NONE` | 0 |  |
| `MW_NAV_ERROR_TOOFAR` | 1 |  |
| `MW_NAV_ERROR_SPOILED_GPS` | 2 |  |
| `MW_NAV_ERROR_WP_CRC` | 3 |  |
| `MW_NAV_ERROR_FINISH` | 4 |  |
| `MW_NAV_ERROR_TIMEWAIT` | 5 |  |
| `MW_NAV_ERROR_INVALID_JUMP` | 6 |  |
| `MW_NAV_ERROR_INVALID_DATA` | 7 |  |
| `MW_NAV_ERROR_WAIT_FOR_RTH_ALT` | 8 |  |
| `MW_NAV_ERROR_GPS_FIX_LOST` | 9 |  |
| `MW_NAV_ERROR_DISARMED` | 10 |  |
| `MW_NAV_ERROR_LANDING` | 11 |  |

---
## <a id="enum-navsystemstatus_flags_e"></a>`navSystemStatus_Flags_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MW_NAV_FLAG_ADJUSTING_POSITION` | 1 << 0 |  |
| `MW_NAV_FLAG_ADJUSTING_ALTITUDE` | 1 << 1 |  |

---
## <a id="enum-navsystemstatus_mode_e"></a>`navSystemStatus_Mode_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MW_GPS_MODE_NONE` | 0 |  |
| `MW_GPS_MODE_HOLD` | 1 |  |
| `MW_GPS_MODE_RTH` | 2 |  |
| `MW_GPS_MODE_NAV` | 3 |  |
| `MW_GPS_MODE_EMERG` | 15 |  |

---
## <a id="enum-navsystemstatus_state_e"></a>`navSystemStatus_State_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MW_NAV_STATE_NONE` | 0 |  |
| `MW_NAV_STATE_RTH_START` | 1 |  |
| `MW_NAV_STATE_RTH_ENROUTE` | 2 |  |
| `MW_NAV_STATE_HOLD_INFINIT` | 3 |  |
| `MW_NAV_STATE_HOLD_TIMED` | 4 |  |
| `MW_NAV_STATE_WP_ENROUTE` | 5 |  |
| `MW_NAV_STATE_PROCESS_NEXT` | 6 |  |
| `MW_NAV_STATE_DO_JUMP` | 7 |  |
| `MW_NAV_STATE_LAND_START` | 8 |  |
| `MW_NAV_STATE_LAND_IN_PROGRESS` | 9 |  |
| `MW_NAV_STATE_LANDED` | 10 |  |
| `MW_NAV_STATE_LAND_SETTLE` | 11 |  |
| `MW_NAV_STATE_LAND_START_DESCENT` | 12 |  |
| `MW_NAV_STATE_HOVER_ABOVE_HOME` | 13 |  |
| `MW_NAV_STATE_EMERGENCY_LANDING` | 14 |  |
| `MW_NAV_STATE_RTH_CLIMB` | 15 |  |

---
## <a id="enum-navwaypointactions_e"></a>`navWaypointActions_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_WP_ACTION_WAYPOINT` | 1 |  |
| `NAV_WP_ACTION_HOLD_TIME` | 3 |  |
| `NAV_WP_ACTION_RTH` | 4 |  |
| `NAV_WP_ACTION_SET_POI` | 5 |  |
| `NAV_WP_ACTION_JUMP` | 6 |  |
| `NAV_WP_ACTION_SET_HEAD` | 7 |  |
| `NAV_WP_ACTION_LAND` | 8 |  |

---
## <a id="enum-navwaypointflags_e"></a>`navWaypointFlags_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_WP_FLAG_HOME` | 72 |  |
| `NAV_WP_FLAG_LAST` | 165 |  |

---
## <a id="enum-navwaypointheadings_e"></a>`navWaypointHeadings_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_WP_HEAD_MODE_NONE` | 0 |  |
| `NAV_WP_HEAD_MODE_POI` | 1 |  |
| `NAV_WP_HEAD_MODE_FIXED` | 2 |  |

---
## <a id="enum-navwaypointp3flags_e"></a>`navWaypointP3Flags_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_WP_ALTMODE` | (1<<0) |  |
| `NAV_WP_USER1` | (1<<1) |  |
| `NAV_WP_USER2` | (1<<2) |  |
| `NAV_WP_USER3` | (1<<3) |  |
| `NAV_WP_USER4` | (1<<4) |  |

---
## <a id="enum-opflowquality_e"></a>`opflowQuality_e`

> Source: ../../../src/main/sensors/opflow.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OPFLOW_QUALITY_INVALID` | 0 |  |
| `OPFLOW_QUALITY_VALID` | 1 |  |

---
## <a id="enum-opticalflowsensor_e"></a>`opticalFlowSensor_e`

> Source: ../../../src/main/sensors/opflow.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OPFLOW_NONE` | 0 |  |
| `OPFLOW_CXOF` | 1 |  |
| `OPFLOW_MSP` | 2 |  |
| `OPFLOW_FAKE` | 3 |  |

---
## <a id="enum-osd_adsb_warning_style_e"></a>`osd_adsb_warning_style_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_ADSB_WARNING_STYLE_COMPACT` | 0 |  |
| `OSD_ADSB_WARNING_STYLE_EXTENDED` | 1 |  |

---
## <a id="enum-osd_ahi_style_e"></a>`osd_ahi_style_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_AHI_STYLE_DEFAULT` | 0 |  |
| `OSD_AHI_STYLE_LINE` | 1 |  |

---
## <a id="enum-osd_alignment_e"></a>`osd_alignment_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_ALIGN_LEFT` | 0 |  |
| `OSD_ALIGN_RIGHT` | 1 |  |

---
## <a id="enum-osd_crosshairs_style_e"></a>`osd_crosshairs_style_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_CROSSHAIRS_STYLE_DEFAULT` | 0 |  |
| `OSD_CROSSHAIRS_STYLE_AIRCRAFT` | 1 |  |
| `OSD_CROSSHAIRS_STYLE_TYPE3` | 2 |  |
| `OSD_CROSSHAIRS_STYLE_TYPE4` | 3 |  |
| `OSD_CROSSHAIRS_STYLE_TYPE5` | 4 |  |
| `OSD_CROSSHAIRS_STYLE_TYPE6` | 5 |  |
| `OSD_CROSSHAIRS_STYLE_TYPE7` | 6 |  |

---
## <a id="enum-osd_crsf_lq_format_e"></a>`osd_crsf_lq_format_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_CRSF_LQ_TYPE1` | 0 |  |
| `OSD_CRSF_LQ_TYPE2` | 1 |  |
| `OSD_CRSF_LQ_TYPE3` | 2 |  |

---
## <a id="enum-osd_items_e"></a>`osd_items_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_RSSI_VALUE` | 0 |  |
| `OSD_MAIN_BATT_VOLTAGE` | 1 |  |
| `OSD_CROSSHAIRS` | 2 |  |
| `OSD_ARTIFICIAL_HORIZON` | 3 |  |
| `OSD_HORIZON_SIDEBARS` | 4 |  |
| `OSD_ONTIME` | 5 |  |
| `OSD_FLYTIME` | 6 |  |
| `OSD_FLYMODE` | 7 |  |
| `OSD_CRAFT_NAME` | 8 |  |
| `OSD_THROTTLE_POS` | 9 |  |
| `OSD_VTX_CHANNEL` | 10 |  |
| `OSD_CURRENT_DRAW` | 11 |  |
| `OSD_MAH_DRAWN` | 12 |  |
| `OSD_GPS_SPEED` | 13 |  |
| `OSD_GPS_SATS` | 14 |  |
| `OSD_ALTITUDE` | 15 |  |
| `OSD_ROLL_PIDS` | 16 |  |
| `OSD_PITCH_PIDS` | 17 |  |
| `OSD_YAW_PIDS` | 18 |  |
| `OSD_POWER` | 19 |  |
| `OSD_GPS_LON` | 20 |  |
| `OSD_GPS_LAT` | 21 |  |
| `OSD_HOME_DIR` | 22 |  |
| `OSD_HOME_DIST` | 23 |  |
| `OSD_HEADING` | 24 |  |
| `OSD_VARIO` | 25 |  |
| `OSD_VERTICAL_SPEED_INDICATOR` | 26 |  |
| `OSD_AIR_SPEED` | 27 |  |
| `OSD_ONTIME_FLYTIME` | 28 |  |
| `OSD_RTC_TIME` | 29 |  |
| `OSD_MESSAGES` | 30 |  |
| `OSD_GPS_HDOP` | 31 |  |
| `OSD_MAIN_BATT_CELL_VOLTAGE` | 32 |  |
| `OSD_SCALED_THROTTLE_POS` | 33 |  |
| `OSD_HEADING_GRAPH` | 34 |  |
| `OSD_EFFICIENCY_MAH_PER_KM` | 35 |  |
| `OSD_WH_DRAWN` | 36 |  |
| `OSD_BATTERY_REMAINING_CAPACITY` | 37 |  |
| `OSD_BATTERY_REMAINING_PERCENT` | 38 |  |
| `OSD_EFFICIENCY_WH_PER_KM` | 39 |  |
| `OSD_TRIP_DIST` | 40 |  |
| `OSD_ATTITUDE_PITCH` | 41 |  |
| `OSD_ATTITUDE_ROLL` | 42 |  |
| `OSD_MAP_NORTH` | 43 |  |
| `OSD_MAP_TAKEOFF` | 44 |  |
| `OSD_RADAR` | 45 |  |
| `OSD_WIND_SPEED_HORIZONTAL` | 46 |  |
| `OSD_WIND_SPEED_VERTICAL` | 47 |  |
| `OSD_REMAINING_FLIGHT_TIME_BEFORE_RTH` | 48 |  |
| `OSD_REMAINING_DISTANCE_BEFORE_RTH` | 49 |  |
| `OSD_HOME_HEADING_ERROR` | 50 |  |
| `OSD_COURSE_HOLD_ERROR` | 51 |  |
| `OSD_COURSE_HOLD_ADJUSTMENT` | 52 |  |
| `OSD_SAG_COMPENSATED_MAIN_BATT_VOLTAGE` | 53 |  |
| `OSD_MAIN_BATT_SAG_COMPENSATED_CELL_VOLTAGE` | 54 |  |
| `OSD_POWER_SUPPLY_IMPEDANCE` | 55 |  |
| `OSD_LEVEL_PIDS` | 56 |  |
| `OSD_POS_XY_PIDS` | 57 |  |
| `OSD_POS_Z_PIDS` | 58 |  |
| `OSD_VEL_XY_PIDS` | 59 |  |
| `OSD_VEL_Z_PIDS` | 60 |  |
| `OSD_HEADING_P` | 61 |  |
| `OSD_BOARD_ALIGN_ROLL` | 62 |  |
| `OSD_BOARD_ALIGN_PITCH` | 63 |  |
| `OSD_RC_EXPO` | 64 |  |
| `OSD_RC_YAW_EXPO` | 65 |  |
| `OSD_THROTTLE_EXPO` | 66 |  |
| `OSD_PITCH_RATE` | 67 |  |
| `OSD_ROLL_RATE` | 68 |  |
| `OSD_YAW_RATE` | 69 |  |
| `OSD_MANUAL_RC_EXPO` | 70 |  |
| `OSD_MANUAL_RC_YAW_EXPO` | 71 |  |
| `OSD_MANUAL_PITCH_RATE` | 72 |  |
| `OSD_MANUAL_ROLL_RATE` | 73 |  |
| `OSD_MANUAL_YAW_RATE` | 74 |  |
| `OSD_NAV_FW_CRUISE_THR` | 75 |  |
| `OSD_NAV_FW_PITCH2THR` | 76 |  |
| `OSD_FW_MIN_THROTTLE_DOWN_PITCH_ANGLE` | 77 |  |
| `OSD_DEBUG` | 78 |  |
| `OSD_FW_ALT_PID_OUTPUTS` | 79 |  |
| `OSD_FW_POS_PID_OUTPUTS` | 80 |  |
| `OSD_MC_VEL_X_PID_OUTPUTS` | 81 |  |
| `OSD_MC_VEL_Y_PID_OUTPUTS` | 82 |  |
| `OSD_MC_VEL_Z_PID_OUTPUTS` | 83 |  |
| `OSD_MC_POS_XYZ_P_OUTPUTS` | 84 |  |
| `OSD_3D_SPEED` | 85 |  |
| `OSD_IMU_TEMPERATURE` | 86 |  |
| `OSD_BARO_TEMPERATURE` | 87 |  |
| `OSD_TEMP_SENSOR_0_TEMPERATURE` | 88 |  |
| `OSD_TEMP_SENSOR_1_TEMPERATURE` | 89 |  |
| `OSD_TEMP_SENSOR_2_TEMPERATURE` | 90 |  |
| `OSD_TEMP_SENSOR_3_TEMPERATURE` | 91 |  |
| `OSD_TEMP_SENSOR_4_TEMPERATURE` | 92 |  |
| `OSD_TEMP_SENSOR_5_TEMPERATURE` | 93 |  |
| `OSD_TEMP_SENSOR_6_TEMPERATURE` | 94 |  |
| `OSD_TEMP_SENSOR_7_TEMPERATURE` | 95 |  |
| `OSD_ALTITUDE_MSL` | 96 |  |
| `OSD_PLUS_CODE` | 97 |  |
| `OSD_MAP_SCALE` | 98 |  |
| `OSD_MAP_REFERENCE` | 99 |  |
| `OSD_GFORCE` | 100 |  |
| `OSD_GFORCE_X` | 101 |  |
| `OSD_GFORCE_Y` | 102 |  |
| `OSD_GFORCE_Z` | 103 |  |
| `OSD_RC_SOURCE` | 104 |  |
| `OSD_VTX_POWER` | 105 |  |
| `OSD_ESC_RPM` | 106 |  |
| `OSD_ESC_TEMPERATURE` | 107 |  |
| `OSD_AZIMUTH` | 108 |  |
| `OSD_RSSI_DBM` | 109 |  |
| `OSD_LQ_UPLINK` | 110 |  |
| `OSD_SNR_DB` | 111 |  |
| `OSD_TX_POWER_UPLINK` | 112 |  |
| `OSD_GVAR_0` | 113 |  |
| `OSD_GVAR_1` | 114 |  |
| `OSD_GVAR_2` | 115 |  |
| `OSD_GVAR_3` | 116 |  |
| `OSD_TPA` | 117 |  |
| `OSD_NAV_FW_CONTROL_SMOOTHNESS` | 118 |  |
| `OSD_VERSION` | 119 |  |
| `OSD_RANGEFINDER` | 120 |  |
| `OSD_PLIMIT_REMAINING_BURST_TIME` | 121 |  |
| `OSD_PLIMIT_ACTIVE_CURRENT_LIMIT` | 122 |  |
| `OSD_PLIMIT_ACTIVE_POWER_LIMIT` | 123 |  |
| `OSD_GLIDESLOPE` | 124 |  |
| `OSD_GPS_MAX_SPEED` | 125 |  |
| `OSD_3D_MAX_SPEED` | 126 |  |
| `OSD_AIR_MAX_SPEED` | 127 |  |
| `OSD_ACTIVE_PROFILE` | 128 |  |
| `OSD_MISSION` | 129 |  |
| `OSD_SWITCH_INDICATOR_0` | 130 |  |
| `OSD_SWITCH_INDICATOR_1` | 131 |  |
| `OSD_SWITCH_INDICATOR_2` | 132 |  |
| `OSD_SWITCH_INDICATOR_3` | 133 |  |
| `OSD_TPA_TIME_CONSTANT` | 134 |  |
| `OSD_FW_LEVEL_TRIM` | 135 |  |
| `OSD_GLIDE_TIME_REMAINING` | 136 |  |
| `OSD_GLIDE_RANGE` | 137 |  |
| `OSD_CLIMB_EFFICIENCY` | 138 |  |
| `OSD_NAV_WP_MULTI_MISSION_INDEX` | 139 |  |
| `OSD_GROUND_COURSE` | 140 |  |
| `OSD_CROSS_TRACK_ERROR` | 141 |  |
| `OSD_PILOT_NAME` | 142 |  |
| `OSD_PAN_SERVO_CENTRED` | 143 |  |
| `OSD_MULTI_FUNCTION` | 144 |  |
| `OSD_ODOMETER` | 145 |  |
| `OSD_PILOT_LOGO` | 146 |  |
| `OSD_CUSTOM_ELEMENT_1` | 147 |  |
| `OSD_CUSTOM_ELEMENT_2` | 148 |  |
| `OSD_CUSTOM_ELEMENT_3` | 149 |  |
| `OSD_ADSB_WARNING` | 150 |  |
| `OSD_ADSB_INFO` | 151 |  |
| `OSD_BLACKBOX` | 152 |  |
| `OSD_FORMATION_FLIGHT` | 153 |  |
| `OSD_CUSTOM_ELEMENT_4` | 154 |  |
| `OSD_CUSTOM_ELEMENT_5` | 155 |  |
| `OSD_CUSTOM_ELEMENT_6` | 156 |  |
| `OSD_CUSTOM_ELEMENT_7` | 157 |  |
| `OSD_CUSTOM_ELEMENT_8` | 158 |  |
| `OSD_LQ_DOWNLINK` | 159 |  |
| `OSD_RX_POWER_DOWNLINK` | 160 |  |
| `OSD_RX_BAND` | 161 |  |
| `OSD_RX_MODE` | 162 |  |
| `OSD_COURSE_TO_FENCE` | 163 |  |
| `OSD_H_DIST_TO_FENCE` | 164 |  |
| `OSD_V_DIST_TO_FENCE` | 165 |  |
| `OSD_NAV_FW_ALT_CONTROL_RESPONSE` | 166 |  |
| `OSD_NAV_MIN_GROUND_SPEED` | 167 |  |
| `OSD_THROTTLE_GAUGE` | 168 |  |
| `OSD_ITEM_COUNT` | 169 |  |

---
## <a id="enum-osd_sidebar_arrow_e"></a>`osd_sidebar_arrow_e`

> Source: ../../../src/main/io/osd_grid.c

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_SIDEBAR_ARROW_NONE` | 0 |  |
| `OSD_SIDEBAR_ARROW_UP` | 1 |  |
| `OSD_SIDEBAR_ARROW_DOWN` | 2 |  |

---
## <a id="enum-osd_sidebar_scroll_e"></a>`osd_sidebar_scroll_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_SIDEBAR_SCROLL_NONE` | 0 |  |
| `OSD_SIDEBAR_SCROLL_ALTITUDE` | 1 |  |
| `OSD_SIDEBAR_SCROLL_SPEED` | 2 |  |
| `OSD_SIDEBAR_SCROLL_HOME_DISTANCE` | 3 |  |
| `OSD_SIDEBAR_SCROLL_MAX` | OSD_SIDEBAR_SCROLL_HOME_DISTANCE |  |

---
## <a id="enum-osd_speedtypes_e"></a>`osd_SpeedTypes_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_SPEED_TYPE_GROUND` | 0 |  |
| `OSD_SPEED_TYPE_AIR` | 1 |  |
| `OSD_SPEED_TYPE_3D` | 2 |  |
| `OSD_SPEED_TYPE_MIN_GROUND` | 3 |  |

---
## <a id="enum-osd_stats_energy_unit_e"></a>`osd_stats_energy_unit_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_STATS_ENERGY_UNIT_MAH` | 0 |  |
| `OSD_STATS_ENERGY_UNIT_WH` | 1 |  |

---
## <a id="enum-osd_unit_e"></a>`osd_unit_e`

> Source: ../../../src/main/io/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_UNIT_IMPERIAL` | 0 |  |
| `OSD_UNIT_METRIC` | 1 |  |
| `OSD_UNIT_METRIC_MPH` | 2 |  |
| `OSD_UNIT_UK` | 3 |  |
| `OSD_UNIT_GA` | 4 |  |
| `OSD_UNIT_MAX` | OSD_UNIT_GA |  |

---
## <a id="enum-osdcustomelementtype_e"></a>`osdCustomElementType_e`

> Source: ../../../src/main/io/osd/custom_elements.h

| Enumerator | Value | Condition |
|---|---:|---|
| `CUSTOM_ELEMENT_TYPE_NONE` | 0 |  |
| `CUSTOM_ELEMENT_TYPE_TEXT` | 1 |  |
| `CUSTOM_ELEMENT_TYPE_ICON_STATIC` | 2 |  |
| `CUSTOM_ELEMENT_TYPE_ICON_GV` | 3 |  |
| `CUSTOM_ELEMENT_TYPE_ICON_LC` | 4 |  |
| `CUSTOM_ELEMENT_TYPE_GV_1` | 5 |  |
| `CUSTOM_ELEMENT_TYPE_GV_2` | 6 |  |
| `CUSTOM_ELEMENT_TYPE_GV_3` | 7 |  |
| `CUSTOM_ELEMENT_TYPE_GV_4` | 8 |  |
| `CUSTOM_ELEMENT_TYPE_GV_5` | 9 |  |
| `CUSTOM_ELEMENT_TYPE_GV_FLOAT_1_1` | 10 |  |
| `CUSTOM_ELEMENT_TYPE_GV_FLOAT_1_2` | 11 |  |
| `CUSTOM_ELEMENT_TYPE_GV_FLOAT_2_1` | 12 |  |
| `CUSTOM_ELEMENT_TYPE_GV_FLOAT_2_2` | 13 |  |
| `CUSTOM_ELEMENT_TYPE_GV_FLOAT_3_1` | 14 |  |
| `CUSTOM_ELEMENT_TYPE_GV_FLOAT_3_2` | 15 |  |
| `CUSTOM_ELEMENT_TYPE_GV_FLOAT_4_1` | 16 |  |
| `CUSTOM_ELEMENT_TYPE_LC_1` | 17 |  |
| `CUSTOM_ELEMENT_TYPE_LC_2` | 18 |  |
| `CUSTOM_ELEMENT_TYPE_LC_3` | 19 |  |
| `CUSTOM_ELEMENT_TYPE_LC_4` | 20 |  |
| `CUSTOM_ELEMENT_TYPE_LC_5` | 21 |  |
| `CUSTOM_ELEMENT_TYPE_LC_FLOAT_1_1` | 22 |  |
| `CUSTOM_ELEMENT_TYPE_LC_FLOAT_1_2` | 23 |  |
| `CUSTOM_ELEMENT_TYPE_LC_FLOAT_2_1` | 24 |  |
| `CUSTOM_ELEMENT_TYPE_LC_FLOAT_2_2` | 25 |  |
| `CUSTOM_ELEMENT_TYPE_LC_FLOAT_3_1` | 26 |  |
| `CUSTOM_ELEMENT_TYPE_LC_FLOAT_3_2` | 27 |  |
| `CUSTOM_ELEMENT_TYPE_LC_FLOAT_4_1` | 28 |  |
| `CUSTOM_ELEMENT_TYPE_END` | 29 |  |

---
## <a id="enum-osdcustomelementtypevisibility_e"></a>`osdCustomElementTypeVisibility_e`

> Source: ../../../src/main/io/osd/custom_elements.h

| Enumerator | Value | Condition |
|---|---:|---|
| `CUSTOM_ELEMENT_VISIBILITY_ALWAYS` | 0 |  |
| `CUSTOM_ELEMENT_VISIBILITY_GV` | 1 |  |
| `CUSTOM_ELEMENT_VISIBILITY_LOGIC_CON` | 2 |  |

---
## <a id="enum-osddrawpointtype_e"></a>`osdDrawPointType_e`

> Source: ../../../src/main/io/osd_common.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_DRAW_POINT_TYPE_GRID` | 0 |  |
| `OSD_DRAW_POINT_TYPE_PIXEL` | 1 |  |

---
## <a id="enum-osddriver_e"></a>`osdDriver_e`

> Source: ../../../src/main/drivers/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_DRIVER_NONE` | 0 |  |
| `OSD_DRIVER_MAX7456` | 1 |  |

---
## <a id="enum-osdspeedsource_e"></a>`osdSpeedSource_e`

> Source: ../../../src/main/io/osd_common.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OSD_SPEED_SOURCE_GROUND` | 0 |  |
| `OSD_SPEED_SOURCE_3D` | 1 |  |
| `OSD_SPEED_SOURCE_AIR` | 2 |  |

---
## <a id="enum-outputmode_e"></a>`outputMode_e`

> Source: ../../../src/main/flight/mixer.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OUTPUT_MODE_AUTO` | 0 |  |
| `OUTPUT_MODE_MOTORS` | 1 |  |
| `OUTPUT_MODE_SERVOS` | 2 |  |
| `OUTPUT_MODE_LED` | 3 |  |

---
## <a id="enum-pageid_e"></a>`pageId_e`

> Source: ../../../src/main/io/dashboard.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PAGE_WELCOME` | 0 |  |
| `PAGE_ARMED` | 1 |  |
| `PAGE_STATUS` | 2 |  |

---
## <a id="enum-persistentobjectid_e"></a>`persistentObjectId_e`

> Source: ../../../src/main/drivers/persistent.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PERSISTENT_OBJECT_MAGIC` | 0 |  |
| `PERSISTENT_OBJECT_RESET_REASON` | 1 |  |
| `PERSISTENT_OBJECT_COUNT` | 2 |  |

---
## <a id="enum-pidautotunestate_e"></a>`pidAutotuneState_e`

> Source: ../../../src/main/flight/pid_autotune.c

| Enumerator | Value | Condition |
|---|---:|---|
| `DEMAND_TOO_LOW` | 0 |  |
| `DEMAND_UNDERSHOOT` | 1 |  |
| `DEMAND_OVERSHOOT` | 2 |  |
| `TUNE_UPDATED` | 3 |  |

---
## <a id="enum-pidcontrollerflags_e"></a>`pidControllerFlags_e`

> Source: ../../../src/main/common/fp_pid.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PID_DTERM_FROM_ERROR` | 1 << 0 |  |
| `PID_ZERO_INTEGRATOR` | 1 << 1 |  |
| `PID_SHRINK_INTEGRATOR` | 1 << 2 |  |
| `PID_LIMIT_INTEGRATOR` | 1 << 3 |  |
| `PID_FREEZE_INTEGRATOR` | 1 << 4 |  |

---
## <a id="enum-pidindex_e"></a>`pidIndex_e`

> Source: ../../../src/main/flight/pid.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PID_ROLL` | 0 |  |
| `PID_PITCH` | 1 |  |
| `PID_YAW` | 2 |  |
| `PID_POS_Z` | 3 |  |
| `PID_POS_XY` | 4 |  |
| `PID_VEL_XY` | 5 |  |
| `PID_SURFACE` | 6 |  |
| `PID_LEVEL` | 7 |  |
| `PID_HEADING` | 8 |  |
| `PID_VEL_Z` | 9 |  |
| `PID_POS_HEADING` | 10 |  |
| `PID_ITEM_COUNT` | 11 |  |

---
## <a id="enum-pidtype_e"></a>`pidType_e`

> Source: ../../../src/main/flight/pid.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PID_TYPE_NONE` | 0 |  |
| `PID_TYPE_PID` | 1 |  |
| `PID_TYPE_PIFF` | 2 |  |
| `PID_TYPE_AUTO` | 3 |  |

---
## <a id="enum-pinlabel_e"></a>`pinLabel_e`

> Source: ../../../src/main/drivers/pwm_mapping.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PIN_LABEL_NONE` | 0 |  |
| `PIN_LABEL_LED` | 1 |  |

---
## <a id="enum-pitotsensor_e"></a>`pitotSensor_e`

> Source: ../../../src/main/sensors/pitotmeter.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PITOT_NONE` | 0 |  |
| `PITOT_AUTODETECT` | 1 |  |
| `PITOT_MS4525` | 2 |  |
| `PITOT_ADC` | 3 |  |
| `PITOT_VIRTUAL` | 4 |  |
| `PITOT_FAKE` | 5 |  |
| `PITOT_MSP` | 6 |  |
| `PITOT_DLVR` | 7 |  |

---
## <a id="enum-polltype_e"></a>`pollType_e`

> Source: ../../../src/main/io/smartport_master.c

| Enumerator | Value | Condition |
|---|---:|---|
| `PT_ACTIVE_ID` | 0 |  |
| `PT_INACTIVE_ID` | 1 |  |

---
## <a id="enum-portsharing_e"></a>`portSharing_e`

> Source: ../../../src/main/io/serial.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PORTSHARING_UNUSED` | 0 |  |
| `PORTSHARING_NOT_SHARED` | 1 |  |
| `PORTSHARING_SHARED` | 2 |  |

---
## <a id="enum-pwminiterror_e"></a>`pwmInitError_e`

> Source: ../../../src/main/drivers/pwm_mapping.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PWM_INIT_ERROR_NONE` | 0 |  |
| `PWM_INIT_ERROR_TOO_MANY_MOTORS` | 1 |  |
| `PWM_INIT_ERROR_TOO_MANY_SERVOS` | 2 |  |
| `PWM_INIT_ERROR_NOT_ENOUGH_MOTOR_OUTPUTS` | 3 |  |
| `PWM_INIT_ERROR_NOT_ENOUGH_SERVO_OUTPUTS` | 4 |  |
| `PWM_INIT_ERROR_TIMER_INIT_FAILED` | 5 |  |

---
## <a id="enum-quadrant_e"></a>`quadrant_e`

> Source: ../../../src/main/io/ledstrip.c

| Enumerator | Value | Condition |
|---|---:|---|
| `QUADRANT_NORTH` | 1 << 0 |  |
| `QUADRANT_SOUTH` | 1 << 1 |  |
| `QUADRANT_EAST` | 1 << 2 |  |
| `QUADRANT_WEST` | 1 << 3 |  |
| `QUADRANT_NORTH_EAST` | 1 << 4 |  |
| `QUADRANT_SOUTH_EAST` | 1 << 5 |  |
| `QUADRANT_NORTH_WEST` | 1 << 6 |  |
| `QUADRANT_SOUTH_WEST` | 1 << 7 |  |
| `QUADRANT_NONE` | 1 << 8 |  |
| `QUADRANT_NOTDIAG` | 1 << 9 |  |
| `QUADRANT_ANY` | QUADRANT_NORTH | QUADRANT_SOUTH | QUADRANT_EAST | QUADRANT_WEST | QUADRANT_NONE |  |

---
## <a id="enum-quadspiclockdivider_e"></a>`QUADSPIClockDivider_e`

> Source: ../../../src/main/drivers/bus_quadspi.h

| Enumerator | Value | Condition |
|---|---:|---|
| `QUADSPI_CLOCK_INITIALISATION` | 255 |  |
| `QUADSPI_CLOCK_SLOW` | 19 |  |
| `QUADSPI_CLOCK_STANDARD` | 9 |  |
| `QUADSPI_CLOCK_FAST` | 3 |  |
| `QUADSPI_CLOCK_ULTRAFAST` | 1 |  |

---
## <a id="enum-quadspimode_e"></a>`quadSpiMode_e`

> Source: ../../../src/main/drivers/bus_quadspi.h

| Enumerator | Value | Condition |
|---|---:|---|
| `QUADSPI_MODE_BK1_ONLY` | 0 |  |
| `QUADSPI_MODE_BK2_ONLY` | 1 |  |
| `QUADSPI_MODE_DUAL_FLASH` | 2 |  |

---
## <a id="enum-rangefindertype_e"></a>`rangefinderType_e`

> Source: ../../../src/main/sensors/rangefinder.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RANGEFINDER_NONE` | 0 |  |
| `RANGEFINDER_SRF10` | 1 |  |
| `RANGEFINDER_VL53L0X` | 2 |  |
| `RANGEFINDER_MSP` | 3 |  |
| `RANGEFINDER_BENEWAKE` | 4 |  |
| `RANGEFINDER_VL53L1X` | 5 |  |
| `RANGEFINDER_US42` | 6 |  |
| `RANGEFINDER_TOF10102I2C` | 7 |  |
| `RANGEFINDER_FAKE` | 8 |  |
| `RANGEFINDER_TERARANGER_EVO` | 9 |  |
| `RANGEFINDER_USD1_V0` | 10 |  |
| `RANGEFINDER_NANORADAR` | 11 |  |

---
## <a id="enum-rcdevice_5key_connection_event_e"></a>`RCDEVICE_5key_connection_event_e`

> Source: ../../../src/main/io/rcdevice.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RCDEVICE_PROTOCOL_5KEY_CONNECTION_OPEN` | 1 |  |
| `RCDEVICE_PROTOCOL_5KEY_CONNECTION_CLOSE` | 2 |  |

---
## <a id="enum-rcdevice_5key_simulation_operation_e"></a>`rcdevice_5key_simulation_operation_e`

> Source: ../../../src/main/io/rcdevice.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RCDEVICE_PROTOCOL_5KEY_SIMULATION_NONE` | 0 |  |
| `RCDEVICE_PROTOCOL_5KEY_SIMULATION_SET` | 1 |  |
| `RCDEVICE_PROTOCOL_5KEY_SIMULATION_LEFT` | 2 |  |
| `RCDEVICE_PROTOCOL_5KEY_SIMULATION_RIGHT` | 3 |  |
| `RCDEVICE_PROTOCOL_5KEY_SIMULATION_UP` | 4 |  |
| `RCDEVICE_PROTOCOL_5KEY_SIMULATION_DOWN` | 5 |  |

---
## <a id="enum-rcdevice_camera_control_opeation_e"></a>`rcdevice_camera_control_opeation_e`

> Source: ../../../src/main/io/rcdevice.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RCDEVICE_PROTOCOL_CAM_CTRL_SIMULATE_WIFI_BTN` | 0 |  |
| `RCDEVICE_PROTOCOL_CAM_CTRL_SIMULATE_POWER_BTN` | 1 |  |
| `RCDEVICE_PROTOCOL_CAM_CTRL_CHANGE_MODE` | 2 |  |
| `RCDEVICE_PROTOCOL_CAM_CTRL_START_RECORDING` | 3 |  |
| `RCDEVICE_PROTOCOL_CAM_CTRL_STOP_RECORDING` | 4 |  |
| `RCDEVICE_PROTOCOL_CAM_CTRL_UNKNOWN_CAMERA_OPERATION` | 255 |  |

---
## <a id="enum-rcdevice_features_e"></a>`rcdevice_features_e`

> Source: ../../../src/main/io/rcdevice.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RCDEVICE_PROTOCOL_FEATURE_SIMULATE_POWER_BUTTON` | (1 << 0) |  |
| `RCDEVICE_PROTOCOL_FEATURE_SIMULATE_WIFI_BUTTON` | (1 << 1) |  |
| `RCDEVICE_PROTOCOL_FEATURE_CHANGE_MODE` | (1 << 2) |  |
| `RCDEVICE_PROTOCOL_FEATURE_SIMULATE_5_KEY_OSD_CABLE` | (1 << 3) |  |
| `RCDEVICE_PROTOCOL_FEATURE_START_RECORDING` | (1 << 6) |  |
| `RCDEVICE_PROTOCOL_FEATURE_STOP_RECORDING` | (1 << 7) |  |
| `RCDEVICE_PROTOCOL_FEATURE_CMS_MENU` | (1 << 8) |  |

---
## <a id="enum-rcdevice_protocol_version_e"></a>`rcdevice_protocol_version_e`

> Source: ../../../src/main/io/rcdevice.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RCDEVICE_PROTOCOL_RCSPLIT_VERSION` | 0 |  |
| `RCDEVICE_PROTOCOL_VERSION_1_0` | 1 |  |
| `RCDEVICE_PROTOCOL_UNKNOWN` | 2 |  |

---
## <a id="enum-rcdevicecamsimulationkeyevent_e"></a>`rcdeviceCamSimulationKeyEvent_e`

> Source: ../../../src/main/io/rcdevice.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RCDEVICE_CAM_KEY_NONE` | 0 |  |
| `RCDEVICE_CAM_KEY_ENTER` | 1 |  |
| `RCDEVICE_CAM_KEY_LEFT` | 2 |  |
| `RCDEVICE_CAM_KEY_UP` | 3 |  |
| `RCDEVICE_CAM_KEY_RIGHT` | 4 |  |
| `RCDEVICE_CAM_KEY_DOWN` | 5 |  |
| `RCDEVICE_CAM_KEY_CONNECTION_CLOSE` | 6 |  |
| `RCDEVICE_CAM_KEY_CONNECTION_OPEN` | 7 |  |
| `RCDEVICE_CAM_KEY_RELEASE` | 8 |  |

---
## <a id="enum-rcdeviceresponsestatus_e"></a>`rcdeviceResponseStatus_e`

> Source: ../../../src/main/io/rcdevice.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RCDEVICE_RESP_SUCCESS` | 0 |  |
| `RCDEVICE_RESP_INCORRECT_CRC` | 1 |  |
| `RCDEVICE_RESP_TIMEOUT` | 2 |  |

---
## <a id="enum-resolutiontype_e"></a>`resolutionType_e`

> Source: ../../../src/main/io/displayport_msp_osd.c

| Enumerator | Value | Condition |
|---|---:|---|
| `SD_3016` | 0 |  |
| `HD_5018` | 1 |  |
| `HD_3016` | 2 |  |
| `HD_6022` | 3 |  |
| `HD_5320` | 4 |  |

---
## <a id="enum-resourceowner_e"></a>`resourceOwner_e`

> Source: ../../../src/main/drivers/resource.h

| Enumerator | Value | Condition |
|---|---:|---|
| `OWNER_FREE` | 0 |  |
| `OWNER_PWMIO` | 1 |  |
| `OWNER_MOTOR` | 2 |  |
| `OWNER_SERVO` | 3 |  |
| `OWNER_SOFTSERIAL` | 4 |  |
| `OWNER_ADC` | 5 |  |
| `OWNER_SERIAL` | 6 |  |
| `OWNER_TIMER` | 7 |  |
| `OWNER_RANGEFINDER` | 8 |  |
| `OWNER_SYSTEM` | 9 |  |
| `OWNER_SPI` | 10 |  |
| `OWNER_QUADSPI` | 11 |  |
| `OWNER_I2C` | 12 |  |
| `OWNER_SDCARD` | 13 |  |
| `OWNER_FLASH` | 14 |  |
| `OWNER_USB` | 15 |  |
| `OWNER_BEEPER` | 16 |  |
| `OWNER_OSD` | 17 |  |
| `OWNER_BARO` | 18 |  |
| `OWNER_MPU` | 19 |  |
| `OWNER_INVERTER` | 20 |  |
| `OWNER_LED_STRIP` | 21 |  |
| `OWNER_LED` | 22 |  |
| `OWNER_RX` | 23 |  |
| `OWNER_TX` | 24 |  |
| `OWNER_VTX` | 25 |  |
| `OWNER_SPI_PREINIT` | 26 |  |
| `OWNER_COMPASS` | 27 |  |
| `OWNER_TEMPERATURE` | 28 |  |
| `OWNER_1WIRE` | 29 |  |
| `OWNER_AIRSPEED` | 30 |  |
| `OWNER_OLED_DISPLAY` | 31 |  |
| `OWNER_PINIO` | 32 |  |
| `OWNER_IRLOCK` | 33 |  |
| `OWNER_TOTAL_COUNT` | 34 |  |

---
## <a id="enum-resourcetype_e"></a>`resourceType_e`

> Source: ../../../src/main/drivers/resource.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RESOURCE_NONE` | 0 |  |
| `RESOURCE_INPUT` | 1 |  |
| `RESOURCE_TIMER` | 2 |  |
| `RESOURCE_UART_TX` | 3 |  |
| `RESOURCE_EXTI` | 4 |  |
| `RESOURCE_I2C_SCL` | 5 |  |
| `RESOURCE_SPI_SCK` | 6 |  |
| `RESOURCE_QUADSPI_CLK` | 7 |  |
| `RESOURCE_QUADSPI_BK1IO2` | 8 |  |
| `RESOURCE_QUADSPI_BK2IO0` | 9 |  |
| `RESOURCE_QUADSPI_BK2IO3` | 10 |  |
| `RESOURCE_ADC_CH1` | 11 |  |
| `RESOURCE_RX_CE` | 12 |  |
| `RESOURCE_TOTAL_COUNT` | 13 |  |

---
## <a id="enum-reversiblemotorsthrottlestate_e"></a>`reversibleMotorsThrottleState_e`

> Source: ../../../src/main/flight/mixer.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MOTOR_DIRECTION_FORWARD` | 0 |  |
| `MOTOR_DIRECTION_BACKWARD` | 1 |  |
| `MOTOR_DIRECTION_DEADBAND` | 2 |  |

---
## <a id="enum-rollpitchstatus_e"></a>`rollPitchStatus_e`

> Source: ../../../src/main/fc/rc_controls.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NOT_CENTERED` | 0 |  |
| `CENTERED` | 1 |  |

---
## <a id="enum-rssisource_e"></a>`rssiSource_e`

> Source: ../../../src/main/rx/rx.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RSSI_SOURCE_NONE` | 0 |  |
| `RSSI_SOURCE_AUTO` | 1 |  |
| `RSSI_SOURCE_ADC` | 2 |  |
| `RSSI_SOURCE_RX_CHANNEL` | 3 |  |
| `RSSI_SOURCE_RX_PROTOCOL` | 4 |  |
| `RSSI_SOURCE_MSP` | 5 |  |

---
## <a id="enum-rthstate_e"></a>`rthState_e`

> Source: ../../../src/main/flight/failsafe.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RTH_IDLE` | 0 |  |
| `RTH_IN_PROGRESS` | 1 |  |
| `RTH_HAS_LANDED` | 2 |  |

---
## <a id="enum-rthtargetmode_e"></a>`rthTargetMode_e`

> Source: ../../../src/main/navigation/navigation_private.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RTH_HOME_ENROUTE_INITIAL` | 0 |  |
| `RTH_HOME_ENROUTE_PROPORTIONAL` | 1 |  |
| `RTH_HOME_ENROUTE_FINAL` | 2 |  |
| `RTH_HOME_FINAL_LOITER` | 3 |  |
| `RTH_HOME_FINAL_LAND` | 4 |  |

---
## <a id="enum-rthtrackbackmode_e"></a>`rthTrackbackMode_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RTH_TRACKBACK_OFF` | 0 |  |
| `RTH_TRACKBACK_ON` | 1 |  |
| `RTH_TRACKBACK_FS` | 2 |  |

---
## <a id="enum-rxframestate_e"></a>`rxFrameState_e`

> Source: ../../../src/main/rx/rx.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RX_FRAME_PENDING` | 0 |  |
| `RX_FRAME_COMPLETE` | (1 << 0) |  |
| `RX_FRAME_FAILSAFE` | (1 << 1) |  |
| `RX_FRAME_PROCESSING_REQUIRED` | (1 << 2) |  |
| `RX_FRAME_DROPPED` | (1 << 3) |  |

---
## <a id="enum-rxreceivertype_e"></a>`rxReceiverType_e`

> Source: ../../../src/main/rx/rx.h

| Enumerator | Value | Condition |
|---|---:|---|
| `RX_TYPE_NONE` | 0 |  |
| `RX_TYPE_SERIAL` | 1 |  |
| `RX_TYPE_MSP` | 2 |  |
| `RX_TYPE_SIM` | 3 |  |

---
## <a id="enum-rxserialreceivertype_e"></a>`rxSerialReceiverType_e`

> Source: ../../../src/main/rx/rx.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SERIALRX_SPEKTRUM1024` | 0 |  |
| `SERIALRX_SPEKTRUM2048` | 1 |  |
| `SERIALRX_SBUS` | 2 |  |
| `SERIALRX_SUMD` | 3 |  |
| `SERIALRX_IBUS` | 4 |  |
| `SERIALRX_JETIEXBUS` | 5 |  |
| `SERIALRX_CRSF` | 6 |  |
| `SERIALRX_FPORT` | 7 |  |
| `SERIALRX_SBUS_FAST` | 8 |  |
| `SERIALRX_FPORT2` | 9 |  |
| `SERIALRX_SRXL2` | 10 |  |
| `SERIALRX_GHST` | 11 |  |
| `SERIALRX_MAVLINK` | 12 |  |
| `SERIALRX_FBUS` | 13 |  |
| `SERIALRX_SBUS2` | 14 |  |

---
## <a id="enum-safehomeusagemode_e"></a>`safehomeUsageMode_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SAFEHOME_USAGE_OFF` | 0 |  |
| `SAFEHOME_USAGE_RTH` | 1 |  |
| `SAFEHOME_USAGE_RTH_FS` | 2 |  |

---
## <a id="enum-sbasmode_e"></a>`sbasMode_e`

> Source: ../../../src/main/io/gps.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SBAS_AUTO` | 0 |  |
| `SBAS_EGNOS` | 1 |  |
| `SBAS_WAAS` | 2 |  |
| `SBAS_MSAS` | 3 |  |
| `SBAS_GAGAN` | 4 |  |
| `SBAS_SPAN` | 5 |  |
| `SBAS_NONE` | 6 |  |

---
## <a id="enum-sbusdecoderstate_e"></a>`sbusDecoderState_e`

> Source: ../../../src/main/rx/sbus.c

| Enumerator | Value | Condition |
|---|---:|---|
| `STATE_SBUS_SYNC` | 0 |  |
| `STATE_SBUS_PAYLOAD` | 1 |  |
| `STATE_SBUS26_PAYLOAD` | 2 |  |
| `STATE_SBUS_WAIT_SYNC` | 3 |  |

---
## <a id="enum-sdcardblockoperation_e"></a>`sdcardBlockOperation_e`

> Source: ../../../src/main/drivers/sdcard/sdcard.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SDCARD_BLOCK_OPERATION_READ` | 0 |  |
| `SDCARD_BLOCK_OPERATION_WRITE` | 1 |  |
| `SDCARD_BLOCK_OPERATION_ERASE` | 2 |  |

---
## <a id="enum-sdcardoperationstatus_e"></a>`sdcardOperationStatus_e`

> Source: ../../../src/main/drivers/sdcard/sdcard.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SDCARD_OPERATION_IN_PROGRESS` | 0 |  |
| `SDCARD_OPERATION_BUSY` | 1 |  |
| `SDCARD_OPERATION_SUCCESS` | 2 |  |
| `SDCARD_OPERATION_FAILURE` | 3 |  |

---
## <a id="enum-sdcardreceiveblockstatus_e"></a>`sdcardReceiveBlockStatus_e`

> Source: ../../../src/main/drivers/sdcard/sdcard_spi.c

| Enumerator | Value | Condition |
|---|---:|---|
| `SDCARD_RECEIVE_SUCCESS` | 0 |  |
| `SDCARD_RECEIVE_BLOCK_IN_PROGRESS` | 1 |  |
| `SDCARD_RECEIVE_ERROR` | 2 |  |

---
## <a id="enum-sdcardreceiveblockstatus_e"></a>`sdcardReceiveBlockStatus_e`

> Source: ../../../src/main/drivers/sdcard/sdcard_sdio.c

| Enumerator | Value | Condition |
|---|---:|---|
| `SDCARD_RECEIVE_SUCCESS` | 0 |  |
| `SDCARD_RECEIVE_BLOCK_IN_PROGRESS` | 1 |  |
| `SDCARD_RECEIVE_ERROR` | 2 |  |

---
## <a id="enum-sdcardstate_e"></a>`sdcardState_e`

> Source: ../../../src/main/drivers/sdcard/sdcard_impl.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SDCARD_STATE_NOT_PRESENT` | 0 |  |
| `SDCARD_STATE_RESET` | 1 |  |
| `SDCARD_STATE_CARD_INIT_IN_PROGRESS` | 2 |  |
| `SDCARD_STATE_INITIALIZATION_RECEIVE_CID` | 3 |  |
| `SDCARD_STATE_READY` | 4 |  |
| `SDCARD_STATE_READING` | 5 |  |
| `SDCARD_STATE_SENDING_WRITE` | 6 |  |
| `SDCARD_STATE_WAITING_FOR_WRITE` | 7 |  |
| `SDCARD_STATE_WRITING_MULTIPLE_BLOCKS` | 8 |  |
| `SDCARD_STATE_STOPPING_MULTIPLE_BLOCK_WRITE` | 9 |  |

---
## <a id="enum-sdiodevice"></a>`SDIODevice`

> Source: ../../../src/main/drivers/sdio.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SDIOINVALID` | -1 |  |
| `SDIODEV_1` | 0 |  |
| `SDIODEV_2` | 1 |  |

---
## <a id="enum-sensor_align_e"></a>`sensor_align_e`

> Source: ../../../src/main/drivers/sensor.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ALIGN_DEFAULT` | 0 |  |
| `CW0_DEG` | 1 |  |
| `CW90_DEG` | 2 |  |
| `CW180_DEG` | 3 |  |
| `CW270_DEG` | 4 |  |
| `CW0_DEG_FLIP` | 5 |  |
| `CW90_DEG_FLIP` | 6 |  |
| `CW180_DEG_FLIP` | 7 |  |
| `CW270_DEG_FLIP` | 8 |  |

---
## <a id="enum-sensorindex_e"></a>`sensorIndex_e`

> Source: ../../../src/main/sensors/sensors.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SENSOR_INDEX_GYRO` | 0 |  |
| `SENSOR_INDEX_ACC` | 1 |  |
| `SENSOR_INDEX_BARO` | 2 |  |
| `SENSOR_INDEX_MAG` | 3 |  |
| `SENSOR_INDEX_RANGEFINDER` | 4 |  |
| `SENSOR_INDEX_PITOT` | 5 |  |
| `SENSOR_INDEX_OPFLOW` | 6 |  |
| `SENSOR_INDEX_COUNT` | 7 |  |

---
## <a id="enum-sensors_e"></a>`sensors_e`

> Source: ../../../src/main/sensors/sensors.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SENSOR_GYRO` | 1 << 0 |  |
| `SENSOR_ACC` | 1 << 1 |  |
| `SENSOR_BARO` | 1 << 2 |  |
| `SENSOR_MAG` | 1 << 3 |  |
| `SENSOR_RANGEFINDER` | 1 << 4 |  |
| `SENSOR_PITOT` | 1 << 5 |  |
| `SENSOR_OPFLOW` | 1 << 6 |  |
| `SENSOR_GPS` | 1 << 7 |  |
| `SENSOR_GPSMAG` | 1 << 8 |  |
| `SENSOR_TEMP` | 1 << 9 |  |

---
## <a id="enum-sensortempcalstate_e"></a>`sensorTempCalState_e`

> Source: ../../../src/main/sensors/sensors.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SENSOR_TEMP_CAL_INITIALISE` | 0 |  |
| `SENSOR_TEMP_CAL_IN_PROGRESS` | 1 |  |
| `SENSOR_TEMP_CAL_COMPLETE` | 2 |  |

---
## <a id="enum-serialportfunction_e"></a>`serialPortFunction_e`

> Source: ../../../src/main/io/serial.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FUNCTION_NONE` | 0 |  |
| `FUNCTION_MSP` | (1 << 0) |  |
| `FUNCTION_GPS` | (1 << 1) |  |
| `FUNCTION_UNUSED_3` | (1 << 2) |  |
| `FUNCTION_TELEMETRY_HOTT` | (1 << 3) |  |
| `FUNCTION_TELEMETRY_LTM` | (1 << 4) |  |
| `FUNCTION_TELEMETRY_SMARTPORT` | (1 << 5) |  |
| `FUNCTION_RX_SERIAL` | (1 << 6) |  |
| `FUNCTION_BLACKBOX` | (1 << 7) |  |
| `FUNCTION_TELEMETRY_MAVLINK` | (1 << 8) |  |
| `FUNCTION_TELEMETRY_IBUS` | (1 << 9) |  |
| `FUNCTION_RCDEVICE` | (1 << 10) |  |
| `FUNCTION_VTX_SMARTAUDIO` | (1 << 11) |  |
| `FUNCTION_VTX_TRAMP` | (1 << 12) |  |
| `FUNCTION_UNUSED_1` | (1 << 13) |  |
| `FUNCTION_OPTICAL_FLOW` | (1 << 14) |  |
| `FUNCTION_LOG` | (1 << 15) |  |
| `FUNCTION_RANGEFINDER` | (1 << 16) |  |
| `FUNCTION_VTX_FFPV` | (1 << 17) |  |
| `FUNCTION_ESCSERIAL` | (1 << 18) |  |
| `FUNCTION_TELEMETRY_SIM` | (1 << 19) |  |
| `FUNCTION_FRSKY_OSD` | (1 << 20) |  |
| `FUNCTION_DJI_HD_OSD` | (1 << 21) |  |
| `FUNCTION_SERVO_SERIAL` | (1 << 22) |  |
| `FUNCTION_TELEMETRY_SMARTPORT_MASTER` | (1 << 23) |  |
| `FUNCTION_UNUSED_2` | (1 << 24) |  |
| `FUNCTION_MSP_OSD` | (1 << 25) |  |
| `FUNCTION_GIMBAL` | (1 << 26) |  |
| `FUNCTION_GIMBAL_HEADTRACKER` | (1 << 27) |  |

---
## <a id="enum-serialportidentifier_e"></a>`serialPortIdentifier_e`

> Source: ../../../src/main/io/serial.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SERIAL_PORT_NONE` | -1 |  |
| `SERIAL_PORT_USART1` | 0 |  |
| `SERIAL_PORT_USART2` | 1 |  |
| `SERIAL_PORT_USART3` | 2 |  |
| `SERIAL_PORT_USART4` | 3 |  |
| `SERIAL_PORT_USART5` | 4 |  |
| `SERIAL_PORT_USART6` | 5 |  |
| `SERIAL_PORT_USART7` | 6 |  |
| `SERIAL_PORT_USART8` | 7 |  |
| `SERIAL_PORT_USB_VCP` | 20 |  |
| `SERIAL_PORT_SOFTSERIAL1` | 30 |  |
| `SERIAL_PORT_SOFTSERIAL2` | 31 |  |
| `SERIAL_PORT_IDENTIFIER_MAX` | SERIAL_PORT_SOFTSERIAL2 |  |

---
## <a id="enum-servoautotrimstate_e"></a>`servoAutotrimState_e`

> Source: ../../../src/main/flight/servos.c

| Enumerator | Value | Condition |
|---|---:|---|
| `AUTOTRIM_IDLE` | 0 |  |
| `AUTOTRIM_COLLECTING` | 1 |  |
| `AUTOTRIM_SAVE_PENDING` | 2 |  |
| `AUTOTRIM_DONE` | 3 |  |

---
## <a id="enum-servoindex_e"></a>`servoIndex_e`

> Source: ../../../src/main/flight/servos.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SERVO_GIMBAL_PITCH` | 0 |  |
| `SERVO_GIMBAL_ROLL` | 1 |  |
| `SERVO_ELEVATOR` | 2 |  |
| `SERVO_FLAPPERON_1` | 3 |  |
| `SERVO_FLAPPERON_2` | 4 |  |
| `SERVO_RUDDER` | 5 |  |
| `SERVO_BICOPTER_LEFT` | 4 |  |
| `SERVO_BICOPTER_RIGHT` | 5 |  |
| `SERVO_DUALCOPTER_LEFT` | 4 |  |
| `SERVO_DUALCOPTER_RIGHT` | 5 |  |
| `SERVO_SINGLECOPTER_1` | 3 |  |
| `SERVO_SINGLECOPTER_2` | 4 |  |
| `SERVO_SINGLECOPTER_3` | 5 |  |
| `SERVO_SINGLECOPTER_4` | 6 |  |

---
## <a id="enum-servoprotocoltype_e"></a>`servoProtocolType_e`

> Source: ../../../src/main/drivers/pwm_mapping.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SERVO_TYPE_PWM` | 0 |  |
| `SERVO_TYPE_SBUS` | 1 |  |
| `SERVO_TYPE_SBUS_PWM` | 2 |  |

---
## <a id="enum-setting_mode_e"></a>`setting_mode_e`

> Source: ../../../src/main/fc/settings.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MODE_DIRECT` | (0 << SETTING_MODE_OFFSET) |  |
| `MODE_LOOKUP` | (1 << SETTING_MODE_OFFSET) |  |

---
## <a id="enum-setting_section_e"></a>`setting_section_e`

> Source: ../../../src/main/fc/settings.h

| Enumerator | Value | Condition |
|---|---:|---|
| `MASTER_VALUE` | (0 << SETTING_SECTION_OFFSET) |  |
| `PROFILE_VALUE` | (1 << SETTING_SECTION_OFFSET) |  |
| `CONTROL_RATE_VALUE` | (2 << SETTING_SECTION_OFFSET) |  |
| `BATTERY_CONFIG_VALUE` | (3 << SETTING_SECTION_OFFSET) |  |
| `MIXER_CONFIG_VALUE` | (4 << SETTING_SECTION_OFFSET) |  |
| `EZ_TUNE_VALUE` | (5 << SETTING_SECTION_OFFSET) |  |

---
## <a id="enum-setting_type_e"></a>`setting_type_e`

> Source: ../../../src/main/fc/settings.h

| Enumerator | Value | Condition |
|---|---:|---|
| `VAR_UINT8` | (0 << SETTING_TYPE_OFFSET) |  |
| `VAR_INT8` | (1 << SETTING_TYPE_OFFSET) |  |
| `VAR_UINT16` | (2 << SETTING_TYPE_OFFSET) |  |
| `VAR_INT16` | (3 << SETTING_TYPE_OFFSET) |  |
| `VAR_UINT32` | (4 << SETTING_TYPE_OFFSET) |  |
| `VAR_FLOAT` | (5 << SETTING_TYPE_OFFSET) |  |
| `VAR_STRING` | (6 << SETTING_TYPE_OFFSET) |  |

---
## <a id="enum-simatcommandstate_e"></a>`simATCommandState_e`

> Source: ../../../src/main/telemetry/sim.c

| Enumerator | Value | Condition |
|---|---:|---|
| `SIM_AT_OK` | 0 |  |
| `SIM_AT_ERROR` | 1 |  |
| `SIM_AT_WAITING_FOR_RESPONSE` | 2 |  |

---
## <a id="enum-simmodulestate_e"></a>`simModuleState_e`

> Source: ../../../src/main/telemetry/sim.c

| Enumerator | Value | Condition |
|---|---:|---|
| `SIM_MODULE_NOT_DETECTED` | 0 |  |
| `SIM_MODULE_NOT_REGISTERED` | 1 |  |
| `SIM_MODULE_REGISTERED` | 2 |  |

---
## <a id="enum-simreadstate_e"></a>`simReadState_e`

> Source: ../../../src/main/telemetry/sim.c

| Enumerator | Value | Condition |
|---|---:|---|
| `SIM_READSTATE_RESPONSE` | 0 |  |
| `SIM_READSTATE_SMS` | 1 |  |
| `SIM_READSTATE_SKIP` | 2 |  |

---
## <a id="enum-simtelemetrystate_e"></a>`simTelemetryState_e`

> Source: ../../../src/main/telemetry/sim.c

| Enumerator | Value | Condition |
|---|---:|---|
| `SIM_STATE_INIT` | 0 |  |
| `SIM_STATE_INIT2` | 1 |  |
| `SIM_STATE_INIT_ENTER_PIN` | 2 |  |
| `SIM_STATE_SET_MODES` | 3 |  |
| `SIM_STATE_SEND_SMS` | 4 |  |
| `SIM_STATE_SEND_SMS_ENTER_MESSAGE` | 5 |  |

---
## <a id="enum-simtransmissionstate_e"></a>`simTransmissionState_e`

> Source: ../../../src/main/telemetry/sim.c

| Enumerator | Value | Condition |
|---|---:|---|
| `SIM_TX_NO` | 0 |  |
| `SIM_TX_FS` | 1 |  |
| `SIM_TX` | 2 |  |

---
## <a id="enum-simtxflags_e"></a>`simTxFlags_e`

> Source: ../../../src/main/telemetry/sim.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SIM_TX_FLAG` | (1 << 0) |  |
| `SIM_TX_FLAG_FAILSAFE` | (1 << 1) |  |
| `SIM_TX_FLAG_GPS` | (1 << 2) |  |
| `SIM_TX_FLAG_ACC` | (1 << 3) |  |
| `SIM_TX_FLAG_LOW_ALT` | (1 << 4) |  |
| `SIM_TX_FLAG_RESPONSE` | (1 << 5) |  |

---
## <a id="enum-simulatorflags_t"></a>`simulatorFlags_t`

> Source: ../../../src/main/fc/runtime_config.h

| Enumerator | Value | Condition |
|---|---:|---|
| `HITL_RESET_FLAGS` | (0 << 0) |  |
| `HITL_ENABLE` | (1 << 0) |  |
| `HITL_SIMULATE_BATTERY` | (1 << 1) |  |
| `HITL_MUTE_BEEPER` | (1 << 2) |  |
| `HITL_USE_IMU` | (1 << 3) |  |
| `HITL_HAS_NEW_GPS_DATA` | (1 << 4) |  |
| `HITL_EXT_BATTERY_VOLTAGE` | (1 << 5) |  |
| `HITL_AIRSPEED` | (1 << 6) |  |
| `HITL_EXTENDED_FLAGS` | (1 << 7) |  |
| `HITL_GPS_TIMEOUT` | (1 << 8) |  |
| `HITL_PITOT_FAILURE` | (1 << 9) |  |

---
## <a id="enum-smartaudioversion_e"></a>`smartAudioVersion_e`

> Source: ../../../src/main/io/vtx_smartaudio.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SA_UNKNOWN` | 0 |  |
| `SA_1_0` | 1 |  |
| `SA_2_0` | 2 |  |
| `SA_2_1` | 3 |  |

---
## <a id="enum-smartportfuelunit_e"></a>`smartportFuelUnit_e`

> Source: ../../../src/main/telemetry/telemetry.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SMARTPORT_FUEL_UNIT_PERCENT` | 0 |  |
| `SMARTPORT_FUEL_UNIT_MAH` | 1 |  |
| `SMARTPORT_FUEL_UNIT_MWH` | 2 |  |

---
## <a id="enum-softserialportindex_e"></a>`softSerialPortIndex_e`

> Source: ../../../src/main/drivers/serial_softserial.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SOFTSERIAL1` | 0 |  |
| `SOFTSERIAL2` | 1 |  |

---
## <a id="enum-spiclockspeed_e"></a>`SPIClockSpeed_e`

> Source: ../../../src/main/drivers/bus_spi.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SPI_CLOCK_INITIALIZATON` | 0 |  |
| `SPI_CLOCK_SLOW` | 1 |  |
| `SPI_CLOCK_STANDARD` | 2 |  |
| `SPI_CLOCK_FAST` | 3 |  |
| `SPI_CLOCK_ULTRAFAST` | 4 |  |

---
## <a id="enum-srxl2bindrequest"></a>`Srxl2BindRequest`

> Source: ../../../src/main/rx/srxl2_types.h

| Enumerator | Value | Condition |
|---|---:|---|
| `EnterBindMode` | 235 |  |
| `RequestBindStatus` | 181 |  |
| `BoundDataReport` | 219 |  |
| `SetBindInfo` | 91 |  |

---
## <a id="enum-srxl2bindtype"></a>`Srxl2BindType`

> Source: ../../../src/main/rx/srxl2_types.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NotBound` | 0 |  |
| `DSM2_1024_22ms` | 1 |  |
| `DSM2_1024_MC24` | 2 |  |
| `DMS2_2048_11ms` | 18 |  |
| `DMSX_22ms` | 162 |  |
| `DMSX_11ms` | 178 |  |
| `Surface_DSM2_16_5ms` | 99 |  |
| `DSMR_11ms_22ms` | 226 |  |
| `DSMR_5_5ms` | 228 |  |

---
## <a id="enum-srxl2controldatacommand"></a>`Srxl2ControlDataCommand`

> Source: ../../../src/main/rx/srxl2_types.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ChannelData` | 0 |  |
| `FailsafeChannelData` | 1 |  |
| `VTXData` | 2 |  |

---
## <a id="enum-srxl2deviceid"></a>`Srxl2DeviceId`

> Source: ../../../src/main/rx/srxl2_types.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FlightControllerDefault` | 48 |  |
| `FlightControllerMax` | 63 |  |
| `Broadcast` | 255 |  |

---
## <a id="enum-srxl2devicetype"></a>`Srxl2DeviceType`

> Source: ../../../src/main/rx/srxl2_types.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NoDevice` | 0 |  |
| `RemoteReceiver` | 1 |  |
| `Receiver` | 2 |  |
| `FlightController` | 3 |  |
| `ESC` | 4 |  |
| `Reserved` | 5 |  |
| `SRXLServo` | 6 |  |
| `SRXLServo_2` | 7 |  |
| `VTX` | 8 |  |

---
## <a id="enum-srxl2packettype"></a>`Srxl2PacketType`

> Source: ../../../src/main/rx/srxl2_types.h

| Enumerator | Value | Condition |
|---|---:|---|
| `Handshake` | 33 |  |
| `BindInfo` | 65 |  |
| `ParameterConfiguration` | 80 |  |
| `SignalQuality` | 85 |  |
| `TelemetrySensorData` | 128 |  |
| `ControlData` | 205 |  |

---
## <a id="enum-srxl2state"></a>`Srxl2State`

> Source: ../../../src/main/rx/srxl2_types.h

| Enumerator | Value | Condition |
|---|---:|---|
| `Disabled` | 0 |  |
| `ListenForActivity` | 1 |  |
| `SendHandshake` | 2 |  |
| `ListenForHandshake` | 3 |  |
| `Running` | 4 |  |

---
## <a id="enum-stateflags_t"></a>`stateFlags_t`

> Source: ../../../src/main/fc/runtime_config.h

| Enumerator | Value | Condition |
|---|---:|---|
| `GPS_FIX_HOME` | (1 << 0) |  |
| `GPS_FIX` | (1 << 1) |  |
| `CALIBRATE_MAG` | (1 << 2) |  |
| `SMALL_ANGLE` | (1 << 3) |  |
| `FIXED_WING_LEGACY` | (1 << 4) |  |
| `ANTI_WINDUP` | (1 << 5) |  |
| `FLAPERON_AVAILABLE` | (1 << 6) |  |
| `NAV_MOTOR_STOP_OR_IDLE` | (1 << 7) |  |
| `COMPASS_CALIBRATED` | (1 << 8) |  |
| `ACCELEROMETER_CALIBRATED` | (1 << 9) |  |
| `GPS_ESTIMATED_FIX` | (1 << 10) | USE_GPS_FIX_ESTIMATION |
| `NAV_CRUISE_BRAKING` | (1 << 11) |  |
| `NAV_CRUISE_BRAKING_BOOST` | (1 << 12) |  |
| `NAV_CRUISE_BRAKING_LOCKED` | (1 << 13) |  |
| `NAV_EXTRA_ARMING_SAFETY_BYPASSED` | (1 << 14) |  |
| `AIRMODE_ACTIVE` | (1 << 15) |  |
| `ESC_SENSOR_ENABLED` | (1 << 16) |  |
| `AIRPLANE` | (1 << 17) |  |
| `MULTIROTOR` | (1 << 18) |  |
| `ROVER` | (1 << 19) |  |
| `BOAT` | (1 << 20) |  |
| `ALTITUDE_CONTROL` | (1 << 21) |  |
| `MOVE_FORWARD_ONLY` | (1 << 22) |  |
| `SET_REVERSIBLE_MOTORS_FORWARD` | (1 << 23) |  |
| `FW_HEADING_USE_YAW` | (1 << 24) |  |
| `ANTI_WINDUP_DEACTIVATED` | (1 << 25) |  |
| `LANDING_DETECTED` | (1 << 26) |  |
| `IN_FLIGHT_EMERG_REARM` | (1 << 27) |  |
| `TAILSITTER` | (1 << 28) |  |

---
## <a id="enum-stickpositions_e"></a>`stickPositions_e`

> Source: ../../../src/main/fc/rc_controls.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ROL_LO` | (1 << (2 * ROLL)) |  |
| `ROL_CE` | (3 << (2 * ROLL)) |  |
| `ROL_HI` | (2 << (2 * ROLL)) |  |
| `PIT_LO` | (1 << (2 * PITCH)) |  |
| `PIT_CE` | (3 << (2 * PITCH)) |  |
| `PIT_HI` | (2 << (2 * PITCH)) |  |
| `YAW_LO` | (1 << (2 * YAW)) |  |
| `YAW_CE` | (3 << (2 * YAW)) |  |
| `YAW_HI` | (2 << (2 * YAW)) |  |
| `THR_LO` | (1 << (2 * THROTTLE)) |  |
| `THR_CE` | (3 << (2 * THROTTLE)) |  |
| `THR_HI` | (2 << (2 * THROTTLE)) |  |

---
## <a id="enum-systemstate_e"></a>`systemState_e`

> Source: ../../../src/main/fc/fc_init.c

| Enumerator | Value | Condition |
|---|---:|---|
| `SYSTEM_STATE_INITIALISING` | 0 |  |
| `SYSTEM_STATE_CONFIG_LOADED` | (1 << 0) |  |
| `SYSTEM_STATE_SENSORS_READY` | (1 << 1) |  |
| `SYSTEM_STATE_MOTORS_READY` | (1 << 2) |  |
| `SYSTEM_STATE_TRANSPONDER_ENABLED` | (1 << 3) |  |
| `SYSTEM_STATE_READY` | (1 << 7) |  |

---
## <a id="enum-systemstate_e"></a>`systemState_e`

> Source: ../../../src/main/fc/fc_init.h

| Enumerator | Value | Condition |
|---|---:|---|
| `SYSTEM_STATE_INITIALISING` | 0 |  |
| `SYSTEM_STATE_CONFIG_LOADED` | (1 << 0) |  |
| `SYSTEM_STATE_SENSORS_READY` | (1 << 1) |  |
| `SYSTEM_STATE_MOTORS_READY` | (1 << 2) |  |
| `SYSTEM_STATE_TRANSPONDER_ENABLED` | (1 << 3) |  |
| `SYSTEM_STATE_READY` | (1 << 7) |  |

---
## <a id="enum-tchdmastate_e"></a>`tchDmaState_e`

> Source: ../../../src/main/drivers/timer.h

| Enumerator | Value | Condition |
|---|---:|---|
| `TCH_DMA_IDLE` | 0 |  |
| `TCH_DMA_READY` | 1 |  |
| `TCH_DMA_ACTIVE` | 2 |  |

---
## <a id="enum-tempsensortype_e"></a>`tempSensorType_e`

> Source: ../../../src/main/sensors/temperature.h

| Enumerator | Value | Condition |
|---|---:|---|
| `TEMP_SENSOR_NONE` | 0 |  |
| `TEMP_SENSOR_LM75` | 1 |  |
| `TEMP_SENSOR_DS18B20` | 2 |  |

---
## <a id="enum-throttlestatus_e"></a>`throttleStatus_e`

> Source: ../../../src/main/fc/rc_controls.h

| Enumerator | Value | Condition |
|---|---:|---|
| `THROTTLE_LOW` | 0 |  |
| `THROTTLE_HIGH` | 1 |  |

---
## <a id="enum-throttlestatustype_e"></a>`throttleStatusType_e`

> Source: ../../../src/main/fc/rc_controls.h

| Enumerator | Value | Condition |
|---|---:|---|
| `THROTTLE_STATUS_TYPE_RC` | 0 |  |
| `THROTTLE_STATUS_TYPE_COMMAND` | 1 |  |

---
## <a id="enum-timermode_e"></a>`timerMode_e`

> Source: ../../../src/main/drivers/serial_softserial.c

| Enumerator | Value | Condition |
|---|---:|---|
| `TIMER_MODE_SINGLE` | 0 |  |
| `TIMER_MODE_DUAL` | 1 |  |

---
## <a id="enum-timerusageflag_e"></a>`timerUsageFlag_e`

> Source: ../../../src/main/drivers/timer.h

| Enumerator | Value | Condition |
|---|---:|---|
| `TIM_USE_ANY` | 0 |  |
| `TIM_USE_PPM` | (1 << 0) |  |
| `TIM_USE_PWM` | (1 << 1) |  |
| `TIM_USE_MOTOR` | (1 << 2) |  |
| `TIM_USE_SERVO` | (1 << 3) |  |
| `TIM_USE_MC_CHNFW` | (1 << 4) |  |
| `TIM_USE_LED` | (1 << 24) |  |
| `TIM_USE_BEEPER` | (1 << 25) |  |

---
## <a id="enum-timid_e"></a>`timId_e`

> Source: ../../../src/main/io/ledstrip.c

| Enumerator | Value | Condition |
|---|---:|---|
| `timBlink` | 0 |  |
| `timLarson` | 1 |  |
| `timBattery` | 2 |  |
| `timRssi` | 3 |  |
| `timGps` | (4) | USE_GPS |
| `timWarning` | 5 |  |
| `timIndicator` | 6 |  |
| `timAnimation` | (7) | USE_LED_ANIMATION |
| `timRing` | 8 |  |
| `timTimerCount` | 9 |  |

---
## <a id="enum-tristate_e"></a>`tristate_e`

> Source: ../../../src/main/common/tristate.h

| Enumerator | Value | Condition |
|---|---:|---|
| `TRISTATE_AUTO` | 0 |  |
| `TRISTATE_ON` | 1 |  |
| `TRISTATE_OFF` | 2 |  |

---
## <a id="enum-tz_automatic_dst_e"></a>`tz_automatic_dst_e`

> Source: ../../../src/main/common/time.h

| Enumerator | Value | Condition |
|---|---:|---|
| `TZ_AUTO_DST_OFF` | 0 |  |
| `TZ_AUTO_DST_EU` | 1 |  |
| `TZ_AUTO_DST_USA` | 2 |  |

---
## <a id="enum-uartdevice_e"></a>`UARTDevice_e`

> Source: ../../../src/main/drivers/serial_uart.h

| Enumerator | Value | Condition |
|---|---:|---|
| `UARTDEV_1` | 0 |  |
| `UARTDEV_2` | 1 |  |
| `UARTDEV_3` | 2 |  |
| `UARTDEV_4` | 3 |  |
| `UARTDEV_5` | 4 |  |
| `UARTDEV_6` | 5 |  |
| `UARTDEV_7` | 6 |  |
| `UARTDEV_8` | 7 |  |
| `UARTDEV_MAX` | 8 |  |

---
## <a id="enum-uartinverterline_e"></a>`uartInverterLine_e`

> Source: ../../../src/main/drivers/uart_inverter.h

| Enumerator | Value | Condition |
|---|---:|---|
| `UART_INVERTER_LINE_NONE` | 0 |  |
| `UART_INVERTER_LINE_RX` | 1 << 0 |  |
| `UART_INVERTER_LINE_TX` | 1 << 1 |  |

---
## <a id="enum-ublox_nav_sig_health_e"></a>`ublox_nav_sig_health_e`

> Source: ../../../src/main/io/gps_ublox.h

| Enumerator | Value | Condition |
|---|---:|---|
| `UBLOX_SIG_HEALTH_UNKNOWN` | 0 |  |
| `UBLOX_SIG_HEALTH_HEALTHY` | 1 |  |
| `UBLOX_SIG_HEALTH_UNHEALTHY` | 2 |  |

---
## <a id="enum-ublox_nav_sig_quality"></a>`ublox_nav_sig_quality`

> Source: ../../../src/main/io/gps_ublox.h

| Enumerator | Value | Condition |
|---|---:|---|
| `UBLOX_SIG_QUALITY_NOSIGNAL` | 0 |  |
| `UBLOX_SIG_QUALITY_SEARCHING` | 1 |  |
| `UBLOX_SIG_QUALITY_ACQUIRED` | 2 |  |
| `UBLOX_SIG_QUALITY_UNUSABLE` | 3 |  |
| `UBLOX_SIG_QUALITY_CODE_LOCK_TIME_SYNC` | 4 |  |
| `UBLOX_SIG_QUALITY_CODE_CARRIER_LOCK_TIME_SYNC` | 5 |  |
| `UBLOX_SIG_QUALITY_CODE_CARRIER_LOCK_TIME_SYNC2` | 6 |  |
| `UBLOX_SIG_QUALITY_CODE_CARRIER_LOCK_TIME_SYNC3` | 7 |  |

---
## <a id="enum-ubs_nav_fix_type_t"></a>`ubs_nav_fix_type_t`

> Source: ../../../src/main/io/gps_ublox.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FIX_NONE` | 0 |  |
| `FIX_DEAD_RECKONING` | 1 |  |
| `FIX_2D` | 2 |  |
| `FIX_3D` | 3 |  |
| `FIX_GPS_DEAD_RECKONING` | 4 |  |
| `FIX_TIME` | 5 |  |

---
## <a id="enum-ubx_ack_state_t"></a>`ubx_ack_state_t`

> Source: ../../../src/main/io/gps_ublox.h

| Enumerator | Value | Condition |
|---|---:|---|
| `UBX_ACK_WAITING` | 0 |  |
| `UBX_ACK_GOT_ACK` | 1 |  |
| `UBX_ACK_GOT_NAK` | 2 |  |

---
## <a id="enum-ubx_nav_status_bits_t"></a>`ubx_nav_status_bits_t`

> Source: ../../../src/main/io/gps_ublox.h

| Enumerator | Value | Condition |
|---|---:|---|
| `NAV_STATUS_FIX_VALID` | 1 |  |

---
## <a id="enum-ubx_protocol_bytes_t"></a>`ubx_protocol_bytes_t`

> Source: ../../../src/main/io/gps_ublox.h

| Enumerator | Value | Condition |
|---|---:|---|
| `PREAMBLE1` | 181 |  |
| `PREAMBLE2` | 98 |  |
| `CLASS_NAV` | 1 |  |
| `CLASS_ACK` | 5 |  |
| `CLASS_CFG` | 6 |  |
| `CLASS_MON` | 10 |  |
| `MSG_CLASS_UBX` | 1 |  |
| `MSG_CLASS_NMEA` | 240 |  |
| `MSG_VER` | 4 |  |
| `MSG_ACK_NACK` | 0 |  |
| `MSG_ACK_ACK` | 1 |  |
| `MSG_NMEA_GGA` | 0 |  |
| `MSG_NMEA_GLL` | 1 |  |
| `MSG_NMEA_GSA` | 2 |  |
| `MSG_NMEA_GSV` | 3 |  |
| `MSG_NMEA_RMC` | 4 |  |
| `MSG_NMEA_VGS` | 5 |  |
| `MSG_POSLLH` | 2 |  |
| `MSG_STATUS` | 3 |  |
| `MSG_SOL` | 6 |  |
| `MSG_PVT` | 7 |  |
| `MSG_VELNED` | 18 |  |
| `MSG_TIMEUTC` | 33 |  |
| `MSG_SVINFO` | 48 |  |
| `MSG_NAV_SAT` | 53 |  |
| `MSG_CFG_PRT` | 0 |  |
| `MSG_CFG_RATE` | 8 |  |
| `MSG_CFG_SET_RATE` | 1 |  |
| `MSG_CFG_NAV_SETTINGS` | 36 |  |
| `MSG_CFG_SBAS` | 22 |  |
| `MSG_CFG_GNSS` | 62 |  |
| `MSG_MON_GNSS` | 40 |  |
| `MSG_NAV_SIG` | 67 |  |

---
## <a id="enum-vcselperiodtype_e"></a>`vcselPeriodType_e`

> Source: ../../../src/main/drivers/rangefinder/rangefinder_vl53l0x.c

| Enumerator | Value | Condition |
|---|---:|---|
| `VcselPeriodPreRange` | 0 |  |
| `VcselPeriodFinalRange` | 1 |  |

---
## <a id="enum-videosystem_e"></a>`videoSystem_e`

> Source: ../../../src/main/drivers/osd.h

| Enumerator | Value | Condition |
|---|---:|---|
| `VIDEO_SYSTEM_AUTO` | 0 |  |
| `VIDEO_SYSTEM_PAL` | 1 |  |
| `VIDEO_SYSTEM_NTSC` | 2 |  |
| `VIDEO_SYSTEM_HDZERO` | 3 |  |
| `VIDEO_SYSTEM_DJIWTF` | 4 |  |
| `VIDEO_SYSTEM_AVATAR` | 5 |  |
| `VIDEO_SYSTEM_DJICOMPAT` | 6 |  |
| `VIDEO_SYSTEM_DJICOMPAT_HD` | 7 |  |
| `VIDEO_SYSTEM_DJI_NATIVE` | 8 |  |

---
## <a id="enum-voltagesensor_e"></a>`voltageSensor_e`

> Source: ../../../src/main/sensors/battery_config_structs.h

| Enumerator | Value | Condition |
|---|---:|---|
| `VOLTAGE_SENSOR_NONE` | 0 |  |
| `VOLTAGE_SENSOR_ADC` | 1 |  |
| `VOLTAGE_SENSOR_ESC` | 2 |  |
| `VOLTAGE_SENSOR_FAKE` | 3 |  |
| `VOLTAGE_SENSOR_SMARTPORT` | 4 |  |
| `VOLTAGE_SENSOR_MAX` | VOLTAGE_SENSOR_SMARTPORT |  |

---
## <a id="enum-vs600band_e"></a>`vs600Band_e`

> Source: ../../../src/main/io/smartport_master.h

| Enumerator | Value | Condition |
|---|---:|---|
| `VS600_BAND_A` | 0 |  |
| `VS600_BAND_B` | 1 |  |
| `VS600_BAND_C` | 2 |  |
| `VS600_BAND_D` | 3 |  |
| `VS600_BAND_E` | 4 |  |
| `VS600_BAND_F` | 5 |  |

---
## <a id="enum-vs600power_e"></a>`vs600Power_e`

> Source: ../../../src/main/io/smartport_master.h

| Enumerator | Value | Condition |
|---|---:|---|
| `VS600_POWER_PIT` | 0 |  |
| `VS600_POWER_25MW` | 1 |  |
| `VS600_POWER_200MW` | 2 |  |
| `VS600_POWER_600MW` | 3 |  |

---
## <a id="enum-vtxdevtype_e"></a>`vtxDevType_e`

> Source: ../../../src/main/drivers/vtx_common.h

| Enumerator | Value | Condition |
|---|---:|---|
| `VTXDEV_UNSUPPORTED` | 0 |  |
| `VTXDEV_RTC6705` | 1 |  |
| `VTXDEV_SMARTAUDIO` | 3 |  |
| `VTXDEV_TRAMP` | 4 |  |
| `VTXDEV_FFPV` | 5 |  |
| `VTXDEV_MSP` | 6 |  |
| `VTXDEV_UNKNOWN` | 255 |  |

---
## <a id="enum-vtxfrequencygroups_e"></a>`vtxFrequencyGroups_e`

> Source: ../../../src/main/drivers/vtx_common.h

| Enumerator | Value | Condition |
|---|---:|---|
| `FREQUENCYGROUP_5G8` | 0 |  |
| `FREQUENCYGROUP_2G4` | 1 |  |
| `FREQUENCYGROUP_1G3` | 2 |  |

---
## <a id="enum-vtxlowerpowerdisarm_e"></a>`vtxLowerPowerDisarm_e`

> Source: ../../../src/main/io/vtx.h

| Enumerator | Value | Condition |
|---|---:|---|
| `VTX_LOW_POWER_DISARM_OFF` | 0 |  |
| `VTX_LOW_POWER_DISARM_ALWAYS` | 1 |  |
| `VTX_LOW_POWER_DISARM_UNTIL_FIRST_ARM` | 2 |  |

---
## <a id="enum-vtxprotoresponsetype_e"></a>`vtxProtoResponseType_e`

> Source: ../../../src/main/io/vtx_tramp.c

| Enumerator | Value | Condition |
|---|---:|---|
| `VTX_RESPONSE_TYPE_NONE` | 0 |  |
| `VTX_RESPONSE_TYPE_CAPABILITIES` | 1 |  |
| `VTX_RESPONSE_TYPE_STATUS` | 2 |  |

---
## <a id="enum-vtxprotostate_e"></a>`vtxProtoState_e`

> Source: ../../../src/main/io/vtx_tramp.c

| Enumerator | Value | Condition |
|---|---:|---|
| `VTX_STATE_RESET` | 0 |  |
| `VTX_STATE_OFFILE` | 1 |  |
| `VTX_STATE_DETECTING` | 2 |  |
| `VTX_STATE_IDLE` | 3 |  |
| `VTX_STATE_QUERY_DELAY` | 4 |  |
| `VTX_STATE_QUERY_STATUS` | 5 |  |
| `VTX_STATE_WAIT_STATUS` | 6 |  |

---
## <a id="enum-vtxscheduleparams_e"></a>`vtxScheduleParams_e`

> Source: ../../../src/main/io/vtx.c

| Enumerator | Value | Condition |
|---|---:|---|
| `VTX_PARAM_POWER` | 0 |  |
| `VTX_PARAM_BANDCHAN` | 1 |  |
| `VTX_PARAM_PITMODE` | 2 |  |
| `VTX_PARAM_COUNT` | 3 |  |

---
## <a id="enum-warningflags_e"></a>`warningFlags_e`

> Source: ../../../src/main/io/ledstrip.c

| Enumerator | Value | Condition |
|---|---:|---|
| `WARNING_ARMING_DISABLED` | 0 |  |
| `WARNING_LOW_BATTERY` | 1 |  |
| `WARNING_FAILSAFE` | 2 |  |
| `WARNING_HW_ERROR` | 3 |  |

---
## <a id="enum-warningledstate_e"></a>`warningLedState_e`

> Source: ../../../src/main/io/statusindicator.c

| Enumerator | Value | Condition |
|---|---:|---|
| `WARNING_LED_OFF` | 0 |  |
| `WARNING_LED_ON` | 1 |  |
| `WARNING_LED_FLASH` | 2 |  |

---
## <a id="enum-widgetahioptions_t"></a>`widgetAHIOptions_t`

> Source: ../../../src/main/drivers/display_widgets.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DISPLAY_WIDGET_AHI_OPTION_SHOW_CORNERS` | 1 << 0 |  |

---
## <a id="enum-widgetahistyle_e"></a>`widgetAHIStyle_e`

> Source: ../../../src/main/drivers/display_widgets.h

| Enumerator | Value | Condition |
|---|---:|---|
| `DISPLAY_WIDGET_AHI_STYLE_STAIRCASE` | 0 |  |
| `DISPLAY_WIDGET_AHI_STYLE_LINE` | 1 |  |

---
## <a id="enum-wpfwturnsmoothing_e"></a>`wpFwTurnSmoothing_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `WP_TURN_SMOOTHING_OFF` | 0 |  |
| `WP_TURN_SMOOTHING_ON` | 1 |  |
| `WP_TURN_SMOOTHING_CUT` | 2 |  |

---
## <a id="enum-wpmissionplannerstatus_e"></a>`wpMissionPlannerStatus_e`

> Source: ../../../src/main/navigation/navigation.h

| Enumerator | Value | Condition |
|---|---:|---|
| `WP_PLAN_WAIT` | 0 |  |
| `WP_PLAN_SAVE` | 1 |  |
| `WP_PLAN_OK` | 2 |  |
| `WP_PLAN_FULL` | 3 |  |

---
## <a id="enum-zerocalibrationstate_e"></a>`zeroCalibrationState_e`

> Source: ../../../src/main/common/calibration.h

| Enumerator | Value | Condition |
|---|---:|---|
| `ZERO_CALIBRATION_NONE` | 0 |  |
| `ZERO_CALIBRATION_IN_PROGRESS` | 1 |  |
| `ZERO_CALIBRATION_DONE` | 2 |  |
| `ZERO_CALIBRATION_FAIL` | 3 |  |
