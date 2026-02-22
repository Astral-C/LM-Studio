#include "UViewerApplication.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
	UViewerApplication app;

	if (!app.Setup()) {
		return 0;
	}

	app.Run();

	if (!app.Teardown()) {
		return 0;
	}
}