# This script is to aid people in trying out this game in network mode with a couple clients
# If actively working on this project then it is suggested to use 
# NetTestServerClientLaunch.ps1 with a windows environment variable instead of this script

# Paste full path to UnrealEditor.exe
$UEnginePath = "<REPLACEME>\UnrealEditor.exe"

$ServerParms = "$pwd\Tantrumn.uproject $pwd\Content\Tantrumn\Maps\Tantrumn_VerticalRace.umap?listen -Win64 -DebugGame -server -port=7779 -log LOG=TantrumnServerLog.txt"
$ClientParms1 = "$pwd\Tantrumn.uproject 127.0.0.1:7779 -game -clientonly -log LOG=TantrumnClientLog1.txt"
$ClientParms2 = "$pwd\Tantrumn.uproject 127.0.0.1:7779 -game -clientonly -log LOG=TantrumnClientLog2.txt"
$ServerParms = $ServerParms.Split(" ")
$ClientParms1 = $ClientParms1.Split(" ")
$ClientParms2 = $ClientParms2.Split(" ")

& $UEnginePath $ServerParms
Start-Sleep -Seconds 5
& $UEnginePath $ClientParms1
& $UEnginePath $ClientParms2