# OmniView
OmniView ist eine universelle GUI um die Messgeräte der Forschungskooperation AW4null anzuzeigen. 
Mehr Informationen über AW4null finden sich unter [www.autowerkstatt4null.org].

## Prototype 
Ein Prototype der Software befindet sich in [[www.figma.com](https://www.figma.com/file/daQFBzV59T8xOlL4JXXwBG/OmniView?type=design&node-id=0%3A1&t=8xcLaxbuGwCj4EBE-1)](FIGMA). Nach einem Klick auf den Link, muss die Einsicht erst von @bjoekeldude freigegeben werden.
Der Prototype wird kontinuierlich geupdated und soll das Entwicklungsergebnis visualisieren. 
Die konkreten Entwicklungsschritte werden im [https://github.com/orgs/skunkforce/projects/1](Projektplan) festgehalten, welcher von @markuskae gepflegt wird.

## Hilfe und Onboarding
Auf [ https://moodle.aw4null.de/ ](unserer Lernplattform) finden sich weitere Hilfen und Ressourcen zum aw4null Projekt. Ansprechpartner ist @wasilina83 

## Teilnehmen und Mitarbeiten


## Buildprozess Linux
installiere vcpkg

ins OmniView Verzeichnis wechseln

submodule updaten und initalisieren:
`git submodule update --init --recursive`

-buildordner erstellen:
`mkdir build`

in den buildordner navigieren:
`cd build`

cmake ausführen mit dem pfad zu vcpgk:
`cmake .. -DCMAKE_TOOLCHAIN_FILE=<vcpk_root>/scripts/buildsystems/vcpkg.cmake`

make ausführen:
`make`

OmniView ausführen:
`./OmniView`

