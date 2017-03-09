/** \file
 * Userspace interface to the WS281x LED strip driver.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include "ledscape.h"
#include "pru.h"

/** GPIO pins used by the LEDscape.
 *
 * The device tree should handle this configuration for us, but it
 * seems horribly broken and won't configure these pins as outputs.
 * So instead we have to repeat them here as well.
 *
 * If these are changed, be sure to check the mappings in
 * ws281x.p!
 *
 * See https://github.com/ehayon/BeagleBone-GPIO/blob/master/src/am335x.h
 * for a complete list of pins.
 *
 * TODO: Find a way to unify this with the defines in the .p file
 */
static const uint8_t gpios0= 2;

#define ARRAY_COUNT(a) ((sizeof(a) / sizeof(*a)))

/** Command structure shared with the PRU.
 *
 * This is mapped into the PRU data RAM and points to the
 * frame buffer in the shared DDR segment.
 *
 * Changing this requires changes in ws281x.p
 */
typedef struct {
  // in the DDR shared with the PRU
  uintptr_t pixels_dma;

  // Length in pixels of the longest LED strip.
  unsigned num_pixels;

  // write 1 to start, 0xFF to abort. will be cleared when started
  volatile unsigned command;

  // will have a non-zero response written when done
  volatile unsigned response;
} __attribute__((__packed__)) ws281x_command_t;

struct ledscape {
  ws281x_command_t *ws281x;
  pru_t *pru0;
  unsigned num_pixels;
  size_t frame_size;
};

/** Retrieve one of the two frame buffers. */
ledscape_frame_t *ledscape_frame(ledscape_t *const leds, unsigned int frame) {
  if (frame >= 2)
    return NULL;

  return (ledscape_frame_t *)((uint8_t *)leds->pru0->ddr +
                              leds->frame_size * frame);
}

/** Initiate the transfer of a frame to the LED strips */
void ledscape_draw(ledscape_t *const leds, unsigned int frame) {
  leds->ws281x->pixels_dma = leds->pru0->ddr_addr + leds->frame_size * frame;

  // Wait for any current command to have been acknowledged
  while (leds->ws281x->command)
    ;

  // Send the start command
  leds->ws281x->command = 1;
}

/** Wait for the current frame to finish transfering to the strips.
 * \returns a token indicating the response code.
 */
uint32_t ledscape_wait(ledscape_t *const leds) {
  while (1) {
    uint32_t response0 = leds->ws281x->response;

    // printf("pru0: (%d,%d)\n",
    // 	leds->ws281x_0->command, leds->ws281x_0->response,
    // );

    if (response0) {
      leds->ws281x->response = 0;
      // TODO: How to handle both return values?
      return response0;
    }
  }
}

ledscape_t *ledscape_init(unsigned num_pixels) {
  pru_t *const pru0 = pru_init(0);

  const size_t frame_size = num_pixels * LEDSCAPE_NUM_STRIPS * 4;

  if (2 * frame_size > pru0->ddr_size)
    die("Pixel data needs at least 2 * %zu, only %zu in DDR\n", frame_size,
        pru0->ddr_size);

  ledscape_t *const leds = calloc(1, sizeof(*leds));

  *leds = (ledscape_t) { .pru0 = pru0,
                         .num_pixels = num_pixels,
                         .frame_size = frame_size,
                         .ws281x = pru0->data_ram, };

  *(leds->ws281x) =
      (ws281x_command_t) { .pixels_dma = 0, // will be set in draw routine
                           .command = 0,
                           .response = 0,
                           .num_pixels = leds->num_pixels, };

  // Configure all of our output pins.
    pru_gpio(0, gpios0, 1, 0);

  // Initiate the PRU0 program
  pru_exec(pru0, "./ws281x.bin");

  // Watch for a done response that indicates a proper startup
  // TODO: timeout if it fails
  fprintf(stdout, "waiting for response from pru0... ");
  while (!leds->ws281x->response)
    ;
  printf("OK\n");

  return leds;
}

void ledscape_close(ledscape_t *const leds) {
  // Signal a halt command
  leds->ws281x->command = 0xFF;
  pru_close(leds->pru0);
}

void ledscape_set_color(ledscape_frame_t *const frame, uint8_t strip,
                        uint8_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  ledscape_pixel_t *const p = &frame[pixel].strip[strip];
  p->r = r;
  p->g = g;
  p->b = b;
}
