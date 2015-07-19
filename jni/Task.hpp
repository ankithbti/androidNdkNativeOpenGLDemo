/*
 * Task.hpp
 *
 *  Created on: 18-Jul-2015
 *      Author: ankithbti
 */

#ifndef TASK_HPP_
#define TASK_HPP_

namespace playEngine {

class Task {
private:
	unsigned int _priority;
	bool _canKill;
public:
	Task(unsigned int priority) :
			_priority(priority), _canKill(false) {

	}

	virtual ~Task() {

	}

	void SetCanKill(bool canKill) {
		_canKill = canKill;
	}
	bool CanKill() {
		return _canKill;
	}
	unsigned int Priority() {
		return _priority;
	}


	// Abstract Functions
	virtual bool Start() = 0;
	virtual void OnSuspend() = 0;
	virtual void Update() = 0;
	virtual void OnResume() = 0;
	virtual void Stop() = 0;

};
}

#endif /* TASK_HPP_ */
