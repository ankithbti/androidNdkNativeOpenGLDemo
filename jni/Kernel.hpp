/*
 * Kernel.hpp
 *
 *  Created on: 18-Jul-2015
 *      Author: ankithbti
 */

#ifndef KERNEL_HPP_
#define KERNEL_HPP_

#include <Task.hpp>
#include <list>

namespace playEngine {
class Kernel {
private:

	typedef std::list<Task*> TaskList;
	typedef std::list<Task*>::iterator TaskListIt;

	TaskList _activeTasks;
	TaskList _pausedTasks;

public:

	Kernel();
	virtual ~Kernel();

	void Execute();

	bool AddTask(Task* pTask);
	void SuspendTask(Task* task);
	void ResumeTask(Task* task);
	void RemoveTask(Task* task);
	void KillAllTasks();
};
}

#endif /* KERNEL_HPP_ */
