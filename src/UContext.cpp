#include "UContext.hpp"

#include "Camera.hpp"
#include "io/BinIO.hpp"
#include "io/MdlIO.hpp"
#include "ufbx.h"
#include "util/UUIUtil.hpp"

#include "IconsForkAwesome.h"
#include "ImGuiFileDialog.h"
#include "ImGuizmo.h"
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <bstream.h>
#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#include <format>
#include "ImGuiNotify.hpp"

glm::mat4 Identity(1.0f);


namespace ImportSettings {
    bool EnableVertexColors { false };
};


std::vector<const char*> WrapModes {
    "Clamp",
    "Repeat",
    "Mirror"
};

std::vector<const char*> TextureFormatNames {
    "I4",
    "I8",
    "IA4",
    "IA8",
    "RGB565",
    "RGB5A3",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "???",
    "CMPR"
};

inline void ImportModelPane(const char *vFilter, IGFDUserDatas vUserDatas, bool *vCantContinue){
    ImGui::Text("Import Settings");
    if(std::string(vFilter) == ".fbx"){
        ImGui::Checkbox("Enable Vertex Colors", &ImportSettings::EnableVertexColors);
    }
}

void UEditorTab::SaveModel(std::filesystem::path filepath){
    bStream::CFileStream modelStream(filepath.string(), bStream::Endianess::Big, bStream::OpenMode::Out);
    if(mCurrentModelType == EModelType::Furniture && mModelFurniture != nullptr){
        mModelFurniture->Write(&modelStream);
    }
    if(mCurrentModelType == EModelType::Actor && mModelActor != nullptr){
        mModelActor->Save(&modelStream);
    }
}

UEditorTab::UEditorTab(std::filesystem::path resPath){
    if(resPath.extension() == ".bin"){
        mCurrentModelType = EModelType::Furniture;
        bStream::CFileStream modelStream(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
        mModelFurniture = std::make_unique<BIN::Model>();
        mModelFurniture->Load(&modelStream);
        mName = resPath.filename().stem();

    } else if(resPath.extension() == ".mdl"){
        mCurrentModelType = EModelType::Actor;
        bStream::CFileStream modelStream(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
        mModelActor = std::make_unique<MDL::Model>();
        mModelActor->Load(&modelStream);
        mName = resPath.filename().stem();
    }
}

UEditorTab::UEditorTab(EModelType type){
    if(type == EModelType::Furniture){
        mCurrentModelType = EModelType::Furniture;
        mModelFurniture = std::make_unique<BIN::Model>();
        mModelFurniture->mGraphNodes[0] = {};
        mName = "New Bin";
    } else if(type == EModelType::Actor){
        mCurrentModelType = EModelType::Actor;
        mModelActor = std::make_unique<MDL::Model>();
        mName = "New Mdl";
    }
}

UEditorTab::~UEditorTab(){

}

UContext::~UContext(){
	BIN::DestroyShaders();
	MDL::DestroyShaders();
}

UContext::UContext(){
	mGrid.Init();

	glGenFramebuffers(1, &mFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

	glGenRenderbuffers(1, &mRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);

	glGenTextures(1, &mViewTex);
	glGenTextures(1, &mPickTex);

	glBindTexture(GL_TEXTURE_2D, mViewTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, mPickTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, 1280, 720, 0, GL_RED_INTEGER, GL_INT, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mViewTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPickTex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRbo);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	BIN::InitShaders();
	MDL::InitShaders();
}

bool UContext::Update(float deltaTime) {

	return true;
}

void UEditorTab::BinTreeNodeUI(std::unique_ptr<BIN::Model>& model, uint32_t index){
    //bool open = ImGui::TreeNodeEx(std::format("Node {}", model->mGraphNodes[index].Index).c_str(), (SelectedResource == &model->mGraphNodes[index] ? ImGuiTreeNodeFlags_Selected : 0));
    bool selected = (SelectedResource == &model->mGraphNodes[index] ? ImGuiTreeNodeFlags_Selected : 0);
    bool open = UIUtil::RenderNodeSelectableTreeNode(std::format("Node {}", model->mGraphNodes[index].Index), selected, selected);

    if(selected && SelectedResource != &model->mGraphNodes[index]){
        SelectedResource = &model->mGraphNodes[index];
    }

    if(index != 0 && ImGui::BeginPopupContextItem(std::format("##binScenegraphContextMenu{}", index).c_str())){
        if(ImGui::Selectable("Delete")){
            if(model->mGraphNodes[index].PreviousSibIndex != -1){
                model->mGraphNodes[model->mGraphNodes[index].PreviousSibIndex].NextSibIndex = model->mGraphNodes[index].NextSibIndex;
            }
            if(model->mGraphNodes[index].NextSibIndex != -1){
                model->mGraphNodes[model->mGraphNodes[index].NextSibIndex].PreviousSibIndex = model->mGraphNodes[index].PreviousSibIndex;
            }
            if(model->mGraphNodes[index].ParentIndex != -1 && model->mGraphNodes[model->mGraphNodes[index].ParentIndex].ChildIndex == index){
                model->mGraphNodes[model->mGraphNodes[index].ParentIndex].ChildIndex = model->mGraphNodes[index].NextSibIndex;
            }
            model->mGraphNodes.erase(index);
        }
        ImGui::EndPopup();
    }

    if(open){
        if(ImGui::IsItemClicked(0)){
            SelectedType = SelectedResourceType::GraphNode;
            SelectedResource = &model->mGraphNodes[index];
        }


        bool childrenOpen = ImGui::TreeNodeEx(ICON_FK_CUBE "  Draw Elements", model->mGraphNodes[index].mDrawElements.size() == 0 ? ImGuiTreeNodeFlags_Leaf : 0);
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(0)){
            model->mGraphNodes[index].mDrawElements.push_back({});
        }
        if(childrenOpen){
            uint16_t deleteIdx = UINT16_MAX;
            for(int i = 0; i < model->mGraphNodes[index].mDrawElements.size(); i++){ //model->mGraphNodes[index].mDrawElements
                ImGui::Text(std::format("Mesh Part {}", i).c_str());

                if(ImGui::IsItemClicked(0)){
                    SelectedType = SelectedResourceType::DrawElement;
                    SelectedResource = &model->mGraphNodes[index].mDrawElements[i];
                }

                if(ImGui::BeginPopupContextItem(std::format("##drawElementResourceContextMenu{}", i).c_str())){
                    if(ImGui::Selectable("Delete")){
                        deleteIdx = i;
                    }
                    ImGui::EndPopup();
                }
            }

            if(deleteIdx != UINT16_MAX){
                model->mGraphNodes[index].mDrawElements.erase(model->mGraphNodes[index].mDrawElements.begin() + deleteIdx);
            }
            ImGui::TreePop();
        }

        childrenOpen = ImGui::TreeNodeEx(ICON_FK_LEAF " Children", model->mGraphNodes[index].ChildIndex == -1 ? ImGuiTreeNodeFlags_Leaf : 0);
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(0)){
            int id = model->mGraphNodes.size();
            for(int idx = 0; idx < model->mGraphNodes.size(); idx++){
                if(!model->mGraphNodes.contains(idx)){
                    id = idx;
                    break;
                }
            }
            model->mGraphNodes[id] = {};
            model->mGraphNodes[id].Index = id;
            model->mGraphNodes[id].ParentIndex = model->mGraphNodes[index].Index;
            if(model->mGraphNodes[index].ChildIndex != -1){
                for(int idx = model->mGraphNodes[index].ChildIndex; idx != -1;){
                    int nextIdx = model->mGraphNodes[idx].NextSibIndex;
                    if(model->mGraphNodes[idx].NextSibIndex == -1){
                        model->mGraphNodes[idx].NextSibIndex = id;
                        model->mGraphNodes[id].PreviousSibIndex = idx;
                    }
                    idx = nextIdx;
                }
            } else {
                model->mGraphNodes[index].ChildIndex = id;
            }

        }
        if(childrenOpen){
            if(model->mGraphNodes[index].ChildIndex != -1) BinTreeNodeUI(model, model->mGraphNodes[index].ChildIndex);
            ImGui::TreePop();
        }

        ImGui::TreePop();

    }
    if(model->mGraphNodes[index].NextSibIndex != -1){
        BinTreeNodeUI(model, model->mGraphNodes[index].NextSibIndex);
    }
}

void UEditorTab::MdlTreeNodeUI(std::unique_ptr<MDL::Model>& model, uint32_t index){
    bool open = ImGui::TreeNodeEx(std::format("Bone {}", index).c_str(), (SelectedResource == &model->mGraphNodes[index] ? ImGuiTreeNodeFlags_Selected : 0));

    if(index != 0 && ImGui::BeginPopupContextItem(std::format("##binScenegraphContextMenu{}", index).c_str())){
        if(ImGui::Selectable("Delete")){
            model->mGraphNodes.erase(index);
        }
        ImGui::EndPopup();
    }

    if(open){
        if(ImGui::IsItemClicked(0)){
            SelectedType = SelectedResourceType::GraphNode;
            SelectedResource = &model->mGraphNodes[index];
        }

        uint16_t deleteIdx = UINT16_MAX;
        for(int i = 0; i < model->mGraphNodes[index].DrawElementCount; i++){ //model->mGraphNodes[index].mDrawElements
            ImGui::Text(std::format("Mesh Part {}", i).c_str());

            if(ImGui::IsItemClicked(0)){
                SelectedType = SelectedResourceType::DrawElement;
                SelectedResource = &model->mDrawElements[model->mGraphNodes[index].DrawElementBeginIndex + i];
            }
        }

        bool childrenOpen = ImGui::TreeNodeEx(ICON_FK_LEAF " Children", model->mGraphNodes[index].ChildIndexShift > 0 ? ImGuiTreeNodeFlags_Leaf : 0);
        if(childrenOpen){
            if(model->mGraphNodes[index].ChildIndexShift > 0) MdlTreeNodeUI(model, index + model->mGraphNodes[index].ChildIndexShift);
            ImGui::TreePop();
        }


        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Text(ICON_FK_CUBE " Draw Elements");
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(0)){
            //model->mGraphNodes[index].mDrawElements.push_back({});
        }
        ImGui::TreePop();

    }
    if(model->mGraphNodes[index].SiblingIndexShift > 0){
        MdlTreeNodeUI(model, index + model->mGraphNodes[index].SiblingIndexShift);
    }
}

void UEditorTab::RenderModel(float dt){
    mCamera.Update(dt);
    glm::mat4 mvp = mCamera.mProjection * mCamera.mView * Identity;
    if(mCurrentModelType == EModelType::Furniture && mModelFurniture != nullptr){
       	mModelFurniture->Draw(&mvp, 0, false, nullptr);
    } else if(mCurrentModelType == EModelType::Actor && mModelActor != nullptr){
       	mModelActor->Draw(&mvp, 0, false, nullptr, nullptr);
        mModelActor->mSkeletonRenderer.Draw(&mCamera);
    }
}

void UEditorTab::ViewportClicked(int32_t id){
    if(mCurrentModelType == EModelType::Furniture && mModelFurniture != nullptr){
        SelectedType = SelectedResourceType::GraphNode;
        SelectedResource = &mModelFurniture->mGraphNodes[id];
    }
}

void UEditorTab::RenderGizmos(ImVec2 viewportPos, ImVec2 viewportSize){
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);

    float viewManipulateRight = viewportPos.x + viewportSize.x;
    float viewManipulateBottom = viewportPos.y + viewportSize.y;
    ImGuizmo::ViewManipulate(&mCamera.mView[0][0], &mCamera.mProjection[0][0], ImGuizmo::OPERATION::UNIVERSAL, ImGuizmo::LOCAL, &mCameraGizmo[0][0], mCamera.mDistance, { viewManipulateRight - 128, viewManipulateBottom - 128 }, { 128, 128 }, 0x000000F0);
    //mCamera.PostManipulate(view, proj); // stupid

    if(mCurrentModelType == EModelType::Furniture && mModelFurniture != nullptr && SelectedType == SelectedResourceType::GraphNode){
        if(ImGuizmo::Manipulate(&mCamera.mView[0][0], &mCamera.mProjection[0][0], ImGuizmo::OPERATION::TRANSLATE | ImGuizmo::OPERATION::ROTATE | ImGuizmo::OPERATION::SCALE, ImGuizmo::LOCAL, &static_cast<BIN::SceneGraphNode*>(SelectedResource)->Transform[0][0])){
            ImGuizmo::DecomposeMatrixToComponents(&static_cast<BIN::SceneGraphNode*>(SelectedResource)->Transform[0][0], &static_cast<BIN::SceneGraphNode*>(SelectedResource)->Position[0], &static_cast<BIN::SceneGraphNode*>(SelectedResource)->Rotation[0], &static_cast<BIN::SceneGraphNode*>(SelectedResource)->Scale[0]);
        }
    }
}

void UEditorTab::RenderSceneTreePanel(){
    if(mCurrentModelType == EModelType::Furniture){
        ImGui::Text("SceneGraph");
        ImGui::SameLine();
        float widthNeeded = ImGui::CalcTextSize("Save").x + ImGui::GetStyle().FramePadding.x * 2.f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
        if(ImGui::Button("Save")){
            //bStream::CMemoryStream modelData(12, bStream::Endianess::Big, bStream::OpenMode::Out);
            //mModelFurniture->Write(&modelData);
            // write bin file
        }
        ImGui::Separator();
        BinTreeNodeUI(mModelFurniture, 0);
        ImGui::Text("Resources");
        ImGui::Separator();
        bool  open = ImGui::TreeNode(ICON_FK_CUBE " Batches");
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(0)){
            int id = mModelFurniture->mBatches.size();
            for(int idx = 0; idx < mModelFurniture->mBatches.size(); idx++){
                if(!mModelFurniture->mBatches.contains(idx)){
                    id = idx;
                    break;
                }
            }
            mModelFurniture->mBatches[id] = {};
        }
        if(open){
            uint16_t deleteIdx = UINT16_MAX;
            for(auto [idx, batch] : mModelFurniture->mBatches){
                if(&mModelFurniture->mBatches[idx] == SelectedResource){
                    ImGui::TextColored({0x00, 0xFF, 0x00, 0xFF}, "Batch %d", idx);
                } else {
                    ImGui::Text("Batch %d", idx);
                }
                if(ImGui::IsItemClicked(0)){
                    SelectedType = SelectedResourceType::Batch;
                    SelectedResource = &mModelFurniture->mBatches[idx];
                }
                if(ImGui::BeginPopupContextItem(std::format("##batchResourceContextMenu{}",idx).c_str())){
                    if(ImGui::Selectable("Delete")){
                        deleteIdx = idx;
                    }
                    ImGui::EndPopup();
                }
            }
            if(deleteIdx != UINT16_MAX){
                SelectedResource = nullptr;
                SelectedType = SelectedResourceType::None;
                for(auto [idx, node] : mModelFurniture->mGraphNodes){
                    for(int i = 0; i < node.mDrawElements.size(); i++){
                        if(node.mDrawElements[i].BatchIndex == deleteIdx){
                            mModelFurniture->mGraphNodes[idx].mDrawElements[i].BatchIndex = -1;
                        }
                    }
                }
                mModelFurniture->mBatches.erase(deleteIdx);
            }
            ImGui::TreePop();
        }
        open = ImGui::TreeNode(ICON_FK_PAINT_BRUSH " Materials");
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(0)){
            int id = mModelFurniture->mMaterials.size();
            for(int idx = 0; idx < mModelFurniture->mMaterials.size(); idx++){
                if(!mModelFurniture->mMaterials.contains(idx)){
                    id = idx;
                    break;
                }
            }
            mModelFurniture->mMaterials[id] = {};
        }
        if(open){
            uint16_t deleteIdx = UINT16_MAX;
            for(auto [idx, material] : mModelFurniture->mMaterials){
                if(&mModelFurniture->mMaterials[idx] == SelectedResource){
                    ImGui::TextColored({0x00, 0xFF, 0x00, 0xFF}, "Material %d", idx);
                } else {
                    ImGui::Text("Material %d", idx);
                }
                if(ImGui::IsItemClicked(0)){
                    SelectedType = SelectedResourceType::Material;
                    SelectedResource = &mModelFurniture->mMaterials[idx];
                }
                if(ImGui::BeginPopupContextItem(std::format("##materialResourceContextMenu{}",idx).c_str())){
                    if(ImGui::Selectable("Delete")){
                        deleteIdx = idx;
                    }
                    ImGui::EndPopup();
                }
            }
            if(deleteIdx != UINT16_MAX){
                SelectedResource = nullptr;
                SelectedType = SelectedResourceType::None;
                for(auto [idx, node] : mModelFurniture->mGraphNodes){
                    for(int i = 0; i < node.mDrawElements.size(); i++){
                        if(node.mDrawElements[i].MaterialIndex == deleteIdx){
                            mModelFurniture->mGraphNodes[idx].mDrawElements[i].MaterialIndex = -1;
                        }
                    }
                }
                mModelFurniture->mMaterials.erase(deleteIdx);
            }
            ImGui::TreePop();
        }
        open = ImGui::TreeNode(ICON_FK_PAINT_BRUSH " Samplers");
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);

        if(ImGui::IsItemClicked(0)){
            int id = mModelFurniture->mSamplers.size();
            for(int idx = 0; idx < mModelFurniture->mSamplers.size(); idx++){
                if(!mModelFurniture->mSamplers.contains(idx)){
                    id = idx;
                    break;
                }
            }
            mModelFurniture->mSamplers[id] = {};
        }

        if(open){
            uint16_t deleteIdx = UINT16_MAX;
            for(auto [idx, sampler] : mModelFurniture->mSamplers){
                if(&mModelFurniture->mSamplers[idx] == SelectedResource){
                    ImGui::TextColored({0x00, 0xFF, 0x00, 0xFF}, "Sampler %d", idx);
                } else {
                    ImGui::Text("Sampler %d", idx);
                }
                if(ImGui::IsItemClicked(0)){
                    SelectedType = SelectedResourceType::Sampler;
                    SelectedResource = &mModelFurniture->mSamplers[idx];
                }
                if(ImGui::BeginPopupContextItem(std::format("##samplerResourceContextMenu{}",idx).c_str())){
                    if(ImGui::Selectable("Delete")){
                        deleteIdx = idx;
                    }
                    ImGui::EndPopup();
                }
            }
            if(deleteIdx != UINT16_MAX){
                for(auto [idx, material] : mModelFurniture->mMaterials){
                    if(mModelFurniture->mMaterials[idx].SamplerIndices[0] == deleteIdx){
                        mModelFurniture->mMaterials[idx].SamplerIndices[0] = -1;
                        SelectedResource = nullptr;
                        SelectedType = SelectedResourceType::None;
                    }
                }
                mModelFurniture->mSamplers.erase(deleteIdx);
            }
            ImGui::TreePop();
        }

        open = ImGui::TreeNode(ICON_FK_PICTURE_O " Textures");
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(0)){
            int id = mModelFurniture->mTextureHeaders.size();
            for(int idx = 0; idx < mModelFurniture->mTextureHeaders.size(); idx++){
                if(!mModelFurniture->mTextureHeaders.contains(idx)){
                    id = idx;
                    break;
                }
            }
            mModelFurniture->mTextureHeaders[id] = {};
        }

        if(open){
            uint16_t deleteIdx = UINT16_MAX;
            for(auto [idx, texture] : mModelFurniture->mTextureHeaders){
                ImGui::Image(static_cast<uintptr_t>(texture.TextureID), {16, 16});
                ImGui::SameLine();
                if(&mModelFurniture->mTextureHeaders[idx] == SelectedResource){
                    ImGui::TextColored({0x00, 0xFF, 0x00, 0xFF}, "Texture %d", idx);
                } else {
                    ImGui::Text("Texture %d", idx);
                }
                if(ImGui::IsItemClicked(0)){
                    SelectedType = SelectedResourceType::Texture;
                    SelectedResource = &mModelFurniture->mTextureHeaders[idx];
                }
                if(ImGui::BeginPopupContextItem(std::format("##textureResourceContextMenu{}",idx).c_str())){
                    if(ImGui::Selectable("Delete")){
                        deleteIdx = idx;
                    }
                    if(ImGui::Selectable("Replace")){
                        IGFD::FileDialogConfig cfg { .path = std::filesystem::current_path().string(), .flags = ImGuiFileDialogFlags_Modal };
                        ImGuiFileDialog::Instance()->OpenDialog("replaceTextureImageDialog", "Replace Texture", "PNG Image (*.png){.png}", cfg);
                        SelectedResource = &mModelFurniture->mTextureHeaders[idx];
                    }
                    if(ImGui::Selectable("Export")){
                        IGFD::FileDialogConfig cfg { .path = std::filesystem::current_path().string(), .flags = ImGuiFileDialogFlags_Modal };
                        ImGuiFileDialog::Instance()->OpenDialog("exportTextureImageDialog", "Export Texture", "PNG Image (*.png){.png}", cfg);
                        SelectedResource = &mModelFurniture->mTextureHeaders[idx];
                    }
                    ImGui::EndPopup();
                }
            }
            if(deleteIdx != UINT16_MAX){
                mModelFurniture->mTextureHeaders.erase(deleteIdx);
                for(auto [idx, sampler] : mModelFurniture->mSamplers){
                    if(mModelFurniture->mSamplers[idx].TextureIndex == deleteIdx){
                        mModelFurniture->mSamplers[idx].TextureIndex = -1;
                        SelectedResource = nullptr;
                        SelectedType = SelectedResourceType::None;
                    }
                }
            }
            ImGui::TreePop();
        }

        }
    else if(mCurrentModelType == EModelType::Actor) {
        ImGui::Text("SceneGraph");
        ImGui::SameLine();
        float widthNeeded = ImGui::CalcTextSize("Save").x + ImGui::GetStyle().FramePadding.x * 2.f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
        if(ImGui::Button("Save")){
        }
        ImGui::Separator();
        MdlTreeNodeUI(mModelActor, 0);
        ImGui::Text("Resources");
        bool open = false;
        open = ImGui::TreeNode(ICON_FK_PICTURE_O " Textures");
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Text(ICON_FK_PLUS_CIRCLE);
        if(ImGui::IsItemClicked(0)){
            int id = mModelActor->mTextureHeaders.size();
            for(int idx = 0; idx < mModelActor->mTextureHeaders.size(); idx++){
                if(!mModelActor->mTextureHeaders.contains(idx)){
                    id = idx;
                    break;
                }
            }
            mModelActor->mTextureHeaders[id] = {};
        }

        if(open){
            uint16_t deleteIdx = UINT16_MAX;
            for(auto [idx, texture] : mModelActor->mTextureHeaders){
                ImGui::Image(static_cast<uintptr_t>(texture.TextureID), {16, 16});
                ImGui::SameLine();
                if(&mModelActor->mTextureHeaders[idx] == SelectedResource){
                    ImGui::TextColored({0x00, 0xFF, 0x00, 0xFF}, "Texture %d", idx);
                } else {
                    ImGui::Text("Texture %d", idx);
                }
                if(ImGui::IsItemClicked(0)){
                    SelectedType = SelectedResourceType::Texture;
                    SelectedResource = &mModelActor->mTextureHeaders[idx];
                }
                if(ImGui::BeginPopupContextItem(std::format("##textureResourceContextMenu{}",idx).c_str())){
                    if(ImGui::Selectable("Delete")){
                        deleteIdx = idx;
                    }
                    if(ImGui::Selectable("Replace")){
                        IGFD::FileDialogConfig cfg { .path = std::filesystem::current_path().string(), .flags = ImGuiFileDialogFlags_Modal };
                        ImGuiFileDialog::Instance()->OpenDialog("replaceTextureImageDialog", "Replace Texture", "PNG Image (*.png){.png}", cfg);
                        SelectedResource = &mModelActor->mTextureHeaders[idx];
                    }
                    if(ImGui::Selectable("Export")){
                        IGFD::FileDialogConfig cfg { .path = std::filesystem::current_path().string(), .flags = ImGuiFileDialogFlags_Modal };
                        ImGuiFileDialog::Instance()->OpenDialog("exportTextureImageDialog", "Export Texture", "PNG Image (*.png){.png}", cfg);
                        SelectedResource = &mModelActor->mTextureHeaders[idx];
                    }
                    ImGui::EndPopup();
                }
            }
            if(deleteIdx != UINT16_MAX){
                mModelActor->mTextureHeaders.erase(deleteIdx);
                for(auto [idx, sampler] : mModelActor->mSamplers){
                    if(mModelActor->mSamplers[idx].TextureIndex == deleteIdx){
                        mModelActor->mSamplers[idx].TextureIndex = -1;
                        SelectedResource = nullptr;
                        SelectedType = SelectedResourceType::None;
                    }
                }
            }
            ImGui::TreePop();
        }
    }
}

void UEditorTab::RenderDetailsPanel(){
    if(mCurrentModelType == EModelType::Furniture && SelectedType != SelectedResourceType::None && SelectedResource != nullptr){
        switch(SelectedType){
            case SelectedResourceType::GraphNode: {
                ImGui::Text("Graph Node");
                ImGui::Separator();
                auto node = reinterpret_cast<BIN::SceneGraphNode*>(SelectedResource);
                ImGui::Text("Render Flags");
                bool value = node->CastShadow();
                if(ImGui::Checkbox("Cast Shadow", &value)){
                    node->CastShadow(value);
                }
                value = node->FourthWall();
                if(ImGui::Checkbox("Fourth Wall", &value)){
                    node->FourthWall(value);
                }
                value = node->Transparent();
                if(ImGui::Checkbox("Transparent", &value)){
                    node->Transparent(value);
                }
                value = node->FullBright();
                if(ImGui::Checkbox("Full Bright", &value)){
                    node->FullBright(value);
                }
                value = node->Ceiling();
                if(ImGui::Checkbox("Ceiling", &value)){
                    node->Ceiling(value);
                }
                if(ImGui::InputFloat3("Scale", &node->Scale.x) || ImGui::InputFloat3("Rotation", &node->Rotation.x) || ImGui::InputFloat3("Translation", &node->Position.x)){
                    node->Transform = glm::mat4(1.0f);
                    node->Transform = glm::scale(node->Transform, node->Scale);
                    node->Transform = glm::rotate(node->Transform, glm::radians(node->Rotation.x), {1.0f, 0.0f, 0.0f});
                    node->Transform = glm::rotate(node->Transform, glm::radians(node->Rotation.y), {0.0f, 1.0f, 0.0f});
                    node->Transform = glm::rotate(node->Transform, glm::radians(node->Rotation.z), {0.0f, 0.0f, 1.0f});
                    node->Transform = glm::translate(node->Transform, {node->Position.x, node->Position.y, node->Position.z});
                }
                ImGui::InputFloat3("Min Bounds", &node->BoundingBoxMin.x);
                ImGui::InputFloat3("Max Bounds", &node->BoundingBoxMax.x);
                break;
            }
            case SelectedResourceType::Batch: {
                ImGui::Text("Batch");
                ImGui::Separator();
                auto batch = reinterpret_cast<BIN::Batch*>(SelectedResource);
                ImGui::Text("NBT Enabled");
                bool clicked = ImGui::IsItemClicked(); ImGui::SameLine();
                ImGui::Text(batch->NBTFlag == 1 ? ICON_FK_CHECK_CIRCLE : ICON_FK_CIRCLE_O);
                clicked |= ImGui::IsItemClicked();
                if(clicked){
                    if(batch->NBTFlag == 0){
                        batch->NBTFlag = 1;
                        batch->TexCoordFlag = 2;
                        batch->VertexAttributes |= (1 << (int)GXAttribute::Tex1);
                    } else {
                        batch->NBTFlag = 0;
                        batch->TexCoordFlag = 1;
                        batch->VertexAttributes &= ~(1 << (int)GXAttribute::Tex1);
                    }
                }
                if(ImGui::Button("Import")){
                    IGFD::FileDialogConfig cfg { .path = std::filesystem::current_path().string(), .flags = ImGuiFileDialogFlags_Modal };
                    ImGuiFileDialog::Instance()->OpenDialog("replaceBatchDialog", "Replace Batch", "OBJ Model (*.obj){.obj}", cfg);
                }
                break;
            }
            case SelectedResourceType::Texture: {
                ImGui::Text("Texture");
                ImGui::Separator();
                auto texture = reinterpret_cast<BIN::TextureHeader*>(SelectedResource);
                if(ImGui::BeginCombo("Texture Format", TextureFormatNames[texture->Format])){
                    for(int i = 0; i < TextureFormatNames.size(); i++){
                        if(std::string(TextureFormatNames[i]) == "???") continue;
                        if(ImGui::Selectable(TextureFormatNames[i])){
                            texture->Format = i;
                        }
                    }
                    ImGui::EndCombo();
                }
                if(ImGui::Button("Replace")){
                    IGFD::FileDialogConfig cfg { .path = std::filesystem::current_path().string(), .flags = ImGuiFileDialogFlags_Modal };
                    ImGuiFileDialog::Instance()->OpenDialog("replaceTextureImageDialog", "Replace Texture", "PNG Image (*.png){.png}", cfg);
                }
                ImGui::SameLine();
                if(ImGui::Button("Export")){
                    IGFD::FileDialogConfig cfg { .path = std::filesystem::current_path().string(), .flags = ImGuiFileDialogFlags_Modal };
                    ImGuiFileDialog::Instance()->OpenDialog("exportTextureImageDialog", "Export Texture", "PNG Image (*.png){.png}", cfg);
                }
                ImGui::Image(static_cast<uintptr_t>(texture->TextureID), {static_cast<float>(texture->Width), static_cast<float>(texture->Height)});
                break;
            }
            case SelectedResourceType::Material: {
                ImGui::Text("Material");
                ImGui::Separator();
                auto material = reinterpret_cast<BIN::Material*>(SelectedResource);
                bool isLit = material->LightEnabled;
                ImGui::Checkbox("Lit", &isLit);
                material->LightEnabled = isLit;
                int val = material->Unk0;
                ImGui::InputInt("Unknown Value 1", &val);
                material->Unk0 = val;
                val = material->Unk1;
                ImGui::InputInt("Unknown Value 2", &val);
                material->Unk1 = val;
                ImGui::ColorPicker4("Color", (float *)&material->Color.x, 0);
                for(int i = 0; i < 8; i++){
                    if(ImGui::BeginCombo(std::format("Sampler Slot {}", i).c_str(), std::format("Sampler {}", material->SamplerIndices[i]).c_str())){
                        for(auto [idx, sampler] : mModelFurniture->mSamplers){
                            if(ImGui::Selectable(std::format("Sampler {}", idx).c_str())){
                                material->SamplerIndices[i] = idx;
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                break;
            }
            case SelectedResourceType::Sampler: {
                ImGui::Text("Sampler");
                ImGui::Separator();
                auto sampler = reinterpret_cast<BIN::Sampler*>(SelectedResource);
                if(ImGui::BeginCombo("Wrap U", WrapModes[sampler->WrapU])){
                    for(int i = 0; i < 3; i++){
                        if(ImGui::Selectable(WrapModes[i], sampler->WrapU == i)){
                            sampler->WrapU = i;
                        }
                    }
                    ImGui::EndCombo();
                }
                if(ImGui::BeginCombo("Wrap V", WrapModes[sampler->WrapV])){
                    for(int i = 0; i < 3; i++){
                        if(ImGui::Selectable(WrapModes[i], sampler->WrapV == i)){
                            sampler->WrapV = i;
                        }
                    }
                    ImGui::EndCombo();
                }
                int val = sampler->Unk;
                ImGui::InputInt("Unknown Value 1", &val);
                sampler->Unk = val;
                if(ImGui::BeginCombo("Texture", std::format("Texture {}", sampler->TextureIndex).c_str())){
                    for(auto [idx, texture] : mModelFurniture->mTextureHeaders){
                        if(ImGui::Selectable(std::format("Texture {}", idx).c_str())){
                            sampler->TextureIndex = idx;
                        }
                    }
                    ImGui::EndCombo();
                }
                break;
            }
        }
        if (ImGuiFileDialog::Instance()->Display("replaceTextureImageDialog")) {
    		if (ImGuiFileDialog::Instance()->IsOk()) {
    			std::string FilePath = ImGuiFileDialog::Instance()->GetFilePathName();
                if(SelectedResource != nullptr){
                    int w, h, channels;
                    unsigned char* img = stbi_load(FilePath.c_str(), &w, &h, &channels, 4);
                    if(img != nullptr){
                        reinterpret_cast<BIN::TextureHeader*>(SelectedResource)->SetImage(img, w*h*4, w, h);
                        stbi_image_free(img);
                    } else {
                        ImGui::InsertNotification({ImGuiToastType::Error, 3000, std::format("Failed to load image\nPath: {}", FilePath).data()});
                    }
                }
      		}

    		ImGuiFileDialog::Instance()->Close();
    	}
        if (ImGuiFileDialog::Instance()->Display("exportTextureImageDialog")) {
    		if (ImGuiFileDialog::Instance()->IsOk()) {
    			std::string FilePath = ImGuiFileDialog::Instance()->GetFilePathName();
                if(SelectedResource != nullptr){
                    if(!stbi_write_png(FilePath.c_str(), reinterpret_cast<BIN::TextureHeader*>(SelectedResource)->Width, reinterpret_cast<BIN::TextureHeader*>(SelectedResource)->Height, 4, reinterpret_cast<BIN::TextureHeader*>(SelectedResource)->ImageData, 4*reinterpret_cast<BIN::TextureHeader*>(SelectedResource)->Width)){
                        ImGui::InsertNotification({ImGuiToastType::Error, 3000, std::format("Failed to Write Image\nPath: {}", FilePath).data()});
                    }
                }
      		}

    		ImGuiFileDialog::Instance()->Close();
    	}
    } else if(mCurrentModelType == EModelType::Actor && SelectedType != SelectedResourceType::None && SelectedResource != nullptr){
        switch(SelectedType){
            case SelectedResourceType::GraphNode: {
                ImGui::Text("Graph Node");
                ImGui::Separator();
                break;
            }
            case SelectedResourceType::Texture: {
                ImGui::Text("Texture");
                ImGui::Separator();
                auto texture = reinterpret_cast<MDL::TextureHeader*>(SelectedResource);
                if(ImGui::BeginCombo("Texture Format", TextureFormatNames[texture->Format])){
                    for(int i = 0; i < TextureFormatNames.size(); i++){
                        if(std::string(TextureFormatNames[i]) == "???") continue;
                        if(ImGui::Selectable(TextureFormatNames[i])){
                            texture->Format = i;
                        }
                    }
                    ImGui::EndCombo();
                }
                if(ImGui::Button("Replace")){
                    IGFD::FileDialogConfig cfg { .path = std::filesystem::current_path().string(), .flags = ImGuiFileDialogFlags_Modal };
                    ImGuiFileDialog::Instance()->OpenDialog("replaceTextureImageDialog", "Replace Texture", "PNG Image (*.png){.png}", cfg);
                }
                ImGui::SameLine();
                if(ImGui::Button("Export")){
                    IGFD::FileDialogConfig cfg { .path = std::filesystem::current_path().string(), .flags = ImGuiFileDialogFlags_Modal };
                    ImGuiFileDialog::Instance()->OpenDialog("exportTextureImageDialog", "Export Texture", "PNG Image (*.png){.png}", cfg);
                }
                ImGui::Image(static_cast<uintptr_t>(texture->TextureID), {static_cast<float>(texture->Width), static_cast<float>(texture->Height)});
                break;
            }
            case SelectedResourceType::Material: {
                ImGui::Text("Material");
                ImGui::Separator();
                auto material = reinterpret_cast<MDL::Material*>(SelectedResource);
                bool alphaFlag = material->AlphaFlag;
                ImGui::Checkbox("Alpha Flag", &alphaFlag);
                material->AlphaFlag = alphaFlag;
                ImGui::ColorPicker4("Diffuse Color", (float *)&material->DiffuseColor.x, 0);

                break;
            }
            case SelectedResourceType::Sampler: {
                ImGui::Text("Sampler");
                ImGui::Separator();
                auto sampler = reinterpret_cast<MDL::Sampler*>(SelectedResource);
                if(ImGui::BeginCombo("Wrap U", WrapModes[sampler->WrapU])){
                    for(int i = 0; i < 3; i++){
                        if(ImGui::Selectable(WrapModes[i], sampler->WrapU == i)){
                            sampler->WrapU = i;
                        }
                    }
                    ImGui::EndCombo();
                }
                if(ImGui::BeginCombo("Wrap V", WrapModes[sampler->WrapV])){
                    for(int i = 0; i < 3; i++){
                        if(ImGui::Selectable(WrapModes[i], sampler->WrapV == i)){
                            sampler->WrapV = i;
                        }
                    }
                    ImGui::EndCombo();
                }
                if(ImGui::BeginCombo("Texture", std::format("Texture {}", sampler->TextureIndex).c_str())){
                    for(auto [idx, texture] : mModelFurniture->mTextureHeaders){
                        if(ImGui::Selectable(std::format("Texture {}", idx).c_str())){
                            sampler->TextureIndex = idx;
                        }
                    }
                    ImGui::EndCombo();
                }
                break;
            }
        }
    }
}

void UContext::Render(float deltaTime) {
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::BeginFrame();

	RenderMenuBar();

	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoDockingInCentralNode;
	mMainDockSpaceID = ImGui::DockSpaceOverViewport(0, mainViewport, dockFlags);

	if(!bIsDockingSetUp){
		ImGui::DockBuilderRemoveNode(mMainDockSpaceID); // clear any previous layout
		ImGui::DockBuilderAddNode(mMainDockSpaceID, dockFlags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(mMainDockSpaceID, mainViewport->Size);

		mDockNodeHierarchyID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Left, 0.18f, nullptr, &mMainDockSpaceID);
		mDockNodeViewportID = ImGui::DockBuilderSplitNode(mMainDockSpaceID, ImGuiDir_Left, 0.83f, nullptr, &mDockNodeDetailsID);

		ImGui::DockBuilderDockWindow("sceneHierarchy", mDockNodeHierarchyID);
		ImGui::DockBuilderDockWindow("viewportWindow", mDockNodeViewportID);
		ImGui::DockBuilderDockWindow("detailWindow", mDockNodeDetailsID);

		ImGui::DockBuilderFinish(mMainDockSpaceID);
		bIsDockingSetUp = true;
	}


	ImGuiWindowClass mainWindowOverride;
	mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&mainWindowOverride);

	ImGui::Begin("viewportWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	    if(ImGui::BeginTabBar("Models")){
    		for(auto tab : mTabs){
    		    if(ImGui::BeginTabItem(tab->mName.c_str())){
                    mSelectedTab = tab;
                    ImGui::EndTabItem();
                }
    		}
    	    ImGui::EndTabBar();
		}

		ImVec2 winSize = ImGui::GetContentRegionAvail();
		ImVec2 cursorPos = ImGui::GetCursorScreenPos();

		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
		glBindRenderbuffer(GL_RENDERBUFFER, mRbo);

		if(winSize.x != mPrevWinWidth || winSize.y != mPrevWinHeight){
			glDeleteTextures(1, &mViewTex);
			glDeleteTextures(1, &mPickTex);
			glDeleteRenderbuffers(1, &mRbo);

			glGenRenderbuffers(1, &mRbo);
			glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (uint32_t)winSize.x, (uint32_t)winSize.y);

			glGenTextures(1, &mViewTex);
			glGenTextures(1, &mPickTex);

			glBindTexture(GL_TEXTURE_2D, mViewTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, mPickTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, (uint32_t)winSize.x, (uint32_t)winSize.y, 0, GL_RED_INTEGER, GL_INT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mViewTex, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPickTex, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRbo);

			GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, attachments);

			//assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		}

	    glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		mPrevWinWidth = winSize.x;
		mPrevWinHeight = winSize.y;

		glViewport(0, 0, winSize.x, winSize.y);
		glClearColor(0.19f, 0.19f, 0.19f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int32_t unused = -1;

		glClearTexImage(mPickTex, 0, GL_RED_INTEGER, GL_INT, &unused);

		// render tab content
		if(mSelectedTab != nullptr){
		    mSelectedTab->RenderModel(deltaTime);
		}

		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(cursorPos, cursorPos + winSize, 0xFF303030, 0xFF303030, 0xFF202020, 0xFF202020);
		ImGui::Image(static_cast<uintptr_t>(mViewTex), winSize, {0.0f, 1.0f}, {1.0f, 0.0f});
		if(ImGui::IsItemClicked(0) && !ImGuizmo::IsOver()){
    		int32_t id;
    		ImVec2 mousePos = ImGui::GetMousePos();
    		glPixelStorei(GL_PACK_ALIGNMENT, 1);
    		glReadBuffer(GL_COLOR_ATTACHMENT1);
    		glReadPixels(mousePos.x - cursorPos.x, ((uint32_t)winSize.y) - (mousePos.y - cursorPos.y), 1, 1, GL_RED_INTEGER, GL_INT, (void*)&id);
            if(id != -1) mSelectedTab->ViewportClicked(id);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		if(mSelectedTab != nullptr) mSelectedTab->RenderGizmos(cursorPos, winSize);

	ImGui::End();

	ImGui::SetNextWindowClass(&mainWindowOverride);
	ImGui::Begin("sceneHierarchy", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	if(mSelectedTab != nullptr){
	    mSelectedTab->RenderSceneTreePanel();
	}
	ImGui::End();

	ImGui::SetNextWindowClass(&mainWindowOverride);
	ImGui::Begin("detailWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	if(mSelectedTab != nullptr){
	    mSelectedTab->RenderDetailsPanel();
	}
	ImGui::End();

	// Notifications style setup
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.f); // Disable round borders
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f); // Disable borders
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f)); // Background color
    ImGui::RenderNotifications();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
	//mGrid.Render({(sin(Rotate) * (Zoom * 2)), Zoom, (cos(Rotate) * (Zoom * 2))}, mCamera.GetProjectionMatrix(), mCamera.GetViewMatrix());
}

void UContext::RenderMainWindow(float deltaTime) {
}

void UContext::RenderMenuBar() {
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
	    if (ImGui::BeginMenu("New")) {
			if (ImGui::MenuItem("Bin")) {
    			mTabs.push_back(std::make_shared<UEditorTab>(EModelType::Furniture));
    			mSelectedTab = mTabs.back();
    			mSelectedTab->mName = "New Bin";
			}
			if (ImGui::MenuItem("Mdl")) {
    			mTabs.push_back(std::make_shared<UEditorTab>(EModelType::Actor));
    			mSelectedTab = mTabs.back();
    			mSelectedTab->mName = "New Mdl";
			}
			if(ImGui::MenuItem("Bin From FBX")){
                mBinFBXImportOpen = true;
			}
            ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Open")) {
			mModelSelectOpen = true;
		}

		if (ImGui::MenuItem("Save")) {
			mModelSelectSave = true;
		}

		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if (mModelSelectOpen) {
		IGFD::FileDialogConfig config;
		ImGuiFileDialog::Instance()->OpenDialog("OpenModelDialog", "Open Model", "Room Model (*.bin){.bin},Actor Model (*.mdl){.mdl}, Furniture Animation (*.anm){.anm}", config);
	}

	if (mModelSelectSave) {
		IGFD::FileDialogConfig config;
		ImGuiFileDialog::Instance()->OpenDialog("SaveModelDialog", "Save Model", "Room Model (*.bin){.bin},Actor Model (*.mdl){.mdl}", config);
	}

	if (ImGuiFileDialog::Instance()->Display("OpenModelDialog")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string FilePath = ImGuiFileDialog::Instance()->GetFilePathName();
			mTabs.push_back(std::make_shared<UEditorTab>(std::filesystem::path(FilePath)));
			mSelectedTab = mTabs.back();

			mModelSelectOpen = false;
		} else {
			mModelSelectOpen = false;
		}

		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("SaveModelDialog")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string FilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			if(mSelectedTab != nullptr) mSelectedTab->SaveModel(FilePath);

			mModelSelectSave = false;
		} else {
			mModelSelectSave = false;
		}

		ImGuiFileDialog::Instance()->Close();
	}

}
