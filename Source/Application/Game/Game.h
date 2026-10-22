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

class Actor;
class GraphicsCommandList;
class GameStateMachine;

struct FrameStats;

#ifdef IMGUI_ENABLE
class ImGuiLayer;
#endif

class Game
{
public:
	Game();
	~Game();

	bool Initialize(SIZE aWindowSize, WNDPROC aWindowProcess, LPCWSTR aWindowTitle);
	int Run();

private:
	void UpdateScene(float aDeltaTime) const;
	void RenderScene() const;

	std::unique_ptr<GraphicsCommandList> myGraphicsCommandList;
	std::unique_ptr<GameStateMachine> myStateMachine;

	HWND myMainWindowHandle = nullptr;

	bool myIsRunning = false;

#ifdef IMGUI_ENABLE
	std::unique_ptr<ImGuiLayer> myImGuiLayer;

	bool myShowDebugWindow = true;
	bool myShowLevelSelectorWindow = true;

	void DrawImGuiLayer(const FrameStats& aFrameStats);
	void DrawActorNode(Actor* aActor);
#endif
};
