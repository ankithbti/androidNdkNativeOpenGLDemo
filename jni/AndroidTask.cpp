/*
 * AndroidTask.cpp
 *
 *  Created on: 18-Jul-2015
 *      Author: ankithbti
 */

#include "AndroidTask.hpp"

namespace playEngine {

AndroidTask::AndroidTask(android_app* app, size_t priority) :
		Task(priority), _app(app) {
	_app->onAppCmd = handle_cmd;

}

AndroidTask::~AndroidTask() {

}

bool AndroidTask::Start() {
	return true;
}

void AndroidTask::OnSuspend() {

}
void AndroidTask::Update() {
	int events;
	struct android_poll_source* pSource;

	int ident = ALooper_pollAll(0, 0, &events, (void**) &pSource);
	if (ident >= 0) {
		if (pSource) {
			pSource->process(_app, pSource);
		}

		if (_app->destroyRequested) {
			// tell the application to close
		}
	}
}
void AndroidTask::OnResume() {

}
void AndroidTask::Stop() {

}

}

