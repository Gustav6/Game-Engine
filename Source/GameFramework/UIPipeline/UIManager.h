//#pragma once
//
//#include <memory>
//#include <string>
//#include <unordered_map>
//
//class UICanvas;
//
//class UIManager
//{
//public:
//    static UIManager& Get();
//
//    void Update(float aDeltaTime);
//    void Render();
//
//    UICanvas* CreateCanvas(const std::string& aName);
//    UICanvas* GetCanvas(const std::string& aName);
//
//private:
//    std::unordered_map<std::string, std::unique_ptr<UICanvas>> myCanvases;
//};