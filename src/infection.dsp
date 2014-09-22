# Microsoft Developer Studio Project File - Name="infection" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=infection - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "infection.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "infection.mak" CFG="infection - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "infection - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "infection - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "infection - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcd.lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "infection - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "infection - Win32 Release"
# Name "infection - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\edit.c
# End Source File
# Begin Source File

SOURCE=.\entity_iterate.c
# End Source File
# Begin Source File

SOURCE=.\extra_genesis.c
# End Source File
# Begin Source File

SOURCE=.\globalGenesis.c
# End Source File
# Begin Source File

SOURCE=.\globals.c
# End Source File
# Begin Source File

SOURCE=.\inf_actor.c
# End Source File
# Begin Source File

SOURCE=.\inf_gui.c
# End Source File
# Begin Source File

SOURCE=.\inf_lua.c
# End Source File
# Begin Source File

SOURCE=.\inf_meny.c
# End Source File
# Begin Source File

SOURCE=.\infection.c
# End Source File
# Begin Source File

SOURCE=.\init.c
# End Source File
# Begin Source File

SOURCE=.\inventory.c
# End Source File
# Begin Source File

SOURCE=.\level.c
# End Source File
# Begin Source File

SOURCE=.\log.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\muliint.c
# End Source File
# Begin Source File

SOURCE=.\options.c
# End Source File
# Begin Source File

SOURCE=.\resources.rc
# End Source File
# Begin Source File

SOURCE=.\subtitles.c
# End Source File
# Begin Source File

SOURCE=.\time_out.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\decals.h
# End Source File
# Begin Source File

SOURCE=.\edit.h
# End Source File
# Begin Source File

SOURCE=.\entity_iterate.h
# End Source File
# Begin Source File

SOURCE=.\extra_genesis.h
# End Source File
# Begin Source File

SOURCE=.\globalGenesis.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\inf_actor.h
# End Source File
# Begin Source File

SOURCE=.\inf_gui.h
# End Source File
# Begin Source File

SOURCE=.\inf_lua.h
# End Source File
# Begin Source File

SOURCE=.\inf_meny.h
# End Source File
# Begin Source File

SOURCE=.\infection.h
# End Source File
# Begin Source File

SOURCE=.\init.h
# End Source File
# Begin Source File

SOURCE=.\inventory.h
# End Source File
# Begin Source File

SOURCE=.\keys.h
# End Source File
# Begin Source File

SOURCE=.\libraries.h
# End Source File
# Begin Source File

SOURCE=.\log.h
# End Source File
# Begin Source File

SOURCE=.\multiint.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\subtitles.h
# End Source File
# Begin Source File

SOURCE=.\time_out.h
# End Source File
# Begin Source File

SOURCE=.\useful_macros.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# End Group
# Begin Group "DecalMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Decal.c
# End Source File
# Begin Source File

SOURCE=.\Decal.h
# End Source File
# End Group
# Begin Group "EffectManager"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\e_rain.c
# End Source File
# Begin Source File

SOURCE=.\e_rain.h
# End Source File
# Begin Source File

SOURCE=.\e_spout.c
# End Source File
# Begin Source File

SOURCE=.\e_spout.h
# End Source File
# Begin Source File

SOURCE=.\e_sprite.c
# End Source File
# Begin Source File

SOURCE=.\e_sprite.h
# End Source File
# Begin Source File

SOURCE=.\EffManager.c
# End Source File
# Begin Source File

SOURCE=.\EffManager.h
# End Source File
# Begin Source File

SOURCE=.\EffParticle.c
# End Source File
# Begin Source File

SOURCE=.\EffParticle.h
# End Source File
# Begin Source File

SOURCE=.\SPool.c
# End Source File
# Begin Source File

SOURCE=.\SPool.h
# End Source File
# Begin Source File

SOURCE=.\StaticEntity.h
# End Source File
# Begin Source File

SOURCE=.\TPool.c
# End Source File
# Begin Source File

SOURCE=.\TPool.h
# End Source File
# End Group
# Begin Group "BasicGameElements"

# PROP Default_Filter ""
# Begin Group "Hud"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\crosshair.c
# End Source File
# Begin Source File

SOURCE=.\crosshair.h
# End Source File
# Begin Source File

SOURCE=.\hud.c
# End Source File
# Begin Source File

SOURCE=.\hud.h
# End Source File
# End Group
# Begin Group "Console"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\console.c
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\ConsoleBuffer.c
# End Source File
# Begin Source File

SOURCE=.\ConsoleBuffer.h
# End Source File
# End Group
# Begin Group "Sounds"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sound_system.c
# End Source File
# Begin Source File

SOURCE=.\sound_system.h
# End Source File
# End Group
# Begin Group "Messages"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\inf_message_system.c
# End Source File
# Begin Source File

SOURCE=.\inf_message_system.h
# End Source File
# End Group
# Begin Group "conversation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\conversation.c
# End Source File
# Begin Source File

SOURCE=.\conversation.h
# End Source File
# End Group
# End Group
# Begin Group "PlayerData"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\damage.c
# End Source File
# Begin Source File

SOURCE=.\damage.h
# End Source File
# Begin Source File

SOURCE=.\jacktalks.c
# End Source File
# Begin Source File

SOURCE=.\jacktalks.h
# End Source File
# Begin Source File

SOURCE=.\player.c
# End Source File
# Begin Source File

SOURCE=.\player.h
# End Source File
# Begin Source File

SOURCE=.\playerSounds.h
# End Source File
# Begin Source File

SOURCE=.\time_damage.c
# End Source File
# Begin Source File

SOURCE=.\time_damage.h
# End Source File
# Begin Source File

SOURCE=.\timefx.c
# End Source File
# Begin Source File

SOURCE=.\timefx.h
# End Source File
# Begin Source File

SOURCE=.\weapon.c
# End Source File
# Begin Source File

SOURCE=.\weapons.h
# End Source File
# End Group
# Begin Group "Enemies"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\enemies.c
# End Source File
# Begin Source File

SOURCE=.\enemies.h
# End Source File
# Begin Source File

SOURCE=.\enemy_def.h
# End Source File
# Begin Source File

SOURCE=.\enemy_rat.c
# End Source File
# Begin Source File

SOURCE=.\enemy_rat.h
# End Source File
# End Group
# Begin Group "FX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fxbitmaps.c
# End Source File
# Begin Source File

SOURCE=.\fxbitmaps.h
# End Source File
# End Group
# Begin Group "Dictionaries"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\avl.c
# End Source File
# Begin Source File

SOURCE=.\avl.h
# End Source File
# Begin Source File

SOURCE=.\hash_table.c
# End Source File
# Begin Source File

SOURCE=.\hash_table.h
# End Source File
# Begin Source File

SOURCE=.\linked_list.c
# End Source File
# Begin Source File

SOURCE=.\linked_list.h
# End Source File
# End Group
# Begin Group "Save file"

# PROP Default_Filter ""
# Begin Group "MD5"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\save_file\md5.c
# End Source File
# Begin Source File

SOURCE=.\save_file\md5.h
# End Source File
# End Group
# Begin Group "One Time Pads"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\save_file\OneTimePads.c
# End Source File
# Begin Source File

SOURCE=.\save_file\OneTimePads.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\save_file\save_file.c
# End Source File
# Begin Source File

SOURCE=.\save_file\save_file.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\level.h
# End Source File
# Begin Source File

SOURCE=.\todo.txt
# End Source File
# End Target
# End Project
