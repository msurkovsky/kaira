
#include "cailie.h"
#include <sched.h>
#include <stdlib.h>
#include <alloca.h>

#include "thread.h"

namespace ca {
    extern ControlSequence *control_sequence;
}

using namespace ca;

Thread::Thread(Process *process) : process(process), messages(NULL)
{
	pthread_mutex_init(&messages_mutex, NULL);
}

Thread::~Thread()
{
	pthread_mutex_destroy(&messages_mutex);
	if (tracelog) {
		delete tracelog;
	}
}

void Thread::add_message(ThreadMessage *message)
{
	pthread_mutex_lock(&messages_mutex);
	message->next = messages;
	messages = message;
	pthread_mutex_unlock(&messages_mutex);
}

void Thread::process_message(ThreadMessage *message)
{
	if (message->next) {
		process_message(message->next);
	}
	message->process(this);
	delete message;
}

bool Thread::process_thread_messages()
{
	if (messages) {
		ThreadMessage *m;
		pthread_mutex_lock(&messages_mutex);
		m = messages;
		messages = NULL;
		pthread_mutex_unlock(&messages_mutex);
		if (m != NULL) {
			process_message(m);
		}
		return true;
	}
	return false;
}

void Thread::clean_thread_messages()
{
	while(messages) {
		ThreadMessage *next = messages->next;
		delete messages;
		messages = next;
	}
}

int Thread::process_messages()
{
	int result = process_thread_messages();
	return process->process_packets(this) || result;
}

int Thread::process_messages(int from_process) {
    int result = process_thread_messages();
    return process->process_packets(this, from_process) || result;
}

static void * thread_run(void *data)
{
	Thread *thread = (Thread*) data;
	thread->run_scheduler();
	return NULL;
}

static void * thread_controlled_run(void *data) {
    Thread *thread = (Thread*) data;
    thread->run_control_sequence();
    return NULL;
}

void Thread::join()
{
	pthread_join(thread, NULL);
	#ifdef CA_MPI
	get_requests()->check();
	#endif
}

void Thread::start(bool controlled_run)
{
       CA_DLOG("Starting thread process=%i\n", get_process_id());
       if (controlled_run) {
           pthread_create(&thread, NULL, thread_controlled_run, this);
       } else {
           pthread_create(&thread, NULL, thread_run, this);
       }
}

void Thread::quit_all()
{
	if (get_tracelog()) {
		get_tracelog()->event_net_quit();
	}
	process->quit_all();
}

void Thread::run_scheduler()
{
	process_messages();
	bool in_idle = false;
	while(!process->quit_flag) {
		process_messages();
		Net *n = process->get_net();
		if (n == NULL) {
			continue;
		}

		Transition *tr = n->pick_active_transition();
		if (tr == NULL) {
			if (!in_idle && tracelog) {
				tracelog->event_idle();
			}
			in_idle = true;
			continue;
		}
		in_idle = false;
		CA_DLOG("Transition tried id=%i process=%i thread=%i\n",
				 tr->id, get_process_id(), id);
		int res = tr->full_fire(this, n);
		if (res == NOT_ENABLED) {
			CA_DLOG("Transition is not enabled id=%i process=%i thread=%i\n",
					 tr->id, get_process_id(), id);
			tr->set_active(false);
		}
	}
}

void Thread::run_one_step()
{
	process_messages();
	Net *net = process->get_net();
	if (net == NULL) {
		return;
	}
	Transition *tr = net->pick_active_transition();
	if (tr == NULL) {
		return;
	}
	tr->set_active(false);
	tr->full_fire(this, net);
}

void Thread::run_control_sequence() {

    if (control_sequence == NULL) {
        fprintf(stderr, "No control sequence available!\n");
        exit(1);
    }

    control_sequence->begin();

	bool in_idle = false;
    Command *cmd = control_sequence->next();
    while (cmd != NULL && !process->quit_flag) {
        Command::result_t res = cmd->run_command(this);
        if (res == Command::FAIL) {
            fprintf(stderr, "Command failed.\n");
            exit(1);
        }

        if (res == Command::REPEAT) {
            if (!in_idle && tracelog) {
                tracelog->event_idle();
            }
            in_idle = true;
            continue;
        }
        in_idle = false;
        cmd = control_sequence->next();
    }

    // computation may continue, so the program is finished as usual.
    run_scheduler();
}

Net * Thread::spawn_net(int def_index)
{
	return process->spawn_net(def_index, true);
}
