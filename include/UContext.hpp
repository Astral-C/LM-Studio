#pragma once

#include <cstddef>
#include <vector>
#include <filesystem>
#include <memory>
#include <UGrid.hpp>

#include "Archive.hpp"
#include "bstream.h"
#include "io/BinIO.hpp"
#include "io/MdlIO.hpp"
#include "io/TxpIO.hpp"
#include "Camera.hpp"
#include "USequencer.hpp"

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

class UModelEditContext {
	glm::mat4 mCameraGizmo;

	EModelType mCurrentModelType { EModelType::None };

    std::unique_ptr<MDL::Model> mModelActor { nullptr };
    std::unique_ptr<BIN::Model> mModelFurniture { nullptr };

    std::unique_ptr<TXP::Animation> mActorTxp { nullptr };
    std::unique_ptr<MDL::Animation> mActorSkeletalAnimation { nullptr };
    std::unique_ptr<BIN::Animation> mFurnitureAnimation { nullptr };

    bool mModelUnsaved { false };
    void* SelectedResource { nullptr };
    SelectedResourceType SelectedType { SelectedResourceType::None };
    ImTimeline::State mTimelineState;
public:
	UCamera mCamera;

	bool HasAnimation() const { return mActorSkeletalAnimation != nullptr || mActorTxp != nullptr || mFurnitureAnimation != nullptr; }

    bool IsModelType(EModelType type) const { return mCurrentModelType == type; }
    bool IsModified() const { return mModelUnsaved; }

    void MdlTreeNodeUI(std::unique_ptr<MDL::Model>& model, uint32_t index);
    void BinTreeNodeUI(std::unique_ptr<BIN::Model>& model, uint32_t index);

    void RenderDetailsPanel();
    void RenderSceneTreePanel();
    void RenderTimeline();
    void RenderGizmos(ImVec2 viewportPosition, ImVec2 viewportSize);

	void RenderModel(float dt);
    void SaveModel(bStream::CStream* stream);
    void SaveAnimation(bStream::CStream* stream);
    void LoadAnimation(bStream::CStream* stream);

    void SelectNode(int32_t id);

    UModelEditContext();
    UModelEditContext(EModelType type);
    UModelEditContext(bStream::CStream* stream);
    UModelEditContext(bStream::CStream* stream, EModelType type);
    ~UModelEditContext();
};

class UEditorTab {

    std::unique_ptr<UModelEditContext> mModelContext;

	std::shared_ptr<Archive::Rarc> mModelArchive { nullptr };

    // Used for unsaved changes dialog
    bool mOpenUnsavedChangesModal { false };
    std::shared_ptr<Archive::File> mQueuedOpenFile { nullptr };

    std::filesystem::path mLoadedPath {};
    // ref to model file currently open (if this is an archive) so we can save it
    std::shared_ptr<Archive::File> mCurrentModelFile { nullptr };

    inline void TryLoadArchiveFile(std::shared_ptr<Archive::File> file);
public:

	std::string mName;
    inline void RenderDetailsPanel();
    inline void RenderSceneTreePanel(ImVec2 size);
    inline void RenderTimeline();
    inline void RenderGizmos(ImVec2 viewportPosition, ImVec2 viewportSize);
    inline void RenderModelContext(float dt) { if(mModelContext != nullptr) mModelContext->RenderModel(dt); }

    void RenderArcFolderTreeNode(std::shared_ptr<Archive::Folder> dir, ImVec2 size);

    inline void ViewportClicked(int32_t id);
    void SaveModel(std::filesystem::path filepath="");
    void SaveAnimation(std::filesystem::path filepath="");
    void OpenQueuedModel();

    UCamera* GetCamera() { return mModelContext != nullptr ? &mModelContext->mCamera : nullptr; }

    void ShouldOpenUnsavedChanges(bool opened) { mOpenUnsavedChangesModal = opened;}
    bool ShouldOpenUnsavedChanges() const { return mOpenUnsavedChangesModal;}
    bool ShouldShowTimeline() const { return mModelContext != nullptr && mModelContext->HasAnimation(); }
    bool HasArchive() const { return mModelArchive != nullptr; }
    bool IsModelContextType(EModelType type) const { return (mModelContext != nullptr ? mModelContext->IsModelType(type) : false); }

    UEditorTab(std::filesystem::path modelPath);
    UEditorTab(EModelType type);
    ~UEditorTab();
};

class UContext {
	UGrid mGrid;

	uint32_t mMainDockSpaceID, mDockNodeHierarchyID, mDockNodeViewportID, mDockNodeDetailsID, mDockNodeTimelineID;

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

	void RenderPanels(float deltaTime);
	void RenderMenuBar();

public:
	UContext();
	~UContext();

	void DropFiles(int count, const char** paths);

	void HandleSelect();
	bool Update(float deltaTime);
	void Render(float deltaTime);
};
