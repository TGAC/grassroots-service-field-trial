# Field Trials


## Installation

To build this service, you need the [grassroots core](https://github.com/TGAC/grassroots-core) and [grassroots build config](https://github.com/TGAC/grassroots-build-config) installed and configured. 

The files to build the Field Trials services are in the `build` directory. 

### Linux and Mac

Enter the build directory for your your given platform which is inside the `build/unix/<platform>` 

For example, under linux:

```
cd build/unix/linux
```

whereas on MacOS:

```
cd build/unix/mac
```

then

```
make all
```

and then 

```
make install
```

to install the library into the Grassroots system where it will be available for use immediately.


### Windows

Under Windows, there is a Visual Studio project in the `build/windows` folder that allows you to build the geocoder library.


## Service Configuration

