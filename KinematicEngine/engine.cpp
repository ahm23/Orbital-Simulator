#pragma once
#include "engine.h"

ENGINE::ENGINE(int maxThreads, std::mutex* m, std::condition_variable_any* cv, int* status) {
	maxTh = maxThreads;
	threads.push_back(std::thread([this, m, cv, status] { ComputeWorker(1, m, cv, status, true); }));
	for (int i = 1; i < maxTh; i++)
		threads.push_back(std::thread([this, i, m, cv, status] { ComputeWorker(i + 2, m, cv, status, false); }));
	for (auto& th : threads) th.detach();
}

ENGINE::~ENGINE() {
	sig_terminate = true;
	threads.clear();
}

void ENGINE::ComputeWorker(int th_id, std::mutex* m, std::condition_variable_any* cv, int* status, bool lead) {
	Eigen::Vector3d temp_p;
	Eigen::Vector3d calc_p;
	Eigen::Vector3d calc_v;
	Eigen::Vector3d calc_a;
	int count_accelerator;
	uint64_t time;
	uint64_t time_diff;
	object el;
	std::vector<object> sys_compare;
	bool ac = false;
	while (true) {
		time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		// Engine Toggle Check
		if (!toggle) {
			if (lead) *status = 2;
			std::unique_lock<std::mutex> lk(*m);
			cv->wait(lk, [&] { if (!toggle) toggle = true;  return toggle; });
		}
		if (queue.empty()) {
			if (lead && working == 0) {
				if (!buffer_objects.size() && !buffer_sys_objects.size()) {
					std::this_thread::sleep_for(std::chrono::milliseconds(time_increment));
					continue;
				}
				if (buffer_objects.size() > buffer_capacity) {
					buffer_objects.erase(buffer_objects.begin());
					buffer_sys_objects.erase(buffer_sys_objects.begin());
					b_count--;
					if (time < buffer_sys_objects[1][0].time) {
						if (catchup) catchup = false;
						std::this_thread::sleep_for(std::chrono::milliseconds(buffer_sys_objects[1][0].time - time));
						continue;
					}
				}
				buffer_objects.push_back({});
				buffer_sys_objects.push_back({});
				b_count++;
				working = 0;
				m->lock();
				queue.insert(queue.end(), buffer_sys_objects[b_count - 1].begin(), buffer_sys_objects[b_count - 1].end());
				queue.insert(queue.end(), buffer_objects[b_count - 1].begin(), buffer_objects[b_count - 1].end());
				m->unlock();
				//queue = objects;
			}
			else {
				if (!catchup)
					std::this_thread::sleep_for(std::chrono::milliseconds(time_increment / 2));
				continue;
			}
		}
		while (!queue.empty()) {
			if (!ac) {
				working++;
				ac = true;
			}
			/*if (q_busy) continue;
			q_busy = th_id;

			if (q_busy != th_id) continue;
			//std::shared_lock<std::shared_mutex> lock(*m);
			//sys_compare = sys_objects;*/
			m->lock();
			if (queue.empty()) {
				m->unlock();
				continue;
			}
			else {
				el = queue.back();
				queue.pop_back();
				//q_busy = 0;
			}
			m->unlock();
			// v2.1 - Efficiency upgrade.
			calc_a << 0, 0, 0;
			count_accelerator = b_count - 1;
			for (int i = 0; i < buffer_sys_objects[count_accelerator].size(); i++) {
				if (el.id == buffer_sys_objects[count_accelerator][i].id) continue;
				temp_p = el.p - buffer_sys_objects[count_accelerator][i].p;
				calc_a += (calculate_mu(el.mass, buffer_sys_objects[count_accelerator][i].mass) / pow(temp_p.norm(), 3)) * temp_p;
			}
			el.v += calc_a * (double)(el.time + time_increment) / 1000 - calc_a * (double)el.time / 1000;
			el.p += el.v * (double)(el.time + time_increment) / 1000 - el.v * (double)el.time / 1000;
			el.time += time_increment;

			//object_sm e = { el.id, el.time, el.mass, el.p, el.v };

			m->lock();
			if (el.astronomical)
				(buffer_sys_objects.back()).push_back(el);
			else
				(buffer_objects.back()).push_back(el);
			m->unlock();
			time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - time;
		}
		
		if (ac) {
			ac = false;
			working--;
		}
		/*if (lead && queue.empty())
			q_busy = -1;*/

		if (sig_terminate)
			return;
		// not necessary
		//if (!catchup)
	}
}

int ENGINE::addObject(int id, B_INIT o) {
	if (b_count < 0) {
		buffer_objects.push_back({});
		buffer_sys_objects.push_back({});
		b_count++;
	}
	if (o.astronomical)
		buffer_sys_objects[b_count].push_back({id, o.astronomical, o.mass, o.time, o.pos, o.vel});
	else
		buffer_objects[b_count].push_back({ id, o.astronomical, o.mass, o.time, o.pos, o.vel });
	return 0;
}

void ENGINE::addToBuffer(std::mutex* m, object_sm* e, bool sys) {

}
