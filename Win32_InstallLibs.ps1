
# Query paths for MSBuild with:
#   dir HKLM:\SOFTWARE\Microsoft\MSBuild\ToolsVersions\


cd C:\libs\zlib-1.2.11
echo "Moved to Zlib dir"
cmake .
MSBuild.exe zlib.sln
echo "Moving back to Engine"
cd C:\git\engine

