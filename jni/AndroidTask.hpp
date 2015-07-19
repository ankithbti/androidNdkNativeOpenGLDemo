/*
 * AndroidTask.hpp
 *
 *  Created on: 18-Jul-2015
 *      Author: ankithbti
 */

#ifndef ANDROIDTASK_HPP_
#define ANDROIDTASK_HPP_

#include <android_native_app_glue.h>
#include <Task.hpp>

namespace playEngine {

class AndroidTask: public Task {
private:
	android_app * _app;
public:

	AndroidTask(android_app* pState, size_t priority);
	virtual ~AndroidTask();

	virtual bool Start();
	virtual void OnSuspend();
	virtual void Update();
	virtual void OnResume();
	virtual void Stop();
};

static void handle_cmd(struct android_app* app, int32_t cmd) {

	//*** Handle each command.
	switch (cmd) {
	case APP_CMD_GAINED_FOCUS:
		break;
	case APP_CMD_LOST_FOCUS:
		break;
	case APP_CMD_INIT_WINDOW:
		break;
	case APP_CMD_DESTROY:
		break;
	case APP_CMD_TERM_WINDOW:
		break;
	case APP_CMD_PAUSE:
		break;
	case APP_CMD_RESUME:
		break;
	}
}

}

#endif /* ANDROIDTASK_HPP_ */
