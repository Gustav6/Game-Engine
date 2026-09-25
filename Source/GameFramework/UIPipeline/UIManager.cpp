//#include "UIManager.h"
//#include "Canvas/UICanvas.h"
//
//UIManager& UIManager::Get()
//{
//    static UIManager instance;
//    return instance;
//}
//
//void UIManager::Update(float aDeltaTime)
//{
//    for (auto& [name, canvas] : myCanvases)
//    {
//        canvas->Update(aDeltaTime);
//    }
//}
//
//void UIManager::Render()
//{
//    for (auto& [name, canvas] : myCanvases)
//    {
//        canvas->Render();
//    }
//}
//
//UICanvas* UIManager::CreateCanvas(const std::string& aName)
//{
//    auto canvas = std::make_unique<UICanvas>();
//
//    UICanvas* canvasPtr = canvas.get();
//
//    myCanvases[aName] = std::move(canvas);
//
//    return canvasPtr;
//}
//
//UICanvas* UIManager::GetCanvas(const std::string& aName)
//{
//    auto it = myCanvases.find(aName);
//
//    if (it == myCanvases.end())
//        return nullptr;
//
//    return it->second.get();
//}