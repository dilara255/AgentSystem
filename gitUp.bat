echo off

set major=0
set minor=4
set sub=3
set build=1

git status
pause

git add *
git status
pause

echo Enter commit message
set /P "Message="

git commit -m "v%major%.%minor%.%sub% #%build% - %Message%"

git push https://github.com/dilara255/AgentSystem.git

pause