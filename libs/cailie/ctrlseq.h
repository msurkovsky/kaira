
#ifndef CAILIE_CTRLSEQ_H
#define CAILIE_CTRLSEQ_H

#include "cs_command.h"

#include <string>
#include <vector>

namespace ca {

class ControlSequence {

    public:
        static const std::string VERSION;
        static const std::string TYPE;

        enum {
            ANY_PROCESS = -1
        };

        struct Header {
            std::string name;
            std::string type;
            std::string version;
        };

        ControlSequence() : idx(0) {}
        ControlSequence(const std::string &path, int process_id=ANY_PROCESS);
        virtual ~ControlSequence();
        virtual Command* next();
        virtual void begin() { idx = 0; }

    protected:
        virtual ControlSequence::Header read_header(const std::string &header_line);
        virtual void read(const std::string &path, int process_id);
        std::vector<Command* > commands;

    private:
        size_t idx;
};

}

#endif // CAILIE_CTRLSEQ_H
