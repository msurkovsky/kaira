
#include "cs_command.h"

using namespace ca;

void Command::wait() {

}

Command::result_t CmdFireTransition::run_command(Thread *thread) {
    //Transition *tr = net.get_transition(transition_id);
    //if (tr == NULL) {
        //return false;
    //}
    //return true;
    return Command::OK;
}

Command::result_t CmdStartTransition::run_command(Thread *thread) {
    return Command::OK;
}

Command::result_t CmdFinishTransition::run_command(Thread *thread) {
    return Command::OK;
}

Command::result_t CmdReceive::run_command(Thread *thread) {
    return Command::OK;
}

