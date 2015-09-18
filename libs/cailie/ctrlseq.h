
#ifndef CAILIE_CTRLSEQ_H
#define CAILIE_CTRLSEQ_H

#include "cs_command.h"

#include <string>
#include <vector>

namespace ca {

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
