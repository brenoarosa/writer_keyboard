#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "print.h"
#include "debug.h"
#include "util.h"
#include "matrix.h"


#ifndef DEBOUNCE
#   define DEBOUNCE	5
#endif
static uint8_t debouncing = DEBOUNCE;

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];


static matrix_row_t read_cols(void);
static void init_cols(void);
static void unselect_rows(void);
static void select_row(uint8_t row);


inline
uint8_t matrix_rows(void)
{
    return MATRIX_ROWS;
}

inline
uint8_t matrix_cols(void)
{
    return MATRIX_COLS;
}

void matrix_init(void)
{
    // disable JTAG
    MCUCR |= (1 << JTD);
    MCUCR |= (1 << JTD);

    // initialize row and col
    unselect_rows();
    init_cols();

    // initialize matrix state: all keys off
    for (uint8_t i=0; i < MATRIX_ROWS; i++) {
        matrix[i] = 0;
        matrix_debouncing[i] = 0;
    }
}

uint8_t matrix_scan(void)
{
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        select_row(i);
        _delay_us(30);  // without this wait read unstable value.
        matrix_row_t cols = read_cols();
        if (matrix_debouncing[i] != cols) {
            matrix_debouncing[i] = cols;
            if (debouncing) {
                debug("bounce!: "); debug_hex(debouncing); debug("\n");
            }
            debouncing = DEBOUNCE;
        }
        unselect_rows();
    }

    if (debouncing) {
        if (--debouncing) {
            _delay_ms(1);
        } else {
            for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
                matrix[i] = matrix_debouncing[i];
            }
        }
    }

    return 1;
}

bool matrix_is_modified(void)
{
    if (debouncing) return false;
    return true;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & ((matrix_row_t)1<<col));
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

void matrix_print(void)
{
    print("\nr/c 0123456789ABCDEF\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        phex(row); print(": ");
        pbin_reverse16(matrix_get_row(row));
        print("\n");
    }
}

uint8_t matrix_key_count(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        count += bitpop16(matrix[i]);
    }
    return count;
}

static void init_cols(void) {
    DDRB  &= ~(1 << 7);
    PORTB |=  (1 << 7);
    DDRC  &= ~(1 << 6 | 1 << 7);
    PORTC |=  (1 << 6 | 1 << 7);
    DDRD  &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6 | 1 << 7);
    PORTD |=  (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6 | 1 << 7);
    DDRE  &= ~(1 << 2 | 1 << 6);
    PORTE |=  (1 << 2 | 1 << 6);
    DDRF  &= ~(1 << 0 | 1 << 1 | 1 << 4 | 1 << 5);
    PORTF |=  (1 << 0 | 1 << 1 | 1 << 4 | 1 << 5);
}

static matrix_row_t read_cols(void) {
    return
        (PINB & (1 << 7) ? 0 : (1UL <<  0)) |
        (PINC & (1 << 6) ? 0 : (1UL <<  1)) |
        (PINC & (1 << 7) ? 0 : (1UL <<  2)) |
        (PIND & (1 << 0) ? 0 : (1UL <<  3)) |
        (PIND & (1 << 1) ? 0 : (1UL <<  4)) |
        (PIND & (1 << 2) ? 0 : (1UL <<  5)) |
        (PIND & (1 << 3) ? 0 : (1UL <<  6)) |
        (PIND & (1 << 4) ? 0 : (1UL <<  7)) |
        (PIND & (1 << 5) ? 0 : (1UL <<  8)) |
        (PIND & (1 << 6) ? 0 : (1UL <<  9)) |
        (PIND & (1 << 7) ? 0 : (1UL << 10)) |
        (PINE & (1 << 2) ? 0 : (1UL << 11)) |
        (PINE & (1 << 6) ? 0 : (1UL << 12)) |
        (PINF & (1 << 0) ? 0 : (1UL << 13)) |
        (PINF & (1 << 1) ? 0 : (1UL << 14)) |
        (PINF & (1 << 4) ? 0 : (1UL << 15)) |
        (PINF & (1 << 5) ? 0 : (1UL << 16));
}

static void unselect_rows(void) {
    DDRB  &= ~0b01111110;
    PORTB &= ~0b01111110;
}

static void select_row(uint8_t row) {
    switch (row) {
        case 0:
            DDRB  |=  (1 << 1);
            PORTB &= ~(1 << 1);
            break;
        case 1:
            DDRB  |=  (1 << 2);
            PORTB &= ~(1 << 2);
            break;
        case 2:
            DDRB  |=  (1 << 3);
            PORTB &= ~(1 << 3);
            break;
        case 3:
            DDRB  |=  (1 << 4);
            PORTB &= ~(1 << 4);
            break;
        case 4:
            DDRB  |=  (1 << 5);
            PORTB &= ~(1 << 5);
            break;
        case 5:
            DDRB  |=  (1 << 6);
            PORTB &= ~(1 << 6);
            break;
    }
};