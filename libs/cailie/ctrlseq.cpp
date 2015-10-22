
#include <fstream>
#include <iostream>
#include "ctrlseq.h"

using namespace ca;
using namespace std;

const string ControlSequence::VERSION = "1.0";
const string ControlSequence::TYPE = "strict";

ControlSequence::ControlSequence(const string &path, int process_id) : idx(0) {
    read(path, process_id);
}

ControlSequence::~ControlSequence() {
    for (size_t i = 0; i < commands.size(); i++) {
        delete commands[i];
    }
}

ControlSequence::Header ControlSequence::read_header(const string &headerLine) {
    string tag, key, value;
    string parts[3] = {"", "", ""};
    ControlSequence::Header header;

    // take tag sequence without '<' and '>' signs
    stringstream line(headerLine.substr(1, headerLine.length() - 2));
    line >> tag >> parts[0] >> parts[1] >> parts[2];
    if (tag.compare("sequence") != 0) {
        fprintf(stderr, "Unknown format of control sequence header.\n");
        exit(1);
    }

    for (int i = 0; i < 3; i++) {
        size_t pos = parts[i].find("=");
        if (pos == string::npos) {
            fprintf(stderr, "Invalid format of control sequence header.\n");
            exit(1);
        }
        key = parts[i].substr(0, pos);
        value = parts[i].substr(pos+1, string::npos);
        value = value.substr(1, value.length() - 2); // trim quotations marks

        if (key.compare("name") == 0) {
            header.name = value;
        } else if (key.compare("type") == 0) {
            header.type = value;
        } else if (key.compare("version") == 0) {
            header.version = value;
        }
    }

    return header;
}

void ControlSequence::read(const string &path, int process_id) {

    string line;
    ifstream ctrlseq(path.c_str());

    if (ctrlseq.is_open()) {
        // read header
        getline(ctrlseq, line);
        ControlSequence::Header header = read_header(line);
        if (header.type.compare(ControlSequence::TYPE) != 0 ||
                header.version.compare(ControlSequence::VERSION) != 0) {
            fprintf(stderr, "Unsupported version or type of the sequence.\n");
            exit(1);
        }

        while (getline(ctrlseq, line)) {
            if (line.compare("</sequence>") == 0) {
                break;
            }

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
                    int transition_id;
                    cmd >> transition_id;
                    commands.push_back(
                        new CmdFinishTransition(process, transition_id));
                }
                case 'R' : {
                    int from_process_id;
                    cmd >> from_process_id;
                    commands.push_back(
                        new CmdReceive(process, from_process_id));
                } break;
                default:
                    fprintf(stderr, "Unknown action in control sequence.\n");
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
