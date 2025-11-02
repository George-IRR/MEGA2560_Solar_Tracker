#ifndef SCHEDULER_H_
#define SCHEDULER_H_

void process_scheduled_work(void);

extern volatile bool task_pending_usart;

#endif /* SCHEDULER_H_ */