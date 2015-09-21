
#include "cs_command.h"
#include "cailie.h"

using namespace ca;

void Command::wait() {

}

Command::result_t CmdFireTransition::run_command(Thread *thread) {
    if (process_id != thread->get_process_id()) {
        return Command::SKIP;
    }

    if (thread == NULL) {
        return Command::FAIL;
    }

    Net *net = thread->get_process()->get_net();
    if (net == NULL) {
        return Command::REPEAT;
    }

    Transition *tr = net->get_transition(transition_id);
    if (tr == NULL) {
        return Command::REPEAT;
    }
    CA_DLOG("Transition tried id=%i process=%i thread=%i\n",
             tr->id, thread->get_process_id(), id);

    int res = tr->full_fire(thread, net);
    if (res == NOT_ENABLED) {
        CA_DLOG("Transition is not enabled id=%i process=%i thread=%i\n",
                 tr->id, get_process_id(), id);
        tr->set_active(false);
    }
    return Command::OK;
}

Command::result_t CmdStartTransition::run_command(Thread *thread) {
    return Command::OK;
}

Command::result_t CmdFinishTransition::run_command(Thread *thread) {
    return Command::OK;
}

Command::result_t CmdReceive::run_command(Thread *thread) {
    if (process_id != thread->get_process_id()) {
        return Command::SKIP;
    }
    thread->process_messages(from_process_id);
    return Command::OK;
}
