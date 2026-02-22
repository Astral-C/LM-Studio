#pragma once

#include "UApplication.hpp"

class UViewerApplication : public UApplication {
	struct GLFWwindow* mWindow;
	class UContext* mContext;

	virtual bool Execute(float deltaTime) override;

	
public:

	UViewerApplication();
	virtual ~UViewerApplication() {}

	virtual bool Setup() override;
	virtual bool Teardown() override;
};
