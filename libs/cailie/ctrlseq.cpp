
#include <fstream>
#include <iostream>
#include "ctrlseq.h"

using namespace ca;
using namespace std;

bool CmdFireTransition::run_command(/*const Net &net*/) {
    std::cout << "Fire: " << process_id << "; " << transition_id << std::endl;
    return true;
}

bool CmdStartTransition::run_command(/*const Net &net*/) {
    std::cout << "Start: " << process_id << "; " << transition_id << std::endl;
    return true;
}

bool CmdFinishTransition::run_command(/*const Net &net*/) {
    std::cout << "Finish: " << process_id << std::endl;
    return true;
}

bool CmdReceive::run_command(/*const Net &net*/) {
    std::cout << "Receive: " << process_id << "; " << from_process_id << std::endl;
    return true;
}

ControlSequence::ControlSequence(const string &path) : idx(0) {
    read(path);
}

ControlSequence::~ControlSequence() {
    for (size_t i = 0; i < commands.size(); i++) {
        delete commands[i];
    }
}
void ControlSequence::read(const string &path) {

    string line;
    ifstream ctrlseq(path.c_str());

    if (ctrlseq.is_open()) {
        while (getline(ctrlseq, line)) {
            stringstream cmd(line);
            int process;
            char action;
            cmd >> process >> action;
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
