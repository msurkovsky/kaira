
#include <fstream>
#include "ctrlseq.h"

using namespace ca;
using namespace std;

ControlSequence::ControlSequence(const string &path, int process_id) : idx(0) {
    read(path, process_id);
}

ControlSequence::~ControlSequence() {
    for (size_t i = 0; i < commands.size(); i++) {
        delete commands[i];
    }
}

void ControlSequence::read(const string &path, int process_id) {

    string line;
    ifstream ctrlseq(path.c_str());

    if (ctrlseq.is_open()) {
        while (getline(ctrlseq, line)) {
            stringstream cmd(line);
            int process;
            char action;
            cmd >> process >> action;

            if (process_id != ANY_PROCESS && process_id != process) {
                continue;
            }

            switch (action) {
                case 'T': {
                    int transition_id;
                    cmd >> transition_id;
                    commands.push_back(
                        new CmdFireTransition(process, transition_id));
                } break;
                case 'S' : {
                    int transition_id;
                    cmd >> transition_id;
                    commands.push_back(
                        new CmdStartTransition(process, transition_id));
                } break;
                case 'F' : {
                    commands.push_back(
                        new CmdFinishTransition(process));
                }
                case 'R' : {
                    int from_process_id;
                    cmd >> from_process_id;
                    commands.push_back(
                        new CmdReceive(process, from_process_id));
                } break;
                default:
                    fprintf(stderr, "Unknow action in control sequence.\n");
                    exit(1);
            }
        }
        ctrlseq.close();
    }

}

Command* ControlSequence::next() {

    Command* cmd;
    if (idx < commands.size()) {
        cmd = commands[idx];
    } else {
        cmd = NULL;
    }
    idx += 1;
    return cmd;
}
