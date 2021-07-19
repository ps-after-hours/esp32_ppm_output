#define PPM_FRAME_LENGTH 22500
#define PPM_PULSE_LENGTH 300
#define PPM_CHANNELS 8
#define DEFAULT_CHANNEL_VALUE 1500
#define OUTPUT_PIN 14

uint16_t channelValue[PPM_CHANNELS] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

enum ppmState_e
{
    PPM_STATE_IDLE,
    PPM_STATE_PULSE,
    PPM_STATE_FILL,
    PPM_STATE_SYNC
};

void IRAM_ATTR onPpmTimer()
{

    static uint8_t ppmState = PPM_STATE_IDLE;
    static uint8_t ppmChannel = 0;
    static uint8_t ppmOutput = LOW;
    static int usedFrameLength = 0;
    int currentChannelValue;

    portENTER_CRITICAL(&timerMux);

    if (ppmState == PPM_STATE_IDLE)
    {
        ppmState = PPM_STATE_PULSE;
        ppmChannel = 0;
        usedFrameLength = 0;
        ppmOutput = LOW;
    }

    if (ppmState == PPM_STATE_PULSE)
    {
        ppmOutput = HIGH;
        usedFrameLength += PPM_PULSE_LENGTH;
        ppmState = PPM_STATE_FILL;

        timerAlarmWrite(timer, PPM_PULSE_LENGTH, true);
    }
    else if (ppmState == PPM_STATE_FILL)
    {
        ppmOutput = LOW;
        currentChannelValue = channelValue[ppmChannel];

        ppmChannel++;
        ppmState = PPM_STATE_PULSE;

        if (ppmChannel >= PPM_CHANNELS)
        {
            ppmChannel = 0;
            timerAlarmWrite(timer, PPM_FRAME_LENGTH - usedFrameLength, true);
            usedFrameLength = 0;
        }
        else
        {
            usedFrameLength += currentChannelValue - PPM_PULSE_LENGTH;
            timerAlarmWrite(timer, currentChannelValue - PPM_PULSE_LENGTH, true);
        }
    }
    portEXIT_CRITICAL(&timerMux);
    digitalWrite(OUTPUT_PIN, ppmOutput);
}

void setup()
{
    pinMode(OUTPUT_PIN, OUTPUT);
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onPpmTimer, true);
    timerAlarmWrite(timer, 12000, true);
    timerAlarmEnable(timer);
}

void loop()
{
    /*
    Here you can modify the content of channelValue array and it will be automatically
    picked up by the code and outputted as PPM stream. For example:
    */
    channelValue[0] = 1750;
    channelValue[1] = 1350;
    channelValue[2] = 1050;
    channelValue[3] = 1920;
}