#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* =========================
   Constants
   ========================= */

#define TICKS_GREEN  5U
#define TICKS_YELLOW 2U
#define TICKS_RED    4U

#define QUEUE_BUSY 6U
#define LOG_LEN    20U

/* =========================
   Status bits
   ========================= */

#define BIT_NIGHT    0U
#define BIT_BUSY     1U
#define BIT_BLINK_ON 2U

/* =========================
   Bitwise Macros
   ========================= */

#define SET_BIT(reg, bit)    ((reg) |= (uint8_t)(1U << (bit)))
#define CLR_BIT(reg, bit)    ((reg) &= (uint8_t)~(1U << (bit)))
#define TOGGLE_BIT(reg, bit) ((reg) ^= (uint8_t)(1U << (bit)))
#define READ_BIT(reg, bit)   (((reg) >> (bit)) & 1U)

/* =========================
   Light State
   ========================= */

typedef enum
{
    LIGHT_GREEN = 0,
    LIGHT_YELLOW,
    LIGHT_RED
} LightState_t;

/* =========================
   Static Variables
   ========================= */

static LightState_t light;
static uint8_t status;
static uint16_t ticksLeft;
static uint16_t carsWaiting;
static uint32_t carsPassed;
static char logLine[LOG_LEN];

/* Additional variables */
static uint32_t totalTicks;
static uint8_t logCount;

/* =========================
   Function Prototypes
   ========================= */

static void resetCrossing(void);
static uint16_t ticksFor(LightState_t s);
static LightState_t nextState(LightState_t s);
static void drawLight(void);
static void tick(void);
static void addCars(void);
static void toggleNight(void);
static void pushLog(char c);
static void showLog(void);
static void crossingReport(void);

/* =========================
   resetCrossing
   ========================= */

static void resetCrossing(void)
{
    light = LIGHT_RED;
    status = 0U;

    ticksLeft = (uint16_t)TICKS_RED;
    carsWaiting = 0U;
    carsPassed = 0U;
    totalTicks = 0U;

    logCount = 0U;
    memset(logLine, 0, sizeof(logLine));

    /* Start in daytime */
    CLR_BIT(status, BIT_NIGHT);
    CLR_BIT(status, BIT_BUSY);
    CLR_BIT(status, BIT_BLINK_ON);
}

/* =========================
   ticksFor
   ========================= */

static uint16_t ticksFor(LightState_t s)
{
    switch (s)
    {
        case LIGHT_GREEN:
            /*
             * Green gets 2 extra ticks when BUSY is set.
             */
            if (READ_BIT(status, BIT_BUSY) != 0U)
            {
                return (uint16_t)(TICKS_GREEN + 2U);
            }

            return (uint16_t)TICKS_GREEN;

        case LIGHT_YELLOW:
            return (uint16_t)TICKS_YELLOW;

        case LIGHT_RED:
            return (uint16_t)TICKS_RED;

        default:
            return 0U;
    }
}

/* =========================
   nextState
   ========================= */

static LightState_t nextState(LightState_t s)
{
    switch (s)
    {
        case LIGHT_GREEN:
            return LIGHT_YELLOW;

        case LIGHT_YELLOW:
            return LIGHT_RED;

        case LIGHT_RED:
            return LIGHT_GREEN;

        default:
            return LIGHT_RED;
    }
}

/* =========================
   drawLight
   ========================= */

static void drawLight(void)
{
    printf("\n==============================\n");
    printf("       TRAFFIC LIGHT\n");
    printf("==============================\n");

    /*
     * NIGHT MODE:
     * Ignore the 'light' variable completely.
     * Green and Red are always OFF.
     * Yellow depends only on BLINK_ON.
     */
    if (READ_BIT(status, BIT_NIGHT) != 0U)
    {
        printf("   [ ] GREEN\n");

        if (READ_BIT(status, BIT_BLINK_ON) != 0U)
        {
            printf("   [O] YELLOW\n");
        }
        else
        {
            printf("   [ ] YELLOW\n");
        }

        printf("   [ ] RED\n");

        printf("\nCurrent light : NIGHT");
    }
    else
    {
        /*
         * DAY MODE:
         * Draw according to the current light state.
         */
        if (light == LIGHT_GREEN)
        {
            printf("   [O] GREEN\n");
            printf("   [ ] YELLOW\n");
            printf("   [ ] RED\n");

            printf("\nCurrent light : GREEN");
        }
        else if (light == LIGHT_YELLOW)
        {
            printf("   [ ] GREEN\n");
            printf("   [O] YELLOW\n");
            printf("   [ ] RED\n");

            printf("\nCurrent light : YELLOW");
        }
        else
        {
            printf("   [ ] GREEN\n");
            printf("   [ ] YELLOW\n");
            printf("   [O] RED\n");

            printf("\nCurrent light : RED");
        }
    }

    printf("\nTicks left    : %u",
           (unsigned int)ticksLeft);

    printf("\nCars waiting  : %u",
           (unsigned int)carsWaiting);

    printf("\nMode          : ");

    if (READ_BIT(status, BIT_NIGHT) != 0U)
    {
        printf("NIGHT");
    }
    else
    {
        printf("DAY");
    }

    printf("\nBusy          : ");

    if (READ_BIT(status, BIT_BUSY) != 0U)
    {
        printf("YES");
    }
    else
    {
        printf("NO");
    }

    printf("\nBlink         : ");

    if (READ_BIT(status, BIT_BLINK_ON) != 0U)
    {
        printf("ON");
    }
    else
    {
        printf("OFF");
    }

    printf("\n==============================\n");
}

/* =========================
   tick
   ========================= */

static void tick(void)
{
    totalTicks++;

    /*
     * NIGHT MODE
     *
     * At night:
     * - Do NOT decrement the timer.
     * - Do NOT change light state.
     * - Do NOT allow cars to pass.
     * - ONLY toggle BLINK_ON.
     * - Log 'y'.
     */
    if (READ_BIT(status, BIT_NIGHT) != 0U)
    {
        TOGGLE_BIT(status, BIT_BLINK_ON);

        pushLog('y');

        return;
    }

    /*
     * DAY MODE
     *
     * Log the current light every tick.
     */
    if (light == LIGHT_GREEN)
    {
        pushLog('G');
    }
    else if (light == LIGHT_YELLOW)
    {
        pushLog('Y');
    }
    else
    {
        pushLog('R');
    }

    /*
     * One second has passed.
     */
    if (ticksLeft > 0U)
    {
        ticksLeft--;
    }

    /*
     * During GREEN, up to two cars
     * can pass in one tick.
     */
    if (light == LIGHT_GREEN)
    {
        uint16_t passedThisTick = 0U;

        while ((passedThisTick < 2U) &&
               (carsWaiting > 0U))
        {
            carsWaiting--;
            carsPassed++;
            passedThisTick++;
        }
    }

    /*
     * Change light when timer reaches zero.
     */
    if (ticksLeft == 0U)
    {
        light = nextState(light);
        ticksLeft = ticksFor(light);
    }

    /*
     * Update BUSY bit.
     *
     * More than QUEUE_BUSY means BUSY.
     */
    if (carsWaiting > QUEUE_BUSY)
    {
        SET_BIT(status, BIT_BUSY);
    }
    else
    {
        CLR_BIT(status, BIT_BUSY);
    }

    /*
     * BLINK is only meaningful at night.
     */
    CLR_BIT(status, BIT_BLINK_ON);
}

/* =========================
   addCars
   ========================= */

static void addCars(void)
{
    unsigned int amount;

    printf("\nHow many cars arrived? ");

    if (scanf("%u", &amount) != 1)
    {
        printf("Invalid input.\n");

        while (getchar() != '\n')
        {
            /* Clear invalid input */
        }

        return;
    }

    /*
     * Prevent overflow of uint16_t.
     */
    if (amount > 65535U)
    {
        printf("Number is too large for uint16_t.\n");
        return;
    }

    if ((uint32_t)carsWaiting +
        (uint32_t)amount > 65535U)
    {
        printf("Queue overflow. Maximum waiting cars is 65535.\n");
        return;
    }

    carsWaiting = (uint16_t)
                  (carsWaiting + (uint16_t)amount);

    /*
     * BUSY if more than QUEUE_BUSY cars
     * are waiting.
     */
    if (carsWaiting > QUEUE_BUSY)
    {
        SET_BIT(status, BIT_BUSY);
    }
    else
    {
        CLR_BIT(status, BIT_BUSY);
    }

    printf("Cars waiting: %u\n",
           (unsigned int)carsWaiting);
}

/* =========================
   toggleNight
   ========================= */

static void toggleNight(void)
{
    TOGGLE_BIT(status, BIT_NIGHT);

    if (READ_BIT(status, BIT_NIGHT) != 0U)
    {
        /*
         * Enter NIGHT mode.
         *
         * Start blinking immediately.
         * Do not change light or timer.
         */
        SET_BIT(status, BIT_BLINK_ON);

        printf("\nNight mode ON.\n");
    }
    else
    {
        /*
         * Return to DAY mode.
         *
         * Start from RED with a full timer.
         */
        CLR_BIT(status, BIT_BLINK_ON);

        light = LIGHT_RED;
        ticksLeft = (uint16_t)TICKS_RED;

        printf("\nNight mode OFF.\n");
        printf("Traffic light returned to RED with full timer.\n");
    }
}

/* =========================
   pushLog
   ========================= */

static void pushLog(char c)
{
    if (logCount < LOG_LEN)
    {
        /*
         * Log is not full yet.
         */
        logLine[logCount] = c;
        logCount++;
    }
    else
    {
        /*
         * Log is full.
         *
         * Remove the oldest character,
         * shift everything left,
         * and append the new character.
         */
        memmove(&logLine[0],
                &logLine[1],
                LOG_LEN - 1U);

        logLine[LOG_LEN - 1U] = c;
    }
}

/* =========================
   showLog
   ========================= */

static void showLog(void)
{
    uint8_t i;

    printf("\nLog: ");

    if (logCount == 0U)
    {
        printf("(empty)");
    }
    else
    {
        /*
         * Print from oldest to newest.
         */
        for (i = 0U; i < logCount; i++)
        {
            putchar(logLine[i]);
        }
    }

    printf("\n");
}

/* =========================
   crossingReport
   ========================= */

static void crossingReport(void)
{
    int bit;

    printf("\n====================================\n");
    printf("       CROSSING REPORT\n");
    printf("====================================\n");

    printf("Total ticks       : %lu\n",
           (unsigned long)totalTicks);

    printf("Cars passed       : %lu\n",
           (unsigned long)carsPassed);

    printf("Cars waiting      : %u\n",
           (unsigned int)carsWaiting);

    printf("Mode              : ");

    if (READ_BIT(status, BIT_NIGHT) != 0U)
    {
        printf("NIGHT\n");
    }
    else
    {
        printf("DAY\n");
    }

    printf("Busy              : ");

    if (READ_BIT(status, BIT_BUSY) != 0U)
    {
        printf("YES\n");
    }
    else
    {
        printf("NO\n");
    }

    printf("Status byte       : ");

    /*
     * Print status as binary.
     */
    for (bit = 7; bit >= 0; bit--)
    {
        printf("%u",
               (unsigned int)((status >> bit) & 1U));
    }

    /*
     * Print status as hexadecimal.
     */
    printf(" (0x%02X)\n",
           (unsigned int)status);

    printf("====================================\n");
}

/* =========================
   main
   ========================= */

int main(void)
{
    unsigned int choice;

    resetCrossing();

    printf("====================================\n");
    printf("       TRAFFIC LIGHT ROBOT\n");
    printf("====================================\n");

    do
    {
        printf("\n");
        printf("1. Draw traffic light\n");
        printf("2. Tick (1 second)\n");
        printf("3. Add cars\n");
        printf("4. Toggle night/day\n");
        printf("5. Show log\n");
        printf("6. Crossing report\n");
        printf("7. Reset crossing\n");
        printf("0. Exit\n");
        printf("------------------------------------\n");
        printf("Choose: ");

        if (scanf("%u", &choice) != 1)
        {
            printf("Invalid choice.\n");

            while (getchar() != '\n')
            {
                /* Clear invalid input */
            }

            continue;
        }

        switch (choice)
        {
            case 1U:
                drawLight();
                break;

            case 2U:
                tick();
                printf("One second passed.\n");
                drawLight();
                break;

            case 3U:
                addCars();
                break;

            case 4U:
                toggleNight();
                break;

            case 5U:
                showLog();
                break;

            case 6U:
                crossingReport();
                break;

            case 7U:
                resetCrossing();
                printf("Crossing has been reset.\n");
                break;

            case 0U:
                printf("\nExiting program...\n");
                break;

            default:
                printf("Invalid choice.\n");
                break;
        }

    } while (choice != 0U);

    return 0;
}