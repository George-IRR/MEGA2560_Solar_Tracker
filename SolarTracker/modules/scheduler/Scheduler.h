#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdbool.h>

void process_scheduled_work(void);

extern volatile bool task_pending_usart;
extern bool track_manual_block;  // add this line

#endif /* SCHEDULER_H_ */