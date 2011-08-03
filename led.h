#ifndef __LED_H__
#define __LED_H__

#define LED_MAGIC 'k'

#define LED_ON _IO(LED_MAGIC, 1)
#define LED_OFF _IO(LED_MAGIC, 2)

#endif
