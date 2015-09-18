
#ifndef CAILIE_CTRLSEQ_H
#define CAILIE_CTRLSEQ_H

#include <string>
#include <vector>
#include "net.h"

namespace ca {

class Command {
    public:
        Command (int process_id) : process_id(process_id) {}
        virtual ~Command() {}
        virtual bool run_command(Net &net) = 0;

    protected:
        int process_id;
};


class CmdFireTransition : public Command {
    public:
        CmdFireTransition(int process, int transition_id)
            : Command(process), transition_id(transition_id) {}
        virtual bool run_command(Net &net);

    protected:
        int transition_id;
};


class CmdStartTransition : public CmdFireTransition {
    public:
        CmdStartTransition(int process, int transition_id)
            : CmdFireTransition(process, transition_id) {}
        virtual bool run_command(Net &net);
};


class CmdFinishTransition : public Command {
    public:
        CmdFinishTransition(int process) : Command(process) {}
        virtual bool run_command(Net &net);
};


class CmdReceive : public Command {
    public:
        CmdReceive(int process, int from_process_id)
            : Command(process), from_process_id(from_process_id) {}
        virtual bool run_command(Net &net);

    protected:
        int from_process_id;
};


class ControlSequence {

    public:
        enum {
            ANY_PROCESS = -1
        };

        ControlSequence() : idx(0) {}
        ControlSequence(const std::string &path, int process_id=ANY_PROCESS);
        virtual ~ControlSequence();
        virtual Command* next();
        virtual void begin() { idx = 0; }

    protected:
        virtual void read(const std::string &path, int process_id);
        std::vector<Command* > commands;

    private:
        size_t idx;
};

}
#endif // CAILIE_CTRLSEQ_H
