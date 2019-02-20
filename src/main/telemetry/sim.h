#define SIM_AT_COMMAND_MAX_SIZE 256
#define SIM_RESPONSE_BUFFER_SIZE 256
#define SIM_CYCLE_MS 5000 								// wait between sim command cycles
#define SIM_MIN_TRANSMISSION_INTERVAL 5
#define SIM_AT_COMMAND_DELAY_MS 3000
#define SIM_AT_COMMAND_DELAY_MIN_MS 500
#define SIM_STARTUP_DELAY_MS 10000
#define SIM_DEFAULT_TRANSMISSION_INTERVAL -60           // negative value: transmission off
#define SIM_SMS_COMMAND_TRANSMISSION    "T"
#define SIM_SMS_COMMAND_RTH             "RTH"
#define SIM_PIN "0000"

#define SIM_RESPONSE_CODE_OK    ('O' << 24 | 'K' << 16)
#define SIM_RESPONSE_CODE_ERROR ('E' << 24 | 'R' << 16 | 'R' << 8 | 'O')
#define SIM_RESPONSE_CODE_RING  ('R' << 24 | 'I' << 16 | 'N' << 8 | 'G')
#define SIM_RESPONSE_CODE_CLIP  ('C' << 24 | 'L' << 16 | 'I' << 8 | 'P')
#define SIM_RESPONSE_CODE_CSQ   ('C' << 24 | 'S' << 16 | 'Q' << 8 | ':')
#define SIM_RESPONSE_CODE_CMT   ('C' << 24 | 'M' << 16 | 'T' << 8 | ':')

typedef enum  {
    SIM_STATE_INIT = 0,
    SIM_STATE_INIT2,
    SIM_STATE_INIT_ENTER_PIN,
    SIM_STATE_SET_MODES,
    SIM_STATE_SEND_SMS,
    SIM_STATE_SEND_SMS_ENTER_MESSAGE
} simTelemetryState_e;

typedef enum  {
    SIM_AT_OK = 0,
    SIM_AT_ERROR,
    SIM_AT_WAITING_FOR_RESPONSE
} simATCommandState_e;

typedef enum {
    ACC_EVENT_NONE = 0,
    ACC_EVENT_HIGH,
    ACC_EVENT_LOW,
    ACC_EVENT_NEG_X
} accEvent_t;


void readSimResponse(void);
void handleSimTelemetry();
void freeSimTelemetryPort(void);
void initSimTelemetry(void);
void checkSimTelemetryState(void);
void configureSimTelemetryPort(void);
void sendATCommand(const char* command);
void requestSendSMS();
void detectAccEvents();
void detectFailsafe();
void transmit();
void sendSMS(void);
void readSMS(void);
void readOriginatingNumber(uint8_t*);
bool isGroundStationNumberDefined();
