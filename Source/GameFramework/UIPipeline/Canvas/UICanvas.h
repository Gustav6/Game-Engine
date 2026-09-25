//#pragma once
//
//#include <memory>
//#include <string>
//#include <vector>
//
//class UIElement;
//
//class UICanvas
//{
//public:
//    void Update(float aDeltaTime);
//    void Render();
//
//    void AddElement(std::unique_ptr<UIElement> aElement);
//
//    UIElement* GetElement(const std::string& aName);
//
//    void SetVisible(bool aVisible);
//    bool GetVisible() const;
//
//private:
//    std::vector<std::unique_ptr<UIElement>> myElements;
//
//    bool myVisible = true;
//};