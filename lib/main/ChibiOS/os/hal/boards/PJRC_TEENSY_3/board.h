/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for the PJRC Teensy 3.0 board.
 */

/*
 * Board identifier.
 */
#define BOARD_PJRC_TEENSY_3
#define BOARD_NAME                  "PJRC Teensy 3.0"

/* External 16 MHz crystal with PLL for 48 MHz core/system clock. */
#define KINETIS_SYSCLK_FREQUENCY    48000000UL
#define KINETIS_MCG_MODE            KINETIS_MCG_MODE_PEE
#define KINETIS_XTAL_FREQUENCY      16000000UL

/*
 * IO pins assignments.
 */
#define PORTA_PIN0                  0
#define PORTA_PIN1                  1
#define PORTA_PIN2                  2
#define PORTA_PIN3                  3
#define PORTA_TEENSY_PIN33          4
#define PORTA_TEENSY_PIN24          5
#define PORTA_PIN6                  6
#define PORTA_PIN7                  7
#define PORTA_PIN8                  8
#define PORTA_PIN9                  9
#define PORTA_PIN10                 10
#define PORTA_PIN11                 11
#define PORTA_TEENSY_PIN3           12
#define PORTA_TEENSY_PIN4           13
#define PORTA_PIN14                 14
#define PORTA_PIN15                 15
#define PORTA_PIN16                 16
#define PORTA_PIN17                 17
#define PORTA_PIN18                 18
#define PORTA_PIN19                 19
#define PORTA_PIN20                 20
#define PORTA_PIN21                 21
#define PORTA_PIN22                 22
#define PORTA_PIN23                 23
#define PORTA_PIN24                 24
#define PORTA_PIN25                 25
#define PORTA_PIN26                 26
#define PORTA_PIN27                 27
#define PORTA_PIN28                 28
#define PORTA_PIN29                 29
#define PORTA_PIN30                 30
#define PORTA_PIN31                 31

#define PORTB_TEENSY_PIN16          0
#define PORTB_TEENSY_PIN17          1
#define PORTB_TEENSY_PIN19          2
#define PORTB_TEENSY_PIN18          3
#define PORTB_PIN4                  4
#define PORTB_PIN5                  5
#define PORTB_PIN6                  6
#define PORTB_PIN7                  7
#define PORTB_PIN8                  8
#define PORTB_PIN9                  9
#define PORTB_PIN10                 10
#define PORTB_PIN11                 11
#define PORTB_PIN12                 12
#define PORTB_PIN13                 13
#define PORTB_PIN14                 14
#define PORTB_PIN15                 15
#define PORTB_TEENSY_PIN0           16
#define PORTB_TEENSY_PIN1           17
#define PORTB_TEENSY_PIN32          18
#define PORTB_TEENSY_PIN25          19
#define PORTB_PIN20                 20
#define PORTB_PIN21                 21
#define PORTB_PIN22                 22
#define PORTB_PIN23                 23
#define PORTB_PIN24                 24
#define PORTB_PIN25                 25
#define PORTB_PIN26                 26
#define PORTB_PIN27                 27
#define PORTB_PIN28                 28
#define PORTB_PIN29                 29
#define PORTB_PIN30                 30
#define PORTB_PIN31                 31

#define PORTC_TEENSY_PIN15          0
#define PORTC_TEENSY_PIN22          1
#define PORTC_TEENSY_PIN23          2
#define PORTC_TEENSY_PIN9           3
#define PORTC_TEENSY_PIN10          4
#define PORTC_TEENSY_PIN13          5
#define PORTC_TEENSY_PIN11          6
#define PORTC_TEENSY_PIN12          7
#define PORTC_TEENSY_PIN28          8
#define PORTC_TEENSY_PIN27          9
#define PORTC_TEENSY_PIN29          10
#define PORTC_TEENSY_PIN30          11
#define PORTC_PIN12                 12
#define PORTC_PIN13                 13
#define PORTC_PIN14                 14
#define PORTC_PIN15                 15
#define PORTC_PIN16                 16
#define PORTC_PIN17                 17
#define PORTC_PIN18                 18
#define PORTC_PIN19                 19
#define PORTC_PIN20                 20
#define PORTC_PIN21                 21
#define PORTC_PIN22                 22
#define PORTC_PIN23                 23
#define PORTC_PIN24                 24
#define PORTC_PIN25                 25
#define PORTC_PIN26                 26
#define PORTC_PIN27                 27
#define PORTC_PIN28                 28
#define PORTC_PIN29                 29
#define PORTC_PIN30                 30
#define PORTC_PIN31                 31

#define PORTD_TEENSY_PIN2           0
#define PORTD_TEENSY_PIN14          1
#define PORTD_TEENSY_PIN7           2
#define PORTD_TEENSY_PIN8           3
#define PORTD_TEENSY_PIN6           4
#define PORTD_TEENSY_PIN20          5
#define PORTD_TEENSY_PIN21          6
#define PORTD_TEENSY_PIN5           7
#define PORTD_PIN8                  8
#define PORTD_PIN9                  9
#define PORTD_PIN10                 10
#define PORTD_PIN11                 11
#define PORTD_PIN12                 12
#define PORTD_PIN13                 13
#define PORTD_PIN14                 14
#define PORTD_PIN15                 15
#define PORTD_PIN16                 16
#define PORTD_PIN17                 17
#define PORTD_PIN18                 18
#define PORTD_PIN19                 19
#define PORTD_PIN20                 20
#define PORTD_PIN21                 21
#define PORTD_PIN22                 22
#define PORTD_PIN23                 23
#define PORTD_PIN24                 24
#define PORTD_PIN25                 25
#define PORTD_PIN26                 26
#define PORTD_PIN27                 27
#define PORTD_PIN28                 28
#define PORTD_PIN29                 29
#define PORTD_PIN30                 30
#define PORTD_PIN31                 31

#define PORTE_TEENSY_PIN31          0
#define PORTE_TEENSY_PIN26          1
#define PORTE_PIN2                  2
#define PORTE_PIN3                  3
#define PORTE_PIN4                  4
#define PORTE_PIN5                  5
#define PORTE_PIN6                  6
#define PORTE_PIN7                  7
#define PORTE_PIN8                  8
#define PORTE_PIN9                  9
#define PORTE_PIN10                 10
#define PORTE_PIN11                 11
#define PORTE_PIN12                 12
#define PORTE_PIN13                 13
#define PORTE_PIN14                 14
#define PORTE_PIN15                 15
#define PORTE_PIN16                 16
#define PORTE_PIN17                 17
#define PORTE_PIN18                 18
#define PORTE_PIN19                 19
#define PORTE_PIN20                 20
#define PORTE_PIN21                 21
#define PORTE_PIN22                 22
#define PORTE_PIN23                 23
#define PORTE_PIN24                 24
#define PORTE_PIN25                 25
#define PORTE_PIN26                 26
#define PORTE_PIN27                 27
#define PORTE_PIN28                 28
#define PORTE_PIN29                 29
#define PORTE_PIN30                 30
#define PORTE_PIN31                 31

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
