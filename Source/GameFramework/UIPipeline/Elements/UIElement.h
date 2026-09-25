//#pragma once
//
//#include <string>
//
//class UIElement
//{
//public:
//    virtual ~UIElement() = default;
//
//    virtual void Update(float aDeltaTime) {}
//    virtual void Render() = 0;
//
//    void SetPosition(float aX, float aY);
//    void SetSize(float aWidth, float aHeight);
//
//    void SetVisible(bool aVisible);
//    bool GetVisible() const;
//
//    const std::string& GetName() const;
//    void SetName(const std::string& aName);
//
//protected:
//    std::string myName;
//
//    float myX = 0.0f;
//    float myY = 0.0f;
//
//    float myWidth = 100.0f;
//    float myHeight = 100.0f;
//
//    bool myVisible = true;
//};