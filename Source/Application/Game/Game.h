#pragma once

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

#include <memory>

#include "Camera/GameCamera.h"

struct Scene;
class GraphicsCommandList;

class Game
{
public:
	Game();
	~Game();

	bool Initialize(SIZE aWindowSize, WNDPROC aWindowProcess, LPCWSTR aWindowTitle);
	int Run();

private:
	void LoadScene();
	void UpdateScene(float aDeltaTime);
	void BuildSceneForRender();

	bool myIsRunning = false;

	HWND myMainWindowHandle = nullptr;

	std::unique_ptr<GraphicsCommandList> myGraphicsCommandList;
	std::unique_ptr<Scene> myScene;



	// TODO: TEMP

	Actor* myPlayer = nullptr;

	Actor* myAnimatedObject = nullptr;

	Actor* myDirectionLightObject = nullptr;
	Actor* myPointLightObject = nullptr;
	Actor* mySpotLightObject = nullptr;
};
