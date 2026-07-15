#pragma once

#pragma region WindowsIncludes
#define	WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS     
//#define NOVIRTUALKEYCODES 
//#define NOWINMESSAGES     
//#define NOWINSTYLES       
//#define NOSYSMETRICS      
#define NOMENUS           
#define NOICONS           
#define NOKEYSTATES       
#define NOSYSCOMMANDS     
#define NORASTEROPS       
//#define NOSHOWWINDOW      
#define OEMRESOURCE       
#define NOATOM            
#define NOCLIPBOARD       
#define NOCOLOR           
#define NOCTLMGR          
#define NODRAWTEXT        
#define NOGDI             
//#define NOKERNEL          
//#define NOUSER            
//#define NONLS
#define NOMB              
#define NOMEMMGR          
#define NOMETAFILE        
#define NOMINMAX          
//#define NOMSG             
#define NOOPENFILE        
#define NOSCROLL          
#define NOSERVICE         
#define NOSOUND           
#define NOTEXTMETRIC      
#define NOWH              
#define NOWINOFFSETS      
#define NOCOMM            
#define NOKANJI           
#define NOHELP            
#define NOPROFILER        
#define NODEFERWINDOWPOS  
#define NOMCX

#include <Windows.h>

#pragma endregion

#include <memory>

#include "Game/GameCamera.h"
#include "Game/GameObject.hpp"
#include "Game/GameWorld.h"

struct Scene;
class GraphicsCommandList;

class ModelViewer
{
public:
	ModelViewer();
	~ModelViewer();

	bool Initialize(SIZE aWindowSize, WNDPROC aWindowProcess, LPCWSTR aWindowTitle);
	int Run();

private:
	void LoadScene();
	void UpdateScene(float aDeltaTime);
	void BuildRenderScene();

	bool myIsRunning = false;

	HWND myMainWindowHandle = nullptr;

	std::unique_ptr<GraphicsCommandList> myGraphicsCommandList;
	std::unique_ptr<Scene> myScene;

	GameWorld myGameWorld;
	GameCamera myGameCamera;

	// TODO: TEMP
	GameObject* myAnimatedObject = nullptr;

	GameObject* myDirectionLightObject = nullptr;
	GameObject* myPointLightObject = nullptr;
	GameObject* mySpotLightObject = nullptr;
};
