# Five C Practice Projects — Function-by-Function Briefs

**Module:** Microcontroller-Based Systems (90 Hours)
**Author:** Ahmed Ellamie · ahmed.ellamiee@gmail.com
**Scope:** Days 01 – 03 (plain C only — no AVR, no hardware, no extra libraries)
**Environment:** Any C99 compiler · `gcc -std=c99 -Wall -Wextra -o app main.c`

> **How to read this file.** Every project gives you the story, the data, and
> the list of functions. Each function comes with a plain-words description of
> **what it must do** — never **how** to do it. The body is your job.
>
> Pick **one** project, write all of its functions, and make it run with zero
> warnings. Then pick another one.

---

## Rules for all five projects

1. One file, `main.c`. `#include <stdio.h>`, `<stdint.h>`, `<string.h>` only.
2. `main()` shows a menu in a `do-while` loop and calls functions. `main()`
   never touches the data directly.
3. Every function is `static`, and every function stays under 40 lines.
4. `scanf` returns a number. Check it. Bad input must never crash the program.


---

# Project 1 — My Piggy Bank

## The story 

You have a piggy bank. You drop coins in it. Sometimes you take coins out to
buy something. The piggy bank tells you how much money is inside, which coin
you have the most of, and whether you have enough for a toy.

## The data

```c
#define COIN_KINDS   5U
#define TOY_COUNT    4U
#define NAME_LEN    16U

/* the coins we know about, in piastres */
static const uint16_t COIN_VALUE[COIN_KINDS] = { 25U, 50U, 100U, 200U, 500U };

/* how many of each coin are in the bank right now */
static uint16_t coinCount[COIN_KINDS];

typedef struct {
    char     name[NAME_LEN];
    uint32_t price;        /* in piastres */
} Toy_t;

static Toy_t shop[TOY_COUNT];
```

## The functions — you write the bodies

```c
static void     seedBank(void);
static void     addCoins(void);
static void     takeCoins(void);
static uint32_t bankTotal(void);
static uint32_t sumCoins(const uint16_t *counts, uint8_t n);
static uint8_t  biggestPile(void);
static void     showBank(void);
static void     drawBar(uint16_t value, uint16_t full, uint8_t width);
static void     buyToy(void);
static void     bankReport(void);
```

| Function | What it must do, in easy words |
|---|---|
| `seedBank()` | Put some starting coins in the piggy bank and write the four toy names and prices into `shop`. This is the "new game" button. |
| `addCoins()` | Ask which coin (0–4) and how many. Drop them in. If the person types a coin that does not exist, say so and change nothing. |
| `takeCoins()` | Ask which coin and how many to take out. If the bank does not have that many, refuse and take **nothing**. Never let a count go below zero. |
| `bankTotal()` | Give back how much money is in the bank altogether, in piastres. Every coin counted, nothing left out. |
| `sumCoins()` | Add up the numbers in an array — but with **recursion**, no loop. Zero coins means zero. Otherwise: the last one plus all the ones before it. |
| `biggestPile()` | Tell which coin you have the most of, by giving back its position (0–4). If two are tied, the smaller position wins. |
| `showBank()` | Draw the bank: one line per coin kind showing the coin's value, how many you have, and a bar so you can *see* which pile is tallest. |
| `drawBar()` | Print exactly `width` characters: `#` for the full part, `-` for the empty part. A value bigger than `full` fills the whole bar and not one character more. |
| `buyToy()` | Show the four toys and their prices. Ask which one. If the bank has enough money, say "you can buy it" and how much is left over. If not, say how much more you need. Do not remove coins. |
| `bankReport()` | The summary page: total money, the number of coins altogether, the tallest pile's name, and how many of the four toys you can afford today. |

## When you are done, this must be true

- Taking out more coins than you own changes nothing at all.
- `sumCoins()` has no `for` and no `while` anywhere inside it.
- Typing `hello` at the menu prints one polite message, not an endless spin.
- An empty piggy bank shows a total of 0 and four toys you cannot afford.

---

# Project 2 — Traffic Light Robot

## The story 

There is one traffic light at a crossing. It goes green, then yellow, then red,
then green again — round and round. Cars wait at the red light. When a lot of
cars are waiting, the light stays green a bit longer. At night the light just
blinks yellow and everybody is careful.

## The data

```c
#define TICKS_GREEN    5U
#define TICKS_YELLOW   2U
#define TICKS_RED      4U
#define QUEUE_BUSY     6U      /* more cars than this means "busy" */
#define LOG_LEN       20U

typedef enum { LIGHT_GREEN = 0, LIGHT_YELLOW, LIGHT_RED } LightState_t;

/* status bits */
#define BIT_NIGHT      0U
#define BIT_BUSY       1U
#define BIT_BLINK_ON   2U

#define SET_BIT(reg, n)    ((reg) |=  (uint8_t)(1U << (n)))
#define CLR_BIT(reg, n)    ((reg) &= (uint8_t)~(1U << (n)))
#define TOGGLE_BIT(reg, n) ((reg) ^=  (uint8_t)(1U << (n)))
#define READ_BIT(reg, n)   ((uint8_t)(((reg) >> (n)) & 1U))

static LightState_t light;
static uint8_t      status;         /* NIGHT / BUSY / BLINK_ON       */
static uint8_t      ticksLeft;      /* time left in this colour      */
static uint8_t      carsWaiting;
static uint32_t     carsPassed;
static char         logLine[LOG_LEN];  /* last 20 colours as letters */
```

## The functions — you write the bodies

```c
static void         resetCrossing(void);
static uint8_t      ticksFor(LightState_t s);
static LightState_t nextState(LightState_t s);
static void         drawLight(void);
static void         tick(void);
static void         addCars(void);
static void         toggleNight(void);
static void         pushLog(char c);
static void         showLog(void);
static void         crossingReport(void);
```

| Function | What it must do, in easy words |
|---|---|
| `resetCrossing()` | Start fresh: the light is red, nobody has passed, no cars are waiting, it is daytime, and the log is empty. |
| `ticksFor()` | Say how long a colour lasts. Green lasts longer when the `BUSY` bit is set — two extra ticks. Yellow and red never change. |
| `nextState()` | Say which colour comes after this one. Green → yellow → red → green. Nothing else is allowed. |
| `drawLight()` | Draw the light standing up: three round lamps, and only the lamp that is on is filled in. Under it print the colour name, the ticks left, and how many cars are waiting. |
| `tick()` | One second of time passes. Take one off `ticksLeft`. When it hits zero, change to the next colour and refill the timer. While the light is green, let up to two waiting cars go through. At night, forget the colours and just flip the blink bit. Write one letter into the log every tick. |
| `addCars()` | Ask how many cars just arrived and add them to the queue. If the queue is now over `QUEUE_BUSY`, set the `BUSY` bit; if it drops back down, clear it. Refuse silly numbers. |
| `toggleNight()` | Flip the `NIGHT` bit. Going into night mode, the light starts blinking. Coming back to day, the light starts at red with a full timer. |
| `pushLog()` | Remember one letter (`G`, `Y`, `R`, or `y` for a night blink). The log keeps only the newest 20 letters — the oldest one falls off the front. |
| `showLog()` | Print the log as one line so you can read the light's history left to right, oldest first. |
| `crossingReport()` | Total ticks, cars passed, cars still waiting, whether it is night, whether it is busy, and the status byte printed in binary and in hex. |

## When you are done, this must be true

- Run 30 ticks in a row: the colours cycle green → yellow → red forever, and no
  colour ever lasts the wrong number of ticks.
- With 10 cars waiting, green lasts 7 ticks; with 2 cars waiting, 5 ticks.
- Night mode never shows green or red — only a blinking yellow.
- `light` is only ever changed by `nextState()`. Nothing else assigns it.

---

# Project 3 — Candy Shop Cashier

## The story 

You are the person behind the candy counter. Kids come and ask for candy. You
put candy in a basket, add up the price, take the money, and give change back.
At the end of the day you count how much you sold and which candy everybody
loved the most.

## The data

```c
#define CANDY_KINDS   6U
#define BASKET_MAX    8U
#define NAME_LEN     16U

typedef struct {
    char     name[NAME_LEN];
    uint16_t price;      /* piastres */
    uint16_t stock;      /* how many are left on the shelf */
    uint16_t sold;       /* how many we sold today         */
} Candy_t;

typedef struct {
    uint8_t  candyId;
    uint8_t  qty;
} Line_t;

static Candy_t shelf[CANDY_KINDS];
static Line_t  basket[BASKET_MAX];
static uint8_t basketLines;      /* how many slots of the basket are used */
static uint32_t cashDrawer;
```

## The functions — you write the bodies

```c
static void     openShop(void);
static void     showShelf(void);
static void     addToBasket(void);
static void     removeFromBasket(void);
static uint32_t basketTotal(void);
static void     showBasket(void);
static void     checkout(void);
static void     giveChange(uint32_t change);
static uint8_t  bestSeller(void);
static void     dayReport(void);
```

| Function | What it must do, in easy words |
|---|---|
| `openShop()` | Fill the shelf with six candies — a name, a price, and how many are in the box. Nothing has been sold yet, and the cash drawer is empty. |
| `showShelf()` | Print the shelf as a neat table: number, name, price, how many are left. If a candy has run out, write `SOLD OUT` instead of `0`. |
| `addToBasket()` | Ask which candy and how many. Say no if that candy does not exist, if the shelf does not have enough, or if the basket is already full. If the same candy is already in the basket, add to that line instead of making a new one. |
| `removeFromBasket()` | Ask which basket line to take out and remove it. The lines after it slide up so there is never a hole in the middle. |
| `basketTotal()` | Add up what the basket costs: every line's price times its quantity. An empty basket costs 0. |
| `showBasket()` | Print the basket like a receipt: one line per candy with the name, how many, the price each, and the line's cost. Then the total at the bottom. |
| `checkout()` | Show the total and ask how much money the kid handed over. If it is not enough, say so and keep the basket as it is. If it is enough, take the candy off the shelf, count it as sold, put the money in the drawer, give the change back, and empty the basket. |
| `giveChange()` | Say how to hand back the change using the fewest coins: how many 500s, 200s, 100s, 50s, and 25s. Zero change means "no change, thank you". |
| `bestSeller()` | Give back the position of the candy that sold the most today. If nothing has sold, position 0 is fine. |
| `dayReport()` | The end-of-day page: money in the drawer, how many candies were sold altogether, the best seller's name, and a list of everything that is now sold out. |

## When you are done, this must be true

- The shelf can never go negative, no matter what you type.
- Paying too little leaves the basket untouched — nothing is sold by accident.
- Change for 137 piastres is impossible with these coins; explain in your
  README what your program does about it and why.
- Adding the same candy twice makes **one** basket line, not two.

---

# Project 4 — Fluffy the Robot Pet

## The story 

Fluffy is a little robot pet who lives in your computer. You feed Fluffy, play
with Fluffy, and put Fluffy to bed. If you forget about Fluffy, it gets hungry
and sad and sleepy. Every time you press a key one hour goes by in Fluffy's
world, and Fluffy's face shows how it is feeling.

## The data

```c
#define STAT_MAX      10U
#define LOW_WARNING    3U
#define NAME_LEN      12U

/* mood bits */
#define BIT_ASLEEP     0U
#define BIT_HUNGRY     1U
#define BIT_SAD        2U
#define BIT_SICK       3U

#define SET_BIT(reg, n)    ((reg) |=  (uint8_t)(1U << (n)))
#define CLR_BIT(reg, n)    ((reg) &= (uint8_t)~(1U << (n)))
#define READ_BIT(reg, n)   ((uint8_t)(((reg) >> (n)) & 1U))

typedef struct {
    char     name[NAME_LEN];
    uint8_t  food;      /* 0..10 */
    uint8_t  fun;       /* 0..10 */
    uint8_t  energy;    /* 0..10 */
    uint8_t  mood;      /* the bits above  */
    uint16_t hours;     /* how long Fluffy has been alive */
} Pet_t;

static Pet_t fluffy;
```

## The functions — you write the bodies

```c
static void    hatchPet(void);
static void    drawFace(void);
static void    drawStat(const char *label, uint8_t value);
static void    feed(void);
static void    play(void);
static void    sleepPet(void);
static void    hourPasses(void);
static void    updateMood(void);
static uint8_t isHappy(void);
static void    petReport(void);
```

| Function | What it must do, in easy words |
|---|---|
| `hatchPet()` | A brand-new Fluffy: give it a name, fill food, fun, and energy right up to 10, clear every mood bit, and set the clock to zero hours. |
| `drawFace()` | Draw Fluffy's face in a little box. Sleeping shows closed eyes, sad shows a frown, sick shows a poorly face, happy shows a big smile. One face only — pick the most important feeling. |
| `drawStat()` | Print one bar: the label, then 10 boxes with the full ones filled in, then the number. Used for food, fun, and energy — write it once, call it three times. |
| `feed()` | Give Fluffy food. Food goes up by 3, but never above 10. You cannot feed a sleeping Fluffy — wake it up first. Eating a lot when food is already full makes Fluffy sick. |
| `play()` | Play with Fluffy. Fun goes up by 3 but energy goes down by 2 — playing is tiring. Never above 10 and never below 0. A sleeping Fluffy does not play. |
| `sleepPet()` | Put Fluffy to bed or wake it up. While asleep, every hour gives 2 energy back instead of taking it away. |
| `hourPasses()` | One hour of Fluffy's life. Food goes down by 1 and fun goes down by 1. Energy goes down by 1 if awake, or up by 2 if asleep. Nothing ever goes below 0 or above 10. Add one to the clock, then work out the new mood. |
| `updateMood()` | Look at the three numbers and set or clear each mood bit: low food means hungry, low fun means sad, no energy at all means Fluffy falls asleep whether you like it or not. Sick only goes away after Fluffy has slept. |
| `isHappy()` | Give back 1 when Fluffy is well: all three numbers above the warning line and no bad mood bits. Otherwise 0. |
| `petReport()` | The care card: name, hours alive, the three bars, the mood byte in binary, and one sentence saying what Fluffy needs next. |

## When you are done, this must be true

- Let 20 hours pass without helping: Fluffy ends hungry, sad, and asleep — and
  no number ever wraps around to 255.
- Feeding a full Fluffy makes it sick, and sickness only clears after sleep.
- Every `if` that sets a mood bit has an `else` that clears it — otherwise
  Fluffy stays sad forever after one bad hour.
- `mood` is only ever touched through the bit macros.

---

# Project 5 — The Secret Message Machine

## The story 

You and your friend want to send notes nobody else can read. So you shift every
letter along the alphabet: with a shift of 3, `A` becomes `D` and `cat` becomes
`fdw`. Your friend shifts it back and reads the note. The machine can also read
messages backwards, count the letters, and guess the secret shift all by
itself.

## The data

```c
#define MSG_MAX     128U
#define ALPHABET     26U

static char message[MSG_MAX];    /* what we are working on right now */
static char scratch[MSG_MAX];    /* somewhere to build the answer     */
static uint8_t shiftKey;         /* 0..25                             */
```

## The functions — you write the bodies

```c
static void     readMessage(void);
static uint16_t textLength(const char *s);
static char     shiftChar(char c, uint8_t key);
static void     encode(void);
static void     decode(void);
static void     reverseText(char *s, uint16_t n);
static uint16_t countVowels(const char *s);
static uint8_t  isPalindrome(const char *s);
static void     letterHistogram(const char *s);
static void     showMessage(void);
```

| Function | What it must do, in easy words |
|---|---|
| `readMessage()` | Ask for a sentence and keep it. Stop at the end of the line. Never write past the end of the box, even if somebody types a hundred words. |
| `textLength()` | Count the characters up to the end marker, without using `strlen`. An empty message is 0. |
| `shiftChar()` | Move one letter along the alphabet by the key. Past `Z` it wraps back to `A`; past `z` back to `a`. Capital letters stay capital. Spaces, numbers, and `!` come back exactly as they were. |
| `encode()` | Make the secret version of the message using the key, letter by letter, and show it. The original message stays as it was. |
| `decode()` | Turn a secret message back into a normal one. Shifting forward by `26 - key` is the same as shifting backwards — think about why before you write it. |
| `reverseText()` | Flip the message back to front **in place**: first letter swaps with last, second with second-last, and so on until you meet in the middle. No second array. |
| `countVowels()` | Count how many `a`, `e`, `i`, `o`, `u` are in the message. Big and small letters both count. |
| `isPalindrome()` | Say 1 if the message reads the same forwards and backwards, ignoring spaces and ignoring big/small letters. `Never odd or even` is a yes. |
| `letterHistogram()` | Count each letter of the alphabet and draw a bar chart, one row per letter that appears at least once. Letters nobody used are skipped so the chart stays short. |
| `showMessage()` | Print the message inside a frame with its length, the current key, and how many vowels it has. |

## When you are done, this must be true

- Encode with key 3, then decode with key 3, and you get the exact original
  back — including the punctuation.
- A key of 0 and a key of 26 both leave the text unchanged.
- `zebra` with key 1 becomes `afcsb`, not `{fcsb`. If you got the `{`, you
  forgot to wrap.
- Reversing twice gives you back what you started with.
- A 200-character line typed into a 128-character box does not crash anything.

---

## What to hand in

For the project you chose:

| File | What is in it |
|---|---|
| `main.c` | Your whole program, zero warnings with `-Wall -Wextra` |
| `README.md` | Which project, how to build it, and the answer to that project's "explain why" question |
| Screenshots | Three pictures: the fresh start, one interesting action, and the report page |

Put it in `09_Student_Submissions/Practice_Projects/<Your_Name>/`.

---

## Marking (100)

| Part | Marks | What earns them |
|---|---|---|
| It builds and runs | 15 | One command, no warnings, no crash |
| Every function done | 30 | All 10 do what the table says |
| Input is never trusted | 15 | Letters, negatives, and huge numbers are all handled |
| Numbers stay in range | 15 | Nothing goes below 0 or wraps past its top |
| Clean functions | 15 | Short, one job each, `main()` only dispatches |
| README and screenshots | 10 | Present, honest, and answers the question |

---

*Read the table, write one function, run it, then write the next one.
Ten small victories beat one big unfinished program.*

**Ahmed Ellamie** · ahmed.ellamiee@gmail.com
