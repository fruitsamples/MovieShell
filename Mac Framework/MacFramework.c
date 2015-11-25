/*	File:		MacFramework.c	Contains:	Basic Macintosh Functions for Window, Menu handling and similar things.	Written by:	DTS	Copyright:	� 1994-1995 by Apple Computer, Inc., all rights reserved.	Change History (most recent first):	   <1>	 	12/20/94	khs		first file	   */// INCLUDES#include <SegLoad.h>#include <ToolUtils.h>#include <Devices.h>#include <Fonts.h>#include "DTSQTUtilities.h"#include "AppConfiguration.h"#include "MacFramework.h"// WINDOW DEFINITIONSconst Rect kDefaultWinRect;Rect kLimitRect = {0, 0, 480, 640};							//Max size for any windowlong gMCFlags = kMCFlags;// GLOBALSBoolean 			gQuitFlag = false;								// Flag that keeps track of termination state.unsigned long 	gWNEsleep = kWNEDefaultSleep;			// WaitNextEvent sleep time.Str255 			gWindowTitle = "\pUntitled";			// Default name for created windows.GrowZoneUPP	gAppGrowZoneUPP;							// Our grow zone callback.	Boolean				gJustOneMovie = false;						// Flag for indication that one movie has been created,																				// in other words a limitation flag for single movie environment.// PURE MAC TOOLBOX FUNCTIONS// ______________________________________________________________________void InitMacEnvironment(long nMasters){	long i;	MaxApplZone();		for(i = 0; i <nMasters; i++)		MoreMasters();		InitGraf(&qd.thePort);	InitFonts();	InitWindows();	InitMenus();	FlushEvents(everyEvent, 0);	TEInit();	InitCursor();	InitDialogs(NULL);		// Additional goodie, install a growzone proc warning about low memory situation	 gAppGrowZoneUPP = NewGrowZoneProc(AppGrowZoneCallback);	 SetGrowZone(gAppGrowZoneUPP);}// ______________________________________________________________________void InitStack(long extraStackSpace){	Ptr size = GetApplLimit();	SetApplLimit(size - extraStackSpace);	// make room on the stack}// ______________________________________________________________________pascal void AppGrowZoneCallback(void){	long 		theA5;	Size		tempSize, availMem;		theA5 = SetCurrentA5();		availMem = MaxMem(&tempSize);	if(availMem < kAvailableMem)	{		ShowWarning("\pWe are running out of memory, increase the application heap! Exiting the application.", 0);		ExitToShell();	}		SetA5(theA5);}// ______________________________________________________________________Boolean InitMenubar(void){	Handle aMenuHandle = NULL;		aMenuHandle = GetNewMBar(mMenubar); DebugAssert(aMenuHandle != NULL);	if(aMenuHandle == NULL)	{		ShowWarning("\pCould not find the Menubar resource!", 0);		return false;	}		SetMenuBar(aMenuHandle);	DisposeHandle(aMenuHandle);  DebugAssert(MemError() == noErr);		AddResMenu(GetMHandle(mApple), 'DRVR');	DrawMenuBar();	return true;}// ______________________________________________________________________void HandleMenuCommand(long theMenuResult){	short		 	aMenuID, aMenuItem;	Str255			daName;		aMenuID = HiWord(theMenuResult);	aMenuItem = LoWord(theMenuResult);		switch(aMenuID)	{		// APPLE MENU		case mApple:			switch(aMenuItem)			{				case iAbout:	// about box					ShowAboutDialogBox();	 					break;								default:	 // Apple menu handling					GetItem(GetMHandle(mApple), aMenuItem, daName);					(void)OpenDeskAcc(daName);					break;			} // end switch(aMenuItem)			break;		// FILE MENU					case mFile:			switch(aMenuItem)			{				case iNew:					{#ifdef ONEMOVIELIMIT						if(gJustOneMovie == false)#endif						{							if ( !DoCreateNewMovie())							{								SysBeep(kDefaultSysBeep);								ShowWarning("\pCould not create a new movie!", 0);								break;							}						}#ifdef ONEMOVIELIMIT						gJustOneMovie = true;#endif					}					break;									case iOpen:#ifdef ONEMOVIELIMIT					if(gJustOneMovie == false)#endif					{						if (!DoCreateMovieWindow( NULL) )						{							ShowWarning("\pCould not create a new movie window!", 0);							SysBeep(kDefaultSysBeep); 							break;						}					}					#ifdef ONEMOVIELIMIT					gJustOneMovie = true;#endif					break;								case iClose:					DoDestroyMovieWindow( FrontWindow() );#ifdef ONEMOVIELIMIT					gJustOneMovie = false;#endif					break;								case iSave:					{						if( !DoUpdateMovieFile( FrontWindow()) )						{							SysBeep(kDefaultSysBeep);							ShowWarning("\pCould not save the movie file!", 0);							 break;						}					}					break;									case iSaveAs:					{						MovieController mc;											mc = GetMCFromFrontWindow();						if(mc == NULL) 						{							SysBeep(kDefaultSysBeep); 							break;						}								 				if( QTUSaveMovie(MCGetMovie(mc)) != noErr)		 				{		 					SysBeep(kDefaultSysBeep);		 					ShowWarning("\pCould not save the movie file!", 0);		 					break;		 				}					 }					break;													case iPrint:					{						MovieController mc;						OSErr anErr = noErr;						mc = GetMCFromFrontWindow();						if(mc != NULL)						{							anErr = QTUPrintMoviePICT( MCGetMovie(mc), kDefaultX, kDefaultY, kPrintFrame); 							if(anErr != noErr)							{								ShowWarning("\pCould not print!", anErr);								SysBeep(kDefaultSysBeep);							}						}						else								SysBeep(kDefaultSysBeep);						break;					}				case iQuit:					{						gQuitFlag = true;						break;					}			} // end switch(aMenuItem), mFile			break;					// EDIT MENU		// Provide the default controller cut, copy and paste functionality.			case mEdit:		{			Movie aMovie = NULL;			MovieController mc;						mc = GetMCFromFrontWindow();			if (mc == NULL) break;						switch(aMenuItem)			{				case iUndo: MCUndo(mc); break;								case iCut: aMovie = MCCut(mc); break;								case iCopy: aMovie = MCCopy(mc); break;								case iPaste: MCPaste(mc, NULL); break;								case iClear: MCClear(mc); break;								case iSelectAll:  					if(QTUSelectAllMovie(mc) != noErr)						SysBeep(kDefaultSysBeep);					break;			} // end switch(aMenuItem)						if(aMovie)			{				PutMovieOnScrap(aMovie, 0);				DisposeMovie(aMovie);  DebugAssert(MemError() == noErr);			}			break;		} // end case mEdit	default:		HandleApplicationMenu(aMenuID, aMenuItem);		break;	} // end switch(aMenuID)		HiliteMenu(0);}// ______________________________________________________________________void AdjustMenus(void){	WindowRef 			aWindow;	MovieController	mc;	WindowObject		aWindowObject;		#ifdef ONEMOVIELIMIT				if(gJustOneMovie == true)				{					DisableItem(GetMHandle(mFile), iNew);					DisableItem(GetMHandle(mFile), iOpen);				}				else if(gJustOneMovie == false)				{					EnableItem(GetMHandle(mFile), iNew);					EnableItem(GetMHandle(mFile), iOpen);				}#endif		aWindow = FrontWindow();	if(aWindow != NULL)	{		// Enable the close entry of we have windows = movies.		EnableItem( GetMHandle(mFile), iClose);				// Handle the edit menu.		if( (aWindowObject = (WindowObject)GetWRefCon(aWindow) ) != NULL)		{			mc = (**aWindowObject).controller;			if( (IsWindowObjectOurs(aWindowObject)) && (mc != NULL))			{				MCSetUpEditMenu(mc, 0L, GetMHandle(mEdit));				EnableItem(GetMHandle(mEdit), iSelectAll);				EnableItem(GetMHandle(mFile), iSave);				EnableItem(GetMHandle(mFile), iSaveAs);				EnableItem(GetMHandle(mFile), iClose);				EnableItem(GetMHandle(mFile), iPrint);			}		}	} // end if(aWindow != NULL)	else 	{		DisableItem(GetMHandle(mFile), iSave);		DisableItem(GetMHandle(mFile), iSaveAs);		DisableItem(GetMHandle(mFile), iClose);		DisableItem(GetMHandle(mFile), iPrint);				DisableItem(GetMHandle(mEdit), iCut);		DisableItem(GetMHandle(mEdit), iCopy);		DisableItem(GetMHandle(mEdit), iPaste);		DisableItem(GetMHandle(mEdit), iUndo);		DisableItem(GetMHandle(mEdit), iClear);		DisableItem(GetMHandle(mEdit), iSelectAll);			}		AdjustApplicationMenus();					// fix any specific app menus as well.}// ______________________________________________________________________void MainEventLoop(void){	EventRecord 			anEvent;	WindowRef			whichWindow, aWindow;	Boolean					aMovieEvent;	short					aWindowPart;	Rect						aScreenRect;	Rect						aRefreshArea;	Point						aPoint  = {100, 100};	WindowObject		aWindowObject;	MovieController	mc;		while(!gQuitFlag)	{		WaitNextEvent(everyEvent, &anEvent, gWNEsleep, NULL);		#ifdef USESIOUX		SIOUXHandleOneEvent(&anEvent);#endif USESIOUX		AdjustMenus();		aMovieEvent = false;				if( (whichWindow = FrontWindow() ) != NULL)			DoIdle(whichWindow);		// First, let the movie controller have access to the event.		for( aWindow = FrontWindow(); aWindow != NULL ; aWindow = (WindowPtr)((WindowPeek)aWindow)->nextWindow)			if(( aWindowObject = (WindowObject)GetWRefCon(aWindow)) != NULL) 						if((IsWindowObjectOurs(aWindowObject)) && ( (mc = (**aWindowObject).controller) != NULL) ) 					if(MCIsPlayerEvent(mc, &anEvent)) 						aMovieEvent = true ;						// Then, if this wasn't a movie controller event, pass it on to the case statement that dispatches the event	// to the right function.	if(!aMovieEvent)	{		switch(anEvent.what)		{			case mouseDown:				aWindowPart = FindWindow(anEvent.where, &whichWindow);				// Window related events:							switch(aWindowPart)				{					case inMenuBar:						HandleMenuCommand(MenuSelect(anEvent.where));						break;											case inDrag:					{						Rect 						aRect;						Movie					aMovie = NULL;						MovieController 	mc = NULL;						WindowObject 		aWindowObject = NULL;												aWindowObject = (WindowObject)GetWRefCon(whichWindow);						mc = (**aWindowObject).controller;						if (! (IsWindowObjectOurs(aWindowObject)) && (mc == NULL))							break;													aMovie = MCGetMovie(mc);												GetMovieBox(aMovie, &aRect);						aScreenRect = (**GetGrayRgn()).rgnBBox;						DragAlignedWindow(whichWindow, anEvent.where, &aScreenRect, &aRect, NULL);					}  // end case inDrag;							break;											case inContent:						SelectWindow(whichWindow);						HandleContentClick(whichWindow, &anEvent);						break;										case inGoAway:						// if the window is closed, dispose the movie, the controller and the window						if( TrackGoAway(whichWindow, anEvent.where) )							DoDestroyMovieWindow(whichWindow);#ifdef ONEMOVIELIMIT							gJustOneMovie = false;#endif						break;				} // end switch(aWindowPart):				break;				// System level events:				case updateEvt:					whichWindow = (WindowRef)anEvent.message;					aRefreshArea = ((**(whichWindow->visRgn)).rgnBBox);					DoUpdateWindow(whichWindow, &aRefreshArea);					break;									case keyDown:				case autoKey:					HandleKeyPress(&anEvent);					break;								case diskEvt:					if(HiWord(anEvent.message) != noErr)						(void)DIBadMount(aPoint, anEvent.message);					break;								case activateEvt:					whichWindow = (WindowRef)anEvent.message;										 if ( IsAppWindow(whichWindow) )					{						DoActivateWindow(whichWindow, ((anEvent.modifiers & activeFlag) != 0 ));					}					break;									case osEvt:					switch(( anEvent.message > 24) & 0x00FF )		// get high byte of word					{						case suspendResumeMessage:							if( FrontWindow() )							{								DoActivateWindow(FrontWindow(), !((anEvent.message & resumeFlag) == 0));							}							break;												case mouseMovedMessage:							break;					} // end switch(anEvent.message > 24) & 0x00FF)						break;								case nullEvent:					if(( whichWindow = FrontWindow() ) != NULL)						DoIdle(whichWindow);					break;		} // end switch(anEvent.what)	} // end if(!aMovieEvent)		} // end while(!gQuitFlag)}// ______________________________________________________________________Boolean IsAppWindow(WindowRef theWindow){	short aWindowKind;		if (theWindow == NULL)		return false;	else	{		aWindowKind = ((WindowPeek)theWindow)->windowKind;		return ( (aWindowKind >= userKind) || (aWindowKind == dialogKind) );	}}// ______________________________________________________________________WindowObject CreateWindowObject(WindowRef theWindow){	WindowObject aWindowObject = NULL;		// WindowObjectRecord = 90 bytes (good to know if chasing for handles in the heap).	aWindowObject = (WindowObject)NewHandle(sizeof(WindowObjectRecord));		if(aWindowObject != NULL)	{		(**aWindowObject).controller = NULL;		(**aWindowObject).ObjectType = kMovieControllerObject;				SetWRefCon(theWindow, (long)aWindowObject);		// store a ref to the record/handle into the window	}		return aWindowObject;}// ______________________________________________________________________void HandleKeyPress(EventRecord *theEvent){	char aKey;		aKey = theEvent->message & charCodeMask;		if(theEvent->modifiers & cmdKey)		// command key down?	{		HandleMenuCommand(MenuKey(aKey));	}}// ______________________________________________________________________void ShowAboutDialogBox(void){	DialogPtr aDialog;	short 		itemHit;	FontInfo	aFontInfo;	GrafPtr		aSavedPort;		GetPort(&aSavedPort);	aDialog = GetNewDialog(kAboutBox, NULL, (WindowPtr) - 1L); DebugAssert(aDialog != NULL);	SetPort(aDialog);	// Change font to Geneva, 9pt, bold, just for the sake of it...	TextFont(applFont); TextSize(9); TextFace(bold);	GetFontInfo(&aFontInfo);		(*((DialogPeek)aDialog)->textH)->txFont = applFont;	(*((DialogPeek)aDialog)->textH)->txSize = 9;	(*((DialogPeek)aDialog)->textH)->lineHeight = aFontInfo.ascent + aFontInfo.descent + aFontInfo.leading;	(*((DialogPeek)aDialog)->textH)->fontAscent = aFontInfo.ascent;	SetDialogDefaultItem(aDialog, 1);			do	{		ModalDialog(NULL, &itemHit);	} while(itemHit != ok);		SetPort(aSavedPort);	DisposeDialog(aDialog);  DebugAssert(MemError() == noErr);}// ______________________________________________________________________void ShowWarning(Str255 theMessage, OSErr theErr){	Str255 errString;		NumToString(theErr, errString);	ParamText("\pWarning!", theMessage, theErr ? errString:  NULL, NULL);	Alert(kAlertError, NULL);}// MOVIE RELATED TOOLBOX FUNCTIONS// ______________________________________________________________________MovieController  GetMCFromFrontWindow(void){	MovieController 	mc = NULL;	WindowRef 			aWindow = NULL;	WindowObject		aWindowObject = NULL;	Movie					aMovie = NULL;	OSErr					anErr = noErr;	OSType					aType = NULL;	if( ( aWindow = FrontWindow() ) == NULL )		return NULL;	if( !IsAppWindow(aWindow) )		return NULL;				aWindowObject = (WindowObject)GetWRefCon(aWindow);	if(aWindowObject == NULL)		return NULL;			MoveHHi((Handle)aWindowObject); HLock((Handle)aWindowObject);	// Test if this is indeed a movie controller, and not an otherwise valid pointer (non-NULL value)	if(!IsWindowObjectOurs(aWindowObject))		return NULL;			mc = (**aWindowObject).controller;	HUnlock((Handle)aWindowObject);		return mc;}// ______________________________________________________________________Boolean IsWindowObjectOurs(WindowObject theObject){	OSType		aType = NULL;		aType = (**theObject).ObjectType;	if(aType == kMovieControllerObject)		return true;	else return false;}// ______________________________________________________________________Boolean DoCreateNewMovie(void){	Movie aMovie = NULL;		aMovie = NewMovie(newMovieActive); DebugAssert(aMovie != NULL);	if(aMovie == NULL)		return false;		if(!DoCreateMovieWindow(aMovie))		return false;	else			return true;}// ______________________________________________________________________Boolean DoCreateMovieWindow(Movie theMovie){	Rect 						aRect = kDefaultWinRect;	WindowRef			aWindow = NULL;	MovieController	mc = NULL;	WindowObject		aWindowObject = NULL;	GrafPtr					aSavedPort;	short					aRefNum;	short					aResID;	FSSpec					aFileFSSpec;		aFileFSSpec.vRefNum = 0;			// we want to use the FSSpec later	GetPort(&aSavedPort);	aWindow = CreateMovieWindow(&aRect, gWindowTitle);	SetPort((GrafPtr) aWindow);		if(aWindow == NULL)		return false;		aWindowObject = CreateWindowObject(aWindow);	if(aWindowObject == NULL)		return false;//If we don't get a movie, call the internal QTUGetMovie that will get us one.	if(theMovie == NULL)	{		theMovie = QTUGetMovie(&aFileFSSpec, &aRefNum, &aResID);  DebugAssert(theMovie != NULL);		if(theMovie == NULL)  // User selected cancel, or otherwise something bad happened.			return false;					// Add the FSSpec, refnum and resid values to the window object (we need these when we save the movie).		(**aWindowObject).FileFSSpec = aFileFSSpec;		(**aWindowObject).FileRefNum = aRefNum;		(**aWindowObject).FileResID = aResID;		// Get movie title and set this to the window title.		SetWTitle(aWindow, aFileFSSpec.name);	}		SetMovieGWorld(theMovie, (CGrafPtr)aWindow, 0);		// make sure the movie uses the window GWorld at all situations	mc = SetupMovieWindowWithController(theMovie, aWindow);		ShowWindow(aWindow);	SelectWindow(aWindow);									// make it front-most as it's just created	InvalRect( &((GrafPtr)aWindow)->portRect);		MCEnableEditing(mc, true);								// enable the default movie controller editing		SetPort(aSavedPort);	return true;}// ______________________________________________________________________MovieController SetupMovieWindowWithController(Movie theMovie, WindowRef theWindow){	MovieController	 mc;	Rect						aRect;	GrafPtr					aSavedPort;	WindowObject		aWindowObject;	short					aMovieWidth;	short					aMovieHeight;		DebugAssert(theMovie != NULL); 	DebugAssert(theWindow != NULL);		aWindowObject = (WindowObject)GetWRefCon(theWindow);			// Get our window specific data.	if(!IsWindowObjectOurs(aWindowObject))		return NULL;																				// Quick sanity test of the window created.	GetPort(&aSavedPort);	SetPort( (GrafPtr)theWindow);	// Resize the movie bounding rect.	GetMovieBox(theMovie, &aRect);	 SetMovieBox(theMovie, &aRect);// Create the movie controller.		mc = NewMovieController(theMovie, &aRect, gMCFlags);	if(mc == NULL)		return NULL;	MCGetControllerBoundsRect(mc, &aRect);	// Add grow box for the movie controller and also an action filter that resizes the controllers	MCDoAction(mc, mcActionSetGrowBoxBounds, &kLimitRect);	MCSetActionFilterWithRefCon(mc, 													NewMCActionFilterWithRefConProc(QTUResizeMCActionFilter),													(long) theWindow);														// Check if the bounding rects are sane.	aMovieWidth = aRect.right - aRect.left;	aMovieHeight = aRect.bottom - aRect.top;		aRect.top = aRect.left  = 0;	aRect.right = aMovieWidth;	aRect.bottom = aMovieHeight;		// Resize the window as well.	SizeWindow(theWindow, aMovieWidth, aMovieHeight, true);	MoveWindow(theWindow, kDefaultX, kDefaultY, false);									SetPort(aSavedPort);	// Add any additional controller functionality.	AddControllerFunctionality(mc);	// Save important stuff into the window object	 and the original size of the movie	{		Rect aRect;	OSErr anErr;	(**aWindowObject).controller = mc;	anErr = MCGetControllerBoundsRect(mc, &aRect); DebugAssert(anErr == noErr);	(**aWindowObject).originalSize = aRect;	}		return mc;}// ______________________________________________________________________Boolean DoUpdateMovieFile(WindowRef theWindow){	Movie 					aMovie = NULL;	WindowObject		aWindowObject = NULL;	MovieController	mc = NULL;	OSErr					anErr;		if ( (theWindow == NULL) || !IsAppWindow(theWindow) )		return false;			aWindowObject = (WindowObject)GetWRefCon(theWindow); DebugAssert(aWindowObject != NULL);	mc = (**aWindowObject).controller; DebugAssert(mc != NULL);		if( !(IsWindowObjectOurs(aWindowObject)) && (mc == NULL) )		return false;		aMovie = MCGetMovie(mc); DebugAssert(aMovie != NULL);	if(aMovie == NULL)		return false;			if( (**aWindowObject).FileRefNum == -1)					// brand new movie, no file attached to it.	{		if ( QTUSaveMovie(aMovie) != noErr)			return false;		}	else																			// we have an existing file, just update the movie resource	{		// Open the movie resource file, update the resource, and then close it again!		anErr = OpenMovieFile(& (**aWindowObject).FileFSSpec, & (**aWindowObject).FileRefNum, fsRdWrPerm);		DebugAssert(anErr == noErr);		if(anErr != noErr)			return false;				anErr = UpdateMovieResource(aMovie, (**aWindowObject).FileRefNum, (**aWindowObject).FileResID, NULL);		DebugAssert(anErr == noErr);				CloseMovieFile( (**aWindowObject).FileRefNum );	}		if(anErr == noErr)		return true;	else		return false;}// ______________________________________________________________________void DoDestroyMovieWindow(WindowRef theWindow){	Movie 					aMovie;	MovieController	mc;	WindowObject		aWindowObject;		DebugAssert(theWindow != NULL); if(theWindow == NULL) return;		aWindowObject =(WindowObject)GetWRefCon(theWindow);	MoveHHi((Handle)aWindowObject);	HLock((Handle)aWindowObject);		if ( IsWindowObjectOurs(aWindowObject)) // our window?	{		mc = (**aWindowObject).controller;		aMovie = MCGetMovie(mc);				if(aMovie != NULL)			DisposeMovie(aMovie); DebugAssert(MemError() == noErr);			if(mc != NULL)			DisposeMovieController(mc); DebugAssert(MemError() == noErr);			if( (**aWindowObject).FileRefNum != -1)			CloseMovieFile((**aWindowObject).FileRefNum);				(**aWindowObject).ObjectType = NULL;		(**aWindowObject).controller = NULL;		(**aWindowObject).FileResID = NULL;		(**aWindowObject).FileRefNum = NULL;				DisposeHandle((Handle)aWindowObject); DebugAssert(MemError() == noErr);		DisposeWindow(theWindow); DebugAssert(MemError() == noErr);				CompactMem(0xFFFFFFFF);		//We might as well compact the mem here for getting better performance later.	}}// ______________________________________________________________________void DoActivateWindow(WindowRef theWindow, Boolean becomingActive){	WindowObject 		aWindowObject = NULL;	MovieController	mc = NULL;	GrafPtr					aSavedPort = NULL;		GetPort(&aSavedPort);	SetPort((GrafPtr)theWindow);		if( (aWindowObject = (WindowObject)GetWRefCon(theWindow)) != NULL)	{		mc = (**aWindowObject).controller;		if( (IsWindowObjectOurs(aWindowObject)) && (mc != NULL) )			MCActivate(mc, theWindow, becomingActive);	}	SetPort(aSavedPort);}