#pragma once

#include <cstddef>
#include <vector>
#include <filesystem>
#include <memory>
#include <UGrid.hpp>

#include "Archive.hpp"
#include "io/BinIO.hpp"
#include "io/MdlIO.hpp"
#include "io/TxpIO.hpp"

#include "Camera.hpp"

enum class EModelType {
    Actor,
    Furniture,
    None
};

enum class SelectedResourceType {
  Texture,
  Sampler,
  Material,
  Batch,
  DrawElement,
  GraphNode,
  Bone,
  None
};

class UEditorTab {
	UCamera mCamera;
	glm::mat4 mCameraGizmo;
	EModelType mCurrentModelType { EModelType::None };

	std::shared_ptr<Archive::Rarc> mModelArchive { nullptr };

    std::unique_ptr<BIN::Model> mModelFurniture { nullptr };
    std::unique_ptr<BIN::Animation> mFurnitureAnimation { nullptr };
    std::unique_ptr<MDL::Model> mModelActor { nullptr };
    std::unique_ptr<MDL::Animation> mActorSkeletalAnimation { nullptr };
    std::unique_ptr<TXP::Animation> mActorTxp { nullptr };

    bool mModelChanged { false };
    std::filesystem::path mModelPath {};
    void* SelectedResource { nullptr };
    SelectedResourceType SelectedType { SelectedResourceType::None };

    void MdlTreeNodeUI(std::unique_ptr<MDL::Model>& model, uint32_t index);
    void BinTreeNodeUI(std::unique_ptr<BIN::Model>& model, uint32_t index);
public:

	std::string mName;
	void RenderModel(float dt);
    void RenderSceneTreePanel();
    void RenderDetailsPanel();
    void RenderGizmos(ImVec2 viewportPosition, ImVec2 viewportSize);
    void ViewportClicked(int32_t id);
    void RenderTimelineAndRes();
    void SaveModel(std::filesystem::path filepath="");
    void RenderArcFolderTreeNode(std::shared_ptr<Archive::Folder> dir);

    UCamera& GetCamera() { return mCamera; }

    bool ShouldShowTimeline() const { return mActorSkeletalAnimation != nullptr || mFurnitureAnimation != nullptr; }
    bool HasArchive() const { return mModelArchive != nullptr; }
    bool IsModelType(EModelType type) const { return mCurrentModelType == type; }

    UEditorTab(std::filesystem::path modelPath);
    UEditorTab(EModelType type);
    ~UEditorTab();
};

class UContext {
	UGrid mGrid;

	uint32_t mMainDockSpaceID, mDockNodeHierarchyID, mDockNodeViewportID, mDockNodeDetailsID, mDockNodeTimelineID;

	float Rotate {25.0f}, Zoom { 500.0f };

	bool mShowTimeline { false };
	bool mModelSelectNew { false };
	bool mBinFBXImportOpen { false };
	bool mModelSelectOpen { false };
	bool mModelSelectSaveAs { false };

	bool bIsDockingSetUp { false };
	bool bIsFileDialogOpen { false };
	bool bIsSaveDialogOpen { false };

	uint32_t mFbo, mRbo, mViewTex, mPickTex;

	float mPrevWinWidth { -1.0f };
	float mPrevWinHeight { -1.0f };

	std::shared_ptr<UEditorTab> mSelectedTab { nullptr };
	std::vector<std::shared_ptr<UEditorTab>> mTabs;

	void RenderMainWindow(float deltaTime);
	void RenderPanels(float deltaTime);
	void RenderMenuBar();


public:
	UContext();
	~UContext();

	void HandleSelect();
	bool Update(float deltaTime);
	void Render(float deltaTime);
};
