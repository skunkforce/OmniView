
![example workflow](https://github.com/skunkforce/omniview/actions/workflows/build.yaml/badge.svg) 
# OmniView
OmniView is an experimental GUI for the measurement-devices of the research-collaboration AW4null.
It's purpose is the display and the handling of data, generated by [Auto-Intern OmniScopes](https://www.auto-intern.de/shop/). 
 

## Planned functionallity
The plan of AW4null is to enhance car-diagnostics with AI-powered diagnostic-services.
Diagnostic-services will be reachable through differen REST-APIs and will be specialized on a lot of different purposes.
Physical-measurements of automotive-sensors will be taken, displayed in OmniView and send to an API at the users will.
OmniView shall receive data from multiple generators, such as OmniScopes and OmniB-, OmniE-Clamp and OmniPressure. 
The data will come in as a binary stream of 24-bit integers.
Since the incoming data represents measurement-samples in the time-domain, the individual channels will be synchronized.
This is done in a currently non-public-repo of Auto-Intern, but will be separated into a `.dll` or `.so` respectively.
The acquired data can be viewed on the OmniView GUI and may then be saved to the hard-drive in `.csv` or binary format. 
Anyhow, another possibility is to send the measurement to a REST-API provided by the [THGA](https://www.thga.de). 

### Phase One Development
In its first phase, only one API will be used, documented by [this Repository](https://gitlab.thga.de/aw4null/aw4null_api-description_ref/-/blob/main/openapi.json?ref_type=heads). 
The API will be able to receive training-data for AI-training purposes.

### Phase Two Development
Phase two will implement the interface to the data-producer as a `vector` of a multitude of objects, that encapsulate each an individual `.dll` or `.so`.
The first two shared libs that shall be used will be a software-function-generator as well as an abstraction of the OmniScope, following the _Unified-abstract-Dataproducer-Interface_ which is displayed in the [UaDI-Demonstrator-Repo](https://github.com/skunkforce/Unified-Dataproducer-Interface-Demonstrator).

### Phase Three Development
In phase three, OmniView will be extended to use a multitude of service-APIs that can be loaded at runtime.
![Architecture Diagram](https://lucid.app/publicSegments/view/113f9581-f43f-435a-a36d-6f9e0276adec/image.png)

# Building OmniView
Building is currently only possible with specially provided access-rights, but will be made available to the wider public as soon as the migration to a closed-source shared-library for device handling has been done as described in [Issue #36](https://github.com/skunkforce/omniview/issues/36).
In order to ask for access-rights, contact [@bjoekeldude](https://github.com/bjoekeldude).


Anyhow, building __IS__ available in CI. A push into any branch will generate a new binary, that can be downloaded via the [Action-Tab](https://github.com/skunkforce/omniview/actions).


Building requires `cmake --version > 3.20` as well as a `c++20`-Compiler.
## Linux

### Prerequisites
In order to start the compilation, following packages need to be ensured:
* build-essential 
* autoconf 
* gtk-doc-tools 
* libtool 
* intltool 
* curl 
* zip 
* unzip 
* tar 
* libudev-dev
* libxinerama-dev 
* libxcursor-dev 
* xorg-dev 
* libglu1-mesa-dev 
* pkg-config

### Compile
Move into OmniView-directory, update and initialize all submodules:
```shell
git submodule update --init --recursive
```
create build-directory and run cmake from there:
```shell
mkdir build
cd build
cmake ..
```
Start the compilation by using the CMake-command:
```shell
cmake --build .
```

In order to run OmniView it needs admin-rights right now:
```shell
sudo ./OmniView
```

## Windows
Move into OmniView-directory, update and initialize all submodules:
```shell
git submodule update --init --recursive
```

create build-directory and run cmake from there with defined `VCPKG_TARGET_TRIPLET`:
```shell
mkdir build
cd build
cmake .. -DVCPKG_TARGET_TRIPLET="x64-windows-static"
```

Start compilation using CMake:
```shell
cmake --build .
```

# General Code Structure 

The code should follow the c++ standard and contains the following structure :

## Interfaces : 

The following Interfaces are used : 

popups.hpp 
handler.hpp 
regions.hpp 
Style.hpp 

The functions are declared in the .hpp files and implemented in the .cpp files 

Every function that does not depend on another function should be implemented in an individual .cpp file. 

The functions declaration, implementation and usage can be found in the documentation. (right now there is no documentation)

## Namespaces : 

Namespaces should only be used when needed. 

The different PlotRegions have namespaces for readability: SideBarRegion, DevicesRegion, ToolbarRegion and PlotRegion. 

The design and usage of the Regions can be found in the skunkforce/OmniView-Product-Vision Document. 

# Workflow: Adding new Code 

To add new code to the project, follow these steps:

1. Create an issue describing the bug you want to fix or the feature you want to add.
2. Create a branch with the same name as the issue.
3. Pull the code to your local device.
4. Switch to your branch and start working on the issue.
5. If the master branch has been updated since you started working, perform a rebase to incorporate the latest changes.
6. Before committing, run the Clang-Formatter over the files you modified.
7. For significant changes, create a commit with a clear description of what has been modified.
8. After testing the code on your local device and ensuring proper formatting, push the changes to your branch.
9. If the code passes the continuous integration (CI) tests, create a pull request for the master branch.
10. Your code will undergo a review process.

## Using the Clang-Formatter 

To use the Clang-Formatter, follow these steps:

1. Navigate to your /OmniView folder.
2. Check if you have Clang-Formatter installed on your system.
3. If Clang-Formatter is installed, use the following command in the /OmniView directory to format the desired file:

```shell 
clang-format -style=file -i directory/filetochange
```

Note: You don't need to modify the `-style=file` option.

4. To review the changes made by Clang-Formatter, use the command: 

```shell
git diff
```

5. Ensure that the Clang-Formatter configuration file is named: `.clang-format`.


## Project Onboarding
In order to learn more about the project as such, you may visit [www.autowerkstatt40.org](https://www.autowerkstatt40.org/) or have a look at [our online-learning resources](https://moodle.aw4null.de/)

## Support 

This work has been supported by the BMWi- project number 68GX21005E

![BMWiPicture](https://upload.wikimedia.org/wikipedia/commons/3/38/BMWi_Logo_2021.svg)