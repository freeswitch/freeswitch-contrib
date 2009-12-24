'
' Contributor(s):
' Michael Jerris <mike@jerris.com>
' David A. Horner http://dave.thehorners.com
'----------------------------------------------

'On Error Resume Next
' **************
' Initialization
' **************

Set WshShell = CreateObject("WScript.Shell")
Set FSO = CreateObject("Scripting.FileSystemObject")
Set WshSysEnv = WshShell.Environment("SYSTEM")
Set xml = CreateObject("Microsoft.XMLHTTP")
Dim UseWgetEXE

On Error Resume Next
Set oStream = CreateObject("Adodb.Stream")
On Error Goto 0

If Not IsObject(oStream)  Then
	wscript.echo("Failed to create Adodb.Stream, using alternative download method.")
	UseWgetEXE=true
Else
	UseWgetEXE=false
End If
Randomize
Set objArgs = WScript.Arguments
quote=Chr(34)
ScriptDir=Left(WScript.ScriptFullName,Len(WScript.ScriptFullName)-Len(WScript.ScriptName))
UtilsDir=Showpath(ScriptDir)
ToolsBase="http://files.freeswitch.org/downloads/win32/"

If UseWgetEXE Then
	GetWgetEXE UtilsDir
End If

GetCompressionTools UtilsDir


If objArgs.Count >=3 Then
	Select Case objArgs(0)
		Case "Get"		
			Wget objArgs(1), Showpath(objArgs(2))
		Case "GetUnzip"		
			WgetUnCompress objArgs(1), Showpath(objArgs(2))
		Case "Version"					
			'CreateVersion(tmpFolder, VersionDir, includebase, includedest)
			CreateVersion Showpath(objArgs(1)), Showpath(objArgs(2)), objArgs(3), objArgs(4)
	End Select
End If


' *******************
' Utility Subroutines
' *******************


Sub WgetUnCompress(URL, DestFolder)
	If Right(DestFolder, 1) <> "\" Then DestFolder = DestFolder & "\" End If
	StartPos = InstrRev(URL, "/", -1, 1) 
	strlength = Len(URL)
	filename=Right(URL,strlength-StartPos)
	NameEnd = InstrRev(filename, ".",-1, 1)
	filestrlength = Len(filename)
	filebase = Left(filename,NameEnd)
	fileext = Right(filename, Len(filename) - NameEnd)
	Wget URL, DestFolder
	If fileext = "zip" Then
		UnCompress Destfolder & filename, DestFolder & filebase
	Else
		UnCompress Destfolder & filename, DestFolder	
	End If
End Sub

Sub GetCompressionTools(DestFolder)
	Dim oExec
	If Right(DestFolder, 1) <> "\" Then DestFolder = DestFolder & "\" End If
	If Not FSO.FileExists(DestFolder & "7za.exe") Then 
		If Not FSO.FileExists(DestFolder & "7za.tag") Then 
			Set MyFile = fso.CreateTextFile(DestFolder & "7za.tag", True)
			MyFile.WriteLine("This file marks a pending download for 7za.exe so we don't download it twice at the same time")
			MyFile.Close
				
				Wget ToolsBase & "7za.exe", DestFolder
			
			FSO.DeleteFile DestFolder & "7za.tag" ,true 
		Else
			WScript.Sleep(5000)
		End If	
	End If	
End Sub

Sub GetWgetEXE(DestFolder)
	Dim oExec
	If Right(DestFolder, 1) <> "\" Then DestFolder = DestFolder & "\" End If
	If Not FSO.FileExists(DestFolder & "wget.exe") Then 
		Slow_Wget ToolsBase & "wget.exe", DestFolder
	End If	
End Sub

Sub UnCompress(Archive, DestFolder)
	batname = "tmp" & CStr(Int(10000*Rnd)) & ".bat"
	wscript.echo("Extracting: " & Archive)
	Set MyFile = fso.CreateTextFile(UtilsDir & batname, True)
	MyFile.WriteLine("@" & quote & UtilsDir & "7za.exe" & quote & " x " & quote & Archive & quote & " -y -o" & quote & DestFolder & quote )
	MyFile.Close
	Set oExec = WshShell.Exec(UtilsDir & batname)
	Do
		WScript.Echo OExec.StdOut.ReadLine()
	Loop While Not OExec.StdOut.atEndOfStream
	If FSO.FileExists(Left(Archive, Len(Archive)-3))Then  
		Set MyFile = fso.CreateTextFile(UtilsDir & batname, True)
		MyFile.WriteLine("@" & quote & UtilsDir & "7za.exe" & quote & " x " & quote & Left(Archive, Len(Archive)-3) & quote & " -y -o" & quote & DestFolder & quote )
		MyFile.Close
		Set oExec = WshShell.Exec(UtilsDir & batname)
		Do
			WScript.Echo OExec.StdOut.ReadLine()
		Loop While Not OExec.StdOut.atEndOfStream
		WScript.Sleep(500)
		FSO.DeleteFile Left(Archive, Len(Archive)-3) ,true 
	End If
	If FSO.FileExists(Left(Archive, Len(Archive)-3) & "tar")Then  
		Set MyFile = fso.CreateTextFile(UtilsDir & batname, True)
		MyFile.WriteLine("@" & quote & UtilsDir & "7za.exe" & quote & " x " & quote & Left(Archive, Len(Archive)-3) & "tar" & quote & " -y -o" & quote & DestFolder & quote )
		MyFile.Close
		Set oExec = WshShell.Exec(UtilsDir & batname)
		Do
			WScript.Echo OExec.StdOut.ReadLine()
		Loop While Not OExec.StdOut.atEndOfStream
		WScript.Sleep(500)
		FSO.DeleteFile Left(Archive, Len(Archive)-3) & "tar",true 
	End If
	
	WScript.Sleep(500)
	If FSO.FileExists(UtilsDir & batname)Then  
		FSO.DeleteFile UtilsDir & batname, True
	End If
End Sub

Sub Wget(URL, DestFolder)
	StartPos = InstrRev(URL, "/", -1, 1)   
	strlength = Len(URL)
	filename=Right(URL,strlength-StartPos)
	If Right(DestFolder, 1) <> "\" Then DestFolder = DestFolder & "\" End If

	Wscript.echo("Downloading: " & URL)
	
If UseWgetEXE Then
	batname = "tmp" & CStr(Int(10000*Rnd)) & ".bat"
	Set MyFile = fso.CreateTextFile(UtilsDir & batname, True)
	MyFile.WriteLine("@cd " & quote & DestFolder & quote)
	MyFile.WriteLine("@" & quote & UtilsDir & "wget.exe" & quote & " " & URL)
	MyFile.Close
	Set oExec = WshShell.Exec(UtilsDir & batname)
	Do
		WScript.Echo OExec.StdOut.ReadLine()
	Loop While Not OExec.StdOut.atEndOfStream

Else
	xml.Open "GET", URL, False
	xml.Send
	
	Const adTypeBinary = 1
	Const adSaveCreateOverWrite = 2
	Const adSaveCreateNotExist = 1 

	oStream.type = adTypeBinary
	oStream.open
	oStream.write xml.responseBody
	oStream.savetofile DestFolder & filename, adSaveCreateOverWrite
	oStream.close
End If

End Sub

Sub Slow_Wget(URL, DestFolder)
	StartPos = InstrRev(URL, "/", -1, 1)   
	strlength = Len(URL)
	filename=Right(URL,strlength-StartPos)
	If Right(DestFolder, 1) <> "\" Then DestFolder = DestFolder & "\" End If

	Wscript.echo("Downloading: " & URL)
	xml.Open "GET", URL, False
	xml.Send
	
	const ForReading = 1 , ForWriting = 2 , ForAppending = 8 
Set MyFile = fso.OpenTextFile(DestFolder & filename ,ForWriting, True)
For i = 1 to lenb(xml.responseBody)
 MyFile.write Chr(Ascb(midb(xml.responseBody,i,1)))
Next
MyFile.Close()

End Sub

Function Showpath(folderspec)
	Set f = FSO.GetFolder(folderspec)
	showpath = f.path & "\"
End Function


Function FindVersionStringInConfigure(strConfigFile, strVersionString)

Set objRegEx = CreateObject("VBScript.RegExp")
objRegEx.Pattern = "[^#]AC_SUBST\(" & strVersionString & ".*\[([^\[]*)\]"
 
Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objFile = objFSO.OpenTextFile(strConfigFile, 1)
strSearchString = objFile.ReadAll
objFile.Close

Set colMatches = objRegEx.Execute(strSearchString)  

strResult = ""
If colMatches.Count > 0 Then
   For Each strMatch in colMatches   
	strResult = objRegEx.Replace(strMatch.Value, "$1")
    Next
End If

	FindVersionStringInConfigure = strResult

End Function

Sub FindReplaceInFile(FileName, sFind, sReplace)
	Const OpenAsASCII = 0  ' Opens the file as ASCII (TristateFalse) 
	Const OpenAsUnicode = -1  ' Opens the file as Unicode (TristateTrue) 
	Const OpenAsDefault = -2  ' Opens the file using the system default 
	
	Const OverwriteIfExist = -1 
	Const FailIfNotExist   =  0 
	Const ForReading       =  1 
	
	Set fOrgFile = FSO.OpenTextFile(FileName, ForReading, FailIfNotExist, OpenAsASCII)
	sText = fOrgFile.ReadAll
	fOrgFile.Close
	sText = Replace(sText, sFind, sReplace)
	Set fNewFile = FSO.CreateTextFile(FileName, OverwriteIfExist, OpenAsASCII)
	fNewFile.WriteLine sText
	fNewFile.Close
End Sub

Sub CreateVersion(tmpFolder, VersionDir, includebase, includedest)
	Dim oExec
	
	strVerMajor = FindVersionStringInConfigure(VersionDir & "configure.in", "SWITCH_VERSION_MAJOR")
	strVerMinor = FindVersionStringInConfigure(VersionDir & "configure.in", "SWITCH_VERSION_MINOR")
	strVerMicro = FindVersionStringInConfigure(VersionDir & "configure.in", "SWITCH_VERSION_MICRO")
	strVerRev   = FindVersionStringInConfigure(VersionDir & "configure.in", "SWITCH_VERSION_REVISION")
	
	If Right(tmpFolder, 1) <> "\" Then tmpFolder = tmpFolder & "\" End If
	If Not FSO.FileExists(tmpFolder & "fs_svnversion.exe") Then 
		Wget ToolsBase & "fs_svnversion.exe", tmpFolder
	End If	

	If Not FSO.FileExists(tmpFolder & "libdb44.dll") Then 
		Wget ToolsBase & "libdb44.dll", tmpFolder
	End If	
	If Not FSO.FileExists(tmpFolder & "libsvn_diff-1.dll") Then 
		Wget ToolsBase & "libsvn_diff-1.dll", tmpFolder
	End If	
	If Not FSO.FileExists(tmpFolder & "libsvn_subr-1.dll") Then 
		Wget ToolsBase & "libsvn_subr-1.dll", tmpFolder
	End If	
	If Not FSO.FileExists(tmpFolder & "libsvn_wc-1.dll") Then 
		Wget ToolsBase & "libsvn_wc-1.dll", tmpFolder
	End If	
	If Not FSO.FileExists(tmpFolder & "intl3_svn.dll") Then 
		Wget ToolsBase & "intl3_svn.dll", tmpFolder
	End If	
	If Not FSO.FileExists(tmpFolder & "libapr-1.dll") Then 
		Wget ToolsBase & "libapr-1.dll", tmpFolder
	End If	
	If Not FSO.FileExists(tmpFolder & "libaprutil-1.dll") Then 
		Wget ToolsBase & "libaprutil-1.dll", tmpFolder
	End If	
	If Not FSO.FileExists(tmpFolder & "libapriconv-1.dll") Then 
		Wget ToolsBase & "libapriconv-1.dll", tmpFolder
	End If	
	If Not FSO.FileExists(tmpFolder & "libsvn_delta-1.dll") Then 
		Wget ToolsBase & "libsvn_delta-1.dll", tmpFolder
	End If	

	Dim sLastFile
	Const OverwriteIfExist = -1
	Const ForReading       =  1

	if strVerRev = "" Then
	    VersionCmd="fs_svnversion " & quote & VersionDir & "." & quote &  " -n"
	    Set MyFile = fso.CreateTextFile(tmpFolder & "tmpVersion.Bat", True)
	    MyFile.WriteLine("@" & "cd " & quote & tmpFolder & quote )
	    MyFile.WriteLine("@" & VersionCmd)
	    MyFile.Close
	    Set oExec = WshShell.Exec("cmd /C " & quote & tmpFolder & "tmpVersion.Bat" & quote)
	    Do
		    strFromProc = OExec.StdOut.ReadLine()
		    VERSION=strFromProc
	    Loop While Not OExec.StdOut.atEndOfStream
	    sLastVersion = ""
	    Set sLastFile = FSO.OpenTextFile(tmpFolder & "lastversion", ForReading, true, OpenAsASCII)
	    If Not sLastFile.atEndOfStream Then
		    sLastVersion = sLastFile.ReadLine()
	    End If
	    sLastFile.Close
    End If
	
	if strVerRev <> "" Then
	    VERSION = strVerRev
	End If

	If VERSION = "" Then
		VERSION = "UNKNOWN"
	End If

	If VERSION <> sLastVersion Then
		Set MyFile = fso.CreateTextFile(tmpFolder & "lastversion", True)
		MyFile.WriteLine(VERSION)
		MyFile.Close
	
		FSO.CopyFile includebase, includedest, true
		FindReplaceInFile includedest, "@SWITCH_VERSION_REVISION@", VERSION
		FindReplaceInFile includedest, "@SWITCH_VERSION_MAJOR@", strVerMajor
		FindReplaceInFile includedest, "@SWITCH_VERSION_MINOR@", strVerMinor
		FindReplaceInFile includedest, "@SWITCH_VERSION_MICRO@", strVerMicro

	End If
	
End Sub
