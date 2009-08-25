#include <inttypes.h>

#include "std.h"
#include "init.h"
#include "sys_time.h"
#include "led.h"
#include "comm.h"

static inline void main_init( void );
static inline void main_periodic_task( void );
static inline void main_event_task( void );

#define USE_DA_USB 0
#define USE_DA_UART0 1
#define USE_DA_UART1 1


int main( void ) {
  main_init();
  while(1) {
    if (sys_time_periodic())
      main_periodic_task();
    main_event_task();
  }
  return 0;
}

static inline void main_init( void ) {
  hw_init();
  sys_time_init();
  led_init();

#if USE_DA_UART0
  /* Uart 0 (aka gps) */
  comm_init(COMM_0);
#endif

#if USE_DA_UART1
  /* Uart 1 (aka telemetry) */
  comm_init(COMM_1);
#endif

#if USE_DA_USB
  /* USB */
  comm_init(COMM_USB);
#endif

  int_enable();
}

static inline void main_periodic_task( void ) {
    static uint8_t c = 'a';

  RunOnceEvery(200, {
    led_toggle(4);

#if USE_DA_UART0
    comm_send_ch(COMM_0, c);
#endif

#if USE_DA_UART1
    comm_send_ch(COMM_1, c);
#endif

#if USE_DA_USB
    comm_send_ch(COMM_USB, c);
#endif

    c = (c < 'z' ? c + 1 : 'a');

  });

}

static inline void main_event_task( void ) {

}
