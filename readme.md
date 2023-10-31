# Field Trials


## Installation

There are a number of dependencies for these services. You need the [grassroots core](https://github.com/TGAC/grassroots-core) and [grassroots build config](https://github.com/TGAC/grassroots-build-config) installed and configured. Alongside these you need to install the [frictionless data](https://github.com/TGAC/grassroots-frictionless-data) and [geocoder](https://github.com/TGAC/grassroots-geocoder) libraries. 


The field trials services also use the [libexif](https://github.com/libexif/libexif) library which
we previously installed with the `install_dependencies` script that we ran previously. We need
to let this service know the folder where we installed libexif. This is done by creating a 
properties file which specifies this. An example file is part of the field trials directory 
structure `build/unix/linux/example_user.prefs`. We copy this to a file called `linux/user.prefs` 
which we will then edit to specify the location of libexif.


```
cp build/unix/example_user.prefs build/unix/linux/user.prefs
```

The content of this file is shown below

```
#
# field trial dependencies
#
# Set this to where you have the libexif directory 
# containing "include" and "lib" subdirectories.
export LIBEXIF_HOME := /opt/libexif
```

and we need to change this to point where libexif is installed. For example, 
if it is installed at
`/opt/grassroots/extras/linexif` by changing the variable to 

```
export LIBEXIF_HOME := /opt/grassroots/extras/linexif
```

with this in place, the field trials services can be built correctly. We also need to make sure 
that the libexif libraries are in the library path when we run httpd, so we need to amend the
`bin/envvars_grassroots` file within the Apache installation that is being used to run Grassroots 
and add the appropriate line

```
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<WHERE YOU HAVE LIBEXIF INSTALLED>/libexif/lib
```

prior to the 
```
export LD_LIBRARY_PATH
```

which is at the bottom of the file.


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


## Services and configuration

There are a number of services as part of the Field Trials service module and these are detailed 
below. As well as their individual configuration options, they all share the following keys:


 * **so:image**: The web address of the image to use for this service's logo
 * **database**: The name of the database that the service should query


We will now describe each of the individual services and their configuration options.

### Browse Field Trial Revisions	

Within Grassroots, each piece of Field Trial data can be potentially edited multiple times 


Browse all of the revisions of a given Field Trial. Following the same nomenclature as BrAPI, a Field Trial contains multiple Studies. This is equivalent to an Investigation in MIAPPE.	


### Edit Field Trial Rack	

A service to edit an individual field trial rack.	


### Manage Field Trial data	

A service to manage field trial data	


### Manage Study	

Following the same nomenclature as BrAPI, a Study is a phenotyping experiment taking place at a single location. One or more Studies can take place within a single Trial.	


### Search Field Trials	

A service to search field trial data	



### Search Measured Phenotype Variables	

Search field trial measured phenotype variables	


### Search Treatments	

Search field trial treatments	



### Submit Field Trial Crop	

This is a service to define crops that are available within the system. 
The only configuration keys that it has are the common ones of `so:image` and 
`database`.


### Submit Field Trial Gene Banks	

A service to submit field trial GeneBanks	



### Submit Field Trial Measured Variables	

A service to submit field trial measured variables	



### Submit Field Trial Programme	

Add a Programme to the system. A Programme contains one or more Trials.	



### Submit Field Trial Study	

Following the same nomenclature as BrAPI, a Study is a phenotyping experiment taking place at a single location. One or more Studies can take place within a single Trial. It contains substantial amounts of metadata 	



### Submit Field Trial Treatment Factor	


Add a Treatment Factor to the system	



### Submit Field Trial Treatments	

Add Treatments to the system	



### Submit Field Trials

Add a Field Trial to the system. Following the same nomenclature as BrAPI, a Field Trial contains multiple Studies. This is equivalent to an Investigation in MIAPPE.