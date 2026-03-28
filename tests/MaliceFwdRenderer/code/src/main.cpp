#include "application.h"

int main()
{
	// Create and run the application.
	Application* app = new Application("MaliceRHI App", 1280, 720);
	app->Run();
	delete app;

	return 0;
}
