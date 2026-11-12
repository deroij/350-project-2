#include "WindowManager.h"

namespace ECSEngine {

WindowManager::WindowManager(unsigned int width, unsigned int height, const std::string& title)
    : mWindow(std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), title)) {
        mWorldCenter.x = width / 2.0f;
        mWorldCenter.y = height / 2.0f;
    }

std::shared_ptr<sf::RenderWindow> WindowManager::GetWindow() const { return mWindow; }

Rect WindowManager::GetWindowBounds() const { 
    Rect r;
    r.topLeft.x = 0;
    r.topLeft.y = 0;
    r.width = static_cast<float>(mWindow->getSize().x);
    r.height = static_cast<float>(mWindow->getSize().y);
    return r;
 }

void WindowManager::DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha) {}

void WindowManager::DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha) {}

Rect WindowManager::WindowToWorld(const Rect& rect) const { 
    Rect out;
    out.topLeft = WindowToWorld(rect.topLeft);
    out.width = WindowToWorld(rect.width);
    out.height = WindowToWorld(rect.height);
    return out;
}

Point2D WindowManager::WindowToWorld(const Point2D& pt) const { 
    auto size = mWindow->getSize();
    Point2D out;
    out.x = (pt.x - size.x / 2.0f) * mWorldUnitsPerPixel + mWorldCenter.x;
    out.y = (pt.y - size.y / 2.0f) * mWorldUnitsPerPixel + mWorldCenter.y;
    return out;
 }

Rect WindowManager::WorldToWindow(const Rect& rect) const { 
    Rect out;
    out.topLeft = WorldToWindow(rect.topLeft);
    out.width = WorldToWindow(rect.width);
    out.height = WorldToWindow(rect.height);
    return out;
 }

Point2D WindowManager::WorldToWindow(const Point2D& pt) const { 
    auto size = mWindow->getSize();
    Point2D out;
    out.x = (pt.x - mWorldCenter.x) / mWorldUnitsPerPixel + size.x / 2.0f;
    out.y = (pt.y - mWorldCenter.y) / mWorldUnitsPerPixel + size.y / 2.0f;
    return out;
 }

void WindowManager::SetCamera(const Point2D& worldPt, const Point2D& screenPt) {}

Point2D WindowManager::GetWorldCenter() const { return mWorldCenter; }

float WindowManager::GetWorldScale() const { return mWorldUnitsPerPixel; }

void WindowManager::SetWorldCenter(const Point2D& worldCenter) { mWorldCenter = worldCenter; }

void WindowManager::SetWorldScale(float worldUnitsPerPixel) {   mWorldUnitsPerPixel = worldUnitsPerPixel; }

void WindowManager::SetWorldRect(const Rect& worldCameraRect) {}

float WindowManager::WindowToWorld(float x) const {
    return x * mWorldUnitsPerPixel; 
}

float WindowManager::WorldToWindow(float x) const {
    return x / mWorldUnitsPerPixel;
 }

}  // namespace ECSEngine
