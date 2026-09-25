//#include "UICanvas.h"
//#include "../Elements/UIElement.h"
//
//void UICanvas::Update(float aDeltaTime)
//{
//    if (!myVisible)
//        return;
//
//    for (auto& element : myElements)
//    {
//        if (element->GetVisible())
//            element->Update(aDeltaTime);
//    }
//}
//
//void UICanvas::Render()
//{
//    if (!myVisible)
//        return;
//
//    for (auto& element : myElements)
//    {
//        if (element->GetVisible())
//            element->Render();
//    }
//}
//
//void UICanvas::AddElement(std::unique_ptr<UIElement> aElement)
//{
//    myElements.push_back(std::move(aElement));
//}
//
//UIElement* UICanvas::GetElement(const std::string& aName)
//{
//    for (auto& element : myElements)
//    {
//        if (element->GetName() == aName)
//            return element.get();
//    }
//
//    return nullptr;
//}
//
//void UICanvas::SetVisible(bool aVisible)
//{
//    myVisible = aVisible;
//}
//
//bool UICanvas::GetVisible() const
//{
//    return myVisible;
//}