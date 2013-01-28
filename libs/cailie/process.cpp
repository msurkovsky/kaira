
#include "cailie.h"

#include <alloca.h>
#include <stdio.h>

#include <sstream>



namespace ca {
extern size_t trace_log_size;
extern char * project_description_string;
}

using namespace ca;

void Process::multisend(int target, Net *net, int place_pos, int tokens_count, const Packer &packer, Thread *thread)
{
	std::vector<int> a(1);
	a[0] = target;
	multisend_multicast(a, net, place_pos, tokens_count, packer, thread);
	/*
	#ifdef CA_MPI
		Packet *packet = (Packet*) packer.get_buffer();
		packet->unit_id = unit_id;
		packet->place_pos = place_pos;
		eacket->tokens_count = tokens_count;
		path.copy_to_mem(packet + 1);
		MPI_Request *request = requests.new_request(buffer);
		MPI_Isend(packet, packer.get_size(), MPI_CHAR, path.owner_id(process, unit_id), CA_MPI_TAG_TOKENS, MPI_COMM_WORLD, request);
	#else
	#endif */
}

void Process::process_packet(Thread *thread, int tag, void *data)
{
	if (tag == CA_TAG_SERVICE) {
		process_service_message(thread, (ServiceMessage*) data);
		free(data);
		return;
	}
	Tokens *tokens = (Tokens*) data;
	if(net == NULL) {
		CA_DLOG("Too early message on process=%d", get_process_id());
		too_early_message.push_back(data);
		return;
	}
	Unpacker unpacker(tokens + 1);
	Net *n = net;
	TraceLog *tracelog = thread->get_tracelog();
	if (tracelog) {
		tracelog->event_receive(tokens->msg_id);
	}
	if (n == NULL) {
		CA_DLOG("Net not found process=%i thread=%i\n",
			get_process_id(), thread->get_id());
		// Net is already stopped therefore we can throw tokens away
		return;
	}
	n->lock();
	int place_index = tokens->place_index;
	int tokens_count = tokens->tokens_count;
	CA_DLOG("RECV net=%i index=%i process=%i thread=%i\n",
		tokens->net_id, place_index, get_process_id(), thread->get_id());
	for (int t = 0; t < tokens_count; t++) {
		n->receive(thread, place_index, unpacker);
	}
	CA_DLOG("EOR index=%i process=%i thread=%i\n", place_index, get_process_id(), thread->get_id());
	n->unlock();
	free(data);
}

void Process::process_service_message(Thread *thread, ServiceMessage *smsg)
{
	switch (smsg->type) {
		case CA_SM_QUIT:
			CA_DLOG("SERVICE CA_SM_QUIT on process=%i thread=%i\n", get_process_id(), thread->get_id());
			if(net == NULL) {
				CA_DLOG("Quitting not created net on process=%d\n", get_process_id());
				net_is_quit = true;
			}
			too_early_message.clear();
			quit();
			break;
		case CA_SM_NET_CREATE:
		{
			CA_DLOG("SERVICE CA_SM_NET_CREATE on process=%i thread=%i\n", get_process_id(), thread->get_id());
			ServiceMessageNetCreate *m = (ServiceMessageNetCreate*) smsg;
			if(net_is_quit) {
				CA_DLOG("Stop creating quit net on process=%i thread=%i\n", get_process_id(), thread->get_id());
				too_early_message.clear();
				net_is_quit = false;
				break;
			}
			Net *net = (Net *) spawn_net(thread, m->def_index, false);
			net->unlock();
			if(too_early_message.size() > 0) {
				std::vector<void* >::const_iterator i;
				for (i = too_early_message.begin(); i != too_early_message.end(); i++) {
					process_packet(thread, CA_TAG_TOKENS, *i);
				}
				too_early_message.clear();
			}
			break;
		}
		case CA_SM_WAKE:
		{
			CA_DLOG("SERVICE CA_SM_WAKE on process=%i thread=%i\n", get_process_id(), thread->get_id());
			start_and_join();
			clear();
			#ifdef CA_MPI
			MPI_Barrier(MPI_COMM_WORLD);
			#endif
			break;
		}
		case CA_SM_EXIT:
			CA_DLOG("SERVICE CA_SM_EXIT on process=%i thread=%i\n", get_process_id(), thread->get_id());
			too_early_message.clear();
			free(smsg);
			exit(0);
	}
}

Net * Process::spawn_net(Thread *thread, int def_index, bool globally)
{
	TraceLog *tracelog = thread->get_tracelog();
	if (tracelog) {
		tracelog->event_net_spawn(defs[def_index]->get_id());
	}

	CA_DLOG("Spawning def_id=%i globally=%i\n",
		 def_index, globally);
	if (globally && !defs[def_index]->is_local()) {
		ServiceMessageNetCreate *m =
			(ServiceMessageNetCreate *) malloc(sizeof(ServiceMessageNetCreate));
		m->type = CA_SM_NET_CREATE;
		m->def_index = def_index;
		broadcast_packet(CA_TAG_SERVICE, m, sizeof(ServiceMessageNetCreate), thread, process_id);
	}

	net = (Net *) defs[def_index]->spawn(thread);
	net->lock();
	return net;
}


Process::Process(
	int process_id,
	int process_count,
	int threads_count,
	int defs_count,
	NetDef **defs)
{
	this->process_id = process_id;
	this->process_count = process_count;
	this->defs_count = defs_count;
	this->defs = defs;
	this->threads_count = threads_count;
	this->net_is_quit = false;
	this->quit_flag = false;
	threads = new Thread[threads_count];
	// TODO: ALLOCTEST
	for (int t = 0; t < threads_count; t++) {
		threads[t].set_process(this, t);
	}

	if (trace_log_size > 0) {

		if (process_id == 0) {
			FILE *f = fopen("trace.kth", "w");
			if (f == NULL) {
				perror("trace.kth");
				exit(-1);
			}
			ca::write_header(f, process_count, threads_count);
			fclose(f);
		}

		for (int t = 0; t < threads_count; t++) {
			std::stringstream s;
			s << "trace-" << process_id << "-" << t << ".ktt";
			threads[t].set_tracelog(new TraceLog(trace_log_size, s.str()), process_id * threads_count + t);
		}
	}

	#ifdef CA_SHMEM
	this->packets = NULL;
	pthread_mutex_init(&packet_mutex, NULL);
	#endif
}

Process::~Process()
{
	delete [] threads;

	#ifdef CA_SHMEM
	pthread_mutex_destroy(&packet_mutex);
	#endif
}

void Process::start()
{
	quit_flag = false;

	int t;
	for (t = 0; t < threads_count; t++) {
		threads[t].start();
	}
}

void Process::join()
{
	int t;
	for (t = 0; t < threads_count; t++) {
		threads[t].join();
	}
}

void Process::start_and_join()
{
	if (threads_count == 1) {
		// If there is only one process them process thread runs scheduler,
		// it is important because if threads_count == 1 we run MPI in MPI_THREAD_FUNELLED mode
		quit_flag = false;
		threads[0].run_scheduler();
	} else {
		start();
		join();
	}
}

void Process::clear()
{
	if(net != NULL && !net->get_manual_delete()) {
		delete net;
	}
	net = NULL;
}

Thread * Process::get_thread(int id)
{
	return &threads[id];
}

void Process::send_barriers(pthread_barrier_t *barrier1, pthread_barrier_t *barrier2)
{
	for (int t = 0; t < threads_count; t++) {
		threads[t].add_message(new ThreadMessageBarriers(barrier1, barrier2));
	}
}

void Process::quit_all(Thread *thread)
{
	ServiceMessage *m = (ServiceMessage*) malloc(sizeof(ServiceMessage));
	m->type = CA_SM_QUIT;
	broadcast_packet(CA_TAG_SERVICE, m, sizeof(ServiceMessage), thread, process_id);
	quit();
}

void Process::quit()
{
	quit_flag = true;
}

void Process::write_reports(FILE *out) const
{
	Output output(out);
	output.child("process");
	output.set("id", process_id);
	output.set("running", !quit_flag);

	net->write_reports(&threads[0], output);
	output.back();
}

// Designed for calling during simulation
void Process::fire_transition(int transition_id)
{
	net->fire_transition(&threads[0], transition_id);
}



