#include "Application.h"

int main()
{
	// Create and run the application.
	Application* app = new Application("MaliceRHI App", 800, 600);
	app->Run();
	delete app;

	return 0;
}
