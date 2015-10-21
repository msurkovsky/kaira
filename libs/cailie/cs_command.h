
#ifndef CAILIE_CS_COMMAND_H
#define CAILIE_CS_COMMAND_H

#include "thread.h"
#include "net.h"

namespace ca {

class Command {
    public:

        enum result_t { OK, FAIL, REPEAT, SKIP };

        Command (int process_id) : process_id(process_id) {}
        virtual ~Command() {}
        virtual result_t run_command(Thread *thread) = 0;
        int get_process_id() { return process_id; }

    protected:
        int process_id;

        void wait();
};


class CmdFireTransition : public Command {
    public:
        CmdFireTransition(int process, int transition_id)
            : Command(process), transition_id(transition_id) {}
        virtual result_t run_command(Thread *thread);

    protected:
        int transition_id;
};


class CmdStartTransition : public CmdFireTransition {
    public:
        CmdStartTransition(int process, int transition_id)
            : CmdFireTransition(process, transition_id) {}
        virtual result_t run_command(Thread *thread);
};


class CmdFinishTransition : public Command {
    public:
        CmdFinishTransition(int process) : Command(process) {}
        virtual result_t run_command(Thread *thread);
};


class CmdReceive : public Command {
    public:
        CmdReceive(int process, int from_process_id)
            : Command(process), from_process_id(from_process_id) {}
        virtual result_t run_command(Thread *thread);

    protected:
        int from_process_id;
};

}

#endif // CAILIE_CS_COMMAND_H
