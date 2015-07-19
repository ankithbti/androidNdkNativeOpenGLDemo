#include <jni.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <Logger.hpp>

using namespace playEngine;

/**
 * Our saved state data.
 */
struct AppSavedState {
	float angle;
	int32_t x;
	int32_t y;
};

struct AppState {

	struct android_app* _app;

	ASensorManager* _sensorManager;
	const ASensor* _accelerometerSensor;
	ASensorEventQueue* _sensorEventQueue;

	int _animating;
	EGLDisplay _display;
	EGLSurface _surface;
	EGLContext _context;
	int32_t _width;
	int32_t _height;
	struct AppSavedState _state;

};

static void printGLString(const char *name, GLenum s) {
	const char *v = (const char *) glGetString(s);
	LOG_INFO("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
	for (GLint error = glGetError(); error; error = glGetError()) {
		LOG_ERROR("after %s() glError (0x%x)\n", op, error);
	}
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* androidApp,
		AInputEvent* event) {

	struct AppState * app = (struct AppState *) androidApp->userData;

	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		app->_animating = 1;
		app->_state.x = AMotionEvent_getX(event, 0);
		app->_state.y = AMotionEvent_getY(event, 0);
		LOG_INFO("INPUT_EVENT_TYPE_MOTION x: %f y: %f",
				AMotionEvent_getRawX(event, 0), AMotionEvent_getRawY(event, 0));
		return 1;
	}
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
		LOG_INFO("INPUT_EVENT_TYPE_KEY");
		return 1;
	}
	return 0;
}

static int initDisplay(struct AppState* app) {
	// initialize OpenGL ES and EGL
	/*
	 * Here specify the attributes of the desired configuration.
	 * Below, we select an EGLConfig with at least 8 bits per color
	 * component compatible with on-screen windows
	 */
	const EGLint attribs[] = {
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_BLUE_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_RED_SIZE, 8,
	EGL_NONE };
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	 * sample, we have a very simplified selection process, where we pick
	 * the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(app->_app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, app->_app->window,
	NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOG_ERROR("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	app->_display = display;
	app->_context = context;
	app->_surface = surface;
	app->_width = w;
	app->_height = h;
	app->_state.angle = 0;

	// Initialize GL state.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, app->_width, app->_height);
	checkGlError("glViewport");

	return 0;
}

static void termDisplay(struct AppState* app) {
	if (app->_display != EGL_NO_DISPLAY) {
		eglMakeCurrent(app->_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
		EGL_NO_CONTEXT);
		if (app->_context != EGL_NO_CONTEXT) {
			eglDestroyContext(app->_display, app->_context);
		}
		if (app->_surface != EGL_NO_SURFACE) {
			eglDestroySurface(app->_display, app->_surface);
		}
		eglTerminate(app->_display);
	}
	app->_animating = 0;
	app->_display = EGL_NO_DISPLAY;
	app->_context = EGL_NO_CONTEXT;
	app->_surface = EGL_NO_SURFACE;
}

GLfloat line1[] = { 0.5, 0.5, 0, -0.5, 0.5, 0 };
GLfloat line2[] = { -0.5, 0.5, 0, -0.5, -0.5, 0 };
GLfloat line3[] = { -0.5, -0.5, 0, 0.5, -0.5, 0 };
GLfloat line4[] = { 0.5, -0.5, 0, 0.5, 0.5, 0 };

GLfloat colors[] = { 1.0f, 0.0f, 0.0f, 1.0f };

static void drawOnDisplay(struct AppState* app) {
	if (app->_display == NULL) {
		// No display.
		return;
	}

	// Just fill the screen with a color.
	glClearColor(((float) app->_state.x) / app->_width /*Red*/,
			app->_state.angle /*Green*/,
			((float) app->_state.y) / app->_height /*Blue*/, 1 /*Alpha*/);
	glClear(GL_COLOR_BUFFER_BIT);

	glLineWidth(2.5);
	glShadeModel(GL_SMOOTH);
	glVertexPointer(3, GL_FLOAT, 0, line1);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_LINES, 0, 2);

	glVertexPointer(3, GL_FLOAT, 0, line2);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_LINES, 0, 2);

	glVertexPointer(3, GL_FLOAT, 0, line3);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_LINES, 0, 2);

	glVertexPointer(3, GL_FLOAT, 0, line4);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_LINES, 0, 2);

	glFlush();

//	printGLString("Version", GL_VERSION);
//	printGLString("Vendor", GL_VENDOR);
//	printGLString("Renderer", GL_RENDERER);
//	printGLString("Extensions", GL_EXTENSIONS);
//	LOG_INFO(" Window - Width: %d Height: %d", app->_width, app->_height);

	eglSwapBuffers(app->_display, app->_surface);
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* androidApp, int32_t cmd) {

	struct AppState * app = (struct AppState *) androidApp->userData;

	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		LOG_INFO("SVAE_STATE");
		app->_app->savedState = malloc(sizeof(struct AppSavedState));
		*((struct AppSavedState *) app->_app->savedState) = app->_state;
		app->_app->savedStateSize = sizeof(struct AppSavedState);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		LOG_INFO("INIT_WINDOW");
		if (app->_app->window != NULL) {
			initDisplay(app);
			drawOnDisplay(app);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		LOG_INFO("TERM_WINDOW");
		termDisplay(app);
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		LOG_INFO("GAINED_FOCUS");
		if (app->_accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(app->_sensorEventQueue,
					app->_accelerometerSensor);
			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(app->_sensorEventQueue,
					app->_accelerometerSensor, (1000L / 60) * 1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		LOG_INFO("LOST_FOCUS");
		if (app->_accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(app->_sensorEventQueue,
					app->_accelerometerSensor);
		}
		// Also stop animating.
		app->_animating = 0;
		drawOnDisplay(app);
		break;
	}
}

void android_main(struct android_app* state) {

	struct AppState app;
	LOG_INFO("Starting App....");
	//*** Stop glue from being stripped.
	app_dummy();

	memset(&app, 0, sizeof(app));
	state->userData = &app;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	app._app = state;

	// Prepare to monitor accelerometer
	app._sensorManager = ASensorManager_getInstance();

	app._accelerometerSensor = ASensorManager_getDefaultSensor(
			app._sensorManager, ASENSOR_TYPE_ACCELEROMETER);

	app._sensorEventQueue = ASensorManager_createEventQueue(app._sensorManager,
			state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		app._state = *(struct AppSavedState*) state->savedState;
	}

	// Run loop now
	while (true) {

		// Read all pending events.
		int events;
		int ident;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(0, NULL, &events, (void**) &source))
				>= 0) {

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) {
				if (app._accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(app._sensorEventQueue,
							&event, 1) > 0) {
//						LOG_INFO("accelerometer: x=%f y=%f z=%f",
//								event.acceleration.x, event.acceleration.y,
//								event.acceleration.z);
					}
				}
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				// Destroy your game world
				termDisplay(&app);
				return;
			}
		}		// end of pollEvent While loop

		// Draw Animation
		if (app._animating) {
			// Done with events; draw next animation frame.
			app._state.angle += .01f;
			if (app._state.angle > 1) {
				app._state.angle = 0;
			}

			// Drawing is throttled to the screen update rate, so there
			// is no need to do timing here.
			drawOnDisplay(&app);
		}

	}
//	// Create a kernel for this thread
//	    Kernel kernel;
//
//	    // Create an Android platform task for this thread
//	    AndroidTask android(state, 10);
//	    kernel.AddTask(&android);
//
//	    // Execute the kernel
//	    kernel.Execute();

// Clear the static closing flag
//AndroidTask::ClearClosing();
}
